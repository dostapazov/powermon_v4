#ifndef ZRMCELLVIEWMINIMAL_H
#define ZRMCELLVIEWMINIMAL_H

#include "ui_zrmcellviewminimal.h"
#include <zrmbasewidget.h>

class ZrmCellViewMinimal : public ZrmChannelWidget, private Ui::ZrmCellViewMinimal
{
    Q_OBJECT
public:
    explicit ZrmCellViewMinimal(QWidget* parent = nullptr);
    enum ColumnRoles : int {number, volt, temp};
    int getCellsCount() const;
    QSize sizeHint() const override;


protected:
    virtual void resizeEvent(QResizeEvent* re) override;
    virtual void showEvent(QShowEvent*    se) override;
    virtual void channel_param_changed(unsigned channel, const zrm::params_list_t& params_list  ) override;
    virtual void update_controls() override;
    virtual void clear_controls() override;
    virtual void channel_session(unsigned ch_num) override;
private:
    void initCellsTree();
    void cellsCount(uint16_t ccnt);
    void cellsParam() ;
    void updateColumnWidth() ;
//    QColor normalBackground, normalText;
//    QColor outBoundBackground, outBoundText;
};

#endif // ZRMCELLVIEWMINIMAL_H
