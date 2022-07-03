/*
* Ostapenko D.V. NIKTES 2019-03-11
* inner class ZrmModule for save state of the zrm device
*/

#include "zrmchannel.hpp"
#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <qdebug.h>
#include <memory>

namespace zrm {

ZrmChannel::ZrmChannel(uint16_t channel, zrm_work_mode_t work_mode)
    : m_channel(channel), m_work_mode(work_mode)
{
    m_ctrl_params.reserve(64);
    session_reset();
}


ZrmChannel::~ZrmChannel()
{}


bool ZrmChannel::ping_check     (int timer_value)
{
    m_ping_timeout -= timer_value;
    if (m_ping_timeout <= 0)
    {
        m_ping_timeout = m_ping_period;
        return true;
    }
    return false;
}

/**
 * @brief ZrmModule::param_request_add
 * @param param
 * Добавление параметра для контроля
 */

void     ZrmChannel::param_request_add  (zrm_param_t param)
{
    //locker_t l(m_mut);
    params_t::iterator ptr = this->m_ctrl_params.begin();
    params_t::iterator end = m_ctrl_params.end();
    ptr = std::lower_bound(ptr, end, param) ;

    if (ptr >= end || *ptr != param)
        m_ctrl_params.insert(ptr, params_t::value_type(param));
}


void     ZrmChannel::params_request_add  (const params_t&   params)
{
    //locker_t l(m_mut);
    m_ctrl_params.insert(m_ctrl_params.end(), params.begin(), params.end());
    std::sort  (m_ctrl_params.begin(), m_ctrl_params.end(), std::less<params_t::value_type>());
    m_ctrl_params.erase(std::unique(m_ctrl_params.begin(), m_ctrl_params.end()), m_ctrl_params.end());
}


void     ZrmChannel::param_request_remove(const zrm_param_t param)
{
    //locker_t l(m_mut);
    params_t::iterator ptr = this->m_ctrl_params.begin();
    params_t::iterator end = m_ctrl_params.end();
    ptr = std::lower_bound(ptr, end, params_t::value_type(param)) ;
    if (ptr < end && *ptr == param)
    {
        m_ctrl_params.erase(ptr);
        m_curr_params.erase(param);
        m_chg_params.erase(param);
    }
}

void     ZrmChannel::params_request_remove(const params_t&   params)
{
    //locker_t l(m_mut);
    for (auto p : params)
        param_request_remove(zrm_param_t(p));
}

bool     ZrmChannel::param_is_requested (zrm_param_t param)
{
    return std::binary_search(m_ctrl_params.begin(), m_ctrl_params.end(), params_t::value_type(param));
}

bool     ZrmChannel::params_is_changed   (zrm_param_t param) const
{
    //locker_t l(m_mut);
    return m_chg_params.count(param);
}

param_variant     ZrmChannel::param_get          (zrm_param_t param ) const
{
    param_variant pvar;
    //locker_t l(m_mut);
    if (m_curr_params.count(param))
    {
        pvar = m_curr_params.find(param)->second;
    }
    return pvar;
}

void ZrmChannel::handle_conreq(const session_t* session)
{
    param_variant pv;
    pv.size      = sizeof(*session);
    pv.udword    = session->value;
    m_chg_params.clear();
    m_curr_params.clear();
    m_ping_timeout = 0;
    m_LastPacketNumber = -1;
    m_PacketNumber = -1;
    clearSend();
    param_set  ( PARAM_CON, pv );
}


oper_state_t ZrmChannel::get_state(bool prev) const
{
    oper_state_t state;
    if (prev)
        state = m_prev_state;
    else
    {
        state.state = param_get(PARAM_STATE).uword;
    }
    return state;
}

void ZrmChannel::param_set(zrm_param_t param, const param_variant& pv)
{
    //locker_t l(m_mut);
    auto v = m_curr_params[param];
    if (v != pv)
    {
        if (param == PARAM_STATE && v.is_valid())
            m_prev_state.state = v.uword;
        m_curr_params[param] = pv;
        m_chg_params [param] = pv;

        if (param == PARAM_CCNT)
            m_cells.resize(pv.uword);
    }
}

int ZrmChannel::handle_recv(const zrm::recv_header_t& recv_data)
{

    switch (recv_data.proto_hdr.type)
    {
        case PT_CONCONF  :
        case PT_CONREQ   :
            handle_conreq(recv_data.as<session_t>());
            break;
        case PT_DATAREAD :
            if (session().session_param.ssID == recv_data.proto_hdr.session_id)
                handle_data  (reinterpret_cast<const uint8_t*>(recv_data.data), recv_data.proto_hdr.data_size);
            break;
        default:
            qDebug() << "unhandled header type 0x" <<  Qt::hex  <<  recv_data.proto_hdr.type;
            break;
    }
    m_LastPacketNumber = recv_data.proto_hdr.packet_number;

    if (m_waitReceive)
    {
        m_waitReceive = false;
        m_timeFromRecv.start();
        m_RespondTime = m_timeFromSend.elapsed();
    }

    return int(m_chg_params.size());
}


void ZrmChannel::handle_data(const uint8_t* data, size_t size)
{
    if (!(data && size))
        return;

    auto data_ptr = data;
    auto data_end = data + size ;

#ifndef PROTOCOL_PT_LINE
    // skip the state byte
    /*    uint8_t state = *data_ptr++;
        (void)(state); */
    //  почему не так ?
    ++data_ptr;
#endif

    //locker_t l(m_mut);

    while (data_ptr < data_end)
    {
        zrm_param_t param = zrm_param_t(*data_ptr++);
        uint16_t param_size = *data_ptr++;

        switch (param)
        {
            case PARAM_CON :
                // не принемаем нулевой параметр!
                // он стирает номер сессии
                break;
            case PARAM_METHOD_STAGES :
                param_size = handle_method_stages(param_size, data_ptr, data_end);
                break;
            case PARAM_METH_EXEC_RESULT :
                param_size = handle_results(param_size, data_ptr, data_end);
                break;
            case PARAM_METH_EXEC_RESULT_SENSOR :
                param_size = handle_results_sensor(param_size, data_ptr, data_end);
                break;
            case PARAM_CELL :
                param_size = handle_cells(param_size, data_ptr, data_end);
                break;
            case PARAM_RD_EPROM_METHOD :
                param_size = handle_eprom_method(param_size, data_ptr, data_end);
                break;

            /*case PARAM_LOG_POINT :
            break;*/
            default :
            {
                param_variant pv;
                pv.size = param_size;
                memcpy(&pv.uqword, data_ptr, pv.size);
                param_set(param, pv);
            }
            break;
        }
        data_ptr += param_size;
    }
}

uint16_t ZrmChannel::handle_cells(uint16_t data_size, const uint8_t* beg, const uint8_t* end)
{
    Q_UNUSED(end)
    lpc_zrm_cell_t cbeg = reinterpret_cast<lpc_zrm_cell_t>(beg);
    size_t cnt = data_size / sizeof(*cbeg);
    for (size_t i = 0; i < cnt; i++)
    {
        zrm_cell_t& cell =  m_cells.at(i);
        cell = *cbeg;
        m_chg_params[PARAM_CELL].udword = uint32_t(cnt);
        ++cbeg;
    }
    return data_size;
}

/**
 * @brief ZrmModule::handle_results
 * @param data_size
 * @param beg
 * @param end
 * @return размер обработанных данных
 * Обработка результатов работы метода
 */

uint16_t  ZrmChannel::handle_results (uint16_t data_size, const uint8_t* beg, const uint8_t* end)
{
    const stage_exec_result_t* res_beg = reinterpret_cast<const stage_exec_result_t*>(beg);
    const stage_exec_result_t* res_end = reinterpret_cast<const stage_exec_result_t*>(end);
    if ( data_size < 0xFF )
        res_end = res_beg + data_size / sizeof (*res_beg);
    data_size = 0;
    //qDebug()<<"ZrmModule::Handle results";
    while (res_beg < res_end)
    {
        auto ptr_beg = m_exec_results.begin();
        auto ptr_end = m_exec_results.end() ;
        ptr_beg = std::lower_bound(ptr_beg, ptr_end, *res_beg);

        if (ptr_beg >= ptr_end || !((*res_beg) == *ptr_beg) )
            m_exec_results.insert(ptr_beg, *res_beg);

        ++res_beg;
        data_size += sizeof (*res_beg);
    }
    std::sort(m_exec_results.begin(), m_exec_results.end());
    param_set(PARAM_METHOD_STAGES, init_variant(m_exec_results.size()));
    return data_size;
}

/**
 * @brief ZrmModule::handle_results_sensor
 * @param data_size
 * @param beg
 * @param end
 * @return размер обработанных данных
 * Обработка результатов работы метода - показания датчиков за один этап
 */

uint16_t  ZrmChannel::handle_results_sensor(uint16_t data_size, const uint8_t* beg, const uint8_t* end)
{

    Q_UNUSED(end)
    stage_exec_result_sensors_t res;
    res.stage = *beg++;
    res.count = *beg++;


    if (m_exec_results_sensor.size() >= res.stage)
        return data_size;

    res.sensors.resize(res.count);

    auto source_beg = reinterpret_cast<stage_exec_result_sensors_t::sensor_data_t::const_pointer>(beg);
    auto source_end = reinterpret_cast<stage_exec_result_sensors_t::sensor_data_t::const_pointer>(beg);
    std::copy(source_beg, source_end, res.sensors.begin());
    m_exec_results_sensor.push_back(res);
    param_set(PARAM_METH_EXEC_RESULT_SENSOR, init_variant(m_exec_results_sensor.size()));
    return data_size;
}


/**
 * @brief ZrmModule::handle_method_stages
 * @param data_size
 * @param beg
 * @param end
 * @return Обработка результатов работы метода
 * Обработка текущих методов на устройстве
 */

uint16_t ZrmChannel::handle_method_stages(zrm_method_t& method, uint16_t data_size, const uint8_t* beg, const uint8_t* end)
{
    lpc_method_t recv_meth = reinterpret_cast<lpc_method_t>(beg);
    method.m_stages.clear();
    if (data_size > sizeof (*recv_meth))
    {
        method_t meth;
        recv_meth->get_method(meth);
        method.m_method = meth;
        auto stage_beg = reinterpret_cast<lpc_stage_t>(beg + sizeof (*recv_meth));
        auto stage_end = reinterpret_cast<lpc_stage_t>(end);
        if (data_size < 0xFF)
            stage_end = stage_beg + ((data_size - sizeof (*recv_meth)) / sizeof (*stage_beg));

        data_size = sizeof(*recv_meth);

        while (stage_beg < stage_end)
        {
            method.m_stages.push_back(*stage_beg);
            data_size += sizeof (*stage_beg);
            ++stage_beg;
        }

        std::sort(method.m_stages.begin(), method.m_stages.end());
    }
    return data_size;
}

uint16_t ZrmChannel::handle_method_stages(uint16_t data_size, const uint8_t* beg, const uint8_t* end)
{
    //locker_t l(m_mut);
    method_clear();
    //qDebug()<<"Handle method stages";
    data_size = handle_method_stages( m_current_method, data_size, beg, end);
    param_set( PARAM_METHOD_STAGES, init_variant( m_current_method.m_stages.size()) );
    return   data_size;
}

uint16_t ZrmChannel::handle_eprom_method (uint16_t data_size, const uint8_t* beg, const uint8_t* end)
{
    m_eprom_method.m_method = method_t();
    m_eprom_method.m_stages.clear();
    param_set(PARAM_RD_EPROM_METHOD, init_variant(0));
    data_size = handle_method_stages(m_eprom_method, data_size, beg, end);
    //qDebug()<<"Handle eprom method stages "<< QString::fromLocal8Bit(m_eprom_method.m_method.m_name);
    param_set(PARAM_RD_EPROM_METHOD, init_variant(m_eprom_method.stages_count()));
    return data_size;
}



void        ZrmChannel::method_set  (const zrm_method_t& method)
{
    //locker_t l(m_mut);
    method_clear();
    m_current_method.m_method = method.m_method;
    if (m_current_method.m_method.m_id == uint16_t(-1))
        m_current_method.m_method.m_id = 0;

    stages_t& stages = m_current_method.m_stages;
    stages.reserve(method.stages_count());
    stages.insert(stages.end(), method.m_stages.begin(), method.m_stages.end());
    m_current_method.m_method.m_stages = uint8_t(m_current_method.m_stages.size());
    param_set(PARAM_METHOD_STAGES, init_variant(stages.size()));
}


zrm_cells_t ZrmChannel::cells_get() const
{
    return m_cells;
}


/**
 * @brief ZrmModule::session
 * @return получение текущей сессии
 */
session_t ZrmChannel::session               () const
{
    session_t sess(uint16_t(-1));
    auto param = param_get(PARAM_CON);
    if (param.is_valid())
        sess.value = param.udword;
    return sess;
}


void      ZrmChannel::session_reset         ()
{
    m_curr_params.erase(PARAM_CON);
    m_prev_state.state = 0;
    m_chg_params.clear();
    m_ping_timeout = 0;
}


int ZrmChannel::results_get  (method_exec_results_t& res) const
{
    res.clear();
    //locker_t l(m_mut);
    res.insert(res.end(), m_exec_results.begin(), m_exec_results.end());
    return int(res.size());
}

void      ZrmChannel::results_clear()
{
    //locker_t l(m_mut);
    m_exec_results.clear();
    param_set(PARAM_METH_EXEC_RESULT, param_variant());
    m_exec_results_sensor.clear();
    param_set(PARAM_METH_EXEC_RESULT_SENSOR, param_variant());
}


void     ZrmChannel::method_clear()
{
    //locker_t l(m_mut);
    m_current_method.m_method = method_t();
    m_current_method.m_stages.clear();
    param_set(PARAM_METHOD_STAGES, init_variant(0));
}

std::string  ZrmChannel::time_param(const param_variant& pv)
{
    char text[128];
    if (pv.is_valid())
    {
        snprintf (text, sizeof(text), "%02u:%02u:%02u", unsigned(pv.puchar[2]), unsigned(pv.puchar[1]), unsigned(pv.puchar[0]));
    }
    else
    {
        *text = 0;
    }
    return std::string(text);
}

std::string ZrmChannel::trect_param(const param_variant& pv)
{
    std::string ret;
    size_t pcount = pv.size / sizeof(int32_t);
    ret.reserve(pcount * 16);
    const int32_t* ptr = reinterpret_cast<const int32_t*>(pv.puchar);
    const int32_t* end = ptr + pcount;
    char text[32];
    while (ptr < end)
    {
        snprintf(text, sizeof(text), "%s%2.3f", ret.empty() ? "" : ", ", double(*ptr) / 1000.0);
        ret += text;
        ++ptr;
    }

    return ret;
}

std::string ZrmChannel::fan_param(const param_variant& pv)
{
    std::string fans;
    const uint8_t* beg = pv.puchar;
    const uint8_t* end = beg + pv.size;
    while (beg < end)
    {

        char text[32];
        snprintf(text, sizeof(text), "%s%d", (fans.empty() ? "" : ", "), static_cast<int>(*beg));
        fans += text;
        ++beg;
    }
    return fans;
}

QByteArray ZrmChannel::makeSendPacket
(
    uint16_t ssid, uint16_t packetNumber,
    uint16_t channel, uint8_t packet_type,
    uint16_t data_size, const void* data
)
{
    size_t sz  = send_header_t::need_size(size_t(data_size)) + sizeof (CRC_TYPE) ;
    QByteArray packet(sz, Qt::Initialization::Uninitialized);

    lpsend_header_t phdr = reinterpret_cast<lpsend_header_t> (packet.data());

    phdr->init(proto_header(ssid, packetNumber, channel, packet_type), data_size, data);
    *phdr->last_byte_as<CRC_TYPE>() = proto_header::crcCalc(phdr, phdr->size());
    return packet;
}

bool ZrmChannel::isWriteEnabled( uint8_t type)
{
    return (type != PT_DATAWRITE || !session_readonly());
}

void   ZrmChannel::queuePacket(packet_types_t type, size_t dataSize, const void* data)
{
    if (dataSize && data)
    {
        m_LastPacketIsPing = false;
        m_SendQueue.push_back(makeSendPacket(m_SessionId, ++m_PacketNumber, m_channel, type, dataSize, data));
    }
}

QByteArray   ZrmChannel::getNextPacket()
{
    if (m_SendQueue.empty())
    {
        return QByteArray();
    }


    if (m_SendQueue.count() == 1 && !m_LastPacketIsPing && session_active())
    {
        pingChannel();
    }

    m_waitReceive = true;
    m_timeFromRecv.invalidate();
    m_timeFromSend.start();

    return m_SendQueue.takeFirst();
}

void   ZrmChannel::clearSend()
{
    m_waitReceive = false;
    m_timeFromRecv.start();
    m_PacketNumber = 0;
    m_LastPacketIsPing = false;
    m_SendQueue.clear();
}

bool ZrmChannel::readyToSend() const
{
    if ( m_SendQueue.empty() )
    {
        return false;
    }
    return !m_waitReceive && m_timeFromRecv.hasExpired(m_SendDelay);
}

bool ZrmChannel::hasPacket() const
{
    return  m_SendQueue.empty() ;
}

void ZrmChannel::startSession()
{
    clearSend();
    uint8_t st = m_session_request;
    queuePacket( PT_CONREQ, sizeof(st), &st);
}

void ZrmChannel::stopSession()
{
    uint8_t st = ST_FINISH;
    queuePacket( PT_CONREQ, sizeof(st), &st);
}

void   ZrmChannel::queryParams(size_t psize, const void* params)
{
    if (!psize || !params)
    {
        return;
    }

    params_t data;
    data.reserve(1 + psize);
    data.push_back(WM_NONE);
    const char* params_ptr = reinterpret_cast<const char*>(params);
    data.insert(data.end(), params_ptr, params_ptr + psize);
    queuePacket(PT_DATAREQ, data.size(), data.data());

    if (std::binary_search(data.begin(), data.end(), params_t::value_type(PARAM_METH_EXEC_RESULT), std::less<params_t::value_type>()) )
    {
        //Запрос результатов выполнения
        results_clear();
    }
    if (std::binary_search(data.begin(), data.end(), params_t::value_type(PARAM_METHOD_STAGES), std::less<params_t::value_type>()) )
    {
        //Запрос результатов выполнения
        method_clear();
    }
}

void   ZrmChannel::pingChannel()
{
    if (session_active())
    {
        queryParams(m_ctrl_params.size(), m_ctrl_params.data());
        ping_reset();
        m_LastPacketIsPing = true;
    }
    else
    {
        startSession();
    }
}

qint64 ZrmChannel::getRespondTime()
{
    return m_RespondTime;
}

} // namespace zrm
