/*
 * Class connectivity with zrm module
 * Ostapenko D.V. NIKTES 2019-03-11
 *
 */

#include "zrm_connectivity.hpp"
#include "qcoreevent.h"
#include <crc_unit.hpp>
#include <qdatetime.h>
#include <qcoreapplication.h>
#include <qmetaobject.h>
#include <qtextcodec.h>

#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <QDateTime>
#include <QColor>
#include <QDebug>
#include <zrmparamcvt.h>

#ifndef PROTOCOL_PT_LINE
    constexpr qint64  SEND_PERIOD_DEFAULT  = 10;
#else
    constexpr qint64  SEND_PERIOD_DEFAULT  = 100;
    constexpr qint64  SEND_DELAY_DEFAULT   = 100;
#endif


static void  meta_types_init(bool& inited)
{

    if (!inited)
    {
        qRegisterMetaType<zrm::params_list_t>();
        inited = true;
    }
}


namespace zrm {

bool       ZrmConnectivity::meta_types_inited = false;
ZrmTextMap ZrmConnectivity::m_mode_text;
ZrmTextMap ZrmConnectivity::m_error_text;
ZrmTextMap ZrmConnectivity::m_warning_text;
int        ZrmConnectivity::m_connectivities_changed = 0;

ZrmConnectivity::connectivity_list_t ZrmConnectivity::m_connectivity_list;

int ZrmConnectivity::connectivity_count()
{
    return m_connectivity_list.count();
}

ZrmConnectivity::connectivity_list_t ZrmConnectivity::connectivities()
{
    return m_connectivity_list;
}

int    ZrmConnectivity::channels_total()
{
    int chan_count = 0 ;
    for (ZrmConnectivity* con : qAsConst(m_connectivity_list))
        chan_count += con->channels().size();
    return chan_count;
}


void ZrmConnectivity::register_connectivity(ZrmConnectivity* instance)
{
    if (!m_connectivity_list.count(instance))
    {
        m_connectivity_list.append(instance);
        ++m_connectivities_changed;
    }
}

void ZrmConnectivity::unregister_connectivity(ZrmConnectivity* instance)
{
    if (m_connectivity_list.count(instance))
    {
        m_connectivity_list.removeOne(instance);
        ++m_connectivities_changed;
    }
}




ZrmConnectivity::ZrmConnectivity(const QString& conn_name, QObject* parent)
    : QMultioDevWorker(parent)
    , m_zrm_mutex      (QMutex::Recursive)

{

    m_send_period = SEND_PERIOD_DEFAULT;
    if (!conn_name.isEmpty())
        setObjectName(conn_name);
    meta_types_init(meta_types_inited);
    register_connectivity(this);
    m_send_timer.moveToThread(m_thread);
    connect(&m_send_timer, &QTimer::timeout, this, &ZrmConnectivity::send_next_packet);
    connect(m_thread, &QThread::finished, &m_send_timer, &QTimer::stop);

    m_ping_timer.moveToThread(m_thread);
    connect(&m_ping_timer, &QTimer::timeout, this, &ZrmConnectivity::sl_ping_timer);
    connect(m_thread, &QThread::finished, &m_ping_timer, &QTimer::stop);

    m_wcdg_timer.moveToThread(m_thread);
    connect(&m_wcdg_timer, &QTimer::timeout, this, &ZrmConnectivity::sl_wcdg_timer);
    connect(m_thread, &QThread::finished, &m_wcdg_timer, &QTimer::stop);
}

ZrmConnectivity::~ZrmConnectivity()
{
    unregister_connectivity(this);
}

void    ZrmConnectivity::set_send_period(unsigned long value)
{
    if (value)
    {
        m_send_period = value;
        m_send_timer.setInterval(value);
    }
}

bool  ZrmConnectivity::set_connection_string(const QString& conn_str)
{
    bool ret = QMultioDevWorker::set_connection_string(conn_str);
    if (ret)
        ++m_connectivities_changed;
    return ret;
}


ZrmChannel* ZrmConnectivity::create_zrm_module(uint16_t number, zrm_work_mode_t work_mode)
{
    return new ZrmChannel(number, work_mode);
}




ZrmChannelSharedPointer   ZrmConnectivity::get_channel(uint16_t channel) const
{
    QMutexLocker l(&m_zrm_mutex);

    return channel_exists(channel) ? m_channels[channel] : ZrmChannelSharedPointer();
}

void ZrmConnectivity::handle_write  (qint64 wr_bytes)
{

    Q_UNUSED(wr_bytes)
    QMultioDevWorker::handle_write(wr_bytes);
}

void ZrmConnectivity::notifyRecv(const recv_header_t& recvHeader)
{
    QByteArray packet_data(reinterpret_cast<const char*>(&recvHeader), recv_buffer_t::proto_size<int>(&recvHeader, false) );
    emit sig_recv_packet(packet_data);
}

void ZrmConnectivity::handle_recv   (const QByteArray& recv_data)
{
    //m_enable_send  = true;

    m_recv_buffer.raw_add(recv_data.constData(), size_t( recv_data.size()));
    int packet_count = 0;
    static const QMetaMethod signal_rev_packet = QMetaMethod::fromSignal(&ZrmConnectivity::sig_recv_packet);

    bool is_sig_connected = isSignalConnected(signal_rev_packet);

    while (m_recv_buffer.sync(cu_prolog_t()) && m_recv_buffer.is_proto_complete())
    {
        lprecv_header_t proto   = m_recv_buffer();
        auto crc_ptr = proto->last_byte_as<recv_buffer_t::crc_type>();
        if (proto_header::crcCalc(proto, proto->size()) == *crc_ptr)
        {

            handle_recv_channel(*proto);
            m_recv_buffer.remove_first();
            ++packet_count;

            if (is_sig_connected)
                notifyRecv(*proto);

        }
        else
        {
            // Ошибка CRC  очищаем буфер
            m_recv_buffer.clear();
            break;
        }
    }

    if (packet_count)
    {
        //Обработать список ищменифшихся каналов
        resetWatchDog();
        on_channels_changed();
        send_next_packet();
    }

}


void ZrmConnectivity::writeToDevice(const void* data, size_t size)
{
    static const QMetaMethod signal_send_packet = QMetaMethod::fromSignal(&ZrmConnectivity::sig_send_packet);
    if (device_is_open())
    {
        m_iodev->write(data, size);
        m_iodev->flush();
        if (isSignalConnected(signal_send_packet))
        {
            QByteArray packet_data(reinterpret_cast<const char*>(data), static_cast<int>(size) );
            emit sig_send_packet(packet_data);
        }
    }
}

void ZrmConnectivity::writeToDevice(const QByteArray& data)
{
    writeToDevice(data.constData(), data.size());
}

/**
 * @brief ZrmConnectivity::send_next_packet
 * Отправка пакета из очереди
 */

void ZrmConnectivity::send_next_packet()
{
    QMutexLocker l(&m_zrm_mutex);

    //Начать с последнего проверенного

    QList<ZrmChannelSharedPointer> channels = m_channels.values();
    for (int i = 0; i < channels.count(); i++)
    {
        if (m_currentSendChannel >= channels.count())
            m_currentSendChannel = 0;
        auto channel = channels.at(m_currentSendChannel);
        ++m_currentSendChannel;
        if (channel->readyToSend())
        {
            writeToDevice(channel->getNextPacket());
            break;
        }
    }

}

void   ZrmConnectivity::channel_send_packet           (uint16_t channel, uint8_t type, size_t data_size, const void* data )
{
    QMutexLocker l(&m_zrm_mutex);

    if (!device_is_open() && !channel_exists(channel))
    {
        return;
    }

    auto mod = get_channel(channel);
    mod->queuePacket(static_cast<packet_types_t>(type), data_size, data);
    if (!m_send_timer.isActive())
        send_next_packet();
}


void ZrmConnectivity::handle_connect(bool connected)
{
    m_recv_buffer.clear();
    m_recv_kadr_number = uint32_t(-1);

    if (connected)
    {
        int  time_out = channels_start();
        if (time_out)
        {
            m_ping_timer.start( time_out );
        }
    }
    else
    {
        m_ping_timer.stop();
        send_timer_ctrl(false);
        channels_stop();
    }
    //m_send_buffer.set_packet_number(0);
    QMultioDevWorker::handle_connect(connected);
}

void   ZrmConnectivity::send_timer_ctrl(bool start)
{
    //Управление таймером передачи
    if (!start)
    {
        m_send_timer.stop();
        return;
    }

    if (!m_send_timer.isActive()) // Запуск
    {
        m_send_timer.start(std::chrono::milliseconds(m_send_period));
    }
}

qint64      ZrmConnectivity::channelRespondTime(uint16_t     ch_num)
{
    QMutexLocker l (&m_zrm_mutex);
    ZrmChannelSharedPointer mod = get_channel(ch_num);
    return mod.isNull() ? 0 : mod->getRespondTime();
}


void   ZrmConnectivity::handle_recv_channel (const recv_header_t& recv_hdr)
{
    auto channel = recv_hdr.proto_hdr.channel;
    QMutexLocker l (&m_zrm_mutex);
    ZrmChannelSharedPointer mod = get_channel(channel);
    if (mod.data() && mod->handle_recv(recv_hdr))
    {
        channel_mark_changed( channel );
    }
}


/**
 * @brief ZrmConnectivity::on_channels_changed
 * Обработка изменений канала
 */
void    ZrmConnectivity::on_channels_changed()
{
    static const QMetaMethod ms_channel_changed  = QMetaMethod::fromSignal(&ZrmConnectivity::sig_channel_change );

    bool   is_channel_changed_connected  = isSignalConnected(ms_channel_changed );

    QMutexLocker l (&m_zrm_mutex);
    for (auto  channel : qAsConst(m_changed_channels))
    {
        auto mod = get_channel(channel);
        if (!mod.data())
            continue;

        bool need_request_method = false;
        bool need_ping           = false;
        if (mod->params_is_changed(PARAM_CON) && mod->session_active() && mod->ping_check(0))
        {
            qDebug() << "channels_changed session --> " << mod->session().session_param.ssID;
            qDebug() << "request method stages";
            need_request_method = true;
            channel_refresh_info(channel);

        }

        if (mod->params_is_changed(PARAM_STATE))
        {
            module_state_changed ( mod, &need_request_method, & need_ping);
        }

        if (mod->params_is_changed(PARAM_MID) && mod->get_state().state_bits.auto_on)
        {
            //Изменение ID метода Перезапросить
            need_request_method = true;
        }

        if (need_request_method)
        {
            channel_query_param(channel, zrm_param_t::PARAM_METHOD_STAGES);
//            need_ping = true;
        }

//        if (need_ping)
//        {

//            ping_module(mod.data());
//        }

        if (is_channel_changed_connected)
            emit sig_channel_change(channel, mod->changes());
        mod->clear_changes();
    }

    m_changed_channels.clear();
}


void    ZrmConnectivity::channel_refresh_info   (uint16_t  channel)
{

    char data[] =
    {
        PARAM_ADDR
        , PARAM_MVOLT  //Макс. напряжение
        , PARAM_MCUR   //Макс. ток
        , PARAM_MCURD  //Макс. ток разряда
        , PARAM_DCNT   //Кол-во разрядных модулей
        , PARAM_GCAP   //Кол-во модулей в группе
        , PARAM_GCNT   //Кол-во групп

        , PARAM_VRDEV //Версия блока
        , PARAM_RVDEV //Модификация блока
        , PARAM_RVSW  //Модификация ПО
        , PARAM_SERNM //Зав. номер
    };

    this->channel_query_params(channel, data, sizeof(data));
}

/**
 * @brief ZrmConnectivity::module_state_changed
 * @param mod
 * Изменение состояния модуля старт/стоп/пауза
 * если нулевые адреса pneed_request_method && pneed_ping то запрос метода и ping выполняются самостоятельно
 */
void   ZrmConnectivity::module_state_changed (ZrmChannelSharedPointer& mod, bool* pneed_request_method, bool* pneed_ping)
{

    auto prev_state = mod->get_state(true );
    auto curr_state = mod->get_state(false);
    oper_state_t ch;
    ch.state = prev_state.state ^ curr_state.state;

    if (ch.state_bits.auto_on || ch.state_bits.start_pause )
    {
        bool need_ping = false;
        if (curr_state.state_bits.auto_on)
        {
            //Стал активен запрашиваем метод
            //qDebug()<<"request methods";
            need_ping = true;

            if (pneed_request_method)
                *pneed_request_method = true;
            else
            {
                channel_query_param( mod->channel(), PARAM_METHOD_STAGES);
            }
        }
        else
        {
            //Стал неактивен запрашиваем результат выполнения
            //qDebug()<<"request results";
            if (!curr_state.state_bits.auto_on && !curr_state.state_bits.start_pause)
            {
                channel_query_param( mod->channel(), PARAM_METH_EXEC_RESULT);
                need_ping = true;
                QThread::msleep(100);
            }
        }

        if (pneed_ping)
        {
            *pneed_ping = need_ping;
        }
        else
        {
            if (need_ping)
                ping_module(mod.data());
        }
    }

}

void   ZrmConnectivity::chanel_clear_changes  (uint16_t channel)
{
    QMutexLocker l (&m_zrm_mutex);
    auto mod = get_channel(channel);
    if (mod.data())
    {
        mod->clear_changes();
    }
}


ZrmChannelsKeys  ZrmConnectivity::get_changed_channels()
{
    QMutexLocker l(&m_zrm_mutex);
    auto ret =  std::move(m_changed_channels);
    return ret;
}

int    ZrmConnectivity::channels_start      ()
{
    int ret = INT_MAX;
    send_timer_ctrl(true);
    QMutexLocker l (&m_zrm_mutex);
    for (const auto& mod : qAsConst(m_channels))
    {
        ret = qMin(mod->ping_period(), ret);
        mod->startSession();
    }
    m_ping_timer.setInterval(ret);
    return ret;
}

void   ZrmConnectivity::channels_stop       (bool silent)
{
    QMutexLocker l (&m_zrm_mutex);
    send_timer_ctrl(false);

    for (auto mod : qAsConst(m_channels))
    {
        if (mod->session_active())
        {
            mod->session_reset();
            if (!silent)
            {
                mod->stopSession();
            }
        }
    }

    //TODO make asynchronous
//    while (!m_send_buffer.is_empty())
//    {
//        //m_enable_send = true;
//        send_next_packet();
//        QThread::msleep(m_send_period);
//    }
}



void   ZrmConnectivity::ping_module         (ZrmChannel* mod)
{

    // Отправка запроса параметров устройству
    //  qDebug()<<tr("%2 ping module channel %1 ").arg(mod->channel()).arg(QDateTime::currentDateTime().toString("mm:ss.zzz"));
    mod->pingChannel();

}

void   ZrmConnectivity::sl_ping_timer ()
{
    QMutexLocker l (&m_zrm_mutex);
    int interval = m_ping_timer.interval();
    for (auto mod : qAsConst(m_channels))
    {
        if (mod.data() && mod->ping_check(interval))
            ping_module(mod.data());
    }
}

void   ZrmConnectivity::channels_clear        ()
{
    QMutexLocker l (&m_zrm_mutex);
    m_channels.clear();
    ++m_connectivities_changed;
}

void   ZrmConnectivity::channel_add (uint16_t ch_num, zrm_work_mode_t work_mode)
{
    QMutexLocker l (&m_zrm_mutex);
    if (!m_channels.contains(ch_num))
    {
        m_channels[ch_num].reset(create_zrm_module(ch_num, work_mode));
        if (this->device_is_open())
        {
            //TODO обновить таймер опроса && запустить канал
        }
        ++m_connectivities_changed;
    }
}

bool   ZrmConnectivity::channel_exists        ( uint16_t     ch_num) const
{
    QMutexLocker l (&m_zrm_mutex);
    return m_channels.contains(ch_num);
}

bool   ZrmConnectivity::channel_remove        ( uint16_t     ch_num)
{
    QMutexLocker l (&m_zrm_mutex);
    if (m_channels.contains(ch_num))
    {
        auto mod = m_channels[ch_num];
        if (device_is_open())
        {
            mod->stopSession();

        }
        m_channels.remove(ch_num);
        mod.reset();
        ++m_connectivities_changed;
        channel_mark_changed(ch_num);
        return true;
    }
    return false;
}

zrm_work_mode_t     ZrmConnectivity::channel_work_mode     ( uint16_t     ch_num)
{
    QMutexLocker l (&m_zrm_mutex);
    auto mod = get_channel(ch_num);
    if (mod.data())
        return   mod->work_mode() ;
    return zrm_work_mode_t::as_charger;
}


void   ZrmConnectivity::channel_set_work_mode ( uint16_t     ch_num, zrm_work_mode_t wm)
{
    QMutexLocker l (&m_zrm_mutex);
    auto mod = get_channel(ch_num);
    if (mod.data() && mod->work_mode() != wm)
    {
        mod->set_work_mode(wm);
        ++m_connectivities_changed;
    }
}

void   ZrmConnectivity::channel_subscribe_param     (uint16_t     ch_num, zrm_param_t param, bool add)
{
    QMutexLocker l (&m_zrm_mutex);
    ZrmChannelSharedPointer mod = get_channel(ch_num);
    if ( mod.data() )
    {
        if (add)
            mod->param_request_add(param);
        else
            mod->param_request_remove(param);

    }

}

void   ZrmConnectivity::channel_subscribe_params     ( uint16_t     ch_num, const params_t& params, bool add )
{
    QMutexLocker l (&m_zrm_mutex);
    ZrmChannelSharedPointer mod = get_channel(ch_num);
    if ( mod.data() )
    {
        if (add)
            mod->params_request_add(params);
        else
            mod->params_request_remove(params);
        qDebug() << "Subscribed count" << mod->params_list().size();
    }


}

//void   ZrmConnectivity::handle_thread_finish ()
//{
//    QMultiIoDevWorker::handle_thread_finish();
//}

void   ZrmConnectivity::handle_thread_start  ()
{
    QMultioDevWorker::handle_thread_start();
    m_wcdg_timer.start(std::chrono::seconds(1));
}

void   ZrmConnectivity::sl_wcdg_timer ()
{
    if (check_watchdog())
    {
        if (device_is_open())
            do_device_close();
        else
            do_device_open();
    }
}


bool   ZrmConnectivity::channel_is_executing  (uint16_t ch_num) const
{
    QMutexLocker l(&m_zrm_mutex) ;
    auto mod = get_channel(ch_num);
    return mod.data() ? mod->is_executing() : false ;
}

void              ZrmConnectivity::channel_start       (uint16_t ch_num)
{

    QMutexLocker l(&m_zrm_mutex) ;
    auto mod = get_channel(ch_num);
    if (mod.data() && !mod->is_executing())
    {
        channel_write_method(ch_num);

        auto state = mod->get_state();
        state.state = 0;
        state.state_bits.start_pause = 0;
        state.state_bits.auto_on     = 1;
        channel_write_param(ch_num, WM_PROCESS, PARAM_STATE, &state, sizeof(state));
    }
}

bool              ZrmConnectivity::channel_is_stopped  (uint16_t ch_num) const
{
    QMutexLocker l(&m_zrm_mutex) ;
    auto mod = get_channel(ch_num);
    return mod.data() ? mod->is_stopped() :  true;
}



void              ZrmConnectivity::channel_stop        (uint16_t ch_num)
{
    QMutexLocker l(&m_zrm_mutex) ;
    auto mod = get_channel(ch_num);
    if (mod.data())
    {
        if (!mod->is_stopped())
        {
            auto state = mod->get_state();
            state.state = 0;
            state.state_bits.start_pause = 0;
            state.state_bits.auto_on     = 0;
            channel_write_param(ch_num, WM_PROCESS, PARAM_STATE, &state, sizeof(state));
        }

    }
}

bool    ZrmConnectivity::channel_is_paused   (uint16_t chan) const
{
    QMutexLocker l(&m_zrm_mutex) ;
    auto mod = get_channel(chan);
    return mod.data() ? mod->is_paused() : false;
}

void              ZrmConnectivity::channel_pause       (uint16_t ch_num)
{
    QMutexLocker l(&m_zrm_mutex) ;
    auto mod = get_channel(ch_num);
    if (mod.data() && mod->is_executing())
    {
        auto state = mod->get_state();
        state.state = 0;
        state.state_bits.start_pause = 1;
        state.state_bits.auto_on     = 0;
        channel_write_param(ch_num, WM_PROCESS, PARAM_STATE, &state, sizeof(state));
    }
}

void              ZrmConnectivity::channel_reset_error       (uint16_t ch_num)
{

    QMutexLocker l(&m_zrm_mutex) ;
    auto mod = get_channel(ch_num);
    if (mod.data())
    {
        auto state = mod->get_state();
        //state.state = 0;
        state.state_bits.fault_reset = 1;
        channel_write_param(ch_num, WM_PROCESS, PARAM_STATE, &state, sizeof(state));
    }
}


zrm_maskab_param_t ZrmConnectivity::channel_masakb_param(uint16_t ch_num)
{
    QMutexLocker l(&m_zrm_mutex) ;
    auto mod = get_channel(ch_num);
    if (mod.data())
    {
        return mod->masakb_param();
    }
    return zrm_maskab_param_t();
}

void               ZrmConnectivity::channel_set_masakb_param(uint16_t ch_num, const zrm_maskab_param_t& map)
{
    QMutexLocker l(&m_zrm_mutex) ;
    auto mod = get_channel(ch_num);
    if ( mod.data() )
    {
        mod->set_masakb_param(map);
        ++m_connectivities_changed;
    }
}


ZrmChannelAttributes ZrmConnectivity::channelAttributes(uint16_t ch_num) const
{
    ZrmChannelAttributes attrs;
    QMutexLocker l(&m_zrm_mutex);
    auto mod = get_channel(ch_num);
    if (!mod.isNull())
    {
        attrs = mod->getAttributes();
    }
    return attrs;
}

bool  ZrmConnectivity::setChannelAttributes(uint16_t ch_num, const ZrmChannelAttributes& attrs)
{
    QMutexLocker l(&m_zrm_mutex);
    auto mod = get_channel(ch_num);
    if (mod.isNull())
        return false;
    if (mod->getAttributes() == attrs)
        return true;

    mod->setAttributes(attrs);
    ++ m_connectivities_changed;
    return true;
}

void              ZrmConnectivity::channel_read_eprom_method(uint16_t     ch_num, uint8_t met_number )
{
    channel_write_param(ch_num, WM_PROCESS, PARAM_RD_EPROM_METHOD, &met_number, sizeof(met_number));
}

void ZrmConnectivity::channel_write_param(uint16_t ch_num, param_write_mode_t wr_mode, zrm_param_t param,
                                          const void* param_data, size_t param_data_sz)
{
    QMutexLocker l (&m_zrm_mutex);

    if (!channel_exists(ch_num))
        return;

//    if (QThread::currentThread() != m_thread)
//    {
//        qApp->postEvent(this, new QChannelControlEvent(ctrl_write_param, ch_num, wr_mode, param, param_data, param_data_sz));
//        return;
//    }

    QByteArray data = ZrmChannel::makeParam( wr_mode, param, param_data_sz, param_data);
    auto chan = get_channel(ch_num);
    chan->queuePacket(PT_DATAWRITE, data.size(), data.data());
}


void   ZrmConnectivity::channel_query_params       (uint16_t ch_num, const params_t& params)
{
    if (params.size())
        channel_query_params(ch_num, params.data(), params.size());
}

void   ZrmConnectivity::channel_query_params       (uint16_t ch_num, const char* params, size_t psize)
{
    if (!channel_exists(ch_num))
        return;

    QMutexLocker l (&m_zrm_mutex);
    auto chan = get_channel(ch_num);

    chan->queryParams(psize, params);
}

void   ZrmConnectivity::channel_query_param       (uint16_t chan, const zrm_param_t  param)
{
    params_t params;
    params.push_back(params_t::value_type(param));
    channel_query_params(chan, params);
}


/**
 * @brief ZrmConnectivity::channel_mark_changed
 * @param chan
 * Пометить канал как имеющий измененные данные
 */
void              ZrmConnectivity::channel_mark_changed(uint16_t chan)
{

    QMutexLocker l(&m_zrm_mutex) ;
    if (m_changed_channels.indexOf(chan) < 0)
        m_changed_channels.append(chan);

}

void              ZrmConnectivity::channel_set_method  (uint16_t ch_num, const zrm_method_t& method)
{
    QMutexLocker l(&m_zrm_mutex) ;
    auto mod = get_channel(ch_num);
    if (mod.data())
    {
//     qDebug()<<tr("+++++++++set method id %1  name %2 duration %3")
//                  .arg(method.m_method.m_id)
//                  .arg(QString::fromLocal8Bit(method.m_method.m_name))
//                  .arg(method.m_method.duration())
//                  ;
        mod->method_set(method);
        channel_mark_changed(ch_num);
        on_channels_changed();
    }
}

const zrm_method_t ZrmConnectivity::channel_get_method (uint16_t ch_num, bool eprom) const
{
    QMutexLocker l(&m_zrm_mutex) ;
    auto mod = get_channel(ch_num);

    if (mod.data())
    {
        zrm_method_t& method = mod->method_get(eprom);
//      qDebug()<<tr("get method id %1  name %2 duration %3")
//                .arg(method.m_method.m_id)
//                .arg(QString::fromLocal8Bit(method.m_method.m_name))
//                .arg(method.m_method.duration())
//                ;
        return method;
    }
    return zrm_method_t();
}

const session_t   ZrmConnectivity::channel_session     (uint16_t ch_num ) const
{
    QMutexLocker l(&m_zrm_mutex) ;
    auto mod = get_channel(ch_num);
    return mod.data() ? mod->session() : session_t(0);
}

size_t ZrmConnectivity::channel_write_method(uint16_t ch_num, const zrm_method_t& method, param_write_mode_t wr_mode  )
{
    QMutexLocker l(&m_zrm_mutex);
    auto mod = get_channel(ch_num);
    if (mod.data() && method.m_stages.size())
    {
        mod->write_method(method, wr_mode);
    }
    return 0;
}


size_t ZrmConnectivity::channel_write_method(uint16_t ch_num)
{
    QMutexLocker l(&m_zrm_mutex) ;
    auto mod = get_channel(ch_num);
    if (mod.data())
    {

        return mod->write_method();
    }
    return 0;
}


zrm_cells_t   ZrmConnectivity::channel_cell_info     (uint16_t     channel)
{
    zrm_cells_t ret;
    QMutexLocker l(&m_zrm_mutex) ;
    auto mod = get_channel(channel);
    if (mod.data())
    {
        ret = mod->cells_get();
    }
    return ret;
}

int            ZrmConnectivity::channels_count()
{
    QMutexLocker l(&m_zrm_mutex);
    return m_channels.count();
}

ZrmChannelsKeys ZrmConnectivity::channels()
{
    QMutexLocker l(&m_zrm_mutex);
    return m_channels.keys();
}

params_list_t ZrmConnectivity::channel_params(uint16_t channel)
{
    QMutexLocker l(&m_zrm_mutex);
    if (m_channels.contains(channel))
    {
        return m_channels[channel]->params_current();
    }
    return  params_list_t();
}


method_exec_results_t ZrmConnectivity::results_get(uint16_t channel)
{
    QMutexLocker l(&m_zrm_mutex);
    if (m_channels.contains(channel))
    {
        return m_channels[channel]->results_get();
    }
    return  method_exec_results_t();
}

method_exec_results_sensors_t ZrmConnectivity::results_sensors_get(uint16_t channel)
{
    QMutexLocker l(&m_zrm_mutex);
    if (m_channels.contains(channel))
    {
        return m_channels[channel]->results_sensor_get();
    }
    return  method_exec_results_sensors_t();
}


zrm::param_variant ZrmConnectivity::get_param(uint16_t channel, zrm::zrm_param_t param)
{
    QMutexLocker l(&m_zrm_mutex);
    if (!m_channels.contains(channel))
        return zrm::param_variant();

    return m_channels[channel]->getParameter(param);
}


QString ZrmConnectivity::get_stage_type_name(uint16_t ch_num, zrm::stage_type_t type)
{
    QString ret ;
    QMutexLocker l(&m_zrm_mutex);
    auto mod = get_channel(ch_num);
    if (mod.data())
        ret = tr(zrm::stage_t::stage_type_name(mod->work_mode(), type));
    return ret;
}



void    load_text(const QString& file_name, ZrmTextMap& dest)
{
    QFile file(file_name);
    if (file.open(QFile::ReadOnly))
    {
        dest.clear();
        while (!file.atEnd())
        {
            QString str = file.readLine();
            QStringList sl = str.split(QLatin1Char('='));
            if (sl.count() > 1)
            {
                uint32_t idx = sl[0].toUInt();
                if (!dest.contains(idx))
                    dest[idx] = sl[1].trimmed();
            }
        }
    }
}

QString      ZrmConnectivity::zrm_work_mode_name( zrm_work_mode_t  wm)
{
    QString ret;
    switch (wm)
    {
        case as_power   :
            ret = tr("Источник");
            break;
        case as_charger :
            ret = tr("Зарядное");
            break;
    }
    return ret;
}

QString      ZrmConnectivity::zrm_mode_text   (uint32_t code)
{
    if (!m_mode_text.count())
        load_text(QLatin1String(":/zrm/files/zrm_modes.txt"), m_mode_text);
    return m_mode_text.contains(code) ? m_mode_text[code] : tr("Неизвестная режим : %1").arg(code);
}

QString      ZrmConnectivity::zrm_error_text  (uint32_t code)
{
    if (code)
    {
        if (!m_error_text.count())
            load_text(QLatin1String(":/zrm/files/zrm_errors.txt"), m_error_text);
        return m_error_text.contains(code) ? m_error_text[code] : tr("Неизвестная ошибка : %1").arg(code);
    }
    return QString();
}

QString      ZrmConnectivity::zrm_warning_text(uint32_t code)
{
    if (!m_warning_text.count())
        load_text(QLatin1String(":/zrm/files/zrm_warnings.txt"), m_warning_text);
    return m_warning_text.contains(code) ? m_warning_text[code] : tr("Неизвестная предупреждение : %1").arg(code);
}


/**
 * @brief ZrmConnectivity::read_from_json
 * @param path_to_file
 * @return количество объектов в списке
 */
int ZrmConnectivity::read_from_json(QString path_to_file)
{
    auto defaultConnect = []()
    {
        ZrmConnectivity* con = new ZrmConnectivity;
        QMutexLocker l(&con->m_zrm_mutex);
        con->m_name = "Соединение - 1";
        con->set_connection_string("tcp=192.168.1.237:5000");

        uint16_t chan_number = uint16_t(1);
        zrm_work_mode_t chan_mode   = zrm_work_mode_t(1);
        con->channel_add(chan_number, chan_mode);
        zrm_maskab_param_t _map;
        _map.dU = .0;
        _map.dT = .0;
        con->channel_set_masakb_param(chan_number, _map);
    };

    if (!m_connectivity_list.size())
    {
        QFile file(path_to_file);

        if (file.exists() && file.open(QFile::ReadOnly))
        {
            QJsonDocument jdoc = QJsonDocument::fromJson(file.readAll());
            if (!jdoc.isEmpty())
            {
                for (auto&& jobj : jdoc.array())
                {
                    ZrmConnectivity* con = new ZrmConnectivity;
                    con->readFromJson(jobj.toObject());
                }
            }
            else
                defaultConnect();
            file.close();
        }
        else
            defaultConnect();
        m_connectivities_changed = 0;
        return int(m_connectivity_list.size());
    }
    return -1;
}

constexpr const char* const json_con_name     = "name";
constexpr const char* const json_con_str      = "conn_str";
//constexpr const char* const json_con_ssid     = "sess_id";
constexpr const char* const json_chan_number  = "number";
constexpr const char* const json_chan_mode    = "mode";
constexpr const char* const json_chan_makb_dU = "makb_dU";
constexpr const char* const json_chan_makb_dT = "makb_dT";
constexpr const char* const json_channels     = "channels";
constexpr const char* const json_chan_box     = "box_number";
constexpr const char* const json_chan_device  = "device_number";
constexpr const char* const json_chan_color   = "color";

void ZrmConnectivity::readFromJson(const QJsonObject& jobj)
{
    QMutexLocker l(&m_zrm_mutex);
    m_name = jobj[json_con_name].toString();
    set_connection_string(jobj[json_con_str].toString());

    /*if (jobj.contains(json_con_ssid))
        set_session_id( uint16_t(jobj[json_con_ssid].toInt()));*/

    for (auto&& jelem : jobj[json_channels].toArray())
    {
        auto chobj = jelem.toObject();
        auto chan_number = uint16_t(chobj[json_chan_number].toInt());
        auto chan_mode = zrm_work_mode_t(chobj[json_chan_mode].toInt());
        channel_add(chan_number, chan_mode);
        zrm_maskab_param_t _map;
        _map.dU = chobj[json_chan_makb_dU].toDouble(.0);
        _map.dT = chobj[json_chan_makb_dT].toDouble(.0);
        channel_set_masakb_param(chan_number, _map);
        ZrmChannelAttributes attrs;
        attrs.box_number = chobj[json_chan_box].toInt(0);
        attrs.device_number = chobj[json_chan_device].toInt(0);
        QColor color(chobj[json_chan_color].toString("#4682b4"));
        attrs.color = color.rgb();
        setChannelAttributes(chan_number, attrs);
    }
}

void ZrmConnectivity::writeToJson(QJsonObject& jobj)
{
    QMutexLocker l(&m_zrm_mutex);
    jobj[json_con_name] = m_name;
    jobj[json_con_str] = connection_string();
    QJsonArray jchannels;
    for (auto&& chan_number : channels())
    {
        QJsonObject jchan_obj;
        jchan_obj[json_chan_number] = chan_number;
        jchan_obj[json_chan_mode] = int(channel_work_mode(chan_number));
        auto _map = channel_masakb_param(chan_number);
        jchan_obj[json_chan_makb_dU] = _map.dU;
        jchan_obj[json_chan_makb_dT] = _map.dT;
        ZrmChannelAttributes attrs = channelAttributes(chan_number);
        jchan_obj[json_chan_box] = attrs.box_number;
        jchan_obj[json_chan_device] = attrs.device_number;
        jchan_obj[json_chan_color] = QColor(QRgb(attrs.color)).name();
        jchannels.append(jchan_obj);
    }
    jobj[json_channels] = jchannels;
}

bool  ZrmConnectivity::write_to_json (QString path_to_file)
{
    bool ret = false;

    QJsonArray    jarray;
    for (auto&& con : m_connectivity_list)
    {
        QJsonObject  jobj;
        con->writeToJson  (jobj);
        jarray.append(jobj);
    }
    QFile file(path_to_file);
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QJsonDocument jdoc(jarray);
        ret = file.write(jdoc.toJson());
        file.close();
        m_connectivities_changed =  ret ? 0 : m_connectivities_changed;
    }
    return ret;
}

ZrmConnectivity* ZrmConnectivity::connectivity(int idx)
{
    return idx < m_connectivity_list.count() ? m_connectivity_list[idx] : Q_NULLPTR;
}

void                ZrmConnectivity::start_all()
{
    for (auto con : qAsConst(m_connectivity_list))
        con->start_work();
}

void                ZrmConnectivity::stop_all ()
{
    for (auto con : qAsConst(m_connectivity_list))
        con->stop_work();
}

QString           ZrmConnectivity::name()
{
    QMutexLocker l(&m_zrm_mutex);
    return m_name;
}

void              ZrmConnectivity::set_name(const QString& cname)
{
    QMutexLocker l(&m_zrm_mutex);
    if (m_name != cname)
    {
        m_name = cname;
        ++m_connectivities_changed;
    }
}

} // namespace zrm
