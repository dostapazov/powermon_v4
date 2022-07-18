/* Ostapenko D. V.
 * NIKTES 2019-03-30
 * Zrm Method database editor
 */

#ifndef ZRMMETHODEDITOR_H
#define ZRMMETHODEDITOR_H

#include "ui_zrmmethodeditor.h"

struct zrm_edit_method_t : public zrm::zrm_method_t
{
  zrm::stages_t m_removed_stages;
};

class ZrmMethodEditor : public QWidget, private Ui::zrmmethodeditor
{
    Q_OBJECT
#ifdef Q_OS_ANDROID
void setup_android_ui();
#endif

public:
enum tree_editor_roles_t  { role_changes_mask = ZrmMethodsTree::role_stage_type+1, role_new };
enum change_mask_t        { changes_empty, change_item  , change_stage , change_all_mask = -1 } ;

    explicit ZrmMethodEditor(QWidget *parent = nullptr);
    bool     setAbstract(bool abstract);
    bool     isAbstract();
    bool     setWorkMode(zrm::zrm_work_mode_t mode);
    zrm::zrm_work_mode_t getWorkMode();
    bool     open_db (zrm::zrm_work_mode_t mode, bool all_methods);
    bool     open_db();
    void     close_db();

    void     save_user_values();
    inline bool isEdit() { return bEdit; }
    void setAllMethods(bool all_methods);
    void refresh();

    QList<int> getSplitterSizes();
    void setSplitterSizes(const QList<int> &list);

signals:
    void editChanged(bool edit);

private slots:
    void     act_all_methods(bool checked);
    void     copy_model();
    void     copy_method();
    void     slot_current_item_changed    (QTreeWidgetItem * current, QTreeWidgetItem * prev);
    void     slot_item_data_changed       (QTreeWidgetItem *item, int column);
    //void     on_parameters_changed      ();
    void     on_actApply_triggered      ();
    void     on_actUndo_triggered       ();
    void     on_actDelete_triggered     ();
    void     on_actLink_toggled         (bool checked);
    void     on_actNew_triggered        ();
    void     on_actNewChild_triggered   ();
    void     abstract_method_changed    (QTreeWidgetItem * current, QTreeWidgetItem * prev);
    void     link_abstract_method       ();
    bool     unlink_method              (QTreeWidgetItem *method_item, bool del);

    void     switch_edit_widget(bool checked);
    void     sl_method_changed(int what);

    void unload();
    void load();

protected:
    void connect_signals     ();
    void item_set_inactive   (QTreeWidgetItem * prev);
    bool set_change_mask     (QTreeWidgetItem * item, int mask, bool set);
    void clr_change_mask     (QTreeWidgetItem * item);

    void setup_method        (QTreeWidgetItem * item);
    void setup_akb           (QTreeWidgetItem * item);
    //bool on_dsb_changed      (QDoubleSpinBox  * dsb);


    bool item_is_new         (QTreeWidgetItem * item);
    void item_new_set        (QTreeWidgetItem * item, bool is_new);

    bool write_changes       (QTreeWidgetItem * item);
    bool write_type          (QTreeWidgetItem * item);
    bool write_model         (QTreeWidgetItem * item);
    bool write_method        (QTreeWidgetItem * item);

    bool erase_modles        (QTreeWidgetItem * item, bool commit_trans = true );
    bool erase_types         (QTreeWidgetItem * item);

    void do_undo_changes     (QTreeWidgetItem * item);
    void do_delete_item      (QTreeWidgetItem * item, bool select_next);
    void create_new          (bool child);
    static int change_mask   (const QTreeWidgetItem * item) ;

    zrm_edit_method_t zrm_method;
    QSqlError         m_last_error;
    bool bEdit = false;
};

#endif // ZRMMETHODEDITOR_H
