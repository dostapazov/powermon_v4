#ifndef ZRMTABMETHODEDITOR_H
#define ZRMTABMETHODEDITOR_H

#include "zrmbasewidget.h"

#include "ui_ZrmTabMethodEditor.h"

class ZrmTabMethodEditor : public ZrmGroupWidget, private Ui::ZrmTabMethodEditor
{
    Q_OBJECT
 static constexpr int TAB_AKB = 0;
 static constexpr int TAB_METHODS = 1;
 static constexpr int TAB_EXPORT_IMPORT = 2;

public:
    explicit ZrmTabMethodEditor(QWidget *parent = nullptr);

    void setWorkMode(zrm::zrm_work_mode_t mode);
    void save_user_values();
    inline bool isEdit() { return (tabAKB->isEdit() || tabMethods->isEdit()); }

    QList<int> getSplitterSizes();
    void setSplitterSizes(const QList<int> &list);

private slots:
    void editChanged();
    void currentTabChanged(int index);
    void tabBarClicked(int index);
};

#endif // ZRMTABMETHODEDITOR_H
