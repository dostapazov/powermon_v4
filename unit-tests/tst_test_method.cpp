#include <QtTest>
#include <QCoreApplication>
#include <zrmproto.hpp>

// add necessary includes here

class test_method : public QObject
{
    Q_OBJECT

public:
    test_method();
    ~test_method();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_charge_current();
    void test_charge_voltage();
 private:
    zrm::zrm_method_t method;


};

test_method::test_method()
{

}

test_method::~test_method()
{

}

void test_method::initTestCase()
{
    method.m_method.set_capacity(100);
    method.m_method.set_current(100);
    method.m_method.set_voltage(100);
    method.m_method.set_cycles(1);

}

void test_method::cleanupTestCase()
{

}

void test_method::test_charge_current()
{
    double current = 50;
    zrm::stage_t stage;
    stage.set_charge_curr(current,method.m_method);
    double get_current = stage.charge_curr(method.m_method);
    Q_ASSERT( current == get_current);
}

void test_method::test_charge_voltage()
{
    double volt = 75.5;
    zrm::stage_t stage;
    stage.set_charge_volt(volt,method.m_method);
    double get_volt = stage.charge_volt(method.m_method);
    QCOMPARE(volt ,get_volt);

}


QTEST_MAIN(test_method)

#include "tst_test_method.moc"
