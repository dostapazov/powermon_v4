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
    void test_case1();
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

}

void test_method::cleanupTestCase()
{

}

void test_method::test_case1()
{

}

QTEST_MAIN(test_method)

#include "tst_test_method.moc"
