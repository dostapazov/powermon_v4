#include "zrmdatabase.h"
#include <qsqlrecord.h>

QTextCodec* ZrmDatabase::m_codec  = Q_NULLPTR;
QSqlError  ZrmDatabase::m_last_error;


ZrmDatabase::ZrmDatabase(QObject* parent) : QObject(parent)
{}


constexpr const char* transact_prop =  "trans_active";

int  ZrmDatabase::start_transaction   (QSqlDatabase& db)
{
    int trans_counter = 0;
    if (db.isOpen())
    {
        QSqlDriver* drv = db.driver();
        trans_counter =  drv->property(transact_prop).toInt();
        if (trans_counter || drv->beginTransaction())
        {
            ++trans_counter;
            drv->setProperty(transact_prop, trans_counter);
        }

    }
    return trans_counter;
}

void ZrmDatabase::rollback_transaction(QSqlDatabase& db)
{
    if (db.isOpen())
    {
        QSqlDriver* drv = db.driver();
        int trans_counter =  drv->property(transact_prop).toInt();
        if (trans_counter)
        {
            trans_counter = 0;
            drv->setProperty(transact_prop, trans_counter);
            drv->rollbackTransaction();
        }
    }
}

bool ZrmDatabase::commit_transaction  (QSqlDatabase& db, bool force)
{
    if (db.isOpen())
    {
        QSqlDriver* drv = db.driver();
        int trans_counter =  drv->property(transact_prop).toInt() ;
        if (trans_counter == 1 || force)
        {
            if (drv->commitTransaction())
            {
                drv->setProperty(transact_prop, 0);
                qDebug() << "Commit transaction";
                return true;
            }
        }
        else
            drv->setProperty(transact_prop, trans_counter - 1);
        return  trans_counter;
    }
    return false;
}

bool   ZrmDatabase::transaction_active  (QSqlDatabase& db)
{
    int trans_counter = 0;
    if (db.isOpen())
    {
        QSqlDriver* drv = db.driver();
        trans_counter =  drv->property(transact_prop).toInt();
    }
    return trans_counter;
}


bool     ZrmDatabase::skip_empty      (QSqlQuery& query, int index )
{
    bool ret = !query.isNull(index);
    while (!ret)
    {
        if (!query.next())
            break;
        ret = !query.isNull(index);
    }
    return ret;
}

bool ZrmDatabase::exec_query(QSqlQuery& query, const QString& query_text, const query_args_t& args)
{
    query.clear();
    query.setForwardOnly(true);
    if (query.prepare(query_text))
    {
        foreach (auto arg, args)
            query.bindValue(arg.first, arg.second);
        return query.exec();
    }
    return false;
}


bool ZrmDatabase::exec_write (QSqlDatabase& db, QSqlQuery& query, const QString& qtext, const query_args_t& args)
{
    bool ret = false;
    if (transaction_active(db) || start_transaction(db))
    {
        ret = ZrmDatabase::exec_query(query, qtext, args);
        if (!ret && db.lastError().type() == QSqlError::NoError)
            ret = true;

    }
    if (!ret)
    {
        m_last_error = db.lastError();
        qDebug() << m_last_error.text();
        rollback_transaction(db);
    }
    return ret;
}

QSqlQuery     ZrmDatabase::read_model_methods  (QSqlDatabase& db, QVariant model_id)
{
    QString qtext =
        "select m.id  , m.c_name, m.n_volt_rate, m.n_current_rate"
        ", m.n_duration, m.n_cycle_count "
        "from tl_model_method  tm "
        "left join t_method m on  tm.id_method = m.id "
        "where tm.id_model = :model "
        "order by m.c_name" ;
    query_args_t args;
    args[":model"] = model_id;

    QSqlQuery query(db);
    if (!exec_query(query, qtext, args))
        query.clear();
    return query;

}



QSqlQuery     ZrmDatabase::read_method       (QSqlDatabase& db, QVariant id )
{

    QString qtext =
        "select id  , c_name, n_volt_rate, n_current_rate"
        ", n_duration, n_cycle_count, n_user_volt, n_user_capacity "
        "from t_method where  t_method.id = :id ";
    query_args_t args;
    args[":id"] = id;
    QSqlQuery query(db);
    if (!exec_query(query, qtext, args))
        query.clear();
    return query;
}

QSqlQuery     ZrmDatabase::read_all_methods       (QSqlDatabase& db )
{
    QSqlQuery query(db);
    QString qtext =
        "select id  , c_name, n_volt_rate, n_current_rate"
        ", n_duration, n_cycle_count, n_user_volt, n_user_capacity "
        "from t_method ";
    if (!exec_query(query, qtext))
        query.clear();
    return query;
}


bool          ZrmDatabase::read_method       (QSqlDatabase& db, QVariant id, zrm::method_t& result, double voltage, double capacity)
{
    result = zrm::method_t();
    QSqlQuery query = read_method(db, id );
    if (skip_empty(query))
    {
        double coeff;
        result.m_id = uint16_t(query.value(0).toUInt());
        result.set_capacity   (capacity);
        if (qFuzzyIsNull(voltage))
            voltage = 1.0;

        coeff = query.value(2).toDouble();
        result.set_voltage    (voltage * coeff);
        coeff = query.value(3).toDouble();
        result.set_current    (capacity * coeff);
        result.set_duration   (query.value(4).toUInt());
        result.m_cycles_count = uint8_t(query.value(5).toUInt());
        QString name = query.value(1).toString();
        QByteArray name_array;
        if (m_codec)
            name_array = m_codec->fromUnicode(name);
        else
            name_array = name.toLocal8Bit();
        memset(result.m_name, 0, sizeof (result.m_name));
        memcpy(result.m_name, name_array.constData(), qMin(sizeof(result.m_name), size_t(name_array.size())));

        return true;
    }
    return false;
}

QSqlQuery     ZrmDatabase::read_method_stages(QSqlDatabase& db, QVariant m_id)
{
    QSqlQuery query(db);

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
    query_args_t args;
    args[":method"] = m_id;
    if (!exec_query(query, qtext, args))
        query.clear();
    return query;
}


bool          ZrmDatabase::read_method_stages(QSqlDatabase& db, QVariant m_id, zrm::stages_t& result)
{
    QSqlQuery query = read_method_stages(db, m_id);
    result.clear();
    if (skip_empty(query))
    {
        auto f = [&result, m_id](QSqlRecord & rec)
        {
            zrm::stage_t stage;

            stage.m_method_id = uint16_t (rec.value(0).toUInt() );//ID - этапа см. ZrmConnectivity::write_method;
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

            auto hms = pwm_utils::secunds2hms(rec.value(15).toUInt());
            stage.m_hours   = std::get<0>(hms);
            stage.m_minutes = std::get<1>(hms);
            stage.m_secs    = std::get<2>(hms);

            stage.set_end_cell_volt      (rec.value(16).toDouble());

            stage.m_stage_flags = uint8_t(rec.value(17).toUInt());
            result.push_back(stage);

        };

        ZrmDatabase::fetch_records(query, f);

    }
    return false;
}

QString   ZrmDatabase::read_stage_descript(QSqlDatabase& db, QVariant stage_id)
{
    QSqlQuery query(db)   ;
    QString qtext = "select st.st_descript from t_stage st where st.id = :stage_id";
    query_args_t args;
    args[":stage_id"] = stage_id;
    if (ZrmDatabase::exec_query(query, qtext, args) && skip_empty(query))
    {
        return  query.value(0).toString();
    }
    return QString();
}

bool  ZrmDatabase::write_stage_descript(QSqlDatabase& db, QVariant stage_id, const QString& stage_descript)
{
    QString query_text = "update t_stage set st_descript = :descr where id = :stage_id ";
    QSqlQuery query(db)   ;
    query_args_t args;
    args[":stage_id"] = stage_id;
    args[":descr"   ] = stage_descript;
    return exec_write(db, query, query_text, args);
}

bool ZrmDatabase::link_method(QSqlDatabase& db, QVariant method_id, QVariant model_id)
{
    start_transaction(db);
    QString qtext =
        "INSERT INTO tl_model_method (id_model,id_method) "
        "VALUES (:id_model, :id_method) ";
    query_args_t args;
    args[":id_model" ] = model_id;
    args[":id_method"] = method_id;
    QSqlQuery query(db);
    if (exec_write(db, query, qtext, args))
        return commit_transaction(db);
    rollback_transaction(db);
    return false;

}

bool ZrmDatabase::unlink_method(QSqlDatabase& db, QVariant method_id, QVariant model_id)
{
    start_transaction(db);
    query_args_t args;
    QString qtext =
        "DELETE FROM tl_model_method "
        "WHERE id_method = :id_method ";
    args[":id_method"] = method_id;

    if (model_id.toUInt())
    {
        qtext += " AND id_model = :id_model " ;
        args[":id_model"] = model_id;
    }
    QSqlQuery query(db);
    if (exec_write(db, query, qtext, args))
        return commit_transaction(db);

    rollback_transaction(db);
    return false;

}


bool  ZrmDatabase::erase_method      (QSqlDatabase& db, QVariant method_id)
{
    if (erase_stages(db, method_id))
    {
        QSqlQuery query(db);
        QString qtext = "DELETE FROM t_method  WHERE id = :id ";
        query_args_t args;
        args[":id"] = method_id;
        return exec_write(db, query, qtext, args);
    }
    return false;
}

bool  ZrmDatabase::erase_stages      (QSqlDatabase& db, QVariant method_id )
{
    QSqlQuery query(db);
    QString qtext = "DELETE FROM t_stage  WHERE id_method = :id_method ";
    query_args_t args;
    args[":id_method"] = method_id;
    return exec_write(db, query, qtext, args);

}

bool  ZrmDatabase::erase_stages      (QSqlDatabase& db, const zrm::stages_t& stages)
{
    bool ret = true;
    for (const zrm::stage_t& st : stages)
    {
        ret &= erase_stage(db, st.m_method_id);
        if (!ret)
            break;

    }
    return ret;
}

bool  ZrmDatabase::erase_stage       (QSqlDatabase& db, QVariant stage_id)
{
    QSqlQuery query(db);
    QString qtext = "DELETE FROM t_stage  WHERE id = :id ";
    query_args_t args;
    args[":id"] = stage_id;
    return exec_write(db, query, qtext, args);
}

bool  ZrmDatabase::write_method_uservals (QSqlDatabase& db, QVariant  method_id, QVariant user_volt, QVariant user_capacity, bool commit_trans)
{
    bool ret = false;
    if (!is_null_id(method_id))
    {
        start_transaction(db);
        QString qtext =
            "UPDATE t_method SET "
            "n_user_volt = :volt, n_user_capacity = :capacity "
            "WHERE id = :id";
        query_args_t  args;
        args[":id"      ] = method_id;
        args[":volt"    ] = user_volt;
        args[":capacity"] = user_capacity;
        QSqlQuery query(db);
        ret = exec_write(db, query, qtext, args);
        if (ret && commit_trans)
            ret = commit_transaction(db);

    }
    return ret;
}


bool  ZrmDatabase::write_method       (QSqlDatabase& db, QVariant& method_id, const QString& method_name
                                       , zrm::zrm_method_t& method
                                      )
{
    bool is_new = is_null_id(method_id);
    QString qtext =
        is_new ?
        "INSERT INTO t_method (id, c_name,  n_volt_rate, n_current_rate, n_duration, n_cycle_count, n_user_volt, n_user_capacity  ) "
        "              VALUES (:id, :name,:volt_rate,:current_rate,:duration,:cycle_count,:user_volt,:user_capacity) "

        :
        "UPDATE t_method SET "
        "c_name = :name, n_volt_rate = :volt_rate, n_current_rate = :current_rate,"
        "  n_duration = :duration,  n_cycle_count = :cycle_count,"
        "  n_user_volt = :user_volt, n_user_capacity = :user_capacity "
        " WHERE id = :id"
        ;
    query_args_t args;
    if (!is_new)
    {
        args[":id"] = method_id;
    }

    args[":name"]          = method_name;
    args[":volt_rate"]     = 1.0;
    args[":current_rate"]  = method.m_method.current_ratio(false);
    args[":duration"]      = method.m_method.duration();
    args[":cycle_count"]   = method.m_method.m_cycles_count;
    args[":user_volt"]     = method.m_method.voltage();
    args[":user_capacity"] = method.m_method.capacity();
    QSqlQuery query(db);
    //Записали метод
    if (exec_write(db, query, qtext, args))
    {
        if (is_new)
            method_id = query.lastInsertId();
        return true;
    }
    return false;
}

bool  ZrmDatabase::write_stages(QSqlDatabase& db, const QVariant& method_id,
                                zrm::stages_t& stages, const zrm::stages_t& remove_stages
                               )
{
    if (remove_stages.size() && !erase_stages(db, remove_stages))
    {
        return false;
    }

    for ( zrm::stage_t& st : stages)
    {
        if (!write_stage(db, method_id, st))
            return false;

    }
    return true;
}

bool ZrmDatabase::write_stage       (QSqlDatabase& db, const QVariant& method_id,  zrm::stage_t&   stage)
{
    bool ret = false;
    QString upd_text =
        "UPDATE t_stage   SET   id_type = :id_type, n_pos   = :pos, n_ch_volt_rate = :ch_volt_rate, "
        "n_dis_volt_rate = :dis_volt_rate, n_ch_cur_rate = :ch_cur_rate,  n_dis_cur_rate = :dis_cur_rate,  n_ch_duration = :ch_duration, "
        "n_dis_duration = :dis_duration  , n_finish_flags = :finish_flags,  n_finish_voltage_rate = :finish_voltage_rate, "
        "n_finish_current_rate = :finish_current_rate, n_finish_capacity_rate = :finish_capacity_rate,   n_finish_delta_volt_rate = :finish_delta_volt_rate, "
        "n_finish_temper = :finish_temper,  n_finish_duration = :finish_duration,  n_finish_cell_volt = :finish_cell_volt,  n_stage_flags = :stage_flags "
        " WHERE id = :id "
        ;

    QString ins_text =
        " INSERT INTO t_stage ("
        "                      id, id_method, id_type, n_pos, n_ch_volt_rate, n_dis_volt_rate, n_ch_cur_rate, n_dis_cur_rate, n_ch_duration,"
        "                      n_dis_duration, n_finish_flags, n_finish_voltage_rate, n_finish_current_rate, n_finish_capacity_rate, "
        "                      n_finish_delta_volt_rate, n_finish_temper, n_finish_duration, n_finish_cell_volt, n_stage_flags"
        "                      )"
        "             VALUES ( "
        "                     :id,:id_method,:id_type,:pos,:ch_volt_rate,:dis_volt_rate,:ch_cur_rate, :dis_cur_rate,:ch_duration, "
        "                     :dis_duration,:finish_flags,:finish_voltage_rate,:finish_current_rate,:finish_capacity_rate, "
        "                     :finish_delta_volt_rate,:finish_temper,:finish_duration,:finish_cell_volt, :stage_flags "
        "                    ); "
        ;

    bool is_new = !stage.m_method_id;
    query_args_t args;
    if ( !is_new )
    {
        args[":id"] = stage.m_method_id;
    }
    else
    {
        args[":id_method"] = method_id;
    }

    args[":id_type"]       = stage.m_type;
    args[":pos"    ]       = stage.m_number;
    args[":ch_volt_rate"]  = stage.charge_volt(1.0);
    args[":dis_volt_rate"] = stage.discharge_volt(1.0);
    args[":ch_cur_rate"]   = stage.charge_curr   (1.0);
    args[":dis_cur_rate"]  = stage.discharge_curr(1.0);
    args[":ch_duration"]   = stage.m_char_time;
    args[":dis_duration"]  = stage.m_dis_time;
    args[":finish_flags"]  = stage.m_finish_flags;
    args[":finish_voltage_rate"]    =  stage.end_volt(1.0);
    args[":finish_current_rate"]    =  stage.end_curr(1.0);
    args[":finish_capacity_rate"]   =  stage.end_capacity(1.0);
    args[":finish_delta_volt_rate"] =  stage.end_delta_volt(1.0);
    args[":finish_temper"]    = stage.end_temp();
    args[":finish_duration"]  = pwm_utils::hms2secunds(stage.m_hours, stage.m_minutes, stage.m_secs);
    args[":finish_cell_volt"] = stage.end_cell_volt();
    args[":stage_flags"]      = stage.m_stage_flags;
    QSqlQuery query(db);
    ret = exec_write(db, query, is_new ? ins_text : upd_text, args);

    if (ret && is_new)
    {
        stage.m_method_id = uint16_t(query.lastInsertId().toUInt());
    }
    return ret;
}


bool  ZrmDatabase::write_type (QSqlDatabase& db, QVariant& id, const QString& type_name )
{
    bool ret = false;
    start_transaction(db);
    bool is_new = is_null_id(id);
    QString   qtext = is_new ?  "insert into t_akb_type (id, c_name) values (:id,:name)" : "update t_akb_type set c_name = :name where id = :id";

    query_args_t args;
    args[":name"] = type_name;
    if (!is_new)
        args[":id"] = id;
    QSqlQuery query(db);
    ret = exec_write(db, query, qtext, args);
    if (ret)
    {
        if (is_new)
            id = query.lastInsertId();
        commit_transaction(db, true);
    }
    return ret;
}

bool ZrmDatabase::erase_model        (QSqlDatabase& db, QVariant id, bool commit_trans)
{
    bool ret = false;
    start_transaction(db);
    //Удалить все связи по методам
    //Удалить собственно модель
    QString qtext1 = "delete from tl_model_method where id_model = :id";
    QString qtext2 = "delete from t_model where id = :id";
    QSqlQuery    query(db);
    query_args_t args;
    args[":id"] = id;

    if (exec_write(db, query, qtext1, args))
        ret = exec_write(db, query, qtext2, args);
    if (ret && commit_trans)
        commit_transaction(db);
    return ret;
}

bool ZrmDatabase::erase_type        (QSqlDatabase& db, QVariant id, bool commit_trans)
{
    start_transaction(db);
    QString qtext = "delete from t_akb_type where id = :id";
    QSqlQuery query(db);
    query_args_t args;
    args[":id"] = id;
    bool ret = exec_write(db, query, qtext, args);
    if (ret && commit_trans)
        commit_transaction(db);
    return ret;
}


bool ZrmDatabase::write_model         (QSqlDatabase& db, QVariant id_type, QVariant& id, const QString& name, double voltage, double capacity, bool commit_trans)
{
    bool ret = false;
    start_transaction(db);
    bool is_new = is_null_id(id);
    QString   qtext = is_new ?
                      "INSERT INTO t_model ( c_name, id_type, n_voltage, n_capacity ) VALUES (:name, :id_type, :voltage, :capacity );"
                      :
                      "UPDATE t_model  SET  c_name = :name, id_type = :id_type, n_voltage = :voltage, n_capacity = :capacity WHERE id = :id";

    query_args_t args;
    args[":name"]     = name;
    args[":voltage"]  = voltage;
    args[":capacity"] = capacity;
    args[":id_type"]  = id_type;
    if (!is_new)
        args[":id"]  = id;

    QSqlQuery query(db);
    ret = exec_write(db, query, qtext, args);
    id = ret && is_new ? query.lastInsertId() : id;
    if (ret && commit_trans)
        commit_transaction(db);
    return ret;
}


