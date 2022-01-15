#include "zrmwidget.h"

ZrmWidget::ZrmWidget(QWidget *parent) :
    ZrmGroupWidget (parent)
{
    setupUi(this);
#ifdef Q_OS_ANDROID
    groupBoxMethod->setVisible(false);
    groupBoxLogerChart->setVisible(false);
    groupBoxDevice->setVisible(false);
#else
    zrm_method->set_details_enable(false);
    setSplitterSizes(QList<int>() << 10000 << 10000 << 10000 << 10000 << 10000 << 10000);

    connect(zrm_ready, SIGNAL(channel_activated(zrm::ZrmConnectivity *, unsigned)), this, SIGNAL(channel_activated(zrm::ZrmConnectivity *, unsigned)));
#endif
    connect(splitter, &QSplitter::splitterMoved, [this](){ splitterSizes[0] = splitter->sizes()[0]; splitterSizes[1] = splitter->sizes()[1]; });
    connect(splitter_2, &QSplitter::splitterMoved, [this](){ splitterSizes[2] = splitter_2->sizes()[0]; splitterSizes[3] = splitter_2->sizes()[1]; });
    connect(splitter_3, &QSplitter::splitterMoved, [this](){ splitterSizes[4] = splitter_3->sizes()[0]; splitterSizes[5] = splitter_3->sizes()[1]; });
}

void ZrmWidget::bind(zrm::ZrmConnectivity* src, uint16_t chan, bool _connect_signals)
{
#ifdef Q_OS_ANDROID
    zrm_display->bind(src, chan, _connect_signals);
#else
    //zrm_ready->update_ready();
    zrm_display->bind(src, chan, _connect_signals);
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
    return splitterSizes;
}

void ZrmWidget::setSplitterSizes(const QList<int> &list)
{
    if (list.size() < 6)
        return;
    splitter->setSizes(QList<int>() << list[0] << list[1]);
    splitter_2->setSizes(QList<int>() << list[2] << list[3]);
    splitter_3->setSizes(QList<int>() << list[4] << list[5]);

    splitterSizes.clear();
    splitterSizes << list[0] << list[1] << list[2] << list[3] << list[4] << list[5];
}
