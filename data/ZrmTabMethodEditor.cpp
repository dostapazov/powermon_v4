#include "ZrmTabMethodEditor.h"

ZrmTabMethodEditor::ZrmTabMethodEditor(QWidget *parent) :
    ZrmGroupWidget(parent)
{
    setupUi(this);
    setSplitterSizes(QList<int>() << 10000 << 10000);

    connect(tabAKB, SIGNAL(editChanged(bool)), this, SLOT(editChanged()));
    connect(tabMethods, SIGNAL(editChanged(bool)), this, SLOT(editChanged()));
}

void ZrmTabMethodEditor::open_db(zrm::zrm_work_mode_t as_charger)
{
    tabAKB->open_db(as_charger, false);
    tabMethods->open_db(as_charger, true);
//    tabAKB->setAllMethods(false);
//    tabMethods->setAllMethods(true);
}

void ZrmTabMethodEditor::save_user_values()
{
    tabAKB->save_user_values();
    tabMethods->save_user_values();
}

QList<int> ZrmTabMethodEditor::getSplitterSizes()
{
    return  tabMethods->getSplitterSizes();
}

void ZrmTabMethodEditor::setSplitterSizes(const QList<int> &list)
{
    tabAKB->setSplitterSizes(list);
    tabMethods->setSplitterSizes(list);
}

void ZrmTabMethodEditor::editChanged()
{
    if (0 != tabWidget->currentIndex())
        tabWidget->setTabEnabled(0, !isEdit());
    if (1 != tabWidget->currentIndex())
        tabWidget->setTabEnabled(1, !isEdit());
}

void ZrmTabMethodEditor::on_tabWidget_currentChanged(int index)
{
    if (0 == index)
    {
        tabMethods->save_user_values();
        tabAKB->refresh();
    }
    if (1 == index)
    {
        tabAKB->save_user_values();
        tabMethods->refresh();
    }
}
