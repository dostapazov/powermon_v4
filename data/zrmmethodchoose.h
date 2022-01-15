/* NIKTES 2019-03-22
 * Choose method dialog
 *
 */

#ifndef ZRMMETHODCHOOSE_H
#define ZRMMETHODCHOOSE_H

#include "ui_zrmmethodchoose.h"

class ZrmMethodChoose : public QDialog, private Ui::ZrmMethodChoose
{
    Q_OBJECT

public:
    explicit ZrmMethodChoose(QWidget *parent = nullptr);
    ~ZrmMethodChoose() override;
    void set_mode(zrm::zrm_work_mode_t value);
    bool get_method(zrm::zrm_method_t & zrm_method, QTextCodec * codec, QString * model_name = nullptr);
    virtual int exec() override;
    bool open_database(bool remake_tree = false);
    void close_database();
    void setAbstract(bool a);

private slots:
    void sl_method_selected(QTreeWidgetItem * item);
    void editVoltage();
    void editCapacity();

private:
    zrm::zrm_work_mode_t m_work_mode = zrm::as_charger;
    QTreeWidgetItem * m_current_method = nullptr;
    bool bAbstract = false;
};

#endif // ZRMMETHODCHOOSE_H
