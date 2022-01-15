//Single active instance

#include "zrmreportdatabase.h"
#include "zrmdatasource.h"

ZrmReportDatabase * ZrmReportDatabase::main_instance = Q_NULLPTR;

ZrmReportDatabase::ZrmReportDatabase()
{

}

ZrmReportDatabase::~ZrmReportDatabase()
{
    delete_model( m_snumbers_model);
    if(main_instance == this)
    {
      delete_model(m_users_model);
      delete_model(m_types_model);
      if(m_reports_db)
      {
        m_reports_db->close();
        delete m_reports_db;
      }
    }
}


bool       ZrmReportDatabase::check_db_open()
{
  auto inst = get_main_instance();
  if(inst == this)
  {
    if(!m_reports_db)
    {
      m_reports_db = new QSqlDatabase(ZrmDataSource::reports_database());
    }

   if(!m_reports_db->isOpen())
        return m_reports_db->open();
   return m_reports_db->isOpen();
  }

 return inst->check_db_open();
}

QSqlDatabase   *      ZrmReportDatabase::database()
{
    auto inst = get_main_instance();
    if(inst == this)
    {
      return check_db_open() ? m_reports_db : Q_NULLPTR;
    }
   return inst->database();
}

ZrmReportDatabase *  ZrmReportDatabase::get_main_instance()
{
  if(!main_instance) main_instance = this;
  return main_instance;
}

void  ZrmReportDatabase::delete_model(QAbstractTableModel * model)
{
   if(model)
       delete model;
}


QSqlTableModel *    ZrmReportDatabase::types_model()
{
    auto inst = get_main_instance();
    if(inst == this)
    {
     if(check_db_open())
     {
         if(!m_types_model)
         {
           m_types_model = new QSqlTableModel(Q_NULLPTR, *m_reports_db);
           m_types_model->setEditStrategy(QSqlTableModel::OnFieldChange);
           m_types_model->setTable("tbattery_types");
           m_types_model->setFilter("is_del <> 1");
           int name_idx = field_index(m_types_model,"name");
           m_types_model->setHeaderData(name_idx,Qt::Horizontal,QObject::tr("Тип"));
           m_types_model->setHeaderData(field_index(m_types_model,"voltage"),Qt::Horizontal,QObject::tr("Напряжение"));
           m_types_model->setHeaderData(field_index(m_types_model,"capacity"),Qt::Horizontal,QObject::tr("Ёмкость"));
           m_types_model->setSort(name_idx, Qt::SortOrder::AscendingOrder);
           m_types_model->select();
         }
     }
     return m_types_model;
    }
   return inst->types_model();
}

QSqlTableModel     * ZrmReportDatabase::users_model()
{
    auto inst = get_main_instance();
    if(inst == this)
    {
     if(check_db_open())
     {
         if(!m_users_model)
         {
          m_users_model = new QSqlTableModel(Q_NULLPTR, *m_reports_db);
          m_users_model->setEditStrategy(QSqlTableModel::OnFieldChange);
          m_users_model->setTable("tusers");
          m_users_model->setFilter("is_del <> 1");
          int sname_idx = field_index(m_users_model,"sname");
          m_users_model->setHeaderData(sname_idx,Qt::Horizontal,QObject::tr("Фамилия"));
          m_users_model->setHeaderData(field_index(m_users_model,"fname"),Qt::Horizontal,QObject::tr("Имя"));
          m_users_model->setHeaderData(field_index(m_users_model,"pname"),Qt::Horizontal,QObject::tr("Отчество"));
          m_users_model->setSort(sname_idx,Qt::SortOrder::AscendingOrder);
          m_users_model->select();
         }
     }
     return  m_users_model;
    }
  return inst->users_model();
}

int   ZrmReportDatabase::field_index(const QAbstractItemModel *mod, const QString & field_name)
{
   auto model = dynamic_cast<const QSqlTableModel*>(mod);
   return model ? model->fieldIndex(field_name) : -1;
}

int      ZrmReportDatabase::field_id    (const QAbstractItemModel * mod)
{
  return field_index(mod,"id");
}

int      ZrmReportDatabase::field_is_del(QAbstractItemModel * mod)
{
  return field_index(mod,"is_del");
}

bool     ZrmReportDatabase::mark_del    (QAbstractItemModel *mod, int row_num)
{
   auto model = dynamic_cast<QSqlTableModel*>(mod);
   if(model)
   {
    QModelIndex index =  model->index(row_num, field_is_del(mod));
    if(index.isValid())
    {

      if(model->setData(index,1) && model->select())
      {
       model->submit();
       return model->select();
      }
    }
   }
 return false;
}


int  ZrmReportDatabase::new_record  (QAbstractItemModel  * mod)
{
   int ret = -1;
    if(mod)
    {
        ret = mod->rowCount();
        mod->insertRow(ret);
        QModelIndex index = mod->index(ret, field_is_del(mod));
        mod->setData(index,0);
    }
  return ret;
}

void  ZrmReportDatabase::revert      (QAbstractItemModel  * mod)
{
  mod->revert();
  auto model = dynamic_cast<QSqlTableModel*>(mod);
  if(model)
     model->select();
}

bool  ZrmReportDatabase::submit      (QAbstractItemModel  * mod)
{
    return mod->submit();
}


void  ZrmReportDatabase::assign_model(QTableView * tv,QSqlTableModel * model)
{
   if(tv)
   {
    tv->setModel(model);
    tv->hideColumn(field_id(model));
    tv->hideColumn(field_is_del(model));
   }
}

int   ZrmReportDatabase::users_short_fio()
{
    return  field_index(users_model(),"short_fio");
}

void  ZrmReportDatabase::assign_users_model(QTableView * tv)
{
 if(tv)
 {
   auto model = users_model();
   assign_model(tv,model) ;
   tv->hideColumn(users_short_fio());
 }
}

void  ZrmReportDatabase::assign_types_model(QTableView * tv)
{
 if(tv)
 {
   auto model = types_model();
   assign_model(tv,model);
   tv->hideColumn(field_index(model, "id_alternative"));
 }
}


QSqlTableModel *      ZrmReportDatabase::numbers_model()
{
    auto inst = get_main_instance();
    if(inst == this)
    {
        if(check_db_open())
        {
            if(!m_snumbers_model)
            {
                m_snumbers_model = new QSqlTableModel(Q_NULLPTR, *database());
                m_snumbers_model->setEditStrategy(QSqlTableModel::OnFieldChange);
                m_snumbers_model->setTable ("tbattery_list");
                m_snumbers_model->setHeaderData(field_index(m_snumbers_model,"serial_number"),Qt::Horizontal,"Номер батареи");
            }
        }
        return m_snumbers_model;
    }
    return inst->numbers_model();
}

void   ZrmReportDatabase::assign_numbers_model(QTableView * tv)
{
   if(tv)
   {
     auto model = numbers_model();
     assign_model(tv,model);
     tv->hideColumn(field_index(model,"id_type"));
   }
}

bool  ZrmReportDatabase::numbers_select(int id_type, bool hide_del )
{
   auto model = numbers_model();
   //model->clear();
   QString filter;
   if(hide_del)
       filter = "is_del <> 1 and ";
   filter += QString("id_type = %1").arg(id_type);
   model->setFilter(filter);
   return model->select();
}

int   ZrmReportDatabase::type_text()
{
  auto model = types_model();
  return  model ? field_index(model,"name") : -1;
}

int   ZrmReportDatabase::number_text()
{
  auto model = numbers_model();
  return  model ? field_index(model,"serial_number") : -1;
}

int  ZrmReportDatabase::get_record_id(QAbstractItemModel * model,int row)
{
  if(model )
  {
    QModelIndex index = model->index(row, field_id(model));
    if(index.isValid())
    {
     bool ok = false;
     int value = index.data().toInt(&ok);
     return ok ? value : -1;
    }
  }
  return -1;
}




bool ZrmReportDatabase::report_create  (QSqlDatabase & db, int user_id, int serial_number_id,QVariant & rep_id)
{

  QString  qtext = QLatin1String("INSERT INTO treport (id,id_battery,id_user,dtm)  VALUES ( NULL, :id_battery, :id_user, :dtm );");
  QSqlQuery rep_query(db);
  if(rep_query.prepare(qtext))
  {
    rep_query.bindValue(":id_battery",serial_number_id) ;
    rep_query.bindValue(":id_user"   , user_id);
    rep_query.bindValue(":dtm"       , QDateTime::currentDateTime());

    if(rep_query.exec())
       {
        rep_id = rep_query.lastInsertId();
        return true;
       }
  }

  return false;
}


bool ZrmReportDatabase::report_update       (QSqlDatabase & db, QVariant rep_id, uint32_t total_duration, qreal total_energy, qreal total_capacity)
{
  QString qtext = QLatin1String("UPDATE treport   SET   total_duration = :total_duration,  total_energy   = :total_energy,  total_capacity = :total_capacity WHERE id = :id ;");
  QSqlQuery rep_query(db);
  if(rep_query.prepare(qtext))
  {
    rep_query.bindValue(":id", rep_id);
    rep_query.bindValue(":total_duration", total_duration);
    rep_query.bindValue(":total_energy"  , total_energy  );
    rep_query.bindValue(":total_capacity", (total_capacity > 0 ? total_capacity : QVariant()));
    return rep_query.exec();
  }
  return false;
}

bool ZrmReportDatabase::reportDelete(QVariant rep_id)
{
    auto delTable = [this, rep_id](QString qtext)
    {
        QSqlDatabase* db = database();
        QSqlQuery rep_query(*db);
        if(rep_query.prepare(qtext))
        {
            rep_query.bindValue(":id", rep_id);
            return rep_query.exec();
        }
        return false;
    };

    QString qtext = QLatin1String("DELETE FROM treport_details_sensors WHERE id_report = :id ;");
    bool res = delTable(qtext);
    if (res)
    {
        qtext = QLatin1String("DELETE FROM treport_details WHERE id_report = :id ;");
        res = delTable(qtext);
        if (res)
        {
            qtext = QLatin1String("DELETE FROM treport WHERE id = :id ;");
            res = delTable(qtext);
        }
    }
    return res;
}

bool ZrmReportDatabase::report_write(int user_id, int serial_number_id, const zrm::method_exec_results_t & results , const zrm::method_exec_results_sensors_t &exec_result_sensors)
{

 auto db = database();
 bool ret = db && db->transaction();
 if(ret)
 {
    QVariant rep_id;
    ret = report_create (*db, user_id,serial_number_id, rep_id);
    if(ret)
    {
      uint32_t   total_duration = 0;
      qreal      total_capacity = 0;
      qreal      total_energy   = 0;
      ret = report_write_details(*db, rep_id, results, total_duration, total_energy, total_capacity)
              && report_update(*db, rep_id,total_duration,total_energy,total_capacity)
              && report_write_details_sensors(*db, rep_id, exec_result_sensors);
    }

    if(ret)
       ret = db->commit();
     else
       db->rollback();

 }
 return  ret;
}


bool ZrmReportDatabase::report_write_details(QSqlDatabase & db,const QVariant & rep_id, const zrm::method_exec_results_t &results
                         , uint32_t & total_duration, qreal & total_energy, qreal & total_capacity)
{
    QString qtext = QLatin1String("INSERT INTO treport_details ( id_report, stage_number, stage_type, stage_duration, i_beg, i_end,u_beg, u_end, capacity ) "
                                  " VALUES (:id_report, :stage_number,:stage_type, :stage_duration, :i_beg, :i_end, :u_beg, :u_end, :capacity  ); ");
    QSqlQuery query(db);
    bool ret = query.prepare(qtext);
    query.bindValue(":id_report", rep_id);
    for(auto res : results)
    {
        if(ret)
        {

        query.bindValue(":stage_number", res.stage);
        //query.bindValue(":stage_type", QObject::tr("Этап"));


        uint32_t duration = uint32_t(res.duration[0])*3600 + uint32_t(res.duration[1])*60 + uint32_t(res.duration[2]);
        query.bindValue(":stage_duration", duration);
        total_duration += duration;

        qreal CAP  = qreal(res.capcacity )/1000.0;
        query.bindValue(":capacity", CAP);

        total_energy   += CAP;
        if(res.state & zrm::STFL_CAPACITY_MEASHURE)
           total_capacity = qMax(total_capacity,fabs(CAP));

            qreal Ibeg = qreal(res.curr_begin)/1000.0;
            query.bindValue(":i_beg", Ibeg);

            qreal Iend = qreal(res.curr_end  )/1000.0;
            query.bindValue(":i_end", Iend);

            qreal Ubeg = qreal(res.volt_begin)/1000.0;
            query.bindValue(":u_beg", Ubeg);

            qreal Uend = qreal(res.volt_end  )/1000.0;
            query.bindValue(":u_end", Uend);
            ret = query.exec();
       }
        else
        {
            qDebug()<<query.lastError().text();
            break;
        }

    }
  return ret;
}

bool ZrmReportDatabase::report_write_details_sensors(QSqlDatabase & db, const QVariant & rep_id, const zrm::method_exec_results_sensors_t &results)
{
    QString qtext = QLatin1String("INSERT INTO treport_details_sensors ( id_report, stage_number, sensor_number, t, u ) "
                                  "VALUES (:id_report, :stage_number, :sensor_number, :t, :u ); ");
    QSqlQuery query(db);
    bool ret = query.prepare(qtext);
    query.bindValue(":id_report", rep_id);
    for (auto res : results)
    {
        query.bindValue(":stage_number", res.stage);

        for (uint i = 0; i < res.sensors.size(); i++)
        {
            if (ret)
            {
                query.bindValue(":sensor_number", i);
                query.bindValue(":t", qreal(res.sensors[i].temp) / 1000.);
                query.bindValue(":u", qreal(res.sensors[i].volt) / 1000.);
                ret = query.exec();
            }
            else
            {
                qDebug() << query.lastError().text();
                break;
            }
        }
    }
    return ret;
}
