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
    void    clear_controls() override;

protected slots:
    void    request ();
    void serviceMode();

protected:
    void init_params();
    void appendParam(zrm::zrm_param_t, const QString& text, bool ordered);
    using params_items_t  =  QMap<zrm::zrm_param_t, QTreeWidgetItem* >;
private:
    void onActivate() override;
    void onDeactivate() override;
    zrm::params_t    m_orders;
    zrm::params_t    m_query_parms;
    params_items_t   m_items;
    QTreeWidgetItem* respond = nullptr;
    QTimer           m_request_timer;

};

#endif // ZRMPARAMSVIEW_H
