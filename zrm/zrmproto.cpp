#include "zrmproto.hpp"
#include <crc_unit.hpp>
#include <algorithm>
#include <limits.h>

namespace zrm {


const char * stage_t::st_types_power  [4] = {"Пауза","Источник","Нагрузка","Импульс"};
const char * stage_t::st_types_charger[4] = {"Пауза","Заряд","Разряд","Импульс"};

const char * stage_t::stage_type_name (zrm_work_mode_t work_mode, stage_type_t stage_type )
{
   const char ** text = work_mode == as_charger ? st_types_charger : st_types_power;
   return text[stage_type];
}

// Добавление в очередь кадра на отправку
size_t   send_buffer_t::queue_packet         (uint16_t channel, uint8_t packet_type, uint16_t data_size , const void * data  )
{
  size_t sz  = send_header_t::need_size(size_t(data_size)) ;
         sz += sizeof (send_buffer_t::crc_type);
  size_t old_size = m_storage.size();
  m_storage.resize(old_size+sz);
  auto phdr = header_from_offset(old_size);
#ifndef PROTOCOL_PT_LINE
   phdr->init(proto_header(m_session_id, ++m_packet_number, channel, packet_type), data_size,data);
  *phdr->last_byte_as<crc_type>() = crcunit::CRC::crc32(phdr, phdr->size());
#else
  phdr->init(proto_header(channel, m_session_id, ++m_packet_number, packet_type), data_size,data);
 *phdr->last_byte_as<crc_type>() = crcunit::CRC::crc8_1wire(phdr, phdr->size());
#endif
  return sz;
}


size_t   send_buffer_t::queue_request  (uint16_t channel, const devproto::storage_t & param_list )
{
 if(param_list.size())
 {
   devproto::storage_t data(1+param_list.size());
   devproto::storage_t::pointer beg_ptr = &data.at(0) ;
   *beg_ptr = 0;
   std::copy(param_list.begin(), param_list.end(), ++data.begin());
   return queue_packet(channel, zrm::PT_DATAREQ, uint16_t(data.size()*sizeof(devproto::storage_t::value_type)), beg_ptr);
 }
 return 0;
}

void  send_buffer_t::params_add(devproto::storage_t & data, param_write_mode_t wm, zrm_param_t  param, size_t val_sz, const void * val_ptr)
{
    devproto::storage_t::size_type sz = data.size();
    devproto::storage_t::size_type add_sz;

    add_sz = (!sz ? sizeof(devproto::storage_t::value_type) : 0);
    add_sz += sizeof(devproto::storage_t::value_type) + sizeof (devproto::storage_t::value_type) + ((val_sz && val_ptr) ? val_sz : 0);

    data.resize(data.size()+add_sz);
    if(!sz) sz+=1;
    devproto::storage_t::pointer ptr;

    ptr = &data.at(0); //Указать параметры записи
#ifndef PROTOCOL_PT_LINE
   *ptr = devproto::storage_t::value_type(wm);

    ptr = &data.at(sz);
#endif
   *ptr++ = devproto::storage_t::value_type(param);
   *ptr++ = devproto::storage_t::value_type(std::min(val_sz,size_t(UCHAR_MAX)));

    if(val_sz && val_ptr)
    {
        memcpy(ptr, val_ptr, val_sz);
    }

}

} // namespace zrm
