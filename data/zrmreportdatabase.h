#ifndef ZRMREPORTDATABASE_H
#define ZRMREPORTDATABASE_H

#include <QObject>
#include <qsqldatabase.h>
#include <qsqltablemodel.h>
#include <qtableview.h>
#include <qsqlrelationaltablemodel.h>
#include <zrmproto.hpp>

class ZrmReportDatabase
{
public:
    ZrmReportDatabase();
   ~ZrmReportDatabase();
    QSqlDatabase   *      database();
    int                   get_record_id(QAbstractItemModel * model,int row);
    QSqlTableModel *      users_model();
    void                  assign_users_model(QTableView * tv);
    int                   users_short_fio();

    QSqlTableModel *      types_model();
    void                  assign_types_model(QTableView * tv);
    int                   type_text();


    QSqlTableModel        *numbers_model();
    void                  assign_numbers_model(QTableView * tv);
    int                   number_text();
    bool                  numbers_select (int id_type, bool hide_del = true);

    void                  assign_model(QTableView * tv,QSqlTableModel * model);
    int                   field_index (const QAbstractItemModel * mod, const QString & field_name);
    int                   field_id    (const QAbstractItemModel *mod);
    int                   field_is_del(QAbstractItemModel * mod);
    bool                  mark_del    (QAbstractItemModel * mod, int row_num);
    int                   new_record  (QAbstractItemModel  * mod);
    void                  revert      (QAbstractItemModel  * mod);
    bool                  submit      (QAbstractItemModel  * mod);

    bool                  report_write(int user_id, int serial_number_id, const zrm::method_exec_results_t & exec_result, const zrm::method_exec_results_sensors_t & exec_result_sensors);
    bool reportDelete(QVariant rep_id);

protected:
ZrmReportDatabase  *      get_main_instance();
      void                delete_model(QAbstractTableModel *model);
      bool                check_db_open();
      bool report_create(QSqlDatabase & db, int user_id    , int      serial_number_id, QVariant &rep_id  );
      bool report_update(QSqlDatabase & db, QVariant rep_id, uint32_t total_duration  , qreal total_energy, qreal total_capacity);
      bool report_write_details(QSqlDatabase & db,const QVariant & rep_id, const zrm::method_exec_results_t &results
                               , uint32_t & total_duration, qreal & total_energy, qreal & total_capacity);
      bool report_write_details_sensors(QSqlDatabase & db, const QVariant & rep_id, const zrm::method_exec_results_sensors_t &results);


static ZrmReportDatabase  * main_instance;
       QSqlDatabase       * m_reports_db     = Q_NULLPTR;
       QSqlTableModel     * m_users_model    = Q_NULLPTR;
       QSqlTableModel     * m_types_model    = Q_NULLPTR;
       QSqlTableModel     * m_snumbers_model = Q_NULLPTR;

};

#endif // ZRMREPORTDATABASE_H
