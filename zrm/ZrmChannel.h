#ifndef ZRMCHANNEL_H
#define ZRMCHANNEL_H

#include "zrmbasewidget.h"

class ZrmChannel : public ZrmBaseWidget
{
    Q_OBJECT

public:
    explicit ZrmChannel(QWidget *parent = nullptr);
    void set_active(bool active);
    virtual void bind(zrm::ZrmConnectivity * src,uint16_t chan, bool _connect_signals = true) override;

signals:
    void clicked();

protected:
    virtual void channel_session(unsigned ch_num) override;
    virtual void update_controls() override;
    virtual void clear_controls() override;
    virtual void channel_param_changed(unsigned channel, const zrm::params_list_t & params_list  ) override;

    virtual void mousePressEvent(QMouseEvent * event) override;
    virtual void paintEvent(QPaintEvent* event) override;

    void handle_error_state(unsigned err_code);

private:
    bool bActive = false;
    double volt = 0.;
    double curr = 0.;
};

#endif // ZRMCHANNEL_H
