#include "ZrmReports.h"

ZrmReports::ZrmReports(QWidget *parent) :
    ZrmGroupWidget(parent)
{
    setupUi(this);
}

void ZrmReports::on_tabWidget_currentChanged(int index)
{
    // обновляем список отчетов
    if (1 == index)
        tabReportBase->read_reports();
}
