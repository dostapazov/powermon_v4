#include "ZrmReports.h"

ZrmReports::ZrmReports(QWidget* parent) :
    ZrmGroupWidget(parent)
{
    setupUi(this);
    connect(tabWidget, &QTabWidget::currentChanged, this, &ZrmReports::tabChanged);
}

void ZrmReports::tabChanged(int index)
{
    // обновляем список отчетов
    if (1 == index)
        tabReportBase->read_reports();
}
