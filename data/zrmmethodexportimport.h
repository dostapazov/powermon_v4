#ifndef ZRMMETHODEXPORTIMPORT_H
#define ZRMMETHODEXPORTIMPORT_H

#include <QWidget>
#include <zrmproto.hpp>
#include <methodjsonconverter.h>
#include <zrmbasewidget.h>

namespace Ui {
class ZrmMethodExportImport;
}

/**
 * @brief The ZrmMethodExportImport class
 * implement Import/Export methods
 * QListWidgetItem UserRole hold  method_id as QVariant
 * when method_id isNull it is sign that metod not contais in database
 */

class ZrmMethodExportImport : public QWidget
{
    Q_OBJECT

public:
    explicit ZrmMethodExportImport(QWidget *parent = nullptr);
    ~ZrmMethodExportImport();
    void setWorkMode(zrm::zrm_work_mode_t mode);
    zrm::zrm_work_mode_t getWorkMode();
    bool open_db();
    void close_db();
private slots :
    void importMethod();
    void exportMethod();
    void selectFolder();
    void folderChanged(const QString & folder);
private:
    void initSlost();
    void scanFolder(const QString & folderName);
    QString getMethodFileName(const QString & name);
    /**
     * @brief addMethodToList append fileName to methodList
     * @param fileName - fullNamePath
     * @param methodId - database method id
     */
    void addMethodToList(const QString & fileName, const QVariant & mId = QVariant());
    static IMethodConverter * getConverter();
    Ui::ZrmMethodExportImport *ui;
    static constexpr const char * CHARGE_EXTENSION = ".cmt";
    static constexpr const char * POWER_EXTENSION = ".pmt";
    static constexpr int METHOD_ID_ROLE = Qt::UserRole;
    static constexpr int FILE_NAME_ROLE = Qt::UserRole+1;

};

#endif // ZRMMETHODEXPORTIMPORT_H
