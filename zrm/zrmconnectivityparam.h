#ifndef ZRMCONNECTIVITYPARAM_H
#define ZRMCONNECTIVITYPARAM_H

#include <zrm_connectivity.hpp>
#include "ui_zrmconnectivityparam.h"

class ZrmConnectivityParam : public QWidget, private Ui::ZrmConnectivityParam
{
    Q_OBJECT

public:
    enum
    {
        WorkMode = Qt::UserRole,
        BoxNumber,
        DeviceNumber,
        Color
    };

    explicit ZrmConnectivityParam(QWidget* parent = nullptr);
    bool is_select_mode();
    inline QTreeWidgetItem* current_item() { return tw_connectivity->currentItem(); }
    void setCurrentItem(zrm::ZrmConnectivity* conn, uint16_t chan);

signals:
    void configureApply();

protected:
    virtual void showEvent(QShowEvent* event) override;
    virtual void hideEvent(QHideEvent* event ) override;

private slots:
    void on_tw_connectivity_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void on_tw_connectivity_itemChanged(QTreeWidgetItem* item, int column);
    void tool_buttons_clicked();
    void conn_param_apply();
    void conn_param_undo();
    void updateMonitor();

    void on_channel_type_currentIndexChanged(int index);
    void onAttributesChanged();
    void selectChunnelColor();

private:
    void prepare_ui();
    void init_ui();
    void do_connection_start(zrm::ZrmConnectivity* conn, bool start);
    void do_remove_item(QTreeWidgetItem* item, zrm::ZrmConnectivity* conn);
    void do_connection_add();
    void do_channel_add(QTreeWidgetItem* item, zrm::ZrmConnectivity* conn) ;
    void do_connection_set_string(zrm::ZrmConnectivity* conn);

    void update_tool_buttons(QTreeWidgetItem* item);
    void make_connectivity_tree();
    QTreeWidgetItem* create_connectivity_item(zrm::ZrmConnectivity* conn);
    QTreeWidgetItem* create_channel_item(zrm::ZrmConnectivity* conn, QTreeWidgetItem* conn_item, uint16_t channel_number);
    void channel_renumber(QTreeWidgetItem* item);
    void setup_channel(QTreeWidgetItem* item);
    void setButtonColor(QString color);

    static zrm::ZrmConnectivity* connectivity(QTreeWidgetItem* item);
    static uint16_t channel_number(QTreeWidgetItem* item, bool old_number = false);
    static zrm::zrm_work_mode_t channel_work_mode(QTreeWidgetItem* item);
};

#endif // ZRMCONNECTIVITYPARAM_H
