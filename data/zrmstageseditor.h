#ifndef ZRMSTAGESEDITOR_H
#define ZRMSTAGESEDITOR_H


#include "ui_zrmstageseditor.h"
#include "zrmmethodstree.h"

class ZrmStagesEditor : public QWidget, private Ui::ZrmStagesEditor
{
    Q_OBJECT

public:
    enum stage_columns_t       { stage_number_column  , stage_type_column, stage_descr_column };
    enum method_changes_what_t { method_param_changed ,method_name_changed, method_stage_changed };
    explicit ZrmStagesEditor(QWidget *parent = nullptr);
    ZrmMethodsTree * methods_tree();
    void     set_methods_tree      (ZrmMethodsTree * mtree);
    QVariant method_id       ();
    void     set_method_id   (QVariant  id );
    void     copy_method     (QVariant id);
    double   voltage         ();
    double   capacity        ();
    void     set_capacity    (double cap, bool update = true);
    void     set_voltage     (double volt, bool update = true);
    void     set_abstract    (bool is_abstract);
    void     set_method_name (const QString & name);
    QString  method_name     ();
    double user_voltage();
    double user_capacity();
    bool     write_method    (bool wr_method, bool wr_stages);

    QList<int> getSplitterSizes();
    void setSplitterSizes(const QList<int> &list);

signals:
    void method_changed(int what);


protected slots:
    void sl_voltage_changed    (double val);
    void sl_capacity_changed   (double val);
    void sl_stage_remove       ();
    void sl_stage_add          ();
    void sl_stage_move         ();
    void sl_method_name_changed(const QString & str);

    void sl_database_open       (bool success);
    void sl_stage_changed       (QTreeWidgetItem * current,QTreeWidgetItem * prev);
    void sl_stage_data_changed  (QTreeWidgetItem * item, int column);
    void sl_method_param_changed();
    void sl_stage_flags_changed ();
    void sl_stage_finish_flags_changed ();
    void sl_stage_type_changed       ();
    void sl_stage_charge_changed     ();
    void sl_stage_discharge_changed  ();
    void sl_stage_impule_time_changed();
    void sl_stage_finish_changed     ();

protected:
    void init_controls     ();
    void clear_controls    ();
    void enabled_controls  ();
    void read_method       ();
    void setup_method      ();
    void setup_stages      ();
    void stage_assign      (QTreeWidgetItem * stage_item, const zrm::stage_t & st, const QString &descr);
    void stage_type_changed(zrm::stage_type_t st_type);
    void renumber_stages   ();
    void on_stages_changed ();
    void on_method_changed ();
    bool do_write_stages   ();


    zrm::stage_t &         current_stage(int *idx = Q_NULLPTR);
    QTreeWidgetItem* currentStageItem(QTreeWidgetItem* item = nullptr);
    int currentStageItemIndex(QTreeWidgetItem* item = nullptr);

private:
    QList<int> splitterSizes;
    ZrmMethodsTree        * m_methods_tree = Q_NULLPTR;

    double             m_voltage  = 1.0;
    double             m_capacity = 1.0;

    QVariant           m_method_id;
    zrm::zrm_method_t  m_current_method;
    zrm::stages_t      m_removed_stages;
    zrm::stage_t       m_fake_stage;

};

inline ZrmMethodsTree * ZrmStagesEditor::methods_tree()
{
    return m_methods_tree;
}

inline QVariant ZrmStagesEditor::method_id           ()
{
  return m_method_id;
}

inline double   ZrmStagesEditor::voltage  (){return m_voltage;}
inline double   ZrmStagesEditor::capacity (){return m_capacity;}



#endif // ZRMSTAGESEDITOR_H
