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
    /**
     * @brief scanFolder make methods list
     * @param folder path to folder
     */
    void scanFolder(const QString & folder);
    void rightMethodSelected();
private:
    void initSlost();

    /**
     * @brief getMethodFileName replace symbols \, / to %char_number%
     * @param name
     * @return decorated file name
     */
    QString getMethodFileName(const QString & name);
    /**
     * @brief getMethodNameFromFileName replace %char_number% back to \ or / symbols
     * @param fileName
     * @return method name
     */
    QString getMethodNameFromFileName(const QString & fileName);
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
    static constexpr char slash = '/';
    static constexpr char back_slash = '\\';


};

#endif // ZRMMETHODEXPORTIMPORT_H
