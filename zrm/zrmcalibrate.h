#ifndef ZRMCALIBRATE_H
#define ZRMCALIBRATE_H

#include "ui_zrmcalibrate.h"
#include <zrmbasewidget.h>

class ZrmCalibrate : public ZrmChannelWidget, private Ui::ZrmCalibrate
{
    Q_OBJECT

public:
    explicit ZrmCalibrate(QWidget *parent = nullptr);

private slots:
    void set_real_value();
    void updateButtons();
};

#endif // ZRMCALIBRATE_H
