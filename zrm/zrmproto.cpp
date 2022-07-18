#include "zrmproto.hpp"
#include <crc_unit.hpp>
#include <algorithm>
#include <limits.h>
#include <functional>

namespace zrm {

#ifndef PROTOCOL_PT_LINE
    pCrcFunc proto_header::crcCalc = crcunit::CRC::crc32 ;
#else
    pCrcFunc proto_header::crcCalc = crcunit::CRC::crc8_1wire ;
#endif

const char* stage_t::st_types_power  [4] = {"Пауза", "Источник", "Нагрузка", "Импульс"};
const char* stage_t::st_types_charger[4] = {"Пауза", "Заряд", "Разряд", "Импульс"};

const char* stage_t::stage_type_name (zrm_work_mode_t work_mode, stage_type_t stage_type )
{
    const char** text = work_mode == as_charger ? st_types_charger : st_types_power;
    return text[stage_type];
}


} // namespace zrm
