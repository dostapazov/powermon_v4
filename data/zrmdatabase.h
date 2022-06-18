#ifndef ZRMDATABASE_H
#define ZRMDATABASE_H

#include <QObject>
#include <zrmdatasource.h>
#include <zrmproto.hpp>
#include <qtextcodec.h>

class ZrmDatabase : public QObject
{
    Q_OBJECT
public:
    explicit ZrmDatabase(QObject* parent = nullptr);
    static QTextCodec*   codec();
    static void          set_codec (QTextCodec* codec);
    static int           start_transaction   (QSqlDatabase& db);
    static void          rollback_transaction(QSqlDatabase& db );
    static bool          commit_transaction  (QSqlDatabase& db, bool force = false);
    static bool          transaction_active  (QSqlDatabase& db);

    static bool          skip_empty          (QSqlQuery& query, int index = 0);
    static bool          exec_query          (QSqlQuery& query, const QString& query_text, const query_args_t& args  = query_args_t()) ;
    static bool          exec_write          (QSqlDatabase& db, QSqlQuery& query, const QString& qtext, const query_args_t& args);

    static QSqlQuery     read_all_methods    (QSqlDatabase& db );
    static QSqlQuery     read_model_methods  (QSqlDatabase& db, QVariant model_id);
    static QSqlQuery     read_method         (QSqlDatabase& db, QVariant id  );
    static bool          read_method         (QSqlDatabase& db, QVariant id, zrm::method_t& result, double voltage = 1, double capacity  = 1);
    static QSqlQuery     read_method_stages  (QSqlDatabase& db, QVariant m_id );
    static bool          read_method_stages  (QSqlDatabase& db, QVariant m_id, zrm::stages_t& result);
    static QString       read_stage_descript (QSqlDatabase& db, QVariant stage_id);

    static bool          link_method         (QSqlDatabase& db, QVariant method_id, QVariant model_id);
    static bool          unlink_method       (QSqlDatabase& db, QVariant method_id, QVariant model_id);
    static bool          erase_method        (QSqlDatabase& db, QVariant method_id);
    static bool          erase_stages        (QSqlDatabase& db, QVariant method_id);
    static bool          erase_stages        (QSqlDatabase& db, const zrm::stages_t& stages);
    static bool          erase_stage         (QSqlDatabase& db, QVariant stage_id);

    static bool          write_method         (QSqlDatabase& db, QVariant& method_id, const QString& method_name, zrm::zrm_method_t& method);
    static bool          write_method_uservals(QSqlDatabase& db, QVariant method_id, QVariant user_volt, QVariant user_capacity, bool commit_trans = true);
    static bool          write_stages        (QSqlDatabase& db, const QVariant& method_id, zrm::stages_t &stages, const zrm::stages_t& remove_stages);
    static bool          write_stage         (QSqlDatabase& db, const QVariant& method_id, zrm::stage_t& stage);
    static bool          write_stage_descript(QSqlDatabase& db, QVariant stage_id, const QString& stage_descript);

    static bool          write_type          (QSqlDatabase& db, QVariant& id, const QString& type_name );


    static bool          erase_type          (QSqlDatabase& db, QVariant id, bool commit_trans = true);

    static bool          erase_model         (QSqlDatabase& db, QVariant id, bool commit_trans = true);
    static bool          write_model         (QSqlDatabase& db, QVariant id_type, QVariant& id, const QString& name, double voltage, double capacity, bool commit_trans = true);
    static bool          is_null_id          (const QVariant& id);

    static QString       to_unicode  (const QByteArray& array);
    static QByteArray    from_unicode(const QString& str);
    template <typename _Func>
    static   int         fetch_records(QSqlQuery& query, _Func func);
    static   QSqlError   last_error() {return m_last_error;}

private :
    static QTextCodec* m_codec ;
    static QSqlError  m_last_error;
};


inline QTextCodec*   ZrmDatabase::codec()
{
    return m_codec;
}

inline void          ZrmDatabase::set_codec(QTextCodec* codec)
{
    m_codec = codec;
}

inline QString       ZrmDatabase::to_unicode  (const QByteArray& array)
{
    return  m_codec ?  m_codec->toUnicode(array) : QString::fromLocal8Bit(array.constData(), array.size());
}

inline QByteArray    ZrmDatabase::from_unicode(const QString& str)
{
    return m_codec ? m_codec->fromUnicode(str) : str.toLocal8Bit();
}



template <typename _Func>
int ZrmDatabase::fetch_records(QSqlQuery& query, _Func func)
{
    int count = 0;
    do
    {
        QSqlRecord rec = query.record();
        if (!rec.isEmpty())
        {
            func(rec);
            ++count;
        }
    }
    while (query.next());
    return count;
}


inline bool ZrmDatabase::is_null_id(const QVariant& id)
{
    return id.isNull() || !id.isValid() || !id.toInt() ;
}




#endif // ZRMDATABASE_H
