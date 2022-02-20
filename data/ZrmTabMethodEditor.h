#ifndef ZRMTABMETHODEDITOR_H
#define ZRMTABMETHODEDITOR_H

#include "zrmbasewidget.h"

#include "ui_ZrmTabMethodEditor.h"

class ZrmTabMethodEditor : public ZrmGroupWidget, private Ui::ZrmTabMethodEditor
{
    Q_OBJECT

public:
    explicit ZrmTabMethodEditor(QWidget *parent = nullptr);

    void open_db(zrm::zrm_work_mode_t mode);
    void save_user_values();
    inline bool isEdit() { return (tabAKB->isEdit() || tabMethods->isEdit()); }

    QList<int> getSplitterSizes();
    void setSplitterSizes(const QList<int> &list);

private slots:
    void editChanged();
    void on_tabWidget_currentChanged(int index);
};

#endif // ZRMTABMETHODEDITOR_H
