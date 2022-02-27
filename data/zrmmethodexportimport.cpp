#include "zrmmethodexportimport.h"
#include "ui_zrmmethodexportimport.h"
#include <QFileDialog>


ZrmMethodExportImport::ZrmMethodExportImport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ZrmMethodExportImport)
{
    ui->setupUi(this);
    ui->zrmMethods->setAbstract(true);
    ui->zrmMethods->show_method_params(false);
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
  connect(ui->pathToFolder, &QLineEdit::textChanged, this, &ZrmMethodExportImport::scanFolder);
  connect(ui->methodsList, &QListWidget::itemSelectionChanged, this, &ZrmMethodExportImport::rightMethodSelected);
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


void ZrmMethodExportImport::scanFolder(const QString & folder)
{
  ui->methodsList->clear();
  ui->tbExport->setDisabled(folder.isEmpty());
  QDir dir(folder);
  dir.setNameFilters(QStringList()<<getMethodFileName("*"));
  for( const QString & fileName : dir.entryList(QDir::Filter::Files|QDir::Filter::Readable))
  {
    addMethodToList(dir.absoluteFilePath(fileName));
  }

}

void ZrmMethodExportImport::rightMethodSelected()
{

  QListWidgetItem * item = ui->methodsList->currentItem();
  QVariant id = item->data(METHOD_ID_ROLE);
  if(id.isValid())
  {
    ui->tbOverwrite->setEnabled(true);
    ui->tbImport->setEnabled(false);

  }
  else
  {
      ui->tbOverwrite->setEnabled(false);
      ui->tbImport->setEnabled(true);

  }

}


void ZrmMethodExportImport::importMethod()
{
    QListWidgetItem * item = ui->methodsList->currentItem();
    QVariant id = item->data(METHOD_ID_ROLE);
    QString methodName = item->text();
    QString fileName = item->data(FILE_NAME_ROLE).toString();
    qDebug()<<id << fileName;
    QList<QTreeWidgetItem*> list = ui->zrmMethods->findItems(methodName,Qt::MatchFlag::MatchExactly );

    if(list.size())
    {

    }
}

void ZrmMethodExportImport::exportMethod()
{
    IMethodConverter * converter = getConverter();
    if(converter)
    {
        zrm::zrm_method_t method;
        ui->zrmMethods->get_method(method, ZrmBaseWidget::codec());
        QString methodName = ZrmBaseWidget::codec()->toUnicode(QByteArray(method.m_method.m_name,method.m_method.name_length()));

        QByteArray data = converter->toByteArray(method);
        if(data.size())
        {
            QDir dir(ui->pathToFolder->text());
            QString fileName = dir.absoluteFilePath(getMethodFileName(methodName));
            QFile file (fileName);
            if(file.open(QFile::OpenModeFlag::WriteOnly))
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
    QString folder = QFileDialog::getExistingDirectory(this,"Выбор каталога");
    if(!folder.isEmpty())
    {
        ui->pathToFolder->setText(folder);
    }
}

QString ZrmMethodExportImport::getMethodFileName(const QString & name)
{
    QString decoratedName = name;
    constexpr const char * templ = "\%%1\%";
    decoratedName.replace(QChar(slash),QString(templ).arg(int(slash)));
    decoratedName.replace(QChar(back_slash),QString(templ).arg(int(back_slash)));
    return decoratedName + ((ui->zrmMethods->opened_as() == zrm::zrm_work_mode_t::as_charger) ? CHARGE_EXTENSION : POWER_EXTENSION);
}

QString ZrmMethodExportImport::getMethodNameFromFilrName(const QString & fileName)
{
    QString methodName;
    QStringList sl = fileName.split(QChar('%'),Qt::SplitBehaviorFlags::SkipEmptyParts);
    for(const QString & text : sl)
    {
        bool isNumber(false);
        QChar ch = QChar::fromLatin1(char(text.toInt(&isNumber)));
        if(isNumber && (ch == QChar(slash) || ch == QChar(back_slash)))
            methodName  += ch;
        else
            methodName += text;
    }
    return methodName;
}



void ZrmMethodExportImport::addMethodToList(const QString & fileName, const QVariant & mId )
{
  QFileInfo fInfo(fileName);
  QListWidgetItem * item = new QListWidgetItem;
  item->setText(getMethodNameFromFilrName(fInfo.baseName()));
  item->setData(FILE_NAME_ROLE,fileName);
  item->setData(METHOD_ID_ROLE,mId);
  ui->methodsList->addItem(item);
}

IMethodConverter * ZrmMethodExportImport::getConverter()
{
  static  MethodJsonConverter jcvt;
  return &jcvt;
}



