#ifndef ZRMPARAMSVIEW_H
#define ZRMPARAMSVIEW_H

#include "ui_zrmparamsview.h"
#include <zrmbasewidget.h>
#include <qmap.h>



class ZrmParamsView : public ZrmChannelWidget, private Ui::ZrmParamsView
{
    Q_OBJECT
public:
    enum     columns_t {column_name, column_value};

    explicit ZrmParamsView(QWidget* parent = nullptr);

    void    channel_param_changed(unsigned channel, const zrm::params_list_t& params_list  ) override;
    void    update_controls      () override;
    void    clear_controls       () override;
    void    channel_session      (unsigned channel) override;

protected slots:
    void    request ();
    void serviceMode();

protected:
    void init_params();
    void appendParam(zrm::zrm_param_t, const QString& text);
    using params_items_t  =  QMap<zrm::zrm_param_t, QTreeWidgetItem* >;
private:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    zrm::params_t    m_orders;
    params_items_t   m_items;
    QTimer           m_request_timer;

};

#endif // ZRMPARAMSVIEW_H
