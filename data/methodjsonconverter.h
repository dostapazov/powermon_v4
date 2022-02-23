#ifndef METHODJSONCONVERTER_H
#define METHODJSONCONVERTER_H

#include <QObject>
#include <zrmproto.hpp>

class IMethodConverter
{
public:
    virtual ~IMethodConverter(){}
    virtual QByteArray toByteArray(const zrm::zrm_method_t & method) = 0;
    virtual zrm::zrm_method_t fromByteArray(const QByteArray & byteArray) = 0;
};


class MethodJsonConverter : public QObject, public IMethodConverter
{
    Q_OBJECT
public:
    explicit MethodJsonConverter(QObject *parent = nullptr);

public slots:
    virtual QByteArray toByteArray(const zrm::zrm_method_t & method) override;
    virtual zrm::zrm_method_t fromByteArray(const QByteArray & byteArray) override;
};

#endif // METHODJSONCONVERTER_H
