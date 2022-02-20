#include "zrmmethodexportimport.h"
#include "ui_zrmmethodexportimport.h"

ZrmMethodExportImport::ZrmMethodExportImport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ZrmMethodExportImport)
{
    ui->setupUi(this);
    ui->zrmMethods->setAbstract(true);
}

ZrmMethodExportImport::~ZrmMethodExportImport()
{
    delete ui;
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


