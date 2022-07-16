#ifndef ZRMPARAMSVIEW_H
#define ZRMPARAMSVIEW_H

#include "ui_zrmparamsview.h"
#include <zrmbasewidget.h>
#include <qmap.h>
#include <QIcon>



class ZrmParamsView : public ZrmChannelWidget, private Ui::ZrmParamsView
{
    Q_OBJECT
public:
    enum     columns_t : int {column_name, column_value, column_new_value};

    explicit ZrmParamsView(QWidget* parent = nullptr);
    void    channel_param_changed(unsigned channel, const zrm::params_list_t& params_list  ) override;
    void    clear_controls() override;

private slots:
    void    request ();
    void serviceMode();
    void writeParameters();
    void paramChanged(QTreeWidgetItem*, int column);
    void passwdChanged(const QString& text);

private:
    void init_params();
    void appendParam(zrm::zrm_param_t, const QString& text, bool ordered, bool editable = false);
    using params_items_t  =  QMap<zrm::zrm_param_t, QTreeWidgetItem* >;
    void onActivate() override;
    void onDeactivate() override;
    zrm::params_t    m_orders;
    zrm::params_t    m_query_parms;
    params_items_t   m_items;
    QList<QTreeWidgetItem*> m_EditableItems;
    QTreeWidgetItem* respond = nullptr;
    QTimer           m_request_timer;
    QIcon            m_EditableIcon;

};

#endif // ZRMPARAMSVIEW_H
