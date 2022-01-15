#include "zrmcalibrate.h"

#include <QMessageBox>

ZrmCalibrate::ZrmCalibrate(QWidget *parent) :
    ZrmChannelWidget(parent)
{
    setupUi(this);
    for(auto b : findChildren<QAbstractButton*>())
        connect(b, &QAbstractButton::clicked, this, &ZrmCalibrate::set_real_value);
    connect(vVoltage, SIGNAL(valueChanged(double)), this, SLOT(updateButtons()));
    connect(vCurrent, SIGNAL(valueChanged(double)), this, SLOT(updateButtons()));
    updateButtons();
}

void ZrmCalibrate::set_real_value()
{
    QObject * src = sender();
    double value = .0;
    zrm::zrm_param_t param = zrm::PARAM_CON;
    char p = ' ';
    if(src == this->bSetVoltage)
    {
        param = zrm::PARAM_CALIB_U;
        value = vVoltage->value();
        p = 'U';
    }
    if(src == this->bSetCurrent)
    {
        param = zrm::PARAM_CALIB_I;
        value = vCurrent->value();
        p = 'I';
    }
    if(param && m_source && value != 0.)
    {
        if (QMessageBox::Yes != QMessageBox::question(this, "Калибровка", QString("Выполнить калибровку %1 %2 ?").arg(p).arg(value)))
            return;
        qint32 wr_value = static_cast<qint32>(value < 0 ? value * 1000. - 0.5 : value * 1000. + 0.5);
        m_source->channel_write_param(m_channel, zrm::WM_PROCESS_AND_WRITE, param, &wr_value, sizeof(wr_value));
    }
}

void ZrmCalibrate::updateButtons()
{
    bSetVoltage->setEnabled(vVoltage->value() != 0.);
    bSetCurrent->setEnabled(vCurrent->value() != 0.);
}
