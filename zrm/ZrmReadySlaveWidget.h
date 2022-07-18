#ifndef ZRMREADYSLAVEWIDGET_H
#define ZRMREADYSLAVEWIDGET_H

#include <zrmbasewidget.h>

class ZrmChannelView;
class ZrmReadyLayout;
class QScrollArea;

class ZrmReadySlaveWidget : public ZrmBaseWidget
{
    Q_OBJECT
public:
    explicit ZrmReadySlaveWidget(QWidget *parent = nullptr);
    ~ZrmReadySlaveWidget() override;
    virtual void bind(zrm::ZrmConnectivity* src, uint16_t chan, bool _connect_signals = true) override;
    QSize sizeHint() const  override;

signals:
    void channel_activated (zrm::ZrmConnectivity * conn, unsigned channel);

public slots:
    void update_ready();

private slots:
    void zrm_clicked();

private:
    void set_layout_count(int count);
    ZrmChannelView* create_channel_widget();
    void zrm_chanhel_activate (ZrmChannelView * w);

private:
    QScrollArea* ready_area = nullptr;
    QWidget* ready_widget = nullptr;
    ZrmReadyLayout* m_ready_layout = nullptr;
    ZrmChannelView* m_current      = nullptr;
};

#endif // ZRMREADYSLAVEWIDGET_H
