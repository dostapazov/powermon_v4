#ifndef ZRMRELAYBASE_H
#define ZRMRELAYBASE_H

#include "ui_zrmrelaybase.h"
#include <zrmbasewidget.h>

class ZrmRelayBase : public ZrmChannelWidget, private Ui::ZrmRelayBase
{
    Q_OBJECT

public:

    explicit ZrmRelayBase(QWidget *parent = nullptr);

protected slots:
    //void flash(bool flash_on);

protected:
    void channel_session(unsigned ch_num) override;
    void timerEvent(QTimerEvent * time_event) override;
    void resizeEvent(QResizeEvent * event) override;
    void update_controls() override;
    void clear_controls () override;
    void channel_param_changed(unsigned channel, const zrm::params_list_t & params_list  ) override;
    void on_connected         (bool            conn_state) override;
    void on_ioerror           (const QString & error_string) override;
    void channel_recv_packet  (unsigned channel, const zrm::recv_header_t * recv_data) override;
    void channel_send_packet  (unsigned channel, const zrm::send_header_t * send_data) override;
    void update_state         (uint32_t state);
    //void handle_error_state   (uint32_t error);

    void watch_dog_enable     (bool enable);
    void watch_dog_reset      (){m_watchdog_value = m_watchdog_period;}

    void setLabelPix(QLabel * label, const QString icon);

    static void init_icons();

private:
    int   m_rcv_counter = 0;
    bool  m_switch_led  = true;
    int   m_watchdog_value  = 0;
    int   m_watchdog_period = 3;
    int   m_watchdog_id     = 0;
    //uint32_t   m_error_state     = 0;
    bool  m_session_active  = false;

    static QString network_rx;
    static QString network_tx;
    static QString network_idle;
    static QString network_offline;

    static QString relay_off;
    static QString relay_on;
    static QString current_out;
    static QString current_in;
    static QString modeU;
    static QString modeUstab;
    static QString modeI;
    static QString modeIstab;
    static QString modeP;
    static QString pause_icon;
    static QString empty_icon;
    static QString led_red;
};

#endif // ZRMRELAYBASE_H
