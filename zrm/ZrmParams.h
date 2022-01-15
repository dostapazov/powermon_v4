#ifndef ZRMPARAMS_H
#define ZRMPARAMS_H

#include "zrmbasewidget.h"

#include "ui_ZrmParams.h"

class ZrmParams : public ZrmGroupWidget, private Ui::ZrmParams
{
    Q_OBJECT

public:
    explicit ZrmParams(QWidget *parent = nullptr);

    QList<int> getSplitterSizes();
    void setSplitterSizes(const QList<int> &list);

private:
    QList<int> splitterSizes;
};

#endif // ZRMPARAMS_H
