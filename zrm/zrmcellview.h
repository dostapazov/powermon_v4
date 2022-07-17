#ifndef ZRMCELLVIEW_H
#define ZRMCELLVIEW_H

#include "ui_zrmcellview.h"
#include <zrmbasewidget.h>

class ZrmCellView : public ZrmChannelWidget, private Ui::ZrmCellView
{
    Q_OBJECT
public:
    explicit ZrmCellView(QWidget* parent = nullptr);
    void update_params();
    enum ColumnRoles : int {number, volt, temp};

public slots:
    void saveCell();

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
    QColor normalBackground, normalText;
    QColor outBoundBackground, outBoundText;
};

#endif // ZRMCELLVIEW_H
