#ifndef ZRMREPORTDIALOG_H
#define ZRMREPORTDIALOG_H

#include "ui_ZrmReportViewDialog.h"

#include <QSqlRecord>

namespace QtCharts
{
    class QLineSeries;
    class QDateTimeAxis;
}

class ZrmReportViewDialog : public QDialog, private Ui::ZrmReportViewDialog
{
    Q_OBJECT

public:
    explicit ZrmReportViewDialog(QWidget *parent = nullptr);

    void setReportId(qlonglong id);
    void setResultText(QString result);

protected slots:
    void save_report();
    void openReportFromBase();

protected:
    void save_report_html(const QString & file_name);
    void save_report_pdf (const QString & file_name);

    // chart
    void init_chart();
    void clear_chart();
    void make_chart();

private:
    qlonglong idReport = -1;

    QtCharts::QChart * chart = nullptr;
    QMap<QString, QtCharts::QLineSeries*> map_series;
    QtCharts::QDateTimeAxis *axisTime;
};

#endif // ZRMREPORTDIALOG_H
