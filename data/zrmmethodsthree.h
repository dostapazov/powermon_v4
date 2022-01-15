#ifndef ZRMMETHODSTHREE_H
#define ZRMMETHODSTHREE_H

#include "zrmdatasource.h"
#include "ui_zrmmethodsthree.h"
#include <zrmproto.hpp>
#include <qtextcodec.h>

class ZrmMethodsThree : public QWidget, private Ui::ZrmMethodsThree
{
    Q_OBJECT

public:
    enum tree_roles_t
    {
       role_edit_enable = Qt::UserRole, role_table , role_id, role_voltage  ,role_capacity ,role_volt_rate
      ,role_curr_rate ,role_duration ,role_cycle_count ,role_stage_type
    };

    enum table_types_t{ table_types_t ,table_models_t  ,table_method_t};
    enum column_type_t {column_name, column_voltage, column_capacity};


    explicit  ZrmMethodsThree(QWidget *parent = nullptr);
    void      close();
    bool      open (bool as_charger, bool all_methods);
    QSqlError last_error();
    virtual   bool item_edit_enable(const QModelIndex &index);
    bool      get_method(zrm::zrm_method_t &zrm_method , QTextCodec *codec);
    bool      method_valid(QTreeWidgetItem * item);
  signals:
    void      method_selected(QTreeWidgetItem * item);
protected:
    void      read_methods();
    void      read_typed_methods   ();
    void      read_models          (QTreeWidgetItem * item);
    void      read_methods         (QTreeWidgetItem * item);
    void      read_abstract_methods();

    size_t read_stages(QTreeWidgetItem * item, zrm::stages_t & stages);

    double    get_method_param  (const QTreeWidgetItem *item, column_type_t param);


    virtual   QItemDelegate * create_delegate();

    QSqlDatabase   db;
    QSqlError      m_last_error;
    bool           m_charger      = true;
    bool           m_abstract_methods  = false;
    friend class   QItemDelegate;

static QTreeWidgetItem * new_tree_item(const QString & text, const int table_type , const int column );
private slots:
void on_tw_methods_itemChanged(QTreeWidgetItem *item, int column);
void on_tw_methods_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
};

inline QSqlError  ZrmMethodsThree::last_error()
{
  return m_last_error;
}


#endif // ZRMMETHODSTHREE_H
