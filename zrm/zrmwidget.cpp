#include "zrmwidget.h"

ZrmWidget::ZrmWidget(QWidget *parent) :
    ZrmGroupWidget (parent)
{
    setupUi(this);
    splitter_3->setStretchFactor(0,1);
    splitter_3->setStretchFactor(1,2);
#ifdef Q_OS_ANDROID
    groupBoxMethod->setVisible(false);
    groupBoxLogerChart->setVisible(false);
    groupBoxDevice->setVisible(false);
#else
    zrm_method->set_details_enable(false);

    connect(zrm_ready, SIGNAL(channel_activated(zrm::ZrmConnectivity *, unsigned)), this, SIGNAL(channel_activated(zrm::ZrmConnectivity *, unsigned)));
#endif
}

void ZrmWidget::bind(zrm::ZrmConnectivity* src, uint16_t chan, bool _connect_signals)
{
    zrm_display->bind(src, chan, _connect_signals);
#ifdef Q_OS_ANDROID
    zrm_display->bind(src, chan, _connect_signals);
#else

    zrm_method->bind(src, chan, _connect_signals);
    zrm_display->bind(src, chan, _connect_signals);
    tabChart->bind(src, chan, _connect_signals);
    tabCell->bind(src, chan, _connect_signals);
    zrm_ready->bind(src, chan, _connect_signals);
    //ZrmGroupWidget::bind(src, chan, _connect_signals);
    //zrm_ready->update_ready();

    bool visible = src && chan && src->channel_work_mode(chan) == zrm::as_charger;
    //tabWidget->setTabVisible(1, visible);
    tabWidget->setTabEnabled(1, visible);
    if (!visible && 1 == tabWidget->currentIndex())
        tabWidget->setCurrentIndex(0);
    if (visible)
        tabCell->update_params();
#endif
}

void ZrmWidget::update_ready()
{
    zrm_ready->update_ready();
}

QList<int> ZrmWidget::getSplitterSizes()
{
    QList<int> splitterSizes;
    splitterSizes << splitter->sizes();
    splitterSizes << splitter_2->sizes();
    splitterSizes << splitter_3->sizes();
    return splitterSizes;
}

void ZrmWidget::setSplitterSizes(const QList<int> &list)
{
    if (list.size() < 6)
        return;
    splitter->setSizes(QList<int>() << list[0] << list[1]);
    splitter_2->setSizes(QList<int>() << list[2] << list[3]);
    splitter_3->setSizes(QList<int>() << list[4] << list[5]);

}
