#include "zrmmethodexportimport.h"
#include "ui_zrmmethodexportimport.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <zrmdatabase.h>



ZrmMethodExportImport::ZrmMethodExportImport(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ZrmMethodExportImport)
{
    ui->setupUi(this);
    setMethodsTree(ui->zrmMethods);
    zrmMethods->setAbstract(true);
    zrmMethods->show_method_params(false);
    initMethodLegend();
    initSlost();

}


ZrmMethodExportImport::~ZrmMethodExportImport()
{
    delete ui;
}

void ZrmMethodExportImport::initMethodLegend()
{
    const char* color_templ = "color: rgb(%u, %u, %u)";
    QColor color ;
    color = ColorExported;
    ui->exported->setStyleSheet(QString::asprintf(color_templ, color.red(), color.green(), color.blue()));
    color = ColorImportedExists;
    ui->importedExists->setStyleSheet(QString::asprintf(color_templ, color.red(), color.green(), color.blue()));
    color = ColorImportedNotExists;
    ui->importedNotExists->setStyleSheet(QString::asprintf(color_templ, color.red(), color.green(), color.blue()));
}

void ZrmMethodExportImport::initSlost()
{
    connect(ui->tbExport, &QAbstractButton::clicked, this, &ZrmMethodExportImport::exportMethod);
    connect(ui->tbImport, &QAbstractButton::clicked, this, &ZrmMethodExportImport::importMethod);
    connect(ui->bSelectPath, &QAbstractButton::clicked, this, &ZrmMethodExportImport::selectFolder);
    connect(ui->pathToFolder, &QLineEdit::textChanged, this, &ZrmMethodExportImport::scanFolder);
    connect(ui->methodsList, &QListWidget::itemSelectionChanged, this, &ZrmMethodExportImport::rightMethodSelected);
}


void ZrmMethodExportImport::setWorkMode(zrm::zrm_work_mode_t mode)
{
    zrmMethods->setWorkMode(mode);
}

zrm::zrm_work_mode_t ZrmMethodExportImport::getWorkMode()
{
    return zrmMethods->getWorkMode();
}

bool ZrmMethodExportImport::open_db()
{
    return zrmMethods->open_database();
}

void ZrmMethodExportImport::close_db()
{
    zrmMethods->close_database();
}


void ZrmMethodExportImport::scanFolder(const QString& folder)
{
    ui->methodsList->clear();
    ui->tbExport->setDisabled(folder.isEmpty());
    QDir dir(folder);
    dir.setNameFilters(QStringList() << getMethodFileName("*"));
    for ( auto&& fileName : dir.entryList(QDir::Filter::Files | QDir::Filter::Readable))
    {
        addMethodToList(dir.absoluteFilePath(fileName));
    }

}

zrm::zrm_method_t ZrmMethodExportImport::readMethod(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return zrm::zrm_method_t();
    return getConverter()->fromByteArray(file.readAll());
}

bool ZrmMethodExportImport::writeMethodToDatabase
(
    QVariant& mId,
    const QString& methodName,
    zrm::zrm_method_t& method
)
{
    QSqlDatabase& db = zrmMethods->database();

    if (!ZrmDatabase::start_transaction(db))
        return false;


    if ( ZrmDatabase::write_method(db, mId, methodName, method))
    {
        method.m_method.m_id = static_cast<uint16_t>(mId.toUInt());

        for (zrm::stage_t& st : method.m_stages )
            st.m_method_id = 0;

        zrm::stages_t _del_stages;
        if ( ZrmDatabase::write_stages(db, mId, method.m_stages, _del_stages)  )
        {
            return ZrmDatabase::commit_transaction(db);
        }
    }
    ZrmDatabase::rollback_transaction(db);
    return false;
}

QString ZrmMethodExportImport::getNewMethodName(const QString& name)
{
    QScopedPointer<QInputDialog> idlg( new QInputDialog (this));
    idlg->setInputMode(QInputDialog::InputMode::TextInput);
    idlg->setTextEchoMode(QLineEdit::EchoMode::Normal);
    idlg->setLabelText(tr("Имя"));
    idlg->setWindowTitle(tr("Имя нового метода"));
    idlg->setTextValue(name.trimmed() + tr("_импорт"));

    idlg->show();
    QRect r = idlg->geometry();
    int newWidth = r.width() * 2;
    int toLeft = (newWidth - r.width()) / 2;
    r.setWidth(newWidth);
    r.moveLeft(r.left() - toLeft);
    idlg->setGeometry(r);

    QString newName;
    do
    {
        if (idlg->exec() == QDialog::Rejected)
            return QString();
        newName = idlg->textValue().trimmed();
    }
    while (newName == name.trimmed());
    return newName;
}

void ZrmMethodExportImport::importMethod()
{
    QListWidgetItem* item = ui->methodsList->currentItem();
    QString methodName = item->text();
    QString fileName = item->data(FILE_NAME_ROLE).toString();
    zrm::zrm_method_t method = readMethod(fileName);
    QTreeWidgetItem* dest  = zrmMethods->search_method_by_name(methodName);
    QVariant mId;
    if (dest)
    {
        int answer = QMessageBox::question
                     ( this,
                       tr("Импорт метода"),
                       tr("Метод с таким именем существует.\n""Что делать?"),
                       tr("Отменить"),
                       tr("Заменить"),
                       tr("Создать новый")
                     );

        if (QMessageBox::AcceptRole == answer)
            return;

        if (QMessageBox::DestructiveRole == answer)
        {
            //Создание нового
            methodName = getNewMethodName(methodName);
            if (methodName.isEmpty())
                return;
        }
        else
        {
            mId = dest->data(ZrmMethodsTree::column_name, ZrmMethodsTree::role_id );
        }
    }
    if (writeMethodToDatabase(mId, methodName, method))
    {
        item->setText(methodName);
        item->setData(METHOD_ID_ROLE, mId);
        colorMarkMethod(item);
        zrmMethods->open_database();
    }
}

void ZrmMethodExportImport::exportMethod()
{
    IMethodConverter* converter = getConverter();
    if (converter)
    {
        zrm::zrm_method_t method;
        zrmMethods->get_method(method, ZrmBaseWidget::codec());
        QString methodName = ZrmBaseWidget::codec()->toUnicode(QByteArray(method.m_method.m_name, method.m_method.name_length()));

        QByteArray data = converter->toByteArray(method);
        if (data.size())
        {
            QDir dir(ui->pathToFolder->text());
            QString fileName = dir.absoluteFilePath(getMethodFileName(methodName));
            QFile file (fileName);
            if (file.open(QFile::OpenModeFlag::WriteOnly))
            {
                file.resize(0);
                file.write(data);
                file.close();
                addMethodToList(fileName, method.m_method.m_id);
            }
        }
    }
}

void ZrmMethodExportImport::selectFolder()
{
    QString folder = QFileDialog::getExistingDirectory(this, "Выбор каталога");
    if (!folder.isEmpty())
    {
        ui->pathToFolder->setText(folder);
    }
}

QString ZrmMethodExportImport::getMethodFileName(const QString& name)
{
    QString decoratedName = name;
    constexpr const char* templ = "\%%1\%";
    decoratedName.replace(QChar(slash), QString(templ).arg(int(slash)));
    decoratedName.replace(QChar(back_slash), QString(templ).arg(int(back_slash)));
    return decoratedName + ((zrmMethods->opened_as() == zrm::zrm_work_mode_t::as_charger) ? CHARGE_EXTENSION : POWER_EXTENSION);
}

QString ZrmMethodExportImport::getMethodNameFromFileName(const QString& fileName)
{
    QString methodName;
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    auto splitMode = QString::SplitBehavior::SkipEmptyParts;
#else
    auto splitMode = Qt::SplitBehaviorFlags::SkipEmptyParts;
#endif
    QStringList sl = fileName.split(QChar('%'), splitMode);
    for ( QString& text : sl)
    {
        bool isNumber(false);
        QChar ch = QChar::fromLatin1(char(text.toInt(&isNumber)));
        if (isNumber && (ch == QChar(slash) || ch == QChar(back_slash)))
            methodName  += ch;
        else
            methodName += text;
    }
    return methodName;
}

void ZrmMethodExportImport::colorMarkMethod(QListWidgetItem* item)
{
    QVariant mId = item->data(METHOD_ID_ROLE);
    Qt::GlobalColor color;
    if (!mId.isValid())
    {
        color = zrmMethods->search_method_by_name(item->data(Qt::DisplayRole).toString()) ?
                ColorImportedExists : ColorImportedNotExists;
    }
    else
    {
        color = ColorExported;
    }

    item->setData(Qt::ForegroundRole, QBrush(QColor(color)));
}

void ZrmMethodExportImport::addMethodToList(const QString& fileName, const QVariant& mId )
{
    QFileInfo fInfo(fileName);
    QListWidgetItem* item = new QListWidgetItem;
    item->setText(getMethodNameFromFileName(fInfo.completeBaseName()));
    item->setData(FILE_NAME_ROLE, fileName);
    item->setData(METHOD_ID_ROLE, mId);
    colorMarkMethod(item);

    ui->methodsList->addItem(item);
}

QVariant ZrmMethodExportImport::getCurrentMethodId()
{
    QListWidgetItem* item =  ui->methodsList->currentItem();
    return item ? item->data(METHOD_ID_ROLE) : QVariant();
}

void ZrmMethodExportImport::rightMethodSelected()
{
    QVariant mId = getCurrentMethodId();
    ui->tbImport->setEnabled(!mId.isValid());
}

IMethodConverter* ZrmMethodExportImport::getConverter()
{
    static  MethodJsonConverter jcvt;
    return &jcvt;
}



