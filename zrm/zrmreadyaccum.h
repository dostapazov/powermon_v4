#ifndef ZRMERRORACCUM_H
#define ZRMERRORACCUM_H

#include "zrmbasewidget.h"

class QToolButton;

class ZrmReadyAccum : public ZrmBaseWidget
{
    Q_OBJECT
public:
    explicit ZrmReadyAccum(QWidget *parent = nullptr);
    void     update_connectivities();
    void setButton(QToolButton * tb);

private slots:
    void update_view(bool flash_on);

protected :
    void channel_session(unsigned ch_num) override;
    void channel_param_changed(unsigned channel, const zrm::params_list_t & params_list ) override;
    void source_destroyed  (zrm::ZrmConnectivity * src) override;

    void handle_state      (zrm::ZrmConnectivity* conn, unsigned channel, uint32_t state);
    void handle_error_state(zrm::ZrmConnectivity* conn, unsigned channel, uint32_t error_code);

    QString     get_current_tip ();

    int m_exec_count = 0;
    int m_error_count = 0;
    QAction * m_action = nullptr;
    QToolButton* button = nullptr;
    QString m_action_tip;
};

#endif // ZRMERRORACCUM_H
