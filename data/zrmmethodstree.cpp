// Method database editor

#include "zrmmethodstree.h"

#include <QSpinBox>
#include <QLineEdit>
#include <QHeaderView>
/**
 * @brief The item_delegate class
 * Делегат для определия, что можно редактировать
 * Для разрешения редактирования необходимо у QTreeWidgetItem * item->setData( column_to_edit, role_edit_data, 1)
 */

class mtree_item_delegate : public QItemDelegate
{
    public:
        mtree_item_delegate(ZrmMethodsTree * mtree):QItemDelegate(mtree),m_methods_tree(mtree){}
        QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
        virtual void destroyEditor(QWidget *editor, const QModelIndex &index) const override;
        void setEditorData(QWidget *editor, const QModelIndex &index) const override;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    private slots:
        void slotValueChanged();

    private:
        ZrmMethodsTree  * m_methods_tree = Q_NULLPTR;
};

void mtree_item_delegate::slotValueChanged()
{
    QWidget *isEditor = qobject_cast<QWidget *>(sender());
    if (isEditor)
        emit commitData(isEditor);
}

QWidget *mtree_item_delegate::createEditor(QWidget *parent,
                      const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
 if(m_methods_tree && m_methods_tree->item_edit_enable( index ) )
      {
        int column = index.column();
        if(column == ZrmMethodsTree::column_voltage || column == ZrmMethodsTree::column_capacity)
           {
            auto sb = new QDoubleSpinBox(parent);
            //sb->setLocale(m_methods_tree->parentWidget()->parentWidget()->locale());
            sb->setSingleStep(zrm::method_t::value_step());
            sb->setDecimals(1);
            sb->setRange(0,column == ZrmMethodsTree::column_voltage ? zrm::method_t::max_voltage() : zrm::method_t::max_capacity());
            //sb->setSpecialValueText("--");

            QObject::connect(sb, SIGNAL(valueChanged(const QString &)), m_methods_tree, SLOT(volt_cap_changed(const QString &)));
            sb->setLocale(QLocale::C);
            return sb;
           }
        QWidget* editor = QItemDelegate::createEditor(parent, option, index);
        QLineEdit *le = qobject_cast<QLineEdit *>(editor);
        if (le)
            connect(le, &QLineEdit::textChanged, this, &mtree_item_delegate::slotValueChanged);
        return editor;
      }
  return Q_NULLPTR;
}

void mtree_item_delegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    disconnect(editor);
    QItemDelegate::destroyEditor(editor, index);
}

void mtree_item_delegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int column = index.column();
    if(column)
    {
      auto value = index.data().toString();
      m_methods_tree->number_string(value,true);
//      qDebug()<<Q_FUNC_INFO<<"sender "<<sender()<<" qspinbox value "<<value;
      QDoubleSpinBox * sb = dynamic_cast<QDoubleSpinBox*>(editor);
      if(sb)
      {
        sb->setValue(value.toDouble());
      }
    }
    else
     QItemDelegate::setEditorData(editor, index);
}

void mtree_item_delegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    int column = index.column();
    if(column)
    {
      QDoubleSpinBox * sb = dynamic_cast<QDoubleSpinBox*>(editor);
      if(sb)
         {
           double value = sb->value();
         //qDebug()<<Q_FUNC_INFO<<"qspinbox value "<<value;
           model->setData(index,value);
         }
    }
    else
       QItemDelegate::setModelData(editor, model, index);

}

ZrmMethodsTree::ZrmMethodsTree(QWidget *parent) :
    QTreeWidget(parent)
{
    setColumnCount(3);
    setHeaderLabels(QStringList() << "Наименование" << "Напряжение" << "Ёмкость");
    setAlternatingRowColors(true);
    setSelectionBehavior(SelectItems);
    setSortingEnabled(true);
    sortItems(0, Qt::AscendingOrder);
    header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    header()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
    setItemDelegate(create_delegate());
    connect(this, &QTreeWidget::itemExpanded      , this, &ZrmMethodsTree::slot_item_expanded );
    connect(this, &QTreeWidget::itemCollapsed     , this, &ZrmMethodsTree::slot_item_collapsed);
    connect(this, &QTreeWidget::currentItemChanged, this, &ZrmMethodsTree::current_item_changed);
    connect(this, &QTreeWidget::itemChanged       , this, &ZrmMethodsTree::item_changed);
    connect(this, &QTreeWidget::currentItemChanged, this, &ZrmMethodsTree::onCurrentItemChanged);
    connect(this, &QTreeWidget::itemChanged       , this, &ZrmMethodsTree::onItemChanged);
    setFont(font());
    setStyleSheet("selection-background-color: steelblue;");
}

ZrmMethodsTree::~ZrmMethodsTree()
{
  close_database();
}



QItemDelegate * ZrmMethodsTree::create_delegate()
{
 return new mtree_item_delegate(this);
}

bool            ZrmMethodsTree::item_edit_enable(const QModelIndex &index)
{

 QVariant v  = index.data(role_edit_enable);
 //qDebug()<<QString("%1  edit_enable %2").arg(index.data().toString()).arg(v.toInt());
 return v.toInt();
}

bool ZrmMethodsTree::setAbstract(bool abstract)
{
  if(isAbstract() != abstract)
  {
     if( isOpen())
         return open_database(getWorkMode(), abstract);
     else
         m_abstract_methods = abstract;

  }
  return true;
}


bool ZrmMethodsTree::setWorkMode(zrm::zrm_work_mode_t wm)
{
   if(getWorkMode() != wm)
   {
     if(isOpen())
         return open_database(wm, this->isAbstract());
     else
         m_work_mode = wm;
   }
   return true;
}


void ZrmMethodsTree::close_database()
{
    save_user_values();
    db.close();
    QSignalBlocker sb(this);
    clear();
}

bool ZrmMethodsTree::open_database(zrm::zrm_work_mode_t work_mode, bool _abstract_methods)
{
    if (!db.isOpen() || m_work_mode != work_mode  || m_abstract_methods != _abstract_methods)
    {
        close_database();
        headerItem()->setText(column_capacity, work_mode ? tr("Ёмкость") : tr("Ток"));
        m_work_mode = work_mode ;
        m_abstract_methods = _abstract_methods;
        db = ZrmDataSource::method_database(work_mode);
        headerItem()->setText(2, work_mode ? tr("Ёмкость") : tr("Ток"));
        if (db.isOpen())
            fill_tree();
        else
            qDebug() << "Error open database " << db.lastError().text();
    }
    if (db.isOpen() && !topLevelItemCount())
        fill_tree();
    emit database_open(db.isOpen());
    return db.isOpen();
}

bool ZrmMethodsTree::open_database()
{
  return open_database(m_work_mode, m_abstract_methods);
}

void ZrmMethodsTree::save_user_values()
{
    if (db.isOpen() && this->m_abstract_methods)
    {
        bool ret = true;
        QSignalBlocker sb(this);
        for (int i = 0; ret && i < topLevelItemCount(); i++)
        {
            QTreeWidgetItem * item = topLevelItem(i);
            auto it = item_table(item);
            QVariant uc = item->data(column_name, role_user_changed);

            if (it == table_method && uc.toInt())
            {
                //qDebug() << item->text(column_name) << " table " << it << " uc " << uc;
                ret &= ZrmDatabase::wrte_method_uservals(db ,item_id(item), item->data(column_name, role_user_voltage ), item->data(column_name, role_user_capacity ), false);
                if (ret)
                    item->setData(column_name, role_user_changed, 0);
            }
        }
        if (ret)
            ZrmDatabase::commit_transaction(db, true);
    }
}

void      ZrmMethodsTree::remove_children   (QTreeWidgetItem * parent, bool one_retain)
{
  QSignalBlocker sb(parent->treeWidget());
  auto list = parent->takeChildren();
  int count = 0;
  for(auto&& item : list  )
  {
    if(one_retain && ++count>= list.size())
        {
          parent->addChild(item);
        }
        else
        {
         delete item;
        }
  }
}

QTreeWidgetItem * ZrmMethodsTree::new_tree_item(const QString & text, const unsigned table_type , const QVariant & id,bool prepare_expandable)
{
  auto item = new QTreeWidgetItem;
  item->setData(0, role_table , table_type);
  item->setData(0, role_id    , id );
  item->setText(0,text);
  if (table_type == ZrmMethodsTree::table_method)
      item->setFlags(item->flags() & ~Qt::ItemFlag::ItemIsEditable);
  else
      item->setFlags(item->flags() | Qt::ItemFlag::ItemIsEditable);
  if(prepare_expandable)
  {
     new QTreeWidgetItem(item);
  }
  return item;
}

void ZrmMethodsTree::fill_tree()
{
    clear();
    if (m_abstract_methods)
        read_abstract_methods();
    else
        read_types();
}

QString   ZrmMethodsTree::get_stage_desctipt(unsigned stage_id)
{
  QString  result;
  QSqlQuery query(db)   ;
  QString qtext = "select st.st_descript from t_stage st where st.id = :stage_id";
  query_args_t args;
  args[":stage_id"] = stage_id;
  if(ZrmDatabase::exec_query(query, qtext,args))
  {

  }
  else
  {
      m_last_error = query.lastError();
      qDebug()<<Q_FUNC_INFO<<m_last_error.text();
  }

  return  result;
}

bool ZrmMethodsTree::read_type         (QTreeWidgetItem * item)
{
 bool ret = false;
 QSqlQuery query(db)   ;
 QString qtext = "select id, c_name from t_akb_type where id = :id";
 query_args_t args;
 args[":id"] = item->data(column_name,role_id);
 if(ZrmDatabase::exec_query(query,qtext,args) && ZrmDatabase::skip_empty(query))
 {
    QSqlRecord rec = query.record();
    QVariant v1 = rec.value(1);
    if(v1.isValid() &&  !v1.isNull())
    {
     item->setText(column_name, v1.toString());
     ret = true;
    }
 }
 return ret;
}

void ZrmMethodsTree::read_types()
{
    QSqlQuery query(db);
    QString qtext = "select id, c_name from t_akb_type order by c_name";
    if (ZrmDatabase::exec_query(query, qtext))
    {
        QList<QTreeWidgetItem*> items;

        auto f = [&items](QSqlRecord & rec)
        {
            if (!rec.isNull(0))
            {
                auto item = new_tree_item(rec.value(1).toString(), table_types, rec.value(0), true);
                items.push_back(item);
            }
        };

        ZrmDatabase::fetch_records(query, f);
        addTopLevelItems(items);
    }
    else
    {
        m_last_error = query.lastError();
        qDebug() << Q_FUNC_INFO << m_last_error.text();
    }
}

bool ZrmMethodsTree::read_model(QTreeWidgetItem * item)
{
    bool ret = false;
    QString qtext = "select  id,c_name,n_voltage, n_capacity from t_model  where t_model.id = :id  ";
    QSqlQuery query ( db );
    query_args_t args;
    args[":id"] = item->data(0,role_id);
    if(ZrmDatabase::exec_query(query,qtext,args))
    {
        if(ZrmDatabase::skip_empty(query))
         {
            QSqlRecord rec = query.record();
            item->setText(column_name, rec.value(1).toString());
            item->setData(column_name, role_voltage , rec.value(2));
            item->setData(column_name, role_capacity, rec.value(3));
            QString text;
            text = number(item->data(0, role_voltage ).toDouble(),1);

            item->setText(column_voltage , text);
            text = number(item->data(0, role_capacity).toDouble(),1);
            item->setText(column_capacity, text);
            ret = true;
         }
    }
  return ret;
}

/**
 * @brief ZrmMethodsThree::read_models read all Accumulators battery models for types
 */

void ZrmMethodsTree::read_models(QTreeWidgetItem * parent_item)
{

  if(parent_item)
  {
  remove_children(parent_item,false);
  QString qtext = "select id from t_model  where t_model.id_type = :akb_type  order by c_name";
  QSqlQuery query ( db );
  query_args_t args;
  args[":akb_type"] = parent_item->data(0,role_id);

  if(ZrmDatabase::exec_query(query,qtext,args))
    {
      QList<QTreeWidgetItem*> items;
      auto f = [this,&items](QSqlRecord & rec)
      {
        if(!rec.isNull(0))
         {
            QVariant id = rec.value(0);
            auto item = new_tree_item(QString(),table_models , id, true);
            if(read_model(item))
               items.push_back(item);
              else
               delete item;
         }
      };
      ZrmDatabase::fetch_records(query, f);
      parent_item->addChildren(items);
    }
      else
      {
          m_last_error = query.lastError();
          qDebug()<<Q_FUNC_INFO<<m_last_error.text();
      }

  }
}

bool ZrmMethodsTree::read_method(QTreeWidgetItem * item)
{
 bool ret = false;
 if(item)
  {
    QSqlQuery query = ZrmDatabase::read_method(db,item_id(item));
    if(ZrmDatabase::skip_empty(query))  //ZrmDatabase::exec_query(query, qtext, args) && ZrmDatabase::skip_empty(query) )
    {
      QSqlRecord rec = query.record();
      if(!rec.isEmpty())
      {
        item->setText( column_name, rec.value(    1   ).toString ( ) );
        item->setData( column_name, role_volt_rate    , rec.value(2) );
        item->setData( column_name, role_curr_rate    , rec.value(3) );
        item->setData( column_name, role_duration     , rec.value(4) );
        item->setData( column_name, role_cycle_count  , rec.value(5) );
        item->setData( column_name, role_user_voltage , rec.value(6) );
        item->setData( column_name, role_user_capacity, rec.value(7) );
        ret  = true ;
      }
    }
    else
    {
      m_last_error = query.lastError();
      qDebug()<<Q_FUNC_INFO<<m_last_error.text();
    }
  }
  return ret;
}

bool ZrmMethodsTree::read_method_abstract(QTreeWidgetItem *item)
{
    bool ret = false;
    if (item)
    {
        blockSignals(true);
        QSqlQuery query = ZrmDatabase::read_method(db, item_id(item));
        if(ZrmDatabase::skip_empty(query))  //ZrmDatabase::exec_query(query, qtext, args) && ZrmDatabase::skip_empty(query) )
        {
            QSqlRecord rec = query.record();
            if(!rec.isEmpty())
            {
                item->setText( column_name, rec.value(    1   ).toString ( ) );
                item->setData( column_name, role_volt_rate    , rec.value(2) );
                item->setData( column_name, role_curr_rate    , rec.value(3) );
                item->setData( column_name, role_duration     , rec.value(4) );
                item->setData( column_name, role_cycle_count  , rec.value(5) );
                item->setData( column_name, role_user_voltage , rec.value(6) );
                item->setData( column_name, role_user_capacity, rec.value(7) );

                item->setData(column_voltage , role_edit_enable, 1);
                item->setData(column_capacity, role_edit_enable, 1);
                if (m_editable)
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                item->setText(column_voltage, QString::number(rec.value(rec.isNull(6) ? 2 : 6).toDouble(), 'f', 1));
                item->setText(column_capacity, QString::number(rec.value(rec.isNull(7) ? 3 : 7).toDouble(), 'f', 1));
                ret  = true ;
            }
            blockSignals(false);
        }
        else
        {
            m_last_error = query.lastError();
            qDebug() << Q_FUNC_INFO << m_last_error.text();
        }
    }
    return ret;
}

void ZrmMethodsTree::read_methods(QTreeWidgetItem * model_item)
{

 if(model_item)
 {
   remove_children(model_item,false);
   QSqlQuery query = ZrmDatabase::read_model_methods(db, item_id(model_item));
     if(ZrmDatabase::skip_empty(query))
      {
         QList<QTreeWidgetItem*> items;
         auto f = [this, &items](QSqlRecord & rec)
         {
             if(!rec.isNull(0) )
             {
              auto  item = new_tree_item(QString(), table_method, rec.value(0), m_method_expandable);
              if(read_method(item))
                {
                  items.push_back(item);
                }
              else
                 delete item;

             }
         };
         ZrmDatabase::fetch_records(query,f);
         model_item->addChildren(items);
       }
       else
       {
           m_last_error = query.lastError();
           qDebug()<<Q_FUNC_INFO<<m_last_error.text();
       }
  }
}

void ZrmMethodsTree::method_user_data_to_real(QTreeWidgetItem * item)
{
    if (item)
    {
        QVariant v;
        v = item->data(0, role_user_voltage);
        if (!qFuzzyIsNull(v.toDouble()))
            item->setText(column_voltage , QString::number(v.toDouble(), 'f', 1));
        v = item->data(0, role_user_capacity );
        if (!qFuzzyIsNull(v.toDouble()))
            item->setText(column_capacity, QString::number(v.toDouble(), 'f', 1));
    }
}

void ZrmMethodsTree::read_abstract_methods()
{
    QSqlQuery query = ZrmDatabase::read_all_methods(db);
    if (ZrmDatabase::skip_empty(query))
    {
        QList<QTreeWidgetItem*> items;
        auto f = [this, &items](QSqlRecord & rec)
        {
            if (!rec.isNull(0))
            {
                auto item = new_tree_item(rec.value(1).toString(), table_method, rec.value(0), m_method_expandable);

                item->setData(column_name, role_volt_rate    , rec.value(2));
                item->setData(column_name, role_curr_rate    , rec.value(3));
                item->setData(column_name, role_duration     , rec.value(4));
                item->setData(column_name, role_cycle_count  , rec.value(5));
                item->setData(column_name, role_user_voltage , rec.value(6));
                item->setData(column_name, role_user_capacity, rec.value(7));

                item->setData(column_voltage, role_edit_enable, 1);
                item->setData(column_capacity, role_edit_enable, 1);
                if (m_editable)
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                item->setText(column_voltage, QString::number(rec.value(2).toDouble(), 'f', 1));
                item->setText(column_capacity, QString::number(rec.value(3).toDouble(), 'f', 1));

                method_user_data_to_real(item);
                items.push_back(item);
            }
        };
        ZrmDatabase::fetch_records(query, f);
        addTopLevelItems(items);
    }
    else
    {
        m_last_error = query.lastError();
        qDebug() << Q_FUNC_INFO << m_last_error.text();
    }
}

/**
 * @brief ZrmMethodsThree::get_method_param
 * @param item
 * @param param
 * @return значение метода напряжение или емкость
 */

double ZrmMethodsTree::get_method_param  (const QTreeWidgetItem * item, column_type_t param)
{
  if(item)
  {
    QString str = item->text(param).trimmed();
    if(str.isEmpty())
       return get_method_param(item->parent() , param);
    else
       {
        number_string(str,true);
        return str.toDouble();
       }
  }
  return .0;
}


void      ZrmMethodsTree::set_method_param  (QTreeWidgetItem * item, column_type_t param, double value)
{
  if(item && (abstract_methods() || item_table(item) == table_models))
  {
    item->setText(param,number(value,1));
  }
}


bool ZrmMethodsTree::get_method(zrm::zrm_method_t & zrm_method, QTextCodec * codec, QString * model_name)
{
    QTreeWidgetItem * item = currentItem();
    return get_method(item, zrm_method, codec, model_name);
}

bool      ZrmMethodsTree::get_method(QTreeWidgetItem * item, zrm::zrm_method_t &zrm_method , QTextCodec *codec, QString * pmodel_name)
{
 zrm::method_t  method;
 if(item && item->data(column_name,role_table).toInt() == table_method)
 {

     QString model_name = item->parent() ? item->parent()->text(0) : QString();
     if(pmodel_name ) *pmodel_name =  model_name;


    double volt       = get_method_param(item , column_voltage );
    double capacity   = get_method_param(item , column_capacity);

    double volt_rate  = item->data( column_name, role_volt_rate ).toDouble();
    double curr_rate  = item->data( column_name, role_curr_rate ).toDouble();
    method.m_id       = uint16_t  ( item->data(column_name, role_id).toUInt() );

    method.set_current ( capacity * curr_rate );
    method.set_voltage ( volt * volt_rate     );
    method.set_capacity( capacity );
    method.m_cycles_count   =  uint8_t(item->data(column_name, role_cycle_count).toUInt());
    auto hms = zrm::method_t::secunds2hms(item->data(column_name, role_duration).toUInt());
    method.m_hours   = std::get<0>(hms);
    method.m_minutes = std::get<1>(hms);
    method.m_secs    = std::get<1>(hms);

    QString method_name = model_name.length() ?  QString("%1:%2").arg( model_name ).arg(item->text(column_name)) : item->text(column_name) ;

    QByteArray name_array;
    if( codec )
        name_array = codec->fromUnicode(method_name) ;
    else
        name_array = method_name.toLocal8Bit();

    memset(method.m_name, 0, sizeof(method.m_name));
    memcpy(method.m_name, name_array.constData(),qMin(sizeof(method.m_name), size_t(name_array.size())));

    zrm_method.m_method = method;
    zrm_method.m_method.m_stages = uint8_t( read_stages(item , zrm_method.m_stages) );
    return true;
 }
 else
 {
  zrm_method.m_method = method;
  zrm_method.m_stages.clear();
 }
 return false;
}


void      ZrmMethodsTree::read_stages(QTreeWidgetItem * item)
{
    Q_UNUSED (item)
    zrm::stages_t stages;
    remove_children(item, read_stages(item, stages));
    QList<QTreeWidgetItem *> child_list;

    for(auto st : stages)
    {
      auto st_item  = new_tree_item(QString("%1.%2").arg(st.m_number)
                                                    .arg(zrm::stage_t::stage_type_name(m_work_mode ? zrm::as_charger : zrm::as_power ,zrm::stage_type_t(st.m_type)))
                                    ,table_stages
                                    ,st.m_id_method
                                    ,false
                                    );
      child_list.append(st_item);
    }

    if(child_list.size())
       item->addChildren(child_list);
}

size_t ZrmMethodsTree::read_stages(zrm::stages_t & stages)
{
    return read_stages(currentItem(), stages);
}

/**
 * @brief ZrmMethodsThree::read_stages чтение этапов
 * @param item
 * @param stages
 * @return количество этапов
 */

size_t    ZrmMethodsTree::read_stages(QTreeWidgetItem * item, zrm::stages_t & stages)
{
 stages.clear();
 if(item && item->data(column_name, role_table).toInt() == table_method)
 {
  QString  qtext =
          "SELECT st.id,  "                     // 0
                " st.id_type,"                  // 1
                " st.n_pos,"                    // 2
                " st.n_ch_volt_rate,"           // 3
                " st.n_ch_cur_rate,"            // 4
                " st.n_dis_volt_rate,"          // 5
                " st.n_dis_cur_rate,"           // 6
                " st.n_ch_duration,"            // 7
                " st.n_dis_duration,"           // 8
                " st.n_finish_flags,"           // 9
                " st.n_finish_voltage_rate,"    // 10
                " st.n_finish_current_rate,"    // 11
                " st.n_finish_capacity_rate,"   // 12
                " st.n_finish_delta_volt_rate," // 13
                " st.n_finish_temper,"          // 14
                " st.n_finish_duration,"        // 15
                " st.n_finish_cell_volt,"       // 16
                " st.n_stage_flags,"            // 17
                " st.st_descript"               // 18
            " FROM t_stage st "
            " where st.id_method = :method "
            "order by st.n_pos"
          ;


  QSqlQuery query ( db );
  query_args_t args;
  QVariant method_id = item->data(0,role_id);
  args[":method"] = method_id;

  if(ZrmDatabase::exec_query(query, qtext, args))
    {

     auto f = [&stages,method_id](QSqlRecord & rec)
     {
       if(!rec.isNull(0) )
       {
        zrm::stage_t stage;
        stage.m_id_method = uint16_t (rec.value(0).toUInt() );//ID - этапа см. ZrmConnectivity::write_method;
        stage.m_type      = uint8_t  (rec.value(1).toUInt() );
        stage.m_number    = uint8_t  (rec.value(2).toUInt() );

        stage.set_charge_volt        (rec.value(3).toDouble(), 1.0);
        stage.set_charge_curr        (rec.value(4).toDouble(), 1.0);
        stage.set_discharge_volt     (rec.value(5).toDouble(), 1.0);
        stage.set_discharge_curr     (rec.value(6).toDouble(), 1.0);

        stage.m_char_time   = uint8_t(rec.value(7).toUInt());
        stage.m_dis_time    = uint8_t(rec.value(8).toUInt());
        stage.m_finish_flags    = uint8_t(rec.value(9).toUInt());

        stage.set_end_volt           (rec.value(10).toDouble(), 1.0);
        stage.set_end_curr           (rec.value(11).toDouble(), 1.0);
        stage.set_end_capacity       (rec.value(12).toDouble(), 1.0);
        stage.set_end_delta_volt     (rec.value(13).toDouble(), 1.0);
        stage.set_end_temp           (rec.value(14).toDouble());

        auto hms = zrm::method_t::secunds2hms(rec.value(15).toUInt());
        stage.m_hours   = std::get<0>(hms);
        stage.m_minutes = std::get<1>(hms);
        stage.m_secs    = std::get<2>(hms);

        stage.set_end_cell_volt      (rec.value(16).toDouble());

        stage.m_stage_flags = uint8_t(rec.value(17).toUInt());
        stages.push_back(stage);
       }
     };

     ZrmDatabase::fetch_records(query,f);
    }
  else
    {
      qDebug()<<query.lastError().text();
    }
 }
  return stages.size();
}

void ZrmMethodsTree::onItemChanged(QTreeWidgetItem *item, int column)
{
  if(column )
  {
      QString s = item->text(column).trimmed();
      if(!s.isEmpty() && qFuzzyIsNull(s.toDouble()))
      {
         item->setText(column, QString());
      }

      if(abstract_methods())
      {
        //Для абстрактных методов изменения значения напряжения и ёмкости
        //Записываются в базу
        QVariant v = s.toDouble();
        switch(column)
        {
         case column_voltage:
              item->setData( column_name, role_user_voltage , v );
              break;
         case column_capacity:
              item->setData( column_name, role_user_capacity , v );
              break;
         default:  break;
        }
        item->setData(column_name, role_user_changed,1);
      }
      emit method_selected(item);
   }
}


/**
 * @brief ZrmMethodsThree::method_valid check methot has voltage and capacity
 * @param item
 * @return true or false
 */
bool ZrmMethodsTree::method_valid(QTreeWidgetItem * item)
{
  return
  (
   item
   && !qFuzzyIsNull(get_method_param(item, column_voltage))
   && !qFuzzyIsNull(get_method_param(item, column_capacity))
  );
}


/**
 * @brief ZrmMethodsTree::search_method_by_id
 * @param item
 * @param method_id
 * @return method tree item if found
 */
QTreeWidgetItem * ZrmMethodsTree::search_method_by_id(QTreeWidgetItem * item, QVariant method_id)
{
    QTreeWidgetItem * result = nullptr;

    if (item)
    {
        result = (item->data(column_name, role_table).toInt() == table_models) ?
                    find_method_by_id<QTreeWidgetItem>(item, method_id.toUInt(), &QTreeWidgetItem::childCount, &QTreeWidgetItem::child)
                  : search_method_by_id(item->parent(), method_id);
    }
    else
    {
        if (m_abstract_methods)
            result = find_method_by_id<QTreeWidget>(this, method_id.toUInt(), &QTreeWidget::topLevelItemCount, &QTreeWidget::topLevelItem);
    }

    return result;
}

void ZrmMethodsTree::onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous)
    if (current && current->childCount())
        current->treeWidget()->expandItem(current);
    qApp->processEvents();
    emit method_selected(current && current->data(column_name, role_table).toInt() == table_method ? current : nullptr);
}

void ZrmMethodsTree::slot_item_expanded(QTreeWidgetItem *item)
{
    switch (item->data(0, role_table).toInt())
    {
     case table_types : read_models (item); break;
     case table_models: read_methods(item); break;
     case table_method: read_stages (item); break;
     default : break;
    }
}

void ZrmMethodsTree::slot_item_collapsed(QTreeWidgetItem *item)
{
  remove_children(item,true);
}


QTreeWidgetItem * ZrmMethodsTree::copy_tree_item(QTreeWidgetItem * src, QTreeWidgetItem * new_parent )
{
   QTreeWidgetItem * item = new QTreeWidgetItem(new_parent);
   item->setFlags(item->flags()|Qt::ItemFlag::ItemIsEditable);
   for( int role = ZrmMethodsTree::role_table  ; role <= ZrmMethodsTree::role_stage_type; role++)
        item->setData(ZrmMethodsTree::column_name, role  , src->data(ZrmMethodsTree::column_name, role));

   for( int column = ZrmMethodsTree::column_name; column <= ZrmMethodsTree::column_capacity; column++)
       item->setText(column, src->text(column));
 return item;
}

void ZrmMethodsTree::show_method_params(bool show)
{
    if (show)
    {
        showColumn(column_voltage);
        showColumn(column_capacity);
    }
    else
    {
        hideColumn(column_voltage);
        hideColumn(column_capacity);
    }
}

void ZrmMethodsTree::volt_cap_changed(const QString &val_text)
{
    //auto src = sender();
    auto item = currentItem();
//    qDebug() << Q_FUNC_INFO << src << val_text;
    QString ns = val_text;
//    number_string(ns, false);
    item->setText(currentColumn(), ns);
}

void ZrmMethodsTree::number_string(QString & str, bool to_dot)
{
    Q_UNUSED(str)
    Q_UNUSED(to_dot)
   /*QChar dot  = QLatin1Char('.');
   QChar coma = QLatin1Char(',');
   if(to_dot)
      str.replace(coma, dot);
      else
      {
       QChar dp   = locale().decimalPoint();
       str.replace( dp == dot ? coma : dot, dp );
      }*/
}

QModelIndex ZrmMethodsTree::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    if (QAbstractItemView::CursorAction::MoveNext == cursorAction)
    {
        // порядок смены элементов при редактировании
        QModelIndex index = currentIndex();
        if (!index.isValid())
            return index;
        int column = index.column();
        QModelIndex res;
        while (!res.isValid() || !res.data(role_edit_enable).toBool())
        {
            // перебираем элементы слева направо пока не найдем редактируемый
            column++;
            res = index.siblingAtColumn(column);
            // прошли строку до конца - возвращаемся в начало
            if (!res.isValid())
                column = -1;
            // прошли по кругу - стоп
            if (index.column() == column)
                return index;
        }
        return res;
    }
    return QTreeWidget::moveCursor(cursorAction, modifiers);
}
