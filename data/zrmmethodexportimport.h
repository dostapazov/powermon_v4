#ifndef ZRMMETHODEXPORTIMPORT_H
#define ZRMMETHODEXPORTIMPORT_H

#include <QWidget>

namespace Ui {
class ZrmMethodExportImport;
}

class ZrmMethodExportImport : public QWidget
{
    Q_OBJECT

public:
    explicit ZrmMethodExportImport(QWidget *parent = nullptr);
    ~ZrmMethodExportImport();

private:
    Ui::ZrmMethodExportImport *ui;
};

#endif // ZRMMETHODEXPORTIMPORT_H
