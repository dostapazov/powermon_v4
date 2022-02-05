/*
 * Ostapenko D.V. NIKTES 2019-03-14
 * Main display of the device
*/

#ifndef ZRMMAINDISPLAY_H
#define ZRMMAINDISPLAY_H

//#define DEF_RUPREHT

#include <zrmbasewidget.h>
#include "ui_zrmmaindisplay.h"
#include <qicon.h>

class ZrmMainDisplay : public ZrmChannelWidget, private Ui::ZrmMainDisplay
{
    Q_OBJECT
    void connectSlots();

public:
    explicit ZrmMainDisplay(QWidget *parent = nullptr);
    virtual void update_ui() override;

 protected slots:
    void manual_method_changed();
    void manual_method();
    void currLimitChange();
    void voltLimitChange();
    void start();
    void stop();
    void pause();
    void reset_error();
    void select_method(bool bAbstract);

 protected:
    virtual void update_controls() override;
    virtual void clear_controls() override;
    virtual void channel_param_changed(unsigned channel, const zrm::params_list_t & params_list  ) override;
    virtual void on_connected(bool con_state) override;
    virtual void on_ioerror(const QString & error_string) override;

    void channel_session  (unsigned ch_num) override;

    void make_request();
    void setup_method();
    void update_state(uint32_t state);
    void set_method_duration(zrm::zrm_method_t & method,const QString & str);
    void handle_error_state(uint32_t err_code);
    void update_method_controls();

#ifdef Q_OS_ANDROID
    void update_android_ui();
#endif


    bool m_auto_method = false;
    QString m_model_name;
    uint16_t m_method_id = zrm::METHOD_UNKNOWN_ID;
    bool m_manual_change = false;

#ifdef DEF_RUPREHT
    bool bRupreht = false;
#endif
private:

    void  set_current_limits();
    void  set_volt_limits();
    bool  is_manual() {return m_method_id == zrm::METHOD_MANUAL_ID; }


};

#endif // ZRMMAINDISPLAY_H
