#include "zrmmethodsthree.h"

/**
 * @brief The item_delegate class
 * Делегат для определия, что можно редактировать
 */
class item_delegate : public QItemDelegate
{
    public:
        item_delegate(ZrmMethodsThree * mtree):QItemDelegate(mtree),m_methods_tree(mtree){}
virtual QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    private:
        ZrmMethodsThree * m_methods_tree = Q_NULLPTR;
};

inline QWidget *item_delegate::createEditor(QWidget *parent,
                      const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
 if(m_methods_tree && m_methods_tree->item_edit_enable( index ) )
      return QItemDelegate::createEditor(parent, option, index);
  return Q_NULLPTR;
}


ZrmMethodsThree::ZrmMethodsThree(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
    auto header = tw_methods->header();
    header->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    header->setSectionResizeMode(0,QHeaderView::ResizeMode::Stretch);
    tw_methods->setItemDelegate( create_delegate() );
}


QItemDelegate * ZrmMethodsThree::create_delegate()
{
 return new item_delegate(this);
}

bool            ZrmMethodsThree::item_edit_enable(const QModelIndex &index)
{
 QVariant v  = index.data(role_edit_enable);
 if(v.toInt() == table_models_t && index.column() > 0 )
  {
    return true;
  }
 return false;
}




void     ZrmMethodsThree::close()
{
  tw_methods->clear();
  db.close();
}

bool     ZrmMethodsThree::open (bool as_charger, bool all_methods)
{
 if(!db.isOpen() || m_charger != as_charger  || m_abstract_methods != all_methods)
 {
   close();
   m_charger     = as_charger ;
   m_abstract_methods = all_methods;
   db = ZrmDataSource::database(as_charger);
   auto header = tw_methods->headerItem();
   header->setText(2, as_charger ? tr("Ёмкость") : tr("Мощьность"));
   if(db.isOpen())
   {
     read_methods();
   }
 }
 return db.isOpen();
}


QTreeWidgetItem * ZrmMethodsThree::new_tree_item(const QString & text, const int table_type , const int column )
{
  auto item = new QTreeWidgetItem;
  item->setData(column, role_table , table_type);
  item->setText(column,text);
  item->setFlags(item->flags()|Qt::ItemFlag::ItemIsEditable);
  return item;
}

void ZrmMethodsThree::read_methods()
{
   tw_methods->clear();
    if(m_abstract_methods)
       read_abstract_methods();
   else
       read_typed_methods();
}

void ZrmMethodsThree::read_typed_methods()
{

  QSqlQuery query(db)   ;
  QString qtext = "select id, c_name from t_akb_type order by c_name";
  if(ZrmDataSource::exec_query(query,qtext))
  {
   QList<QTreeWidgetItem*> items;

   auto f = [this,&items](QSqlRecord & rec)
   {
     if(!rec.isNull(0))
       {
        auto item = new_tree_item(rec.value(1).toString(), table_types_t, 0);
        item->setData(0,role_id, rec.value(0));
        items.push_back(item);
        read_models(item);
       }
   };

   ZrmDataSource::fetch_records(query, f);
   tw_methods->addTopLevelItems(items);
  }
  else
  {
      m_last_error = query.lastError();
      qDebug()<<m_last_error.text();
  }
}

/**
 * @brief ZrmMethodsThree::read_models read all Accumulators battery models for types
 */

void ZrmMethodsThree::read_models(QTreeWidgetItem * parent_item)
{

  if(parent_item)
  {
  QString qtext = "select id, c_name,n_voltage, n_capacity from t_model  where t_model.id_type = :akb_type  order by c_name";
  QSqlQuery query ( db );
  query_args_t args;
  args[":akb_type"] = parent_item->data(0,role_id);

  if(ZrmDataSource::exec_query(query,qtext,args))
    {
      QList<QTreeWidgetItem*> items;
      auto f = [this,&items](QSqlRecord & rec)
      {
        if(!rec.isNull(0))
          {
            auto item = new_tree_item(rec.value(1).toString(),table_models_t , 0);
            item->setData(0, role_id      , rec.value(0));
            item->setData(0, role_voltage , rec.value(2));
            item->setData(0, role_capacity, rec.value(3));
            item->setText(column_voltage , QString::number(item->data(0, role_voltage ).toDouble(),'f',1));
            item->setText(column_capacity, QString::number(item->data(0, role_capacity).toDouble(),'f',1));
            items.push_back(item);
            read_methods(item);
          }
      };
      ZrmDataSource::fetch_records(query, f);
      parent_item->addChildren(items);
    }
  }
}

void ZrmMethodsThree::read_methods(QTreeWidgetItem * model_item)
{
 QString qtext =
     "select m.id  , m.c_name, m.n_volt_rate, m.n_current_rate"
     ", m.n_duration, m.n_cycle_count "
     "from tl_model_method  tm "
     "left join t_method m on  tm.id_method = m.id "
     "where tm.id_model = :model";

 if(model_item)
 {
     QSqlQuery query ( db );
     query_args_t args;
     args[":model"] = model_item->data(0,role_id);
     if(ZrmDataSource::exec_query(query, qtext, args))
      {
         QList<QTreeWidgetItem*> items;
         auto f = [&items](QSqlRecord & rec)
         {
             if(!rec.isNull(0) )
             {
              auto  item = new_tree_item(rec.value(1).toString(), table_method_t,0);
              item->setData(0, role_id         , rec.value(0));
              item->setData(0, role_volt_rate  , rec.value(2));
              item->setData(0, role_curr_rate  , rec.value(3));
              item->setData(0, role_duration   , rec.value(4));
              item->setData(0, role_cycle_count, rec.value(5));
              items.push_back(item);
             }
         };
         ZrmDataSource::fetch_records(query,f);
         model_item->addChildren(items);
       }
  }
}




void ZrmMethodsThree::read_abstract_methods()
{
    QString qtext =
            " SELECT id, c_name, n_volt_rate, n_current_rate, n_duration, n_cycle_count "
            " FROM t_method  order by c_name ";
    QSqlQuery query ( db );
    if(ZrmDataSource::exec_query(query, qtext))
     {
        QList<QTreeWidgetItem*> items;
        auto f = [&items](QSqlRecord & rec)
        {
            if(!rec.isNull(0) )
            {
             auto  item = new_tree_item(rec.value(1).toString(), table_method_t,0);
             item->setData(0, role_id         , rec.value(0));
             item->setData(0, role_volt_rate  , rec.value(2));
             item->setData(0, role_curr_rate  , rec.value(3));
             item->setData(0, role_duration   , rec.value(4));
             item->setData(0, role_cycle_count, rec.value(5));
             item->setData(column_voltage , role_edit_enable, 1);
             item->setData(column_capacity, role_edit_enable, 1);
             items.push_back(item);
            }
        };
        ZrmDataSource::fetch_records(query,f);
        tw_methods->addTopLevelItems(items);
      }
    else
    {
     m_last_error = query.lastError();
    }
}


/**
 * @brief ZrmMethodsThree::get_method_param
 * @param item
 * @param param
 * @return значение метода напряжение или емкость
 */
double ZrmMethodsThree::get_method_param  (const QTreeWidgetItem * item, column_type_t param)
{
  if(item)
  {
    QString str = item->text(param).trimmed();
    if(str.isEmpty())
       return get_method_param(item->parent() , param);
    else
       return str.toDouble();

  }
  return .0;
}


bool      ZrmMethodsThree::get_method(zrm::zrm_method_t  & zrm_method, QTextCodec * codec)
{
 QTreeWidgetItem * item = tw_methods->currentItem();

 if(item && item->data(column_name,role_table).toInt() == table_method_t)
 {
    double volt       = get_method_param(item , column_voltage );
    double capacity   = get_method_param(item , column_capacity);
    double volt_rate  = item->data(column_name, role_volt_rate ).toDouble();
    double curr_rate  = item->data(column_name, role_curr_rate ).toDouble();

    zrm::method_t  method;

    method.m_id = uint16_t(item->data(column_name, role_id).toUInt());
    method.set_current ( capacity * curr_rate );
    method.set_voltage ( volt * volt_rate     );
    method.set_capacity( capacity );
    method.m_cicles   =  uint8_t(item->data(column_name, role_cycle_count).toUInt());
    auto hms = zrm::method_t::secunds2hms(item->data(column_name, role_duration).toUInt());
    method.m_hours   = std::get<0>(hms);
    method.m_minutes = std::get<1>(hms);
    method.m_secs    = std::get<1>(hms);

    if(codec)
    {
      auto method_name = codec->fromUnicode(item->text(column_name));
      memset(method.m_name, 0, sizeof(method.m_name));
      memcpy(method.m_name, method_name.constData(),std::min(sizeof(method.m_name), size_t(method_name.size())));
    }

    zrm_method.m_method = method;
    zrm_method.m_method.m_stages = uint8_t( read_stages(item , zrm_method.m_stages) );
    return true;
 }
 return false;
}


/**
 * @brief ZrmMethodsThree::read_stages чтение этапов
 * @param item
 * @param stages
 * @return количество этапов
 */

size_t    ZrmMethodsThree::read_stages(QTreeWidgetItem * item, zrm::stages_t & stages)
{
  stages.clear();
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
            " left join t_stage_type sn on sn.id = st.id_type "
            " where st.id_method = :method "
          ;


  QSqlQuery query ( db );
  query_args_t args;
  QVariant method_id = item->data(0,role_id);
  args[":method"] = method_id;
  qDebug()<<args;
  if(ZrmDataSource::exec_query(query, qtext, args))
    {

     auto f = [&stages,method_id](QSqlRecord & rec)
     {
       if(!rec.isNull(0) )
       {
        zrm::stage_t stage;
        stage.m_id_method = uint16_t ( method_id.toUInt()   );
        stage.m_type      = uint8_t  ( rec.value(1).toUInt());
        stage.m_number    = uint8_t  ( rec.value(2).toUInt());

        stage.set_charge_volt        (rec.value(3).toDouble()  , 1.0);
        stage.set_charge_curr        (rec.value(4).toDouble()  , 1.0);
        stage.set_discharge_volt     (rec.value(5).toDouble()  , 1.0);
        stage.set_discharge_curr     (rec.value(6).toDouble()  , 1.0);

        stage.m_char_time   = uint8_t(rec.value(7).toUInt());
        stage.m_dis_time    = uint8_t(rec.value(8).toUInt());
        stage.m_end_flag    = uint8_t(rec.value(9).toUInt());

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

     ZrmDataSource::fetch_records(query,f);
    }
  else
    {
      qDebug()<<query.lastError().text();
    }

  return stages.size();
}




void ZrmMethodsThree::on_tw_methods_itemChanged(QTreeWidgetItem *item, int column)
{
    if(column )
    {
      QString s = item->text(column).trimmed();
      if(!s.isEmpty() && qFuzzyIsNull(s.toDouble()))
          item->setText(column, QString());

    }
}


/**
 * @brief ZrmMethodsThree::method_valid check methot has voltage and capacity
 * @param item
 * @return true or false
 */
bool ZrmMethodsThree::method_valid(QTreeWidgetItem * item)
{
  return
  (
   item
   && !qFuzzyIsNull(get_method_param(item, column_voltage))
   && !qFuzzyIsNull(get_method_param(item, column_capacity))
  );

}

void ZrmMethodsThree::on_tw_methods_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous)
    emit method_selected( current && current->data(column_name, role_table).toInt() == table_method_t ? current : Q_NULLPTR );
}



