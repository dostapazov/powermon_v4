#ifndef ZRMTCPSETTINGS_H
#define ZRMTCPSETTINGS_H

#include <zrmbasewidget.h>
#include "ui_ZrmTCPSettings.h"

class ZrmTCPSettings : public ZrmChannelWidget, private Ui::ZrmTCPSettings
{
    Q_OBJECT

public:
    explicit ZrmTCPSettings(QWidget *parent = nullptr);

protected:
    virtual void on_connected(bool con_state) override;
    virtual void update_controls() override;
    virtual void clear_controls() override;
    virtual void channel_param_changed(unsigned channel, const zrm::params_list_t & params_list) override;
    virtual void channel_session(unsigned channel) override;

private slots:
    void setSettings();
};

#endif // ZRMTCPSETTINGS_H
