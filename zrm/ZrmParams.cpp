#include "ZrmParams.h"

ZrmParams::ZrmParams(QWidget* parent) :
    ZrmGroupWidget(parent)
{
    setupUi(this);
    setSplitterSizes(QList<int>() << 10000 << 10000 << 10000 << 10000);

    connect(splitter, &QSplitter::splitterMoved, this, [this]() { splitterSizes[0] = splitter->sizes().constFirst(); splitterSizes[1] = splitter->sizes().constLast(); });
    connect(splitter_2, &QSplitter::splitterMoved, this, [this]() { splitterSizes[2] = splitter_2->sizes().constFirst(); splitterSizes[3] = splitter_2->sizes().constLast(); });
}

QList<int> ZrmParams::getSplitterSizes()
{
    return  splitterSizes;
}

void ZrmParams::setSplitterSizes(const QList<int>& list)
{
    if (list.size() < 4)
        return;
    splitter->setSizes(QList<int>() << list[0] << list[1]);
    splitter_2->setSizes(QList<int>() << list[2] << list[3]);

    splitterSizes.clear();
    splitterSizes << list[0] << list[1] << list[2] << list[3];
}
