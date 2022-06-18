#include "methodjsonconverter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <zrmbasewidget.h>

MethodJsonConverter::MethodJsonConverter(QObject* parent)
    : QObject{parent}
{

}



constexpr const char* m_signature = "signature";
constexpr const char* m_name = "name";

constexpr const char* m_current = "current";
constexpr const char* m_voltage = "volatge";
constexpr const char* m_duration = "duration";
constexpr const char* m_cycles_count = "cycles";
constexpr const char* m_stages = "stages";
constexpr const char* m_id = "id";
constexpr const char* m_capacity = "capacity";
constexpr const char* stages_list = "stages_list";

constexpr const char* s_signature      = "sign";
constexpr const char* s_type           = "type";
constexpr const char* s_finish_flags   = "finish";
constexpr const char* s_char_curr      = "ch_curr";
constexpr const char* s_char_volt      = "ch_colt";
constexpr const char* s_dis_curr       = "dis_curr";
constexpr const char* s_dis_volt       = "dis_vold";
constexpr const char* s_char_time      = "ch_time";
constexpr const char* s_dis_time       = "dis_time";
constexpr const char* s_end_cur        = "end_cur";
constexpr const char* s_end_volt       = "end_vold";
constexpr const char* s_end_delta_volt = "end_delta_volt";
constexpr const char* s_end_temper     = "end_temper";
constexpr const char* s_end_cap        = "end_cap";
constexpr const char* s_hours          = "hour";
constexpr const char* s_minutes        = "minutes";
constexpr const char* s_secs           = "secunds";
constexpr const char* s_number         = "number";
constexpr const char* s_stage_flags    = "flasg";
constexpr const char* s_id_method      = "method_id";
constexpr const char* s_end_elem_volt  = "end_elem_volt";


QJsonObject saveStage(const zrm::stage_t& stage   )
{
    QJsonObject jstage;
    jstage[s_signature     ] = stage.m_signature;
    jstage[s_type          ] = stage.m_type;
    jstage[s_finish_flags  ] = stage.m_finish_flags;
    jstage[s_char_curr     ] = stage.m_char_curr;
    jstage[s_char_volt     ] = stage.m_char_volt;
    jstage[s_dis_curr      ] = stage.m_dis_curr;
    jstage[s_dis_volt      ] = stage.m_dis_volt;
    jstage[s_char_time     ] = stage.m_char_time;
    jstage[s_dis_time      ] = stage.m_dis_time;
    jstage[s_end_cur       ] = stage.m_end_cur;
    jstage[s_end_volt      ] = stage.m_end_volt;
    jstage[s_end_delta_volt] = stage.m_end_delta_volt;
    jstage[s_end_temper    ] = stage.m_end_temper;
    jstage[s_end_cap       ] = stage.m_end_cap;
    jstage[s_hours         ] = stage.m_hours;
    jstage[s_minutes       ] = stage.m_minutes;
    jstage[s_secs          ] = stage.m_secs;
    jstage[s_number        ] = stage.m_number;
    jstage[s_stage_flags   ] = stage.m_stage_flags;
    jstage[s_id_method     ] = stage.m_method_id;
    jstage[s_end_elem_volt ] = stage.m_end_elem_volt;
    return jstage;
}

QJsonArray saveStages(const zrm::stages_t& stages)
{
    QJsonArray jstages;
    for (const zrm::stage_t& stage : stages)
    {
        jstages.append( saveStage(stage) );
    }
    return jstages;
}

QJsonObject method2Jobj( const zrm::zrm_method_t& zrm_method)
{
    const zrm::method_t& method = zrm_method.m_method;
    QJsonObject  jmethod;
    jmethod[m_signature] = method.m_signature;
    jmethod[m_id] = method.m_id;
    jmethod[m_name] = ZrmBaseWidget::to_utf(method.m_name, method.name_length());
    jmethod[m_current] = method.current();
    jmethod[m_voltage] = method.voltage();
    jmethod[m_duration] = static_cast<qint64>(method.duration());
    jmethod[m_cycles_count] = method.m_cycles_count;
    jmethod[m_stages] = method.m_stages;
    jmethod[m_capacity] = method.capacity();
    jmethod[stages_list] = saveStages(zrm_method.m_stages);
    return jmethod;

}

zrm::stage_t readStage(const QJsonObject& jstage)
{
    zrm::stage_t stage;
    stage.m_signature       =   jstage[s_signature     ].toInt();
    stage.m_type            =   jstage[s_type          ].toInt();
    stage.m_finish_flags    =   jstage[s_finish_flags  ].toInt();
    stage.m_char_curr       =   jstage[s_char_curr     ].toInt();
    stage.m_char_volt       =   jstage[s_char_volt     ].toInt();
    stage.m_dis_curr        =   jstage[s_dis_curr      ].toInt();
    stage.m_dis_volt        =   jstage[s_dis_volt      ].toInt();
    stage.m_char_time       =   jstage[s_char_time     ].toInt();
    stage.m_dis_time        =   jstage[s_dis_time      ].toInt();
    stage.m_end_cur         =   jstage[s_end_cur       ].toInt();
    stage.m_end_volt        =   jstage[s_end_volt      ].toInt();
    stage.m_end_delta_volt  =   jstage[s_end_delta_volt].toInt();
    stage.m_end_temper      =   jstage[s_end_temper    ].toInt();
    stage.m_end_cap         =   jstage[s_end_cap       ].toInt();
    stage.m_hours           =   jstage[s_hours         ].toInt();
    stage.m_minutes         =   jstage[s_minutes       ].toInt();
    stage.m_secs            =   jstage[s_secs          ].toInt();
    stage.m_number          =   jstage[s_number        ].toInt();
    stage.m_stage_flags     =   jstage[s_stage_flags   ].toInt();
    stage.m_method_id       =   jstage[s_id_method     ].toInt();
    stage.m_end_elem_volt   =   jstage[s_end_elem_volt ].toInt();

    return stage;
}

zrm::stages_t readStages(const QJsonArray& array)
{
    zrm::stages_t stages;
    stages.reserve(array.count());

    for (auto&& ptr : array)
    {
        stages.push_back(readStage(ptr.toObject()));
    }
    return stages;
}

zrm::zrm_method_t JObj2method(const QJsonObject jmethod )
{
    zrm::zrm_method_t  zrm_method;
    zrm::method_t& method =  zrm_method.m_method;

    method.m_signature = jmethod[m_signature].toInt();
    method.m_id = jmethod[m_id].toInt();

    QByteArray name = ZrmBaseWidget::codec()->fromUnicode(jmethod[m_name].toString());
    method.set_name(name.data(), name.size());

    method.set_current(jmethod[m_current].toDouble());
    method.set_voltage( jmethod[m_voltage].toDouble()  );
    method.set_duration(jmethod[m_duration].toVariant().toUInt());
    method.m_cycles_count = jmethod[m_cycles_count].toInt();
    method.m_stages = jmethod[m_stages].toInt();
    method.set_capacity(jmethod[m_capacity].toDouble());
    zrm_method.m_stages = readStages(jmethod[stages_list].toArray());
    return zrm_method;
}


QByteArray MethodJsonConverter::toByteArray(const zrm::zrm_method_t& method)
{
    QJsonObject   jmethod = method2Jobj(method);
    QJsonDocument jdoc(jmethod);
    return jdoc.toJson();
}

zrm::zrm_method_t MethodJsonConverter::fromByteArray(const QByteArray& byteArray)
{
    zrm::zrm_method_t method;
    QJsonDocument jdoc = QJsonDocument::fromJson(byteArray);
    if (jdoc.isObject())
    {
        method = JObj2method(jdoc.object());
    }
    return method;
}

