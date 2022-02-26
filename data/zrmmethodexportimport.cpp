#include "zrmmethodexportimport.h"
#include "ui_zrmmethodexportimport.h"
#include <QFileDialog>
#include <methodjsonconverter.h>

ZrmMethodExportImport::ZrmMethodExportImport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ZrmMethodExportImport)
{
    ui->setupUi(this);
    ui->zrmMethods->setAbstract(true);
    initSlost();
}


ZrmMethodExportImport::~ZrmMethodExportImport()
{
    delete ui;
}

void ZrmMethodExportImport::initSlost()
{
  connect(ui->tbExport, &QAbstractButton::clicked,this, &ZrmMethodExportImport::exportMethod);
  connect(ui->tbImport, &QAbstractButton::clicked,this, &ZrmMethodExportImport::importMethod);
  connect(ui->bSelectPath, &QAbstractButton::clicked, this, &ZrmMethodExportImport::selectFolder);
  connect(ui->pathToFolder, &QLineEdit::textChanged, this, &ZrmMethodExportImport::folderChanged);
}


void ZrmMethodExportImport::setWorkMode(zrm::zrm_work_mode_t mode)
{
 ui->zrmMethods->setWorkMode(mode);
}

zrm::zrm_work_mode_t ZrmMethodExportImport::getWorkMode()
{
  return ui->zrmMethods->getWorkMode();
}

bool ZrmMethodExportImport::open_db()
{
 return ui->zrmMethods->open_database();
}

void ZrmMethodExportImport::close_db()
{
 ui->zrmMethods->close_database();
}

void ZrmMethodExportImport::folderChanged(const QString & folder)
{
  ui->methodsList->clear();
  ui->tbExport->setDisabled(folder.isEmpty());
  scanFolder(folder);
}

void ZrmMethodExportImport::importMethod()
{
}

void ZrmMethodExportImport::exportMethod()
{
    zrm::zrm_method_t method;
    ui->zrmMethods->get_method(method, ZrmBaseWidget::codec());
    QListWidgetItem * item = new QListWidgetItem;
    QString methodName = ZrmBaseWidget::codec()->toUnicode(QByteArray(method.m_method.m_name,method.m_method.name_length()));
    item->setText(methodName);
    item->setData(Qt::UserRole,method.m_method.m_id);
    ui->methodsList->addItem(item);
}

void ZrmMethodExportImport::selectFolder()
{
    QString folder = QFileDialog::getExistingDirectory(this,"Выбор каталога");
    if(!folder.isEmpty())
    {
        ui->pathToFolder->setText(folder);
    }
}

QString ZrmMethodExportImport::getMethodFileName(const QString & name, zrm::zrm_work_mode_t mode)
{
    return name + ((mode == zrm::zrm_work_mode_t::as_charger) ? CHARGE_EXTENSION : POWER_EXTENSION);
}

void ZrmMethodExportImport::scanFolder(const QString & folderName)
{
 ui->methodsList->clear();
    QDir dir(folderName);
 dir.setNameFilters(QStringList()<<getMethodFileName("*",ui->zrmMethods->opened_as()));
 for( const QString & fileName : dir.entryList(QDir::Filter::Files|QDir::Filter::Readable))
 {
   addMethodToList(dir.absoluteFilePath(fileName));
 }
}

void ZrmMethodExportImport::addMethodToList(const QString & fileName)
{
  QFileInfo fInfo(fileName);
  QListWidgetItem * item = new QListWidgetItem;
  item->setText(fInfo.baseName());
  item->setData(FILE_NAME_ROLE,fileName);
  ui->methodsList->addItem(item);
}



