
/* SQL Data source
 * Ostapenko D. V. NIKTES 2019-03-25
 */



#ifndef ZRMDATASOURCE_H
#define ZRMDATASOURCE_H


#include <QtSql>
#include <qsharedpointer.h>
#include <map>

typedef std::map<QString,QVariant> query_args_t;

class ZrmDataSource : public QObject
{
    Q_OBJECT
    explicit ZrmDataSource(QObject *parent = nullptr);
public:
typedef QSharedPointer<ZrmDataSource> data_source_ptr_t;
    static QSqlDatabase      method_database  (bool as_charger);
    static QSqlDatabase      reports_database ();
    static QString           config_file_name (const QString &kind);
//    static bool              exec_query      (QSqlQuery & query, const QString & query_text, const query_args_t & args = query_args_t());
//    static bool              skip_empty      (QSqlQuery & query, int index = 0);
//    template <typename _Func>
//    static   int fetch_records(QSqlQuery & query, _Func func);

    static void unload(bool as_charger);
    static void load(bool as_charger);

private:
    static bool              register_data_base(bool as_charge);
    static ZrmDataSource *   instance();
    static data_source_ptr_t data_source ;
    static const char *      charge_methods;
    static const char *      power_methods;
    static const char *      svc_reports      ;
};

#endif // ZRMDATASOURCE_H
