#ifndef ZRMMETHODBASE_H
#define ZRMMETHODBASE_H

#include "ui_zrmmethodbase.h"
#include <zrmbasewidget.h>

class ZrmMethodBase : public ZrmChannelWidget, private Ui::ZrmMethodBase
{
    Q_OBJECT
    Q_PROPERTY ( bool details READ is_details_enabled WRITE set_details_enable MEMBER m_details_visible CONSTANT)
public:
    explicit ZrmMethodBase(QWidget* parent = nullptr);
    bool     is_details_enabled();
    void     set_details_enable(bool value);

protected :
    virtual void  channel_param_changed(unsigned channel, const zrm::params_list_t& params_list  )  override;
    virtual void  update_controls      () override;
    virtual void  clear_controls       () override;
    virtual void  showEvent            (QShowEvent* event) override;
    virtual void  timerEvent           (QTimerEvent* ev) override;
    void  setup_method         ();
    void  stages_clear         ();
    void  stage_add            (const zrm::stage_t& stage, qreal met_volt, qreal met_current, qreal met_cap);
    void  set_current_stage    (int stage_num);
    uint16_t  m_method_id = uint16_t(-1);
    QBrush    m_item_def_bkgnd;
    bool      m_details_visible = true;
    int       m_timer_id = 0;

};

#endif // ZRMMETHODBASE_H
