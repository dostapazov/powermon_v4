#include "zrmparamcvt.h"


ZrmParamCvt::Converters_t ZrmParamCvt::converters;
void ZrmParamCvt::init()
{
    if (converters.empty())
    {
        converters[zrm::PARAM_WTIME] = ZrmParamCvt::toTime;
        converters[zrm::PARAM_LTIME] = ZrmParamCvt::toTime;
        converters[zrm::PARAM_TEMP]  = ZrmParamCvt::toTemperature;
        converters[zrm::PARAM_TRECT] = ZrmParamCvt::toTemperature;
        converters[zrm::PARAM_FAN_PERCENT] = ZrmParamCvt::toFan;

        Converter_f param2double = std::bind(ZrmParamCvt::toDouble, std::placeholders::_1, zrm::DEFAULT_DOUBLE_PRECISION);

        converters[zrm::PARAM_CUR       ] = param2double;
        converters[zrm::PARAM_LCUR      ] = param2double;
        converters[zrm::PARAM_VOLT      ] = param2double;
        converters[zrm::PARAM_LVOLT     ] = param2double;
        converters[zrm::PARAM_CAP       ] = param2double;
        converters[zrm::PARAM_MVOLT     ] = param2double;
        converters[zrm::PARAM_MCUR      ] = param2double;
        converters[zrm::PARAM_MCURD     ] = param2double;
        converters[zrm::PARAM_DPOW      ] = param2double;
        converters[zrm::PARAM_MAXTEMP   ] = param2double;
        converters[zrm::PARAM_MAX_CHP   ] = param2double;
        converters[zrm::PARAM_TCONV     ] = param2double;
        converters[zrm::PARAM_VOUT      ] = param2double;
        converters[zrm::PARAM_CUR_CONSUMPTION] = param2double;
        converters[zrm::PARAM_VOLT_SUPPLY] = param2double;
        converters[zrm::PARAM_VOLT_HIGH_VOLT_BUS] = param2double;
    }
}

QVariant ZrmParamCvt::toVariant(zrm::zrm_param_t param, const zrm::param_variant& pv)
{
    init();
    if (converters.contains(param))
    {
        auto cvt = converters[param];
        return cvt(pv);
    }
    return toUint32(pv);
}


QVariant ZrmParamCvt::toTime(const zrm::param_variant& pv)
{
    if (pv.is_valid())
    {
        return QString::asprintf("%02u:%02u:%02u", unsigned(pv.puchar[2]), unsigned(pv.puchar[1]), unsigned(pv.puchar[0]));
    }
    return QString();
}

QVariant ZrmParamCvt::toTemperature(const zrm::param_variant& pv)
{
    QString ret;
    size_t pcount = pv.size / sizeof(int32_t);
    const int32_t* ptr = reinterpret_cast<const int32_t*>(pv.puchar);
    const int32_t* end = ptr + pcount;
    while (ptr < end)
    {
        ret += QString::asprintf( "%s%2.3f", ret.isEmpty() ? "" : ", ", double(*ptr) / 1000.0);
        ++ptr;
    }

    return ret;
}

QVariant ZrmParamCvt::toFan(const zrm::param_variant& pv)
{
    QString fans;
    const uint8_t* beg = pv.puchar;
    const uint8_t* end = beg + pv.size;
    while (beg < end)
    {
        fans += QString::asprintf("%s%d", (fans.isEmpty() ? "" : ", "), static_cast<int>(*beg));
        ++beg;
    }
    return fans;
}


QVariant ZrmParamCvt::toDouble(const zrm::param_variant& pv, int prec)
{
    return (pv.value<double>(true) / pow(10.0, prec));
}

QVariant ZrmParamCvt::toUint32  (const zrm::param_variant& pv)
{
    return pv.value<uint32_t>(false);
}

