#ifndef ZRMMETHODEXPORTIMPORT_H
#define ZRMMETHODEXPORTIMPORT_H

#include <QWidget>
#include <zrm_connectivity.hpp>
#include <zrmbasewidget.h>

namespace Ui {
class ZrmMethodExportImport;
}

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
private:
    void initSlost();
    Ui::ZrmMethodExportImport *ui;
};

#endif // ZRMMETHODEXPORTIMPORT_H
