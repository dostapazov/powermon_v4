#ifndef ZRMMETHODINFO_H
#define ZRMMETHODINFO_H

#include "ui_zrmmethodminmax.h"
#include <zrmproto.hpp>

class ZrmMethodMinmax : public QWidget, private Ui::ZrmMethodMinmax
{
    Q_OBJECT

public:
    explicit ZrmMethodMinmax(QWidget *parent = nullptr);
    void     method_minmax(const zrm::zrm_method_t & method);
    void     clear_controls() ;
protected:
    using qvarvect = std::vector<QVariant>  ;
    using mmidx    = std::tuple<int,int>;

    void     set_minmax_values(const zrm::zrm_method_t & met, const qvarvect & vecU, const qvarvect & vecI );
    static   mmidx get_minmax_idx(const qvarvect &vec);
};

#endif // ZRMMETHODINFO_H
