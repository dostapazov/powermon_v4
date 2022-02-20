#include "zrmmethodexportimport.h"
#include "ui_zrmmethodexportimport.h"

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

void ZrmMethodExportImport::importMethod()
{

}

void ZrmMethodExportImport::exportMethod()
{

    zrm::zrm_method_t method;

    ui->zrmMethods->get_method(method, ZrmBaseWidget::codec());
    QListWidgetItem * item = new QListWidgetItem;
    item->setText(ZrmBaseWidget::codec()->toUnicode(QByteArray(method.m_method.m_name,sizeof(method.m_method.m_name))));
    ui->methodsList->addItem(item);
}



