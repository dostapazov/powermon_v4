#include "zrmmethodexportimport.h"
#include "ui_zrmmethodexportimport.h"

ZrmMethodExportImport::ZrmMethodExportImport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ZrmMethodExportImport)
{
    ui->setupUi(this);
}

ZrmMethodExportImport::~ZrmMethodExportImport()
{
    delete ui;
}
