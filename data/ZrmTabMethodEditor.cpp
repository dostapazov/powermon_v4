#include "ZrmTabMethodEditor.h"

ZrmTabMethodEditor::ZrmTabMethodEditor(QWidget* parent) :
    ZrmGroupWidget(parent)
{
    setupUi(this);
    setSplitterSizes(QList<int>() << 10000 << 10000);
    connect(tabAKB, SIGNAL(editChanged(bool)), this, SLOT(editChanged()));
    connect(tabMethods, SIGNAL(editChanged(bool)), this, SLOT(editChanged()));
    connect(tabWidget, &QTabWidget::currentChanged, this, &ZrmTabMethodEditor::currentTabChanged);
    connect(tabWidget, &QTabWidget::tabBarClicked, this, &ZrmTabMethodEditor::tabBarClicked);
    tabAKB->setAbstract(false);
    tabMethods->setAbstract(true);
    tabWidget->setCurrentIndex(TAB_AKB);
}

void ZrmTabMethodEditor::setWorkMode(zrm::zrm_work_mode_t mode)
{
    tabAKB->setWorkMode(mode);
    tabMethods->setWorkMode(mode);
    tabExportImport->setWorkMode(mode);

//    tabAKB->open_db();
//    tabMethods->open_db();
//    tabExportImport->open_db();
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

void ZrmTabMethodEditor::setSplitterSizes(const QList<int>& list)
{
    tabAKB->setSplitterSizes(list);
    tabMethods->setSplitterSizes(list);
}

void ZrmTabMethodEditor::editChanged()
{
//    if (0 != tabWidget->currentIndex())
//        tabWidget->setTabEnabled(0, !isEdit());
//    if (1 != tabWidget->currentIndex())
//        tabWidget->setTabEnabled(1, !isEdit());
}

void ZrmTabMethodEditor::currentTabChanged(int index)
{
    qDebug() << Q_FUNC_INFO;
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

    switch (index)
    {
        case TAB_AKB:
            tabAKB->open_db();
            break;
        case TAB_METHODS:
            tabMethods->open_db();
            break;
        case TAB_EXPORT_IMPORT:
            tabExportImport->open_db();
            break;
        default:
            break;
    }

}

void ZrmTabMethodEditor::tabBarClicked(int index)
{
    int currentIndex = tabWidget->currentIndex();
    qDebug() << Q_FUNC_INFO << " index " << index << "old index " << currentIndex ;
    if (index == currentIndex)
        return;

    switch (tabWidget->currentIndex())
    {
        case TAB_AKB:
            tabAKB->save_user_values();
            tabAKB->close_db();
            break;
        case TAB_METHODS:
            tabMethods->save_user_values();
            tabMethods->close_db();
            break;
        case TAB_EXPORT_IMPORT:
            tabExportImport->close_db();
            break;
        default:
            break;
    }
}

