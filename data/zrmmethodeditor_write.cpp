/* Ostapenko D. V.
 * NIKTES 2019-04-03
 *
 * ZrmMethodEditor write member functions
 *
*/

#include "zrmmethodeditor.h"
#include <qmessagebox.h>


int          ZrmMethodEditor::change_mask     (const QTreeWidgetItem* item)
{
    bool ok = false;
    int ret = item ? item->data(ZrmMethodsTree::column_name, role_changes_mask).toInt(&ok) : 0;
    return ok ? ret : 0;
}

bool              ZrmMethodEditor::set_change_mask (QTreeWidgetItem* item, int mask, bool set)
{
    if (item)
    {
        int old_mask = change_mask(item);
        int new_mask = set ? (old_mask | mask) : (old_mask & ~mask);
        item->setData(ZrmMethodsTree::column_name, role_changes_mask, new_mask);
        actApply->setEnabled(new_mask);
        actUndo->setEnabled (new_mask);
        actLink->setEnabled (!new_mask && !actMethodEdit->isChecked());
        if (actMethodEdit->isChecked())
            actMethodEdit->setEnabled(!new_mask);
        return new_mask;
    }
    return false;
}

void              ZrmMethodEditor::clr_change_mask (QTreeWidgetItem* item)
{
    set_change_mask(item, -1, false);
}


bool ZrmMethodEditor::write_changes     (QTreeWidgetItem* item)
{
    bool ret = change_mask(item);
    if (ret)
    {
        switch (methods_tree->item_table(item))
        {
            case ZrmMethodsTree::table_types :
                ret = write_type  ( item );
                break;
            case ZrmMethodsTree::table_models:
                ret = write_model ( item ) ;
                break;
            case ZrmMethodsTree::table_method:
                ret = write_method( item );
                break;
            default:
                break;
        }
    }

    if (ret)
    {
        clr_change_mask(item);
    }
    return ret;
}


bool ZrmMethodEditor::item_is_new (QTreeWidgetItem* item)
{
    return  item && item->data(ZrmMethodsTree::column_name, role_new).toBool() ? true : false;
}

void ZrmMethodEditor::item_new_set        (QTreeWidgetItem* item, bool is_new)
{
    if (item)
        item->setData(ZrmMethodsTree::column_name, role_new, is_new);
}


bool ZrmMethodEditor::write_type (QTreeWidgetItem* item)
{
    bool ret = false;
    QVariant id = ZrmMethodsTree::item_id(item);
    ret = ZrmDatabase::write_type(methods_tree->database(), id, item->text(ZrmMethodsTree::column_name));
    if (ret)
    {
        ZrmMethodsTree::set_item_id(item, id);
        clr_change_mask(item);
    }
    return ret;
}

bool ZrmMethodEditor::write_model         (QTreeWidgetItem* item)
{
    QVariant id      = ZrmMethodsTree::item_id(item);
    QVariant id_type = ZrmMethodsTree::item_id(item->parent());
    double   voltage    = item->text   ( ZrmMethodsTree::column_voltage  ).toDouble();
    double   capacity   = item->text   ( ZrmMethodsTree::column_capacity ).toDouble();
    QString  model_name = item->text   ( ZrmMethodsTree::column_name     );
    bool ret = ZrmDatabase::write_model( methods_tree->database(), id_type, id, model_name, voltage, capacity);
    if (  ret )
    {
        ZrmMethodsTree::set_item_id(item, id);
        clr_change_mask(item);
    }
    return ret;
}


bool ZrmMethodEditor::write_method        (QTreeWidgetItem* item)
{
    bool ret = true;
    int ch_mask = change_mask(item);
    if (ch_mask)
    {
        ret = stages_page->write_method( ch_mask & change_item, ch_mask & change_stage);

        if (ret )
        {
            ZrmMethodsTree::set_item_id(item, stages_page->method_id());
            if ( item->parent())
                ZrmDatabase::link_method(methods_tree->database(), ZrmMethodsTree::item_id(item), ZrmMethodsTree::item_id(item->parent()));
        }
    }

    if (ret)
        clr_change_mask(item);
    return ret;
}



bool ZrmMethodEditor::erase_modles        (QTreeWidgetItem* item, bool commit_trans)
{
    bool ret = false;
    if ( QMessageBox::Yes == QMessageBox::question(this
                                                   , tr("Удаление модели")
                                                   , tr("Удалить модель %1 ?").arg(item->text(ZrmMethodsTree::column_name)))
       )
    {
        QSqlDatabase& db = methods_tree->database();
        ret = ZrmDatabase::erase_model(db, ZrmMethodsTree::item_id(item), commit_trans);
    }
    return ret;
}

bool ZrmMethodEditor::erase_types         (QTreeWidgetItem* item)
{
    bool ret = false;
    if ( QMessageBox::Yes == QMessageBox::question(this
                                                   , tr("Удаление типа")
                                                   , tr("Удалить тип %1 ?").arg(item->text(ZrmMethodsTree::column_name)))
       )
    {
        item->setExpanded(true);
        QSqlDatabase& db = methods_tree->database();
        ret = ZrmDatabase::start_transaction(db);
        for (int i = 0; ret && i < item->childCount() ; i++ )
        {
            ret &= ZrmDatabase::erase_model(db, ZrmMethodsTree::item_id(item->child(i)), false);

        }

        if (ret)
        {
            ret = ZrmDatabase::erase_type(db, ZrmMethodsTree::item_id(item));
            if (ret)
                ZrmDatabase::commit_transaction(db);
        }
    }
    return ret;
}


void     ZrmMethodEditor::abstract_method_changed    (QTreeWidgetItem* current, QTreeWidgetItem* prev)
{
    //Смена абстрактного метода
    Q_UNUSED(prev)

    bool link_enabled = false;
    if (current)
    {

        auto method_id = ZrmMethodsTree::item_id(current);
        QTreeWidgetItem* dest_item = methods_tree->currentItem();
        auto table_type = ZrmMethodsTree::item_table(dest_item);
        if (table_type == ZrmMethodsTree::table_models || table_type == ZrmMethodsTree::table_method)
        {
            link_enabled = !methods_tree->search_method_by_id(dest_item, method_id);
        }
    }
    tbLinkMethod->setEnabled( link_enabled);

}



void     ZrmMethodEditor::link_abstract_method ()
{
    //Связать абстрактный метод с моделью
    QTreeWidgetItem* dest_item = this->methods_tree->currentItem();
    unsigned  item_table =  ZrmMethodsTree::item_table(dest_item);
    if (item_table == ZrmMethodsTree::table_method)
    {
        dest_item  =  dest_item->parent();
        item_table =  ZrmMethodsTree::item_table(dest_item);
    }

    if (item_table == ZrmMethodsTree::table_models)
    {
        QTreeWidgetItem* method_item = methods_abstract->currentItem();
        if (!dest_item->isExpanded())
            dest_item->setExpanded(true);
        if (ZrmDatabase::link_method(this->methods_tree->database(), ZrmMethodsTree::item_id(method_item), ZrmMethodsTree::item_id(dest_item) ))
        {
            auto new_item = ZrmMethodsTree::new_tree_item(method_item->text(ZrmMethodsTree::column_name), ZrmMethodsTree::item_table(method_item)
                                                          , ZrmMethodsTree::item_id(method_item), false
                                                         );
            dest_item->addChild(new_item);
        }
    }

}

bool   ZrmMethodEditor::unlink_method (QTreeWidgetItem* method_item, bool del)
{
    //Отсоединение метода
    if (!del || QMessageBox::question(this, tr("Удаление метода")
                                      , tr("Удалить метод ") + method_item->text(ZrmMethodsTree::column_name) + QString(" ?")) == QMessageBox::Yes)
    {


        bool success =  ZrmDatabase::unlink_method( methods_tree->database()
                                                    , ZrmMethodsTree::item_id(method_item)
                                                    , ZrmMethodsTree::item_id(del ? Q_NULLPTR : method_item->parent())
                                                  );
        if (success && del)
        {
            // И удаление его
            success = ZrmDatabase::erase_method(methods_tree->database(), ZrmMethodsTree::item_id(method_item));
        }
        return success;
    }

    return false;
}





