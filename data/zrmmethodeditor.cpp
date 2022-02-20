#include "zrmmethodeditor.h"
#include <signal_bloker.hpp>
#include <qstring.h>
#include <qdesktopwidget.h>
#include <QMessageBox>


ZrmMethodEditor::ZrmMethodEditor(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
    methods_tree->setFont(font());

    buttonAllMethods->setDefaultAction(actAllMethods);
    buttonLink->setDefaultAction(actLink);
    buttonMethodEdit->setDefaultAction(actMethodEdit);
    buttonApply->setDefaultAction(actApply);
    buttonUndo->setDefaultAction(actUndo);
    buttonDelete->setDefaultAction(actDelete);
    buttonNew->setDefaultAction(actNew);
    buttonNewChild->setDefaultAction(actNewChild);
    buttonCopyModel->setDefaultAction(actCopyModel);
    buttonCopyMethod->setDefaultAction(actCopyMethod);
    buttonUnload->setDefaultAction(actUnload);
    buttonLoad->setDefaultAction(actLoad);

    frameAbstract->setVisible(false);
    stages_page->set_methods_tree(methods_tree);
    param_widget->setCurrentWidget(link_page);
    methods_abstract->show_method_params(false);
    connect_signals();
}


bool     ZrmMethodEditor::setAbstract(bool abstract)
{
    return methods_tree->setAbstract(abstract);
}

bool     ZrmMethodEditor::isAbstract()
{
  return methods_tree->isAbstract();
}

bool     ZrmMethodEditor::setWorkMode(zrm::zrm_work_mode_t mode)
{
 return methods_tree->setWorkMode(mode);
}

zrm::zrm_work_mode_t ZrmMethodEditor::getWorkMode()
{
  return methods_tree->getWorkMode();
}


bool    ZrmMethodEditor::open_db()
{
  return methods_tree->open_database();
}

bool    ZrmMethodEditor::open_db(zrm::zrm_work_mode_t mode, bool all_methods)
{
    bool   ret =  methods_tree->open_database(mode, all_methods);
    return ret;
}

void     ZrmMethodEditor::close_db()
{
    methods_tree->close_database();
}

void ZrmMethodEditor::connect_signals()
{
    connect(actMethodEdit , &QAction::toggled        , this, &ZrmMethodEditor::switch_edit_widget    );
    connect(actAllMethods , &QAction::toggled        , this, &ZrmMethodEditor::act_all_methods       );
    connect(actCopyModel  , &QAction::triggered      , this, &ZrmMethodEditor::copy_model            );
    connect(actCopyMethod , &QAction::triggered      , this, &ZrmMethodEditor::copy_method           );
    connect(actUnload     , &QAction::triggered      , this, &ZrmMethodEditor::unload                );
    connect(actLoad       , &QAction::triggered      , this, &ZrmMethodEditor::load                  );

    connect(tbUnlinkMethod, &QAbstractButton::clicked, this, &ZrmMethodEditor::on_actDelete_triggered);
    connect(tbLinkMethod  , &QAbstractButton::clicked, this, &ZrmMethodEditor::link_abstract_method  );
    connect(stages_page   , &ZrmStagesEditor::method_changed, this, &ZrmMethodEditor::sl_method_changed);

    connect(methods_tree    , &ZrmMethodsTree::current_item_changed , this, &ZrmMethodEditor::slot_current_item_changed);
    connect(methods_tree    , &ZrmMethodsTree::item_changed         , this, &ZrmMethodEditor::slot_item_data_changed);
    connect(methods_abstract, &ZrmMethodsTree::current_item_changed , this, &ZrmMethodEditor::abstract_method_changed);
}

#ifdef Q_OS_ANDROID
void ZrmMethodEditor::setup_android_ui()
{
    auto tb_list =  findChildren<QToolButton*>();
    QSize sz(96,96);
    for(auto tb: tb_list)
    {
        tb->setIconSize(sz);
    }
}
#endif


void ZrmMethodEditor::act_all_methods(bool checked)
{
    open_db(methods_tree->opened_as(), checked);
    QTreeWidgetItem * item = methods_tree->topLevelItemCount() ? methods_tree->topLevelItem(0) : nullptr;
    methods_tree->setCurrentItem(item);
    actLink->setEnabled(!checked && !actApply->isEnabled());
    actNew->setVisible(!checked);
}


void set_edit_enable(QTreeWidgetItem * item, bool edit_name, bool edit_voltage,bool edit_capacity )
{
   if(item)
   {
     QSignalBlocker bl(item->treeWidget());
     //qDebug()<<"Set enable  edit voltage "<<edit_voltage<<" edit_capacity "<<edit_capacity;
     item->setData(ZrmMethodsTree::column_name    , ZrmMethodsTree::role_edit_enable, edit_name    );
     item->setData(ZrmMethodsTree::column_voltage , ZrmMethodsTree::role_edit_enable, edit_voltage );
     item->setData(ZrmMethodsTree::column_capacity, ZrmMethodsTree::role_edit_enable, edit_capacity);
   }
}


void ZrmMethodEditor::item_set_inactive(QTreeWidgetItem * prev)
{
  if(prev )
  {
   set_edit_enable(prev,false,false,false);
   /*if(change_mask(prev))
   {
     if(item_is_new(prev))
       do_delete_item(prev,false);
        else
       do_undo_changes(prev);
   }*/
  }
}

void ZrmMethodEditor::slot_current_item_changed        (QTreeWidgetItem * current, QTreeWidgetItem * prev)
{
    if (current != prev)
    {
        SignalBlocker sb(findChildren<QWidget*>());
        item_set_inactive(prev);
        auto table_type = methods_tree->item_table(current);
        actDelete->setEnabled(current && (ZrmMethodsTree::table_method != table_type || actAllMethods->isChecked()));
        tbUnlinkMethod->setEnabled(table_type == ZrmMethodsTree::table_method);
        actMethodEdit ->setEnabled(table_type == ZrmMethodsTree::table_method && actAllMethods->isChecked());
        actNewChild   ->setEnabled(actAllMethods->isChecked() || table_type == ZrmMethodsTree::table_types || table_type == ZrmMethodsTree::table_unknown);
        actCopyModel  ->setEnabled(table_type == ZrmMethodsTree::table_models);
        actCopyMethod ->setEnabled(table_type == ZrmMethodsTree::table_method && actAllMethods->isChecked());

        if(actLink->isChecked())
            abstract_method_changed(methods_abstract->currentItem(), Q_NULLPTR);
        else
            switch (table_type)
            {
            case ZrmMethodsTree::table_models :
                set_edit_enable(current, true, true, true);
                setup_akb(current);
                break;
            case  ZrmMethodsTree::table_method :
                if (actAllMethods->isChecked())
                    set_edit_enable(current, true, true, true);
                break;
            default:
                set_edit_enable(current, true, false, false);
                break;
            }
    }
}

void ZrmMethodEditor::setup_akb(QTreeWidgetItem * item)
{
  if(item)
  {
   zrm_method.m_method.set_voltage( methods_tree->get_method_param(item,ZrmMethodsTree::column_voltage));
   zrm_method.m_method.set_capacity(methods_tree->get_method_param(item,ZrmMethodsTree::column_capacity));
  }
}

QString method_time(const zrm::method_t & method)
{
  return QString("%1:%2:%3").arg(method.m_hours,2,10,QLatin1Char('0')).arg(method.m_minutes,2,10,QLatin1Char('0')).arg(method.m_secs,2,10,QLatin1Char('0'));
}

void set_method_time( zrm::method_t & method, const QString & str)
{
  uint8_t tval[3] = {0};
  uint8_t * ptval = tval;
  for(auto sl : str.split(QLatin1String(":")))
      *ptval++ = uint8_t(sl.trimmed().toUInt());
  method.m_hours = tval[0]; method.m_minutes = tval[1]; method.m_secs = tval[2];

}


void ZrmMethodEditor::setup_method(QTreeWidgetItem * item)
{
  bool is_abstarct = methods_tree->abstract_methods();
  set_edit_enable(item,true, is_abstarct, is_abstarct);
  methods_tree->read_method(item);

  //Разрешаем редактирование

  //QTreeWidgetItem * parent = item->parent() ? item->parent() : item;
  //Учитываем  абстрактный метод
  double volt = methods_tree->get_method_param(item,ZrmMethodsTree::column_voltage );
  double cap  = methods_tree->get_method_param(item,ZrmMethodsTree::column_capacity);
  if(qFuzzyIsNull(volt)) volt = 12;
  if(qFuzzyIsNull(cap )) cap  = 55;

  stages_page->set_abstract(is_abstarct);
  stages_page->set_voltage (volt, false);
  stages_page->set_capacity(cap, false);
  stages_page->set_method_id(ZrmMethodsTree::item_id(item));
  stages_page->set_method_name(item->text(0));
}


/**
 * @brief ZrmMethodEditor::on_stages_list_currentItemChanged
 * @param current
 * @param previous
 * stages_list has changes current stage
 * make setup of stages controls
 */


void     ZrmMethodEditor::slot_item_data_changed                  (QTreeWidgetItem *item, int column)
{

    QString item_text = item->text(column);
    qDebug()<<Q_FUNC_INFO<<"sender "<<sender()<<"column "<<column<<" text "<<item_text;
    auto item_table = ZrmMethodsTree::item_table(item);

    switch(column)
    {
     case ZrmMethodsTree::column_name     :
        if(item_table == ZrmMethodsTree::table_method)
            stages_page->set_method_name(item_text);
        break;
     case ZrmMethodsTree::column_voltage  :
        if(item_table == ZrmMethodsTree::table_method)
          {
            stages_page->set_voltage(item_text.toDouble()) ;
          }
          break;
     case ZrmMethodsTree::column_capacity :
        if(item_table == ZrmMethodsTree::table_method)
          {
            stages_page->set_capacity(item_text.toDouble()) ;
          }
        break;

     default: return;
    }

  set_change_mask( item, change_item, true);
  bEdit = true;
  emit editChanged(bEdit);
}

void ZrmMethodEditor::on_actApply_triggered()
{
  //Записать изменения
  SignalBlocker sb(findChildren<QWidget*>());
  //write_changes(methods_tree->current_item());

    std::function<void(QTreeWidgetItem*)> applyItem = [this, &applyItem](QTreeWidgetItem* item)
    {
        write_changes(item);
        if (item_is_new(item))
            item_new_set(item, false);
        for (int i = 0; i < item->childCount(); i++)
        {
            QTreeWidgetItem* itemChild = item->child(i);
            applyItem(itemChild);
        }
    };

    for (int i = 0; i < methods_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = methods_tree->topLevelItem(i);
        applyItem(item);
    }
    bEdit = false;
    emit editChanged(bEdit);
    if (actMethodEdit->isChecked())
        actMethodEdit->setChecked(false);
}

void ZrmMethodEditor::do_undo_changes(QTreeWidgetItem*item )
{
 SignalBlocker sb(findChildren<QWidget*>());

 switch(methods_tree->item_table(item))
 {
   case ZrmMethodsTree::table_types:
        methods_tree->read_type(item);
     break;
 case ZrmMethodsTree::table_models:
      methods_tree->read_model(item);
   break;
 case ZrmMethodsTree::table_method:
      methods_tree->read_method(item);
      stages_page->set_method_id(ZrmMethodsTree::item_id(item));
   break;
 }
  clr_change_mask( item );
}

void ZrmMethodEditor::on_actUndo_triggered()
{
    std::function<void(QTreeWidgetItem*)> undoItem = [this, &undoItem](QTreeWidgetItem* item)
    {
        if (change_mask(item))
        {
            if (item_is_new(item))
            {
                do_delete_item(item, false);
                return;
            }
            else
                do_undo_changes(item);
        }
        for (int i = 0; i < item->childCount(); i++)
        {
            QTreeWidgetItem* itemChild = item->child(i);
            undoItem(itemChild);
        }
    };

    QTreeWidgetItem* itemPrev = methods_tree->currentItem();
    if (item_is_new(itemPrev))
        itemPrev = nullptr;

    //Отменить изменения
    for (int i = 0; i < methods_tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = methods_tree->topLevelItem(i);
        undoItem(item);
    }

    QTreeWidgetItem* itemCurrent = methods_tree->currentItem();
    slot_current_item_changed(itemCurrent, itemPrev);
    actApply->setEnabled(false);
    actUndo->setEnabled (false);
    actLink->setEnabled (!actAllMethods->isChecked());
    bEdit = false;
    emit editChanged(bEdit);
    if (actMethodEdit->isChecked())
        actMethodEdit->setChecked(false);

    /*auto item = methods_tree->current_item();
    do_undo_changes(item);
    slot_current_item_changed(item, Q_NULLPTR);
    if (actMethodEdit->isChecked() && item_is_new(item))
    {
        actMethodEdit->setChecked(false);
        do_delete_item(item, false);
    }*/
}


void ZrmMethodEditor::do_delete_item(QTreeWidgetItem *item , bool select_next)
{
 bool ret = true;
 if(!item_is_new(item))
 {
   switch(methods_tree->item_table(item))
   {
     case ZrmMethodsTree::table_method : ret = unlink_method(item ,actAllMethods->isChecked()); break;
     case ZrmMethodsTree::table_models : ret = erase_modles( item ); break;
     case ZrmMethodsTree::table_types  : ret = erase_types ( item ); break;
     default : break;
   }
 }
 if(ret)
 {
     QTreeWidget * tw = item->treeWidget();
     QTreeWidgetItem * parent_item = item->parent();
     int idx = parent_item ? parent_item->indexOfChild(item) : tw->indexOfTopLevelItem(item);
     item = Q_NULLPTR;

     if(parent_item )
     {
         delete parent_item->takeChild(idx);
         if(select_next)
         {
             idx = std::min(idx,parent_item->childCount()-1);
             item = idx < 0 ? parent_item :  parent_item->child(idx);
         }
     }
     else
     {
         delete tw->takeTopLevelItem(idx);
         if(select_next)
         {
             idx = std::min(idx, tw->topLevelItemCount()-1);
             item = idx < 0 ? Q_NULLPTR :  tw->topLevelItem(idx);
         }
     }

     if(select_next)
         tw->setCurrentItem(item);
 }
}

void ZrmMethodEditor::on_actDelete_triggered()
{
  //Удалить элемент
  QTreeWidgetItem * item = methods_tree->currentItem();
  if(item)
  {
    do_delete_item(item,true);
  }
}

void ZrmMethodEditor::on_actLink_toggled(bool checked)
{
    // Связать
    frameAbstract->setVisible(checked);
    if(checked)
    {
        methods_abstract->open_database(methods_tree->opened_as(), true);
        actMethodEdit->setChecked(false);
    }
    else
        slot_current_item_changed(this->methods_tree->currentItem(), nullptr);

    //methods_tree->show_method_params(!checked);
    actAllMethods->setEnabled(!checked);
    actMethodEdit->setVisible(!checked);
    actApply->setVisible(!checked);
    actUndo->setVisible(!checked);
    actNew->setVisible(!checked);
    actNewChild->setVisible(!checked);
    actDelete->setVisible(!checked);
    actUnload->setVisible(!checked);
    actLoad->setVisible(!checked);
}

//bool ZrmMethodEditor::on_dsb_changed(QDoubleSpinBox * dsb)
//{
//     double  value = dsb->value();
//     auto curr_item  =  methods_tree->current_item();
//     if( dsb == edAkbVoltage)
//     {
//      methods_tree->set_method_param(curr_item, ZrmMethodsTree::column_voltage , value);
//      zrm_method.m_method.set_voltage(value);
//      return true;
//     }

//     if( dsb == edAkbCapacity)
//     {
//      methods_tree->set_method_param(curr_item,  ZrmMethodsTree::column_capacity , value);
//      zrm_method.m_method.set_capacity(value);
//      return  true;
//     }
//   return false;
//}

//void ZrmMethodEditor::on_parameters_changed()
//{
//   QObject * src = sender();

//   auto curr_item  =  methods_tree->current_item();
//   auto table_type =  methods_tree->item_table(curr_item);
//   Q_UNUSED(table_type);
//   QDoubleSpinBox * dsb = dynamic_cast<QDoubleSpinBox*>(src);

//   if(dsb )
//   {
//      if(on_dsb_changed(dsb))
//       set_change_mask(curr_item, change_item, true);
//      return;
//   }

//}


void ZrmMethodEditor::create_new(bool child)
{
    QTreeWidgetItem * cur_item = child ? methods_tree->currentItem() : Q_NULLPTR;
    auto t_type = ZrmMethodsTree::item_table(cur_item);

    if(t_type == ZrmMethodsTree::table_unknown)
    {
        child = false;
        t_type = ZrmMethodsTree::table_types;
    }

    if(child && t_type == ZrmMethodsTree::table_method)
        child = false;

    if(child) ++t_type;

    /*QString item_text;

    switch(t_type)
    {
        case ZrmMethodsTree::table_types : item_text = tr("Новый тип"); break;
        case ZrmMethodsTree::table_models: item_text = tr("Новая модель"); break;
        default : item_text = tr("Новый метод"); break;
    }

    QTreeWidgetItem * item = ZrmMethodsTree::new_tree_item(item_text, t_type, 0, false);*/
    QTreeWidgetItem * item = ZrmMethodsTree::new_tree_item("", t_type, 0, false);
    bool edit_params = t_type == ZrmMethodsTree::table_models || ( t_type == ZrmMethodsTree::table_method && actAllMethods->isChecked());
    if(edit_params)
    {
        QString str = "?";
        item->setText(ZrmMethodsTree::column_voltage, str);
        item->setText(ZrmMethodsTree::column_capacity, str);
    }
    set_edit_enable(item, true, edit_params, edit_params);
    item_new_set(item, true);

    if(cur_item && !child)
        cur_item = cur_item->parent();
    if(cur_item)
    {
        if(!cur_item->isExpanded())
        {
            cur_item->setExpanded(true);
            qApp->processEvents();
        }
        cur_item->addChild(item);
    }
    else
        methods_tree->addTopLevelItem(item);
    methods_tree->setCurrentItem(item);

    if (ZrmMethodsTree::table_method == t_type)
    {
        actMethodEdit->setChecked(true);
        //set_change_mask(item, change_item, true);
    }
    set_change_mask(item, change_item, true);
    bEdit = true;
    emit editChanged(bEdit);
}

void ZrmMethodEditor::on_actNew_triggered()
{
    //Создание нового элемента
    create_new(false);
}

void ZrmMethodEditor::on_actNewChild_triggered()
{
    //Создание нового подчиненного
    create_new(true);
}


void ZrmMethodEditor::switch_edit_widget(bool edit_param)
{
    if(edit_param)
    {
        setup_method(methods_tree->currentItem());
        param_widget->setCurrentWidget(stages_page);
        bEdit = true;
        emit editChanged(bEdit);
    }
    else
    {
        if (methods_tree->abstract_methods())
            methods_tree->read_method_abstract(methods_tree->currentItem());
        param_widget->setCurrentWidget(link_page);
        bEdit = false;
        emit editChanged(bEdit);
    }
    actAllMethods->setEnabled(!edit_param);
    actLink->setEnabled(!edit_param && !actAllMethods->isChecked() && !actApply->isEnabled());
    actNew->setVisible(!edit_param && !actAllMethods->isChecked());
    actNewChild->setVisible(!edit_param);
    actDelete->setVisible(!edit_param);
    actCopyModel->setVisible(!edit_param);
    actCopyMethod->setVisible(!edit_param);
    actUnload->setVisible(!edit_param);
    actLoad->setVisible(!edit_param);
}

void     ZrmMethodEditor::sl_method_changed(int what)
{
  QTreeWidgetItem * method_item =  methods_tree->currentItem();
  if(method_item)
  {
   QSignalBlocker bl(method_item->treeWidget());
   switch(what)
   {
    case ZrmStagesEditor::method_param_changed:
    {
       if (actAllMethods->isChecked())
       {
           method_item->setText(ZrmMethodsTree::column_voltage, QString::number(stages_page->user_voltage(), 'f', 1));
           method_item->setText(ZrmMethodsTree::column_capacity, QString::number(stages_page->user_capacity(), 'f', 1));
       }
       set_change_mask(method_item, change_item, true);
    }
    break;
    case ZrmStagesEditor::method_name_changed:
        {
         QString met_name = stages_page->method_name();
         //qDebug()<<"new name "<<met_name;
         method_item->setText(ZrmMethodsTree::column_name, met_name);
         set_change_mask(method_item, change_item, true);
        }
    break;
    case ZrmStagesEditor::method_stage_changed:
         set_change_mask(method_item, change_stage, true);
    break;

   }
   bEdit = true;
   emit editChanged(bEdit);
  }
}
/**
 * @brief ZrmMethodEditor::copy_model
 * Копирование модели
 */

void ZrmMethodEditor::copy_model()
{
  QTreeWidgetItem * item = methods_tree->currentItem();
  if( ZrmMethodsTree::item_table(item) == ZrmMethodsTree::table_models && item->childCount() )
  {
    setCursor(Qt::WaitCursor);
    bool was_expanding = item->isExpanded();
    if(!was_expanding)
      {item->treeWidget()->expandItem(item);qApp->processEvents();}

    QTreeWidgetItem * new_model = ZrmMethodsTree::copy_tree_item(item, item->parent());
    QString item_name = QString("%1 - КОПИЯ").arg(item->text(ZrmMethodsTree::column_name));
    new_model->setText(ZrmMethodsTree::column_name, item_name);
    ZrmMethodsTree::set_item_id( new_model, QVariant());
    QSqlDatabase & db = methods_tree->database();
    ZrmDatabase::start_transaction(db);
    bool success = write_model(new_model);
    int met_idx = item->childCount();
    QVariant model_id = ZrmMethodsTree::item_id(new_model);

    while(success && met_idx)
    {
      QTreeWidgetItem * met_item = item->child(--met_idx);
      success = ZrmDatabase::link_method(db,ZrmMethodsTree::item_id(met_item),model_id);
    }

    if(success)
    {
      ZrmDatabase::commit_transaction(db);
      new QTreeWidgetItem(new_model);
      methods_tree->setCurrentItem(new_model);
      set_edit_enable(new_model,true,true,true);
    }
    else
    {
         ZrmMethodsTree::set_item_id( new_model, QVariant());
         do_delete_item(new_model,false);
    }

    if(!was_expanding) item->treeWidget()->collapseItem(item);
    setCursor(Qt::ArrowCursor);
  }
}

void ZrmMethodEditor::copy_method()
{
    QTreeWidgetItem * curitem = methods_tree->currentItem();
    if (!curitem)
        return;
    if (QMessageBox::Yes != QMessageBox::question(this, tr("Копирование метода"), tr("Сделать копию метода ") + curitem->text(ZrmMethodsTree::column_name) + QString(" ?")))
        return;
    QTreeWidgetItem * item = ZrmMethodsTree::new_tree_item(curitem->text(ZrmMethodsTree::column_name) + " - КОПИЯ", ZrmMethodsTree::table_method, 0, false);
    item->setText(ZrmMethodsTree::column_voltage, curitem->text(ZrmMethodsTree::column_voltage));
    item->setText(ZrmMethodsTree::column_capacity, curitem->text(ZrmMethodsTree::column_capacity));
    set_edit_enable(item, true, true, true);
    item_new_set(item, true);

    methods_tree->addTopLevelItem(item);
    methods_tree->setCurrentItem(item);

    setup_method(curitem);
    stages_page->copy_method(ZrmMethodsTree::item_id(item));
    stages_page->set_method_name(item->text(0));
    set_change_mask(item, change_item | change_stage, true);

    write_method(item);
    item_new_set(item, false);
}

void    ZrmMethodEditor::save_user_values()
{
  methods_tree->save_user_values();
}

void ZrmMethodEditor::unload()
{
    zrm::zrm_work_mode_t m_work_mode = methods_tree->opened_as();
    ZrmDataSource::unload(m_work_mode);
}

void ZrmMethodEditor::load()
{
    zrm::zrm_work_mode_t m_work_mode = methods_tree->opened_as();
    ZrmDataSource::load(m_work_mode);
    open_db(m_work_mode, actAllMethods->isChecked());
    if (actLink->isChecked())
        methods_abstract->open_database(m_work_mode, true);
}

QList<int> ZrmMethodEditor::getSplitterSizes()
{
    return  stages_page->getSplitterSizes();
}

void ZrmMethodEditor::setSplitterSizes(const QList<int> &list)
{
    stages_page->setSplitterSizes(list);
}

void ZrmMethodEditor::setAllMethods(bool all_methods)
{
    if (all_methods && !actAllMethods->isChecked())
        actAllMethods->toggle();
    buttonAllMethods->setVisible(false);
    buttonLink->setVisible(!all_methods);
    buttonMethodEdit->setVisible(all_methods);
    buttonNew->setVisible(!all_methods);
    buttonCopyModel->setVisible(!all_methods);
    buttonCopyMethod->setVisible(all_methods);
}

void ZrmMethodEditor::refresh()
{
    methods_tree->close_database();
    open_db(methods_tree->opened_as(), actAllMethods->isChecked());
    QTreeWidgetItem * item = methods_tree->topLevelItemCount() ? methods_tree->topLevelItem(0) : nullptr;
    methods_tree->setCurrentItem(item);

    methods_abstract->close_database();
    methods_abstract->open_database(methods_tree->opened_as(), true);
}
