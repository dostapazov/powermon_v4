/* OStapenko D.V. NIKTES 2019-May-03
 * Really, this widget is full single-method editor
 * Will renamed later
*/

#include "zrmstageseditor.h"
#include <zrmdatasource.h>
#include <zrm_connectivity.hpp>
#include <signal_bloker.hpp>



constexpr const char * STAGE_CTRL_TAG        = "ctrl_tag";

class stage_item_delegate : public QItemDelegate
{
    public:
        stage_item_delegate(QObject * parent):QItemDelegate(parent){}
virtual QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
};

QWidget *stage_item_delegate::createEditor(QWidget *parent,
                      const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
 if(index.column() == ZrmStagesEditor::stage_descr_column )
      return QItemDelegate::createEditor(parent, option, index);
  return Q_NULLPTR;
}



ZrmStagesEditor::ZrmStagesEditor(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    QHeaderView * hdr = stages_list->header();
    hdr->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    hdr->setSectionResizeMode(stage_descr_column, QHeaderView::ResizeMode::ResizeToContents);
    stages_list->setItemDelegate(new stage_item_delegate(stages_list));
    init_controls();
}


void ZrmStagesEditor::init_controls()
{
    clear_controls();
    connect(stages_list, &QTreeWidget::currentItemChanged, this, &ZrmStagesEditor::sl_stage_changed);
    connect(stages_list, &QTreeWidget::itemChanged       , this, &ZrmStagesEditor::sl_stage_data_changed);

    connect(sbMCycleCount, QOverload<int>::of   (&QSpinBox::valueChanged)         , this, &ZrmStagesEditor::sl_method_param_changed);
    connect(edMethodTime , QOverload<const QString &>::of(&QLineEdit::textChanged), this, &ZrmStagesEditor::sl_method_param_changed);
    connect(ed_method_name, &QLineEdit::textChanged, this, &ZrmStagesEditor::sl_method_name_changed);

    connect(edVoltage , QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ZrmStagesEditor::sl_voltage_changed);
    connect(edCapacity, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ZrmStagesEditor::sl_capacity_changed);

    tbCharge   ->setProperty(STAGE_CTRL_TAG, zrm::STT_CHARGE);
    tbDischarge->setProperty(STAGE_CTRL_TAG, zrm::STT_DISCHARGE);
    tbImpulse  ->setProperty(STAGE_CTRL_TAG, zrm::STT_IMPULSE);
    tbPause    ->setProperty(STAGE_CTRL_TAG, zrm::STT_PAUSE  );

    connect(tbCharge, QOverload<bool>::of(&QAbstractButton::toggled), this, &ZrmStagesEditor::sl_stage_type_changed);
    connect(tbDischarge, QOverload<bool>::of(&QAbstractButton::toggled), this, &ZrmStagesEditor::sl_stage_type_changed);
    connect(tbImpulse, QOverload<bool>::of(&QAbstractButton::toggled), this, &ZrmStagesEditor::sl_stage_type_changed);
    connect(tbPause, QOverload<bool>::of(&QAbstractButton::toggled), this, &ZrmStagesEditor::sl_stage_type_changed);

    connect(edChargeVoltage, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ZrmStagesEditor::sl_stage_charge_changed);
    connect(edChargeCurrent, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ZrmStagesEditor::sl_stage_charge_changed);

    connect(edDischargeVoltage, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ZrmStagesEditor::sl_stage_discharge_changed);
    connect(edDischargeCurrent, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ZrmStagesEditor::sl_stage_discharge_changed);

    connect(sbChargeTime, QOverload<int>::of(&QSpinBox::valueChanged), this, &ZrmStagesEditor::sl_stage_impule_time_changed);
    connect(sbDischargeTime, QOverload<int>::of(&QSpinBox::valueChanged), this, &ZrmStagesEditor::sl_stage_impule_time_changed);

    cbCapMesure->setProperty(STAGE_CTRL_TAG, zrm::stage_flags_t::STFL_CAPACITY_MEASHURE);
    connect(cbCapMesure, QOverload<bool>::of(&QAbstractButton::toggled), this, &ZrmStagesEditor::sl_stage_flags_changed);

    cbCheckCondition->setProperty(STAGE_CTRL_TAG, zrm::stage_flags_t::STFL_CONDITION_CHECK);
    connect(cbCheckCondition, QOverload<bool>::of(&QAbstractButton::toggled), this, &ZrmStagesEditor::sl_stage_flags_changed);

    cbFinishCurrent      ->setProperty(STAGE_CTRL_TAG, zrm::stage_end_flags_t::stage_end_current);
    cbFinishDeltaVoltage ->setProperty(STAGE_CTRL_TAG, zrm::stage_end_flags_t::stage_end_delta_voltage);
    cbFinishCellVoltage  ->setProperty(STAGE_CTRL_TAG, zrm::stage_end_flags_t::stage_end_cell_voltage);
    cbFinishTemperature  ->setProperty(STAGE_CTRL_TAG, zrm::stage_end_flags_t::stage_end_temper);
    cbFinishCapacity     ->setProperty(STAGE_CTRL_TAG, zrm::stage_end_flags_t::stage_end_capacity);
    cbFinishVoltage      ->setProperty(STAGE_CTRL_TAG, zrm::stage_end_flags_t::stage_end_voltage);
    cbFinishTime         ->setProperty(STAGE_CTRL_TAG, zrm::stage_end_flags_t::stage_end_time);

    for(auto cb : stage_finish->findChildren<QAbstractButton*>())
        connect(cb, QOverload<bool>::of(&QAbstractButton::toggled), this, &ZrmStagesEditor::sl_stage_finish_flags_changed);

    for(auto sb : stage_finish->findChildren<QDoubleSpinBox*>())
        connect(sb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ZrmStagesEditor::sl_stage_finish_changed);

    for(auto sb : stage_finish->findChildren<QSpinBox*>())
        connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, &ZrmStagesEditor::sl_stage_finish_changed);

    connect(this->tbStageAdd     , &QAbstractButton::clicked, this, &ZrmStagesEditor::sl_stage_add);
    connect(this->tbStageRemove  , &QAbstractButton::clicked, this, &ZrmStagesEditor::sl_stage_remove);
    connect(this->tbStageMoveUp  , &QAbstractButton::clicked, this, &ZrmStagesEditor::sl_stage_move);
    connect(this->tbStageMoveDown, &QAbstractButton::clicked, this, &ZrmStagesEditor::sl_stage_move);

    //connect(splitter, &QSplitter::splitterMoved, [this](){ splitterSizes = splitter->sizes(); });
}


void ZrmStagesEditor::clear_controls    ()
{
 ChildrenSignalBlocker<QWidget> sb(this);

 for(auto sb :findChildren<QSpinBox*>())
       sb->setValue(0);

 for(auto sb :findChildren<QDoubleSpinBox*>())
       sb->setValue(.0);

 for(auto cb : stage_finish->findChildren<QAbstractButton*>())
       cb->setChecked(Qt::Unchecked);

 for(auto cb : stage_flags->findChildren<QAbstractButton*>())
       cb->setChecked(Qt::Unchecked);

 for(auto ed :findChildren<QLineEdit*>())
       ed->setText(QString());
}

void ZrmStagesEditor::enabled_controls()
{
    ChildrenSignalBlocker<QWidget> sb(this);

    QTreeWidgetItem * item = currentStageItem();
    bool bEnable = item;

    int idx = currentStageItemIndex(item);
    int delta = stages_list->topLevelItemCount() - idx;
    tbStageMoveUp->setEnabled(bEnable && idx > 0);
    tbStageMoveDown->setEnabled(bEnable && delta > 1);

    tbStageRemove->setEnabled(bEnable);
    tbCharge->setEnabled(bEnable);
    tbDischarge->setEnabled(bEnable);
    tbImpulse->setEnabled(bEnable);
    tbPause->setEnabled(bEnable);
    stage_finish->setEnabled(bEnable);
    stage_flags->setEnabled(bEnable);

    if (idx >= 0)
    {
        auto st = m_current_method.m_stages.at(size_t(idx));
        stage_type_changed(zrm::stage_type_t(st.m_type));
    }
}

void ZrmStagesEditor::on_stages_changed ()
{
    QTreeWidgetItem* item = currentStageItem();
    if (item)
    {
        zrm::stage_type_t st = zrm::STT_PAUSE;
        for (int i = item->childCount() - 1; i >= 0; i--)
        {
            QTreeWidgetItem* c = item->child(i);
            item->removeChild(c);
        }
        const int STAGE_TYPE_COLUMN   = 1;
        const int STAGE_FINISH_COLUMN = 2;
        QList<QTreeWidgetItem*> sub_items;

        if (tbCharge->isChecked())
        {
            st = zrm::STT_CHARGE;
            QTreeWidgetItem * sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr(zrm::stage_t::stage_type_name(m_methods_tree->open_as(), zrm::STT_CHARGE)));
            QString text = tr("U=%1 В, I=%2 А").arg(edChargeVoltage->value(), 0, 'f', 2).arg(edChargeCurrent->value(), 0,'f', 2);
            sitem->setText(STAGE_FINISH_COLUMN, text);
            sub_items << sitem;
        }

        if (tbDischarge->isChecked())
        {
            st = zrm::STT_DISCHARGE;
            QTreeWidgetItem * sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr(zrm::stage_t::stage_type_name(m_methods_tree->open_as(), zrm::STT_DISCHARGE)));
            QString text = tr("U=%1 В, I=%2 А").arg(edDischargeVoltage->value(), 0, 'f', 2).arg(edDischargeCurrent->value(), 0, 'f', 2);
            sitem->setText(STAGE_FINISH_COLUMN, text);
            sub_items << sitem;
        }

        if (tbImpulse->isChecked())
        {
            st = zrm::STT_IMPULSE;
            QTreeWidgetItem * sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr(zrm::stage_t::stage_type_name(m_methods_tree->open_as(), zrm::STT_CHARGE)));
            QString text = tr("U=%1 В, I=%2 А").arg(edChargeVoltage->value(), 0, 'f', 2).arg(edChargeCurrent->value(), 0,'f', 2);
            sitem->setText(STAGE_FINISH_COLUMN, text);
            sub_items << sitem;

            sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr("Время "));
            sitem->setText(STAGE_FINISH_COLUMN, tr("%1 сек").arg(sbChargeTime->value()));
            sub_items << sitem;

            sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr(zrm::stage_t::stage_type_name(m_methods_tree->open_as(), zrm::STT_DISCHARGE)));
            text = tr("U=%1 В, I=%2 А").arg(edDischargeVoltage->value(), 0, 'f', 2).arg(edDischargeCurrent->value(), 0, 'f', 2);
            sitem->setText(STAGE_FINISH_COLUMN, text);
            sub_items << sitem;

            sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr("Время "));
            sitem->setText(STAGE_FINISH_COLUMN, tr("%1 сек").arg(sbDischargeTime->value()));
            sub_items << sitem;
        }

        if (cbFinishTime->isChecked())
        {
            QTreeWidgetItem * sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по времени"));
            QString text = tr("%1:%2:%3").arg(sbFinishHour->value(), 2, 10, QLatin1Char('0'))
                                         .arg(sbFinishMinuts->value(), 2, 10, QLatin1Char('0'))
                                         .arg(sbFinishSecunds->value(), 2, 10, QLatin1Char('0'));
            sitem->setText(STAGE_FINISH_COLUMN, text);
            sub_items << sitem;
        }

        if (cbFinishCurrent->isChecked())
        {
            QTreeWidgetItem * sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по I"));
            sitem->setText(STAGE_FINISH_COLUMN, tr("%1 А").arg(QString::number(sbFinishCurrent->value(), 'f', 2)));
            sub_items << sitem;
        }

        if (cbFinishVoltage->isChecked())
        {
            QTreeWidgetItem * sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по U"));
            sitem->setText(STAGE_FINISH_COLUMN, tr("%1 В").arg(QString::number(sbFinishVoltage->value(), 'f', 2)));
            sub_items << sitem;
        }

        if (cbFinishDeltaVoltage->isChecked())
        {
            QTreeWidgetItem * sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по ΔU"));
            sitem->setText(STAGE_FINISH_COLUMN, tr("%1 В").arg(QString::number(sbFinishDeltaVoltage->value(), 'f', 2)));
            sub_items << sitem;
        }

        if (cbFinishCellVoltage->isChecked())
        {
            QTreeWidgetItem * sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по U-элемента"));
            sitem->setText(STAGE_FINISH_COLUMN, tr("%1 В").arg(QString::number(sbFinishCellVoltage->value(), 'f', 2)));
            sub_items << sitem;
        }

        if (cbFinishTemperature->isChecked())
        {
            QTreeWidgetItem * sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение T"));
            sitem->setText(STAGE_FINISH_COLUMN, tr("%1 C").arg(QString::number(sbFinishTemperature->value(), 'f', 2)));
            sub_items << sitem;
        }

        if(cbFinishCapacity->isChecked())
        {
            QTreeWidgetItem * sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по емкости"));
            sitem->setText(STAGE_FINISH_COLUMN, tr("%1 А*Ч").arg(QString::number(sbFinishCapacity->value(),'f',2 )));
            sub_items << sitem;
        }

        if(sub_items.count())
            item->addChildren(sub_items);

        stage_type_changed(zrm::stage_type_t(st));
    }

    emit method_changed(method_stage_changed);
}

void ZrmStagesEditor::on_method_changed ()
{
 emit method_changed(method_param_changed);
}



void             ZrmStagesEditor::set_methods_tree     (ZrmMethodsTree * mtree)
{
    if(m_methods_tree != mtree)
    {
       if(m_methods_tree)
           m_methods_tree->disconnect(this);
        m_methods_tree = mtree;
       if(m_methods_tree)
          connect(m_methods_tree,&ZrmMethodsTree::database_open, this, &ZrmStagesEditor::sl_database_open);

    }
}

void ZrmStagesEditor::sl_database_open(bool success)
{
   Q_UNUSED(success)
   auto wm = m_methods_tree->open_as();
   tbCharge   ->setText( zrm::stage_t::stage_type_name(wm, zrm::STT_CHARGE    ));
   tbDischarge->setText( zrm::stage_t::stage_type_name(wm, zrm::STT_DISCHARGE ));

}

void     ZrmStagesEditor::set_method_name       (const QString & name)
{
   QSignalBlocker sb(ed_method_name);
   ed_method_name->setText(name);
}

QString  ZrmStagesEditor::method_name           ()
{
   return ed_method_name->text();
}

double ZrmStagesEditor::user_voltage()
{
    return edVoltage->value();
}

double ZrmStagesEditor::user_capacity()
{
    return edCapacity->value();
}

void   ZrmStagesEditor::set_abstract    (bool is_abstract)
{
   edVoltage->setEnabled(is_abstract);
   edCapacity->setEnabled(is_abstract);
}

void ZrmStagesEditor::sl_capacity_changed(double val)
{
    if (0 != val)
        set_capacity(val, true);
    on_method_changed();
}

void   ZrmStagesEditor::set_capacity(double cap, bool update)
{
  m_capacity = cap;
  QSignalBlocker s(edCapacity);
  edCapacity ->setValue( cap );
  m_current_method.m_method.set_capacity(cap);
  m_current_method.m_method.set_current(cap);
  ChildrenSignalBlocker<QWidget> sb(method_box);
  edMMaxCurrent->setValue(m_current_method.m_method.current());
  edCapPercent->setValue (m_current_method.m_method.current_ratio(true));
  if(update && stages_list->topLevelItemCount())
      sl_stage_changed(currentStageItem(), Q_NULLPTR);
}

void ZrmStagesEditor::sl_voltage_changed    (double val)
{
   set_voltage(val, true);
   on_method_changed();
}

void   ZrmStagesEditor::set_voltage(double volt, bool update)
{
  qDebug()<<Q_FUNC_INFO<<volt;
  QSignalBlocker s(edVoltage);
  edVoltage->setValue(volt);
  m_voltage = volt;
  m_current_method.m_method.set_voltage(volt);
  if(update && stages_list->topLevelItemCount())
      sl_stage_changed(currentStageItem(), Q_NULLPTR);
}


void    ZrmStagesEditor::set_method_id           (QVariant  id )
{
   m_method_id = id;
   read_method();
}

void ZrmStagesEditor::copy_method(QVariant id)
{
    m_method_id = id;
    for (zrm::stage_t & st : m_current_method.m_stages)
        st.m_id_method = id.toUInt();
}

void ZrmStagesEditor::sl_method_param_changed()
{
 //изменения параметров метода
 QObject * src = sender();
 if(src)
 {
   ChildrenSignalBlocker<QWidget> sb(method_box);
   if(src == sbMCycleCount)
     {
       m_current_method.m_method.set_cycles(sbMCycleCount->value());
     }

   if(src == edMethodTime)
     {
       m_current_method.m_method.set_duration(zrm::method_t::hms2secunds(zrm::ZrmConnectivity::string2hms(edMethodTime->text())));
     }
   on_method_changed();
 }

}

void ZrmStagesEditor::read_method     ()
{
  m_removed_stages.clear();
  m_current_method.m_stages.clear();
  QSqlDatabase & db = m_methods_tree->database();

  if(ZrmDatabase::read_method       (db, method_id(), m_current_method.m_method, m_voltage, m_capacity))
     {
      ZrmDatabase::read_method_stages(db, method_id(), m_current_method.m_stages);

     }
  else
  {
   //Новый медот
   clear_controls();
   m_current_method.m_method.set_capacity(m_capacity);
   m_current_method.m_method.set_voltage(m_voltage);
  }
  //m_methods_tree ->get_method( m_method, Q_NULLPTR );
  setup_method();
}

void ZrmStagesEditor::setup_method    ()
{
  ChildrenSignalBlocker<QWidget> bl(method_box);
  int name_len = int(m_current_method.m_method.name_length());

  QByteArray ba(m_current_method.m_method.m_name,name_len);
  QString m_name = ZrmDatabase::to_unicode(ba);
  ed_method_name->setText(m_name);
  sbMCycleCount->setValue(m_current_method.m_method.cycles());
  edMMaxCurrent->setValue(m_current_method.m_method.current());
  edCapPercent->setValue (m_current_method.m_method.current_ratio(true));
  edMethodTime->setText(zrm::ZrmConnectivity::hms2string(zrm::method_t::secunds2hms(m_current_method.m_method.duration())));
  setup_stages();
}

void ZrmStagesEditor::setup_stages    ()
{
    ChildrenSignalBlocker<QWidget> bl(stages_box);
    stages_list->clear();
    QList<QTreeWidgetItem* > items;
    for(auto st : m_current_method.m_stages)
    {
        QTreeWidgetItem * stage_item = ZrmMethodsTree::new_tree_item(QString(), ZrmMethodsTree::table_stages, st.m_id_method, false);
        stage_assign(stage_item, st, ZrmDatabase::read_stage_descript(m_methods_tree->database(), st.m_id_method));
        items.append(stage_item);
    }
    stages_list->addTopLevelItems(items);

    if(items.size())
    {
        items[0]->setSelected(true);
        stages_list->setCurrentItem(items[0]);
        sl_stage_changed(items[0], Q_NULLPTR);
    }
    enabled_controls();
}

void ZrmStagesEditor::stage_assign(QTreeWidgetItem * stage_item, const zrm::stage_t & st, const QString &descr)
{
    stage_item->setText(stage_number_column,QString::number(uint(st.m_number)));
    stage_item->setData(stage_number_column,ZrmMethodsTree::role_id,st.m_id_method);
    stage_item->setData(stage_number_column,ZrmMethodsTree::role_table,ZrmMethodsTree::table_stages);
    stage_item->setText(stage_type_column , tr(zrm::stage_t::stage_type_name(m_methods_tree->open_as(),zrm::stage_type_t(st.m_type))));
    stage_item->setText(stage_descr_column, descr);

    for (int i = stage_item->childCount() - 1; i >= 0; i--)
    {
        QTreeWidgetItem* c = stage_item->child(i);
        stage_item->removeChild(c);
    }
    const int STAGE_TYPE_COLUMN   = 1;
    const int STAGE_FINISH_COLUMN = 2;

    QList<QTreeWidgetItem*> sub_items;

    if (st.m_type & zrm::STT_CHARGE)
    {
        QTreeWidgetItem * sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr(zrm::stage_t::stage_type_name(m_methods_tree->open_as(), zrm::STT_CHARGE)));
        QString text = tr("U=%1 В, I=%2 А").arg(st.charge_volt(m_current_method.m_method), 0, 'f', 2).arg(st.charge_curr(m_current_method.m_method), 0,'f', 2);
        sitem->setText(STAGE_FINISH_COLUMN, text);
        sub_items << sitem;

        if(st.m_type == zrm::STT_IMPULSE)
        {
            sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr("Время "));
            sitem->setText(STAGE_FINISH_COLUMN, tr("%1 сек").arg(uint32_t(st.m_char_time)));
            sub_items << sitem;
        }
    }

    if (st.m_type & zrm::STT_DISCHARGE)
    {
        QTreeWidgetItem * sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr(zrm::stage_t::stage_type_name(m_methods_tree->open_as(), zrm::STT_DISCHARGE)));
        QString text = tr("U=%1 В, I=%2 А").arg(st.discharge_volt(m_current_method.m_method), 0, 'f', 2).arg(st.discharge_curr(m_current_method.m_method), 0, 'f', 2);
        sitem->setText(STAGE_FINISH_COLUMN, text);
        sub_items << sitem;

        if(st.m_type == zrm::STT_IMPULSE)
        {
            sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr("Время "));
            sitem->setText(STAGE_FINISH_COLUMN, tr("%1 сек").arg(uint32_t(st.m_dis_time)));
            sub_items << sitem;
        }
    }

    if (st.m_finish_flags & zrm::stage_end_time)
    {
        QTreeWidgetItem * sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по времени"));
        QString text = tr("%1:%2:%3").arg(uint32_t(st.m_hours  ), 2, 10, QLatin1Char('0'))
                                     .arg(uint32_t(st.m_minutes), 2, 10, QLatin1Char('0'))
                                     .arg(uint32_t(st.m_secs   ), 2, 10, QLatin1Char('0'));
        sitem->setText(STAGE_FINISH_COLUMN, text);
        sub_items << sitem;
    }

    if (st.m_finish_flags & zrm::stage_end_current)
    {
        QTreeWidgetItem * sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по I"));
        sitem->setText(STAGE_FINISH_COLUMN, tr("%1 А").arg(QString::number(st.end_curr(m_current_method.m_method), 'f', 2)));
        sub_items << sitem;
    }

    if (st.m_finish_flags & zrm::stage_end_voltage)
    {
        QTreeWidgetItem * sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по U"));
        sitem->setText(STAGE_FINISH_COLUMN, tr("%1 В").arg(QString::number(st.end_volt(m_current_method.m_method), 'f', 2)));
        sub_items << sitem;
    }

    if (st.m_finish_flags & zrm::stage_end_delta_voltage)
    {
        QTreeWidgetItem * sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по ΔU"));
        sitem->setText(STAGE_FINISH_COLUMN, tr("%1 В").arg(QString::number(st.end_delta_volt(m_current_method.m_method), 'f', 2)));
        sub_items << sitem;
    }

    if (st.m_finish_flags & zrm::stage_end_cell_voltage)
    {
        QTreeWidgetItem * sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по U-элемента"));
        sitem->setText(STAGE_FINISH_COLUMN, tr("%1 В").arg(QString::number(st.end_cell_volt(), 'f', 2)));
        sub_items << sitem;
    }

    if (st.m_finish_flags & zrm::stage_end_temper)
    {
        QTreeWidgetItem * sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение T"));
        sitem->setText(STAGE_FINISH_COLUMN, tr("%1 C").arg(QString::number(st.end_temp(), 'f', 2)));
        sub_items << sitem;
    }

    if(st.m_finish_flags & zrm::stage_end_capacity)
    {
        QTreeWidgetItem * sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по емкости"));
        sitem->setText(STAGE_FINISH_COLUMN, tr("%1 А*Ч").arg(QString::number(st.end_capacity(m_current_method.m_method),'f',2 )));
        sub_items << sitem;
    }

    if(sub_items.count())
        stage_item->addChildren(sub_items);
}

void ZrmStagesEditor::stage_type_changed(zrm::stage_type_t st_type)
{
    switch(st_type)
    {
     case zrm::STT_CHARGE    : tbCharge->setChecked(true);break;
     case zrm::STT_DISCHARGE : tbDischarge->setChecked(true);break;
     case zrm::STT_IMPULSE   : tbImpulse->setChecked(true);break;
     default : tbPause->setChecked(true);
    }

    edChargeVoltage->setEnabled(st_type & zrm::STT_CHARGE);
    edChargeCurrent->setEnabled(st_type & zrm::STT_CHARGE);
    edDischargeVoltage->setEnabled(st_type & zrm::STT_DISCHARGE);
    edDischargeCurrent->setEnabled(st_type & zrm::STT_DISCHARGE);
    sbChargeTime->setEnabled(st_type == zrm::STT_IMPULSE);
    sbDischargeTime->setEnabled(st_type == zrm::STT_IMPULSE);
}


void ZrmStagesEditor::sl_method_name_changed(const QString & str)
{
    Q_UNUSED(str)
    emit method_changed(method_name_changed);
}

void ZrmStagesEditor::sl_stage_data_changed(QTreeWidgetItem * item,int column)
{
    if(item &&  column == stage_descr_column)
        on_stages_changed();
}

void ZrmStagesEditor::sl_stage_changed(QTreeWidgetItem * current, QTreeWidgetItem * prev)
{
    Q_UNUSED(prev)
    enabled_controls();
    if(!current)
        return;

    ChildrenSignalBlocker<QWidget> sb(stages_box);

    int idx = currentStageItemIndex(current);
    int delta = stages_list->topLevelItemCount() - idx;
    tbStageMoveUp->setEnabled(idx > 0);
    tbStageMoveDown->setEnabled(delta > 1);

    auto st = m_current_method.m_stages.at(size_t(idx));
    stage_type_changed(zrm::stage_type_t(st.m_type));

    int zero_value = 0.0;
    if( st.m_type & zrm::STT_CHARGE )
    {
        double value;
        value = st.charge_volt(m_current_method.m_method);
        edChargeVoltage->setValue(value);
        value = st.charge_curr(m_current_method.m_method);
        edChargeCurrent->setValue(value);
        sbChargeTime->setValue(st.m_char_time );
    }
    else
    {
        edChargeVoltage->setValue(zero_value);
        edChargeCurrent->setValue(zero_value);
        sbChargeTime->setValue(0);
    }

    if( st.m_type & zrm::STT_DISCHARGE )
    {
        edDischargeVoltage->setValue(st.discharge_volt(m_current_method.m_method));
        edDischargeCurrent->setValue(st.discharge_curr(m_current_method.m_method));
        sbDischargeTime->setValue(st.m_dis_time );
    }
    else
    {
        edDischargeVoltage->setValue(zero_value);
        edDischargeCurrent->setValue(zero_value);
        sbDischargeTime->setValue(0);
    }

    cbFinishTime->setChecked(st.m_finish_flags & zrm::stage_end_time);
    if(st.m_finish_flags & zrm::stage_end_time)
    {
        sbFinishHour->setValue(st.m_hours);
        sbFinishMinuts->setValue(st.m_minutes);
        sbFinishSecunds->setValue(st.m_secs);
        sbFinishHour->setEnabled(true);
        sbFinishMinuts->setEnabled(true);
        sbFinishSecunds->setEnabled(true);
    }
    else
    {
        sbFinishHour->setValue(0);
        sbFinishMinuts->setValue(0);
        sbFinishSecunds->setValue(0);
        sbFinishHour->setEnabled(false);
        sbFinishMinuts->setEnabled(false);
        sbFinishSecunds->setEnabled(false);
    }

    cbFinishCurrent->setChecked(st.m_finish_flags & zrm::stage_end_current);
    if(st.m_finish_flags & zrm::stage_end_current)
    {
        sbFinishCurrent->setValue(st.end_curr(m_current_method.m_method));
        sbFinishCurrent->setEnabled(true);
    }
    else
    {
        sbFinishCurrent->setValue(.0);
        sbFinishCurrent->setEnabled(false);
    }

    cbFinishVoltage->setChecked(st.m_finish_flags & zrm::stage_end_voltage);
    if(st.m_finish_flags & zrm::stage_end_voltage)
    {
        sbFinishVoltage->setValue(st.end_volt(m_current_method.m_method));
        sbFinishVoltage->setEnabled(true);
    }
    else
    {
        sbFinishVoltage->setValue(.0);
        sbFinishVoltage->setEnabled(false);
    }

    cbFinishCapacity->setChecked(st.m_finish_flags & zrm::stage_end_capacity);
    if(st.m_finish_flags & zrm::stage_end_capacity)
    {
        sbFinishCapacity->setValue(st.end_capacity(m_current_method.m_method));
        sbFinishCapacity->setEnabled(true);
    }
    else
    {
        sbFinishCapacity->setValue(0);
        sbFinishCapacity->setEnabled(false);
    }

    cbFinishCellVoltage->setChecked(st.m_finish_flags & zrm::stage_end_cell_voltage);
    if(st.m_finish_flags & zrm::stage_end_cell_voltage)
    {
        sbFinishCellVoltage->setValue(st.end_cell_volt());
        sbFinishCellVoltage->setEnabled(true);
    }
    else
    {
        sbFinishCellVoltage->setValue(0);
        sbFinishCellVoltage->setEnabled(false);
    }

    cbFinishDeltaVoltage->setChecked(st.m_finish_flags & zrm::stage_end_delta_voltage);
    if(st.m_finish_flags & zrm::stage_end_delta_voltage)
    {
        sbFinishDeltaVoltage->setValue(st.end_delta_volt(m_current_method.m_method));
        sbFinishDeltaVoltage->setEnabled(true);
    }
    else
    {
        sbFinishDeltaVoltage->setValue(0);
        sbFinishDeltaVoltage->setEnabled(false);
    }

    cbFinishTemperature->setChecked(st.m_finish_flags & zrm::stage_end_temper);
    if(st.m_finish_flags & zrm::stage_end_temper)
    {
        sbFinishTemperature->setValue(st.end_temp());
        sbFinishTemperature->setEnabled(true);
    }
    else
    {
        sbFinishTemperature->setValue(0);
        sbFinishTemperature->setEnabled(false);
    }

    cbCapMesure->setChecked(st.m_stage_flags & zrm::stage_flags_t::STFL_CAPACITY_MEASHURE);
    cbCheckCondition->setChecked( st.m_stage_flags & zrm::stage_flags_t::STFL_CONDITION_CHECK);
}

zrm::stage_t & ZrmStagesEditor::current_stage( int * idx)
{
  int stage_idx = currentStageItemIndex();
  if(idx) *idx = stage_idx;
  if(stage_idx > -1)
  {
      return m_current_method.m_stages.at(zrm::stages_t::size_type(stage_idx));
  }
  return  m_fake_stage;
}

QTreeWidgetItem* ZrmStagesEditor::currentStageItem(QTreeWidgetItem* item)
{
    QTreeWidgetItem* res = item;
    if (!res)
        res = stages_list->currentItem();
    if (res)
    {
        QTreeWidgetItem* p = res->parent();
        if (p)
            res = p;
    }
    return res;
}

int ZrmStagesEditor::currentStageItemIndex(QTreeWidgetItem *item)
{
    int res = -1;
    QTreeWidgetItem* i = currentStageItem(item);
    if (i)
        res = stages_list->indexOfTopLevelItem(i);
    return res;
}

void ZrmStagesEditor::sl_stage_flags_changed ()
{
 //Изменились флаги этапа
    zrm::stage_t & st = current_stage();
    QObject * src = sender();

    auto  cb = dynamic_cast<QAbstractButton*>(src);

    if(cb)
      {
        uint8_t tag = uint8_t(cb->property(STAGE_CTRL_TAG).toUInt());
        if(cb->isChecked())
          st.m_stage_flags |= tag;
          else
          st.m_stage_flags &= ~tag;
       on_stages_changed();
      }
}

void ZrmStagesEditor::sl_stage_finish_flags_changed ()
{
    //изменились флаги завершения
    zrm::stage_t & st = current_stage();
    QObject * src = sender();
    auto cb = dynamic_cast<QAbstractButton*>(src);
    if (cb)
    {
        uint8_t tag = uint8_t(cb->property(STAGE_CTRL_TAG).toUInt());
        if (cb->isChecked())
            st.m_finish_flags |= tag;
        else
            st.m_finish_flags &= ~tag;
        sl_stage_changed(currentStageItem(), nullptr);
        on_stages_changed();
    }
}

void ZrmStagesEditor::sl_stage_type_changed()
{
  QObject * src = sender();
  if(src)
  {
    int idx = -1;
    zrm::stage_t & st = current_stage(&idx);
    st.m_type = uint8_t(src->property(STAGE_CTRL_TAG).toUInt());
    if(idx>-1)
    {
      stages_list->topLevelItem(idx)->setText(stage_type_column , tr(zrm::stage_t::stage_type_name(m_methods_tree->open_as(),zrm::stage_type_t(st.m_type))));
      on_stages_changed();
    }

  }
}


void ZrmStagesEditor::sl_stage_charge_changed     ()
{
  QObject * src = sender();
  zrm::stage_t & st = current_stage();
  if(src == edChargeVoltage)
      st.set_charge_volt(edChargeVoltage->value(), m_current_method.m_method);
  if(src == edChargeCurrent)
      st.set_charge_curr(edChargeCurrent->value(), m_current_method.m_method);
  on_stages_changed();
}

void ZrmStagesEditor::sl_stage_discharge_changed  ()
{
  QObject * src = sender();
  zrm::stage_t & st = current_stage();
  if(src == edDischargeVoltage)
     st.set_discharge_volt(edDischargeVoltage->value(), m_current_method.m_method);
  if(src == edDischargeCurrent)
     st.set_discharge_curr(edDischargeCurrent->value(), m_current_method.m_method);
  on_stages_changed();
}

void ZrmStagesEditor::sl_stage_impule_time_changed()
{
  QObject * src = sender();
  zrm::stage_t & st = current_stage();
  if(src == sbChargeTime)
     st.m_char_time = uint8_t(sbChargeTime->value());
  if(src == sbDischargeTime)
     st.m_dis_time = uint8_t(sbDischargeTime->value());
  on_stages_changed();

}


void ZrmStagesEditor::sl_stage_finish_changed()
{
  zrm::stage_t & st = current_stage();
  QObject * src = sender();

  if(src == sbFinishHour   )  st.m_hours   = uint8_t(sbFinishHour   ->value());
  if(src == sbFinishMinuts )  st.m_minutes = uint8_t(sbFinishMinuts ->value());
  if(src == sbFinishSecunds)  st.m_secs    = uint8_t(sbFinishSecunds->value());

  if(src == sbFinishVoltage     )  st.set_end_volt(sbFinishVoltage->value(), m_current_method.m_method);
  if(src == sbFinishCurrent     )  st.set_end_curr(sbFinishCurrent->value(), m_current_method.m_method);
  if(src == sbFinishTemperature )  st.set_end_temp(sbFinishTemperature->value());

  if(src == sbFinishDeltaVoltage)  st.set_end_delta_volt(sbFinishDeltaVoltage->value(), m_current_method.m_method);
  if(src == sbFinishCapacity    )  st.set_end_capacity  (sbFinishCapacity    ->value(), m_current_method.m_method);
  if(src == sbFinishCellVoltage )  st.set_end_cell_volt(sbFinishCellVoltage  ->value());

  on_stages_changed();
}


void  ZrmStagesEditor::renumber_stages()
{
  int num = 0;
  for( zrm::stage_t & st : m_current_method.m_stages)
  {
      QTreeWidgetItem * item = stages_list->topLevelItem(num++);
      item->setText(stage_number_column, QString::number(num));
      st.m_number = uint8_t(num);
  }
}

void ZrmStagesEditor::sl_stage_remove ()
{
  //Удаление этапа
  int idx = -1;
  zrm::stage_t & st = current_stage(&idx);
  if(idx>-1)
  {
    zrm::stages_t::iterator ptr = m_current_method.m_stages.begin();
    if(idx)
       std::advance(ptr, idx);
    if(st.m_id_method) //Не новый добавляем в список для удаления
       m_removed_stages.push_back(st);
    m_current_method.m_stages.erase(ptr);
    QTreeWidgetItem * item = currentStageItem();
    QSignalBlocker sb(stages_list);
    delete item;
    renumber_stages  ();
    idx = qMin(idx, stages_list->topLevelItemCount()-1);
    QTreeWidgetItem * next_item = stages_list->topLevelItem(idx);
    stages_list->setCurrentItem(next_item);
    sl_stage_changed(next_item, Q_NULLPTR);
    on_stages_changed();
  }
}

void ZrmStagesEditor::sl_stage_add    ()
{
  //Добавление этапа
  zrm::stage_t st;
  st.m_type = zrm::STT_PAUSE;
  st.m_number =  uint8_t(m_current_method.m_stages.size()+1);
  m_current_method.m_stages.push_back(st);
  QTreeWidgetItem * stage_item = ZrmMethodsTree::new_tree_item( QString(),ZrmMethodsTree::table_stages,st.m_id_method,false );

  stage_assign(stage_item, st,QString());
  stages_list->addTopLevelItem(stage_item);
  stages_list->setCurrentItem (stage_item);
  on_stages_changed();
}

void ZrmStagesEditor::sl_stage_move()
{
    QObject * src = sender();
    if (src == tbStageMoveUp || src == tbStageMoveDown)
    {
        QTreeWidgetItem * curr_item = currentStageItem();
        int curr_idx  = currentStageItemIndex(curr_item);
        int next_idx  = curr_idx + (src == tbStageMoveDown ? 1 : -1);
        QTreeWidgetItem * next_item = stages_list->topLevelItem(next_idx);
        zrm::stage_t & curr_st = m_current_method.m_stages.at(size_t(curr_idx));
        zrm::stage_t & next_st = m_current_method.m_stages.at(size_t(next_idx));

        QString curr_descr = next_item->text(stage_descr_column);
        QString next_descr = curr_item->text(stage_descr_column);

        std::swap(curr_st.m_number, next_st.m_number);
        std::swap(curr_st, next_st);
        stage_assign(curr_item, curr_st, curr_descr);
        stage_assign(next_item, next_st, next_descr);
        stages_list->setCurrentItem(next_item);
    }
}

bool ZrmStagesEditor::do_write_stages()
{
 QSqlDatabase & db = methods_tree()->database();
 bool ret = ZrmDatabase::start_transaction(db);
 if(ret)
 {
  ret = ZrmDatabase::write_stages(db, m_method_id, m_current_method.m_stages, m_removed_stages);
  int i = 0;
  for(const zrm::stage_t & st : m_current_method.m_stages)
  {
      ZrmDatabase::write_stage_descript(db, st.m_id_method, stages_list->topLevelItem(i++)->text(stage_descr_column));
  }
 }
 return ret;
}

bool ZrmStagesEditor::write_method(bool wr_method, bool wr_stages)
{
    //Запись метода в базу
    QSqlDatabase & db = methods_tree()->database();
    bool ret = ZrmDatabase::start_transaction(db);

    if (ret & wr_method )
    {
        ret = ZrmDatabase::wrte_method(db, m_method_id, ed_method_name->text(), m_current_method);
        ret &= ZrmDatabase::wrte_method_uservals(db, m_method_id, edVoltage->value(), edCapacity->value(), false);
    }

    if (ret & wr_stages)
        ret = do_write_stages();
    if (ret)
        ret = ZrmDatabase::commit_transaction(db, true);
    else
        ZrmDatabase::rollback_transaction(db);

    return ret;
}

QList<int> ZrmStagesEditor::getSplitterSizes()
{
    return  splitterSizes;
}

void ZrmStagesEditor::setSplitterSizes(const QList<int> &list)
{
    if (list.size() < 2)
        return;
//    splitter->setSizes(QList<int>() << list[0] << list[1]);

//    splitterSizes.clear();
//    splitterSizes << list[0] << list[1];
}
