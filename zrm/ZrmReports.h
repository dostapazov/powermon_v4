#ifndef ZRMREPORTS_H
#define ZRMREPORTS_H

#include "zrmbasewidget.h"

#include "ui_ZrmReports.h"

class ZrmReports : public ZrmGroupWidget, private Ui::ZrmReports
{
    Q_OBJECT

public:
    explicit ZrmReports(QWidget *parent = nullptr);

private slots:
    void tabChanged(int index);
};

#endif // ZRMREPORTS_H
