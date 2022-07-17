#ifndef ZRMCELLVIEW_H
#define ZRMCELLVIEW_H

#include "ui_zrmcellview.h"
#include <zrmbasewidget.h>

class ZrmCellView : public ZrmChannelWidget, private Ui::ZrmCellView
{
    Q_OBJECT
public:
    explicit ZrmCellView(QWidget *parent = nullptr);
    void update_params();

public slots:
    void saveCell();

protected:
    virtual void resizeEvent(QResizeEvent * re) override;
    virtual void showEvent(QShowEvent   * se) override;
    virtual void channel_param_changed(unsigned channel, const zrm::params_list_t & params_list  ) override;
    virtual void update_controls() override;
    virtual void clear_controls() override;
    virtual void channel_session(unsigned ch_num) override;
            void update_column_width() ;
            void cell_count(uint16_t ccnt) ;
            void cell_params(uint16_t value) ;
            void create_cell_item(int row, int col);
};

#endif // ZRMCELLVIEW_H
