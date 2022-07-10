#ifndef ZRMPARAMCVT_H
#define ZRMPARAMCVT_H

#include <zrmproto.hpp>
#include <QVariant>
#include <functional>
#include <QMap>


class ZrmParamCvt
{
public:
    ZrmParamCvt() = delete;
    static QVariant toVariant(zrm::zrm_param_t param, const zrm::param_variant& pv);
    static QVariant toTime(const zrm::param_variant& pv);
    static QVariant toTemperature(const zrm::param_variant& pv);
    static QVariant toFan  (const zrm::param_variant& pv);
    static QVariant toDouble(const zrm::param_variant& pv, int prec = zrm::DEFAULT_DOUBLE_PRECISION);
    static QVariant toUint32(const zrm::param_variant& pv);
private:
    using Converter_f = std::function<QVariant(const zrm::param_variant& pv)>;
    using Converters_t = QMap<zrm::zrm_param_t, Converter_f>;
    static Converters_t converters;
    static void init();

};

#endif // ZRMPARAMCVT_H
