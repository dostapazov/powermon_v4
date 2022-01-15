#ifndef ZRMDISPLAYWIDGET_H
#define ZRMDISPLAYWIDGET_H

#include <zrmbasewidget.h>
#include <qicon.h>
#include <qtextedit.h>
#include "ui_zrmdisplaywidget.h"



class ZrmDisplayWidget : public ZrmBaseWidget, private Ui::ZrmDisplayWidget
{
    Q_OBJECT
#ifdef Q_OS_ANDROID
void setup_android_ui();
#endif
public:
    explicit ZrmDisplayWidget(QWidget *parent = nullptr);
    virtual ~ZrmDisplayWidget() override;
            void  set_channel(uint16_t value);
         uint16_t channel();
    virtual void  bind(zrm::ZrmConnectivity   * src,bool conn_signals = true) override;

protected slots:

void    on_watchdog_timeout();
void    gen_result_report  ();

protected:
virtual void  channel_param_changed(unsigned channel, const zrm::params_list_t & params_list  ) override;
virtual void  on_connected         ( bool       conn_state) override;
virtual void  on_ioerror           (const QString & error_string) override;
virtual void  channel_recv_packet  (unsigned channel, const zrm::recv_header_t * recv_data) override;
virtual void  channel_send_packet  (unsigned channel, const zrm::send_header_t * send_data) override;
        void  update_minimized_info();


virtual void  update_controls() override;
        void  clear_controls ();

static  void  set_label_icon(QLabel * label, const QIcon *icon);
virtual void  make_request_params(zrm::params_t & req_param);
        enum  msg_type_t  {msg_info, msg_warn, msg_error};
        void  set_message_text(const QString & msg, msg_type_t msg_type);
        void  update_state    (uint32_t state);
        void  setup_method    ();
        void  stages_clear();
        void  stage_add(const zrm::stage_t & stage, qreal met_volt, qreal met_current, qreal met_cap);
        void  set_current_stage(int stage_num);
        void  channel_session  ();
        void  monitor_add      (bool rx, const QByteArray & packet);
   QTextEdit* get_result_text_edit();

    QTextEdit               *m_result_text   = Q_NULLPTR;
    bool                     m_switch_led  = false;
    bool                     m_connected   =  false;
    int                      m_watchdog_value  = 0;
    int                      m_watchdog_period = 3;
    QTimer                   watchdog;

    uint16_t                 m_channel = 1;
    int                      m_rcv_counter = 0;
    QBrush                   m_item_def_bkgnd;


    static QIcon             network_rx;
    static QIcon             network_tx;
    static QIcon             network_idle;
    static QIcon             network_offline;

    static QIcon             relay_off;
    static QIcon             relay_on;
    static QIcon             current_out;
    static QIcon             current_in;
    static QIcon             modeU;
    static QIcon             modeUstab;
    static QIcon             modeI;
    static QIcon             modeIstab;
    static QIcon             pause_icon;
    static QIcon             empty_icon;

    static void              init_icons();
    static QString           make_report(const QString & a_maker_name, const QString & a_akb_type
                                         , const QString & a_akb_number
                                         , bool details, const zrm::method_exec_results_t & results);


private slots:
    void on_bStartStop_clicked();
    void on_bPause_clicked();
    void on_actMinimize_toggled(bool checked);
    void on_tbSave_clicked();
    void on_tabWidget_currentChanged(int index);
    void on_tbMonClear_clicked();
    void on_bMethod_clicked();
};




inline uint16_t ZrmDisplayWidget::channel()
{
  return m_channel;
}


#endif // ZRMDISPLAYWIDGET_H
