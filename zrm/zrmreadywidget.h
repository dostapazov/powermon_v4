#ifndef ZRMREADYWIDGET_H
#define ZRMREADYWIDGET_H

#include "ui_zrmreadywidget.h"
#include <zrmbasewidget.h>
#include <zrm_connectivity.hpp>
#include <zrmreadylayout.h>
#include <zrmreadyaccum.h>

class ZrmChannelMimimal;

class ZrmReadyWidget : public ZrmBaseWidget, private Ui::ZrmReadyWidget
{
    Q_OBJECT
public:
    explicit ZrmReadyWidget(QWidget *parent = nullptr);
    ~ZrmReadyWidget() override;
    QSize sizeHint() const  override;
    inline ZrmChannelMimimal * current_ready() { return m_current; }
    int ready_count();
    ZrmChannelMimimal * ready_at(int idx);
    ZrmReadyAccum * ready_accum();

signals:
    void channel_activated(ZrmChannelMimimal * w, bool bSelect);

public slots:
    void update_ready();
    void next_channel();
    void prev_channel();
    void selectChannel(zrm::ZrmConnectivity * conn, unsigned channel);

private slots:
    void zrm_clicked();

private:
    void                resizeEvent(QResizeEvent * event) override;
    void                set_layout_count     (int count);
    ZrmChannelMimimal * create_channel_widget();
    void                zrm_chanhel_activate(ZrmChannelMimimal * w, bool bSelect);
    ZrmReadyLayout    * m_ready_layout = Q_NULLPTR;
    ZrmChannelMimimal * m_current      = Q_NULLPTR;
    ZrmReadyAccum     * m_ready_accum  = Q_NULLPTR;
};

#endif // ZRMREADYWIDGET_H
