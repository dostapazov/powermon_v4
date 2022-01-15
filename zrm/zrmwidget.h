#ifndef ZRMWIDGET_H
#define ZRMWIDGET_H

#include "ui_zrmwidget.h"
#include <zrmbasewidget.h>

class ZrmWidget : public ZrmGroupWidget, private Ui::ZrmWidget
{
    Q_OBJECT
public:
    explicit ZrmWidget(QWidget *parent = nullptr);
    void  bind(zrm::ZrmConnectivity* src, uint16_t chan, bool _connect_signals = true) override;
    void update_ready();
    ZrmMainDisplay * main_display() { return zrm_display; }
    QList<int> getSplitterSizes();
    void setSplitterSizes(const QList<int> &list);

signals:
    void channel_activated(zrm::ZrmConnectivity * conn, unsigned channel);

private:
    QList<int> splitterSizes;
};

#endif // ZRMWIDGET_H
