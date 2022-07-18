#ifndef ZRMMETHODEXPORTIMPORT_H
#define ZRMMETHODEXPORTIMPORT_H

#include <QWidget>
#include <zrmproto.hpp>
#include <methodjsonconverter.h>
#include <zrmbasewidget.h>
#include <zrmmethodstree.h>
#include <QListWidgetItem>
#include <QTreeWidgetItem>

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
    explicit ZrmMethodExportImport(QWidget* parent = nullptr);
    ~ZrmMethodExportImport();
    void setWorkMode(zrm::zrm_work_mode_t mode);
    void setMethodsTree(ZrmMethodsTree* mTree);
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
    void scanFolder(const QString& folder);
    void rightMethodSelected();
private:
    void initSlost();
    zrm::zrm_method_t readMethod(const QString& fileName);
    QVariant getCurrentMethodId();

    bool writeMethodToDatabase(QVariant& mId,
                               const QString& methodName,
                               zrm::zrm_method_t& method
                              );


    /**
     * @brief getMethodFileName replace symbols \, / to %char_number%
     * @param name
     * @return decorated file name
     */
    QString getMethodFileName(const QString& name);
    /**
     * @brief getMethodNameFromFileName replace %char_number% back to \ or / symbols
     * @param fileName
     * @return method name
     */
    QString getMethodNameFromFileName(const QString& fileName);
    /**
     * @brief addMethodToList append fileName to methodList
     * @param fileName - fullNamePath
     * @param methodId - database method id
     */

    /**
     * @brief colorMarkMethod mark color item
     * when imported item and not exist then color is darkBlue
     * when imported and  exists then color is darkRed
     * when exported then darkGreen
     * @param item
     */
    static constexpr Qt::GlobalColor ColorImportedNotExists = Qt::GlobalColor::darkBlue;
    static constexpr Qt::GlobalColor ColorImportedExists = Qt::GlobalColor::darkRed;
    static constexpr Qt::GlobalColor ColorExported = Qt::GlobalColor::darkGreen;
    void colorMarkMethod(QListWidgetItem* item);
    void addMethodToList(const QString& fileName, const QVariant& mId = QVariant());
    void initMethodLegend();
    static IMethodConverter* getConverter();
    Ui::ZrmMethodExportImport* ui;
    ZrmMethodsTree* zrmMethods = nullptr;
    static constexpr const char* CHARGE_EXTENSION = ".cmt";
    static constexpr const char* POWER_EXTENSION = ".pmt";
    static constexpr int METHOD_ID_ROLE = Qt::UserRole;
    static constexpr int FILE_NAME_ROLE = Qt::UserRole + 1;
    static constexpr char slash = '/';
    static constexpr char back_slash = '\\';
    QString getNewMethodName(const QString& name);
};

inline void ZrmMethodExportImport::setMethodsTree(ZrmMethodsTree* mTree)
{
    zrmMethods = mTree;
}

#endif // ZRMMETHODEXPORTIMPORT_H
