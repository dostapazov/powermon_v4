#include "methodjsonconverter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <zrmbasewidget.h>

MethodJsonConverter::MethodJsonConverter(QObject *parent)
    : QObject{parent}
{

}



constexpr const char * m_signature = "signature";
constexpr const char * m_name = "name";

constexpr const char * m_current = "current";
constexpr const char * m_voltage = "volatge";
constexpr const char * m_duration = "duration";
constexpr const char * m_cycles_count= "cycles";
constexpr const char * m_stages = "stages";
constexpr const char * m_id = "id";
constexpr const char * m_capacity = "capacity";
constexpr const char * stages_list = "stages_list";

void method2Jobj(QJsonObject & jmethod, const zrm::method_t & method)
{
    jmethod[m_signature] = method.m_signature;
    jmethod[m_id] = method.m_id;
    jmethod[m_name] = ZrmBaseWidget::to_utf(method.m_name, method.name_length());
    jmethod[m_current] = method.current();
    jmethod[m_voltage] = method.voltage();
    jmethod[m_duration] = static_cast<qint64>(method.duration());
    jmethod[m_cycles_count] = method.m_cycles_count;
    jmethod[m_stages] = method.m_stages;
    jmethod[m_capacity] = method.capacity();
}

void JObj2method(const QJsonObject jmethod, zrm::method_t & method)
{
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
}


QByteArray MethodJsonConverter::toByteArray(const zrm::zrm_method_t & method)
{
 QJsonObject   jmethod;
 method2Jobj(jmethod,method.m_method);

 QJsonDocument jdoc(jmethod);
 return jdoc.toJson();
}

zrm::zrm_method_t MethodJsonConverter::fromByteArray(const QByteArray & byteArray)
{
zrm::zrm_method_t method;
QJsonDocument jdoc = QJsonDocument::fromJson(byteArray);
if(jdoc.isObject())
{
  JObj2method(jdoc.object(),method.m_method);
}
return method;
}

