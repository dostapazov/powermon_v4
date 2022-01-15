#ifndef ZRMDEVMETHODS_H
#define ZRMDEVMETHODS_H

#include "ui_zrmdevmethods.h"
#include <zrmbasewidget.h>

class ZrmDevMethods : public ZrmChannelWidget, private Ui::ZrmDevMethods
{
    Q_OBJECT

public:
    explicit ZrmDevMethods(QWidget *parent = nullptr);
             ~ZrmDevMethods() override;
    void updateData();

protected:
    virtual void  update_controls() override;
    virtual void  clear_controls() override;
    virtual void  channel_param_changed(unsigned channel, const zrm::params_list_t & params_list  ) override;
            void   update_buttons_enabled   ();
            void   dev_methods_clear();
            void   dev_method_clear (QTreeWidgetItem * item);
            void   dev_method_set   (QTreeWidgetItem * item, const zrm::zrm_method_t & metod);
zrm::zrm_method_t* dev_method_get(QTreeWidgetItem * item);

private slots:
            void   method_clicked   (QTreeWidgetItem * item, int);
            void   method_changed   (QTreeWidgetItem * item, QTreeWidgetItem * prev);
            void   method_add       ();
            void   method_remove    ();
            void   method_download  ();
            void   method_upload    ();

};

#endif // ZRMDEVMETHODS_H
