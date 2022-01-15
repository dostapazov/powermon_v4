/* Get databases for diferent purpose
 * Ostapenko D. V.
 * NIKTES 2019-03-22
 */


#include "zrmdatasource.h"
#include <qdir.h>
#include <qfile.h>
#include <qcoreapplication.h>
#include <qdebug.h>
#include <QMessageBox>
#include <QFileDialog>

ZrmDataSource::data_source_ptr_t ZrmDataSource::data_source ;

const char * ZrmDataSource::charge_methods = "cmethods";
const char * ZrmDataSource::power_methods  = "pmethods";
const char * ZrmDataSource::svc_reports    = "svcreports";


ZrmDataSource::ZrmDataSource(QObject *parent) : QObject(parent)
{
}

ZrmDataSource * ZrmDataSource::instance()
{
  if(data_source.isNull())
     {
      data_source.reset(new ZrmDataSource);
     }
  return data_source.data();
}

// временная функция
// возвращает путь к файлам программы рядом с исполнительным файлом
// если файлы размещены по стандвртным путям, копирует их
QString getPath(QString file, bool bData = false)
{
    Q_UNUSED(file)
    QString path = qApp->applicationDirPath();
    if (bData)
        path += "/data";
    QDir dir(path);
    if (!dir.exists())
        dir.mkdir(path);
    return qApp->applicationDirPath();
}

/**
 * @brief check_and_restore
 * @param conn_str
 * @return full path to data base
 * Проверяет существование файла и восстанавливает его из ресурсов при необходимости
 */

QString check_and_restore(const QString & conn_str)
{
    QString db_path;
#ifdef Q_OS_ANDROID
    db_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    //db_path = qApp->applicationDirPath();
    db_path = getPath(QString("%1.%2").arg(conn_str).arg("db3"), true);
#endif
    //qDebug()<<db_path;
    QDir dir(db_path);
    if(!dir.exists(db_path))
        dir.mkpath(db_path);
    dir.cd(db_path);
    db_path = dir.absolutePath();
    //qDebug()<<db_path;

    QString file_name = QString("%1.%2").arg(conn_str).arg("db3");
    QString src_file = QString(":/data/%1").arg(file_name);
    QString data_dir  = QLatin1String("data");
    if(!dir.exists(data_dir))
        dir.mkdir(data_dir);

    db_path = QString("%1/%2/%3").arg(db_path).arg(data_dir).arg(file_name);
    if (!QFile::exists(db_path))
        QFile::copy(src_file, db_path);

    // права на редактирование базы данных
    QFile f(db_path);
    f.setPermissions(QFileDevice::WriteOther | QFileDevice::WriteGroup | QFileDevice::WriteUser | QFileDevice::WriteOwner
                     | QFileDevice::ReadOther | QFileDevice::ReadGroup | QFileDevice::ReadUser | QFileDevice::ReadOwner);

    return db_path;
}

bool              ZrmDataSource::register_data_base(bool as_charge)
{
   bool ret = true;
    QString conn_str = QLatin1String( as_charge ? charge_methods : power_methods) ;
    if(!QSqlDatabase::contains(conn_str))
    {
      QString db_path =  check_and_restore(conn_str);
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", conn_str);
      db.setDatabaseName(db_path);
      ret = db.open();
      if(!ret) qDebug()<< db.lastError();
    }
  return ret;
}


QSqlDatabase      ZrmDataSource::method_database(bool as_charger)
{
  register_data_base(as_charger);
  return QSqlDatabase::database(QLatin1String(as_charger ? charge_methods : power_methods));
}


QSqlDatabase      ZrmDataSource::reports_database ()
{
   QString conn_str = QLatin1String(svc_reports);
   if(!QSqlDatabase::contains(conn_str))
   {
      QString db_path = check_and_restore(conn_str);
      QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", conn_str);
      db.setDatabaseName(db_path);
      if(!db.open())
        qDebug()<< db.lastError();
   }
  return QSqlDatabase::database(conn_str);
}


QString ZrmDataSource::config_file_name(const QString & kind)
{
#ifdef Q_OS_ANDROID
    QString location = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
#else
    QString location = getPath(QString("%1%2.json").arg(qApp->applicationName()).arg(kind));
#endif
    QDir dir;
    if(!dir.exists(location))
        dir.mkdir(location);
    QString cfg_name = QString("%1/%2%3.json").arg(location).arg(qApp->applicationName()).arg(kind);
    return cfg_name;
}

void ZrmDataSource::unload(bool as_charger)
{
    QString conn_str = QLatin1String(as_charger ? charge_methods : power_methods);
#ifdef Q_OS_ANDROID
    QString db_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    QString db_path = qApp->applicationDirPath();
#endif
    QDir  dir(db_path);
    if(!dir.exists(db_path))
        return;
    QString file_name = QString("%1.%2").arg(conn_str).arg("db3");
    QString data_dir = QLatin1String("data");
    db_path = QString("%1/%2/%3").arg(db_path).arg(data_dir).arg(file_name);
    QString dirSave = QFileDialog::getExistingDirectory(nullptr, tr("Выберите папку для файла"), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirSave.isEmpty())
        return;
    QString newName = QString("%1/%2").arg(dirSave).arg(file_name);
    if (QFile::exists(newName))
    {
        QMessageBox::warning(nullptr, "Внимание!", "Файл уже существует.\nНе удалось выгрузить файл.");
        return;
    }
    bool res = QFile::copy(db_path, newName);
    QMessageBox::information(nullptr, "Выгрузка", res ? QString("Файл %1 успешно выгружен").arg(file_name) : "Не удалось выгрузить файл");
}

void ZrmDataSource::load(bool as_charger)
{
    QString conn_str = QLatin1String(as_charger ? charge_methods : power_methods);
#ifdef Q_OS_ANDROID
    QString db_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    QString db_path = qApp->applicationDirPath();
#endif
    QDir  dir(db_path);
    if(!dir.exists(db_path))
        return;
    QString file_name = QString("%1.%2").arg(conn_str).arg("db3");
    QString data_dir = QLatin1String("data");
    db_path = QString("%1/%2/%3").arg(db_path).arg(data_dir).arg(file_name);
    QString fileLoad = QFileDialog::getOpenFileName(nullptr, tr("Выберите файл с методами"), QString(), "DB (*.db3)");
    if (fileLoad.isEmpty())
        return;
    if(QSqlDatabase::contains(conn_str))
        QSqlDatabase::removeDatabase(conn_str);
    QFile::remove(db_path);
    bool res = QFile::copy(fileLoad, db_path);
    QMessageBox::information(nullptr, "Загрузка", res ? QString("Файл %1 успешно загружен").arg(file_name) : "Не удалось загрузить файл");
    register_data_base(as_charger);
}
