#include "zrmconnectivityparam.h"
#include <qevent.h>
#include <qitemdelegate.h>
#include <qscreen.h>
#include <signal_bloker.hpp>
#include <QColorDialog>

constexpr int old_channel_number = Qt::UserRole + 1;
constexpr const char* const tb_func_id = "tb_func_id";

/* числовой идентификатор кнопки на tool_frame */
enum tb_functions_t { tbf_unknown, tbf_add_connection, tbf_add_channel, tbf_remove };

class zcp_item_delegate : public QItemDelegate
{
public:
    explicit zcp_item_delegate(ZrmConnectivityParam* _zcp): QItemDelegate(_zcp), zcp(_zcp)
    {}
    virtual QWidget* createEditor(QWidget* parent,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const Q_DECL_OVERRIDE;

    void setEditorData(QWidget* editor, const QModelIndex& index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const Q_DECL_OVERRIDE;

private:
    ZrmConnectivityParam* zcp   = Q_NULLPTR;

};


ZrmConnectivityParam::ZrmConnectivityParam(QWidget* parent) :
    QWidget(parent)
{
    setupUi(this);
    prepare_ui();
    init_ui   ();
    ZrmBaseWidget::setWidgetsShadow<QToolButton>(tool_frame, 6, 6);
    ZrmBaseWidget::setWidgetsShadow<QToolButton>(zrm_mon, 6, 6);
    ZrmBaseWidget::setWidgetsShadow<QPushButton>(conn_params, 6, 6);

    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(1, 3);
    conn_params->interface_enable(QMultiIODev::udp, false);
    auto hdr = tw_connectivity->header();

    hdr->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
    hdr->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
    connect(conn_params, &mutli_iodev_params::param_apply, this, &ZrmConnectivityParam::conn_param_apply);
    connect(conn_params, &mutli_iodev_params::param_undo, this, &ZrmConnectivityParam::conn_param_undo );
    conn_params->enable_apply(true);
    conn_params->enable_undo (true);



    int i = 0;
    for (auto&& tb : tool_frame->findChildren<QToolButton*>())
    {
        tb->setProperty(tb_func_id, ++i);
        connect(tb, &QToolButton::clicked, this, &ZrmConnectivityParam::tool_buttons_clicked);
    }
    tw_connectivity->setItemDelegate( new zcp_item_delegate(this));
}


void  ZrmConnectivityParam::init_ui   ()
{
    channel_type->addItem(zrm::ZrmConnectivity::zrm_work_mode_name( zrm::zrm_work_mode_t::as_power  ), zrm::zrm_work_mode_t::as_power  );
    channel_type->addItem(zrm::ZrmConnectivity::zrm_work_mode_name( zrm::zrm_work_mode_t::as_charger), zrm::zrm_work_mode_t::as_charger);
}

void  ZrmConnectivityParam::prepare_ui()
{
    auto screen =  QApplication::primaryScreen();
    qreal ratio = screen->logicalDotsPerInch() / 96.0;
    for (auto&& tb : tool_frame->findChildren<QAbstractButton*>())
    {
        QSizeF szf = tb->iconSize();
        szf  = szf.scaled(szf.width() * ratio, szf.height() * ratio, Qt::AspectRatioMode::KeepAspectRatio);
        tb->setIconSize(szf.toSize());
    }
    QFont font = tw_connectivity->font();
    font.setWeight(int(qreal(font.weight())*ratio));
    tw_connectivity->setFont(font);
}


void ZrmConnectivityParam::showEvent(QShowEvent* event)
{
    if (!tw_connectivity->topLevelItemCount())
        make_connectivity_tree();
    QWidget::showEvent(event);
}

void ZrmConnectivityParam::hideEvent(QHideEvent* event )
{
    tw_connectivity->clear();
    QWidget::hideEvent(event);
}

void ZrmConnectivityParam::setCurrentItem(zrm::ZrmConnectivity* conn, uint16_t chan)
{
    for (int i = 0; i < tw_connectivity->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = tw_connectivity->topLevelItem(i);
        if (connectivity(item) == conn)
        {
            tw_connectivity->setCurrentItem(item);
            tw_connectivity->expandItem(item);
            for (int j = 0; j < item->childCount(); j++)
            {
                QTreeWidgetItem* itemChan = item->child(j);
                if (channel_number(itemChan) == chan)
                    tw_connectivity->setCurrentItem(itemChan);
            }
            break;
        }
    }
}

bool    ZrmConnectivityParam::is_select_mode  ()
{
    return !conn_stacked->isVisible();
}


uint16_t ZrmConnectivityParam::channel_number(QTreeWidgetItem* item, bool old_number)
{
    bool ok = false;
    uint16_t cnumber = 0;
    if (item)
    {
        QVariant var;
        if (old_number)
            var = item->data(1, old_channel_number);
        else
            var = item->data(1, Qt::UserRole);
        cnumber = uint16_t(var.toUInt(&ok)) ;
    }
    return ok ?   cnumber : 0;
}

zrm::zrm_work_mode_t   ZrmConnectivityParam::channel_work_mode(QTreeWidgetItem* item)
{
    return item ? zrm::zrm_work_mode_t(item->data(0, Qt::UserRole).toInt()) : zrm::as_power ;
}


QTreeWidgetItem* ZrmConnectivityParam::create_channel_item(zrm::ZrmConnectivity* conn, QTreeWidgetItem* conn_item, uint16_t chan_number)
{
    if (conn && conn_item)
    {
        if (conn_item->parent())
            conn_item = conn_item->parent();

        QSignalBlocker sb(conn_item->treeWidget());
        auto chann_item = new QTreeWidgetItem(conn_item);
        chann_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        auto wm = conn->channel_work_mode(chan_number);
        chann_item->setText(0, zrm::ZrmConnectivity::zrm_work_mode_name(wm));
        chann_item->setData(0, WorkMode, wm);
        chann_item->setText(1, QString::number(chan_number));
        chann_item->setData(1, Qt::UserRole, chan_number);
        chann_item->setData(0, BoxNumber, conn->channel_box_number(chan_number));
        chann_item->setData(0, DeviceNumber, conn->channel_device_number(chan_number));
        chann_item->setData(0, Color, conn->channel_color(chan_number));
        return chann_item;
    }
    return nullptr;
}


QTreeWidgetItem* ZrmConnectivityParam::create_connectivity_item(zrm::ZrmConnectivity* conn)
{
    auto item = new QTreeWidgetItem;
    QVariant var = QVariant::fromValue(int64_t(conn));
    item->setData(0, Qt::UserRole, var);
    item->setText(0, conn->name());
    item->setText(1, conn->connection_string());
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    for (auto&& chan_number : conn->channels())
        create_channel_item(conn, item, chan_number);
    return item;
}

void ZrmConnectivityParam::make_connectivity_tree()
{
    tw_connectivity->clear();
    QList<QTreeWidgetItem*> items;
    for (auto&& conn : zrm::ZrmConnectivity::connectivities())
    {
        items.append(create_connectivity_item(conn));
    }
    tw_connectivity->addTopLevelItems(items);
    if (items.count())
    {
        tw_connectivity->setCurrentItem(items.at(0));
        // ->setSelected(true);
    }
}

zrm::ZrmConnectivity* ZrmConnectivityParam::connectivity(QTreeWidgetItem* item)
{
    zrm::ZrmConnectivity* conn_obj = Q_NULLPTR;
    item = item && item->parent() ? item->parent() : item;
    if (item && item->treeWidget())
    {

        QVariant v = item->data(0, Qt::UserRole);
        int64_t  i = v.value<int64_t>();
        conn_obj = reinterpret_cast<zrm::ZrmConnectivity*>(reinterpret_cast<QObject*>(i))  ;
    }
    return conn_obj;
}

void ZrmConnectivityParam::update_tool_buttons(QTreeWidgetItem* item)
{
    bool is_chan = item && item->parent();
    bool is_con = item && !item->parent();
    conn_params->setEnabled(is_con);
    tbAddChannel->setEnabled(is_con || is_chan);
    tbRemove->setEnabled(is_con || (is_chan && item->parent()->childCount() > 1));
}

void ZrmConnectivityParam::on_tw_connectivity_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    updateMonitor();
    if (!is_select_mode())
    {
        if (current != previous)
        {
            if (current)
            {
                auto conn_dev = connectivity(current);
                if (current->parent())
                {
                    conn_stacked->setCurrentWidget(channel_page);
                    setup_channel(current);
                }
                else
                {
                    conn_stacked->setCurrentWidget(conn_page);
                    conn_params->setConnectionString(conn_dev ? conn_dev->connection_string() : QString());
                }
            }
            update_tool_buttons(current);
        }
    }
}

void ZrmConnectivityParam::setup_channel(QTreeWidgetItem* item)
{
    SignalBlocker sb(channel_page->findChildren<QWidget*>());
    auto wm = channel_work_mode(item);
    int idx = channel_type->findData(wm);
    channel_type->setCurrentIndex(idx);
    spinBoxBoxNumber->setValue(item->data(0, BoxNumber).toInt());
    spinBoxDeviceNumber->setValue(item->data(0, DeviceNumber).toInt());
    setButtonColor(item->data(0, Color).toString());
}

void ZrmConnectivityParam::on_tw_connectivity_itemChanged(QTreeWidgetItem* item, int column)
{
    auto conn = connectivity(item);
    if (conn)
    {
        switch (column)
        {
            case 0:
                if (item->parent())
                {
                    conn->channel_set_work_mode(channel_number(item), channel_work_mode(item));
                }
                else
                    conn->set_name(item->text(column));
                break;
            case 1:
                if (item->parent())
                    channel_renumber(item);
                break;
        }
        if (item->parent())
            setup_channel(item);
    }
}


void   ZrmConnectivityParam::channel_renumber        (QTreeWidgetItem* item)
{
    uint16_t old_number  = channel_number(item, true);
    uint16_t curr_number = channel_number(item, false);
    if (old_number )
    {

        if (old_number != curr_number)
        {
            auto conn = connectivity(item);
            if (!conn->channel_exists(curr_number))
            {
                conn->channel_remove(old_number);
                conn->channel_add   (curr_number, channel_work_mode(item));
            }
            else
                curr_number = old_number;

            QSignalBlocker sb(item->treeWidget());
            item->setData(1, old_channel_number, QVariant());
            item->setData(1, Qt::UserRole, curr_number);
            item->setText(1, QString::number(curr_number));

        }
    }
}

void  ZrmConnectivityParam::do_connection_start     (zrm::ZrmConnectivity* conn, bool start)
{
    if (conn && conn->is_working() != start)
    {
        if (start)
            conn->start_work();
        else
            conn->stop_work();
    }
}

void ZrmConnectivityParam::do_remove_item(QTreeWidgetItem* item, zrm::ZrmConnectivity* conn)
{
    if (item)
    {
        if (item->parent())
        {
            // не удаляем последний канал
            if (item->parent()->childCount() <= 1)
                return;
            //Удаление элемента
            if (conn)
            {
                uint16_t chn = channel_number(item);
                conn->channel_remove(chn);
            }
        }
        else
        {
            //Удаление соединения
            if (conn)
            {
                conn->stop_work();
                delete conn;
            }
        }
        delete item;
    }
}

void  ZrmConnectivityParam::do_connection_add()
{
    //Добваление соединения
    auto conn = new zrm::ZrmConnectivity(QString("Zrm %1").arg(tw_connectivity->topLevelItemCount()));
    QTreeWidgetItem* item = create_connectivity_item(conn);
    QString name = tr("Соединение - %1").arg(tw_connectivity->topLevelItemCount() + 1);
    conn->set_name(name);
    item->setText(0, name);
    QString conn_str = conn_params->connectionString();
    item->setText(1, conn_str);
    conn->set_connection_string(conn_str);
    tw_connectivity->addTopLevelItem(item);
    do_channel_add(item, conn);
    tw_connectivity->setCurrentItem(item);
}

void  ZrmConnectivityParam::do_channel_add          (QTreeWidgetItem* item, zrm::ZrmConnectivity* conn)
{
    //Добваление Канала
    zrm::channels_key_t channels = conn->channels();
    uint16_t last_number = channels.size() ?  channels.last() : 0;
    if (last_number < zrm::MAX_CHANNEL_NUMBER)
    {
        uint16_t chan_num =  last_number + 1;
        conn->channel_add(chan_num, zrm::as_charger);
        auto chan_item = create_channel_item(conn, item, chan_num);
        if (chan_item)
            tw_connectivity->setCurrentItem(chan_item);
    }
}

void ZrmConnectivityParam::conn_param_apply()
{
    QTreeWidgetItem* item = current_item();
    if (item && item->parent())
        item = item->parent();
    auto conn = connectivity(item);
    if (conn)
    {
        QString conn_str = conn_params->connectionString();
        item->setText(1, conn_str);
        conn->set_connection_string(conn_str);
    }
    updateMonitor();
    emit configureApply();
}

void ZrmConnectivityParam::conn_param_undo()
{
    auto conn = connectivity(current_item());
    if (conn)
        conn_params->setConnectionString(conn->connection_string());
    updateMonitor();
}


void  ZrmConnectivityParam::do_connection_set_string(zrm::ZrmConnectivity* conn)
{
    if (conn)
        conn->set_connection_string(conn_params->connectionString());
}



void ZrmConnectivityParam::tool_buttons_clicked()
{
    auto src      = sender();
    auto curr_item =  current_item();
    auto conn = connectivity(curr_item);
    int tb_prop_value = src->property(tb_func_id).toInt();
    //qDebug()<< tb_func_id<<" "<<tb_prop_value;
    switch (tb_prop_value)
    {
        case tbf_remove         :
            do_remove_item          (curr_item, conn);
            break;
        case tbf_add_connection :
            do_connection_add       ()               ;
            break;
        case tbf_add_channel    :
            do_channel_add          (curr_item, conn);
            break;

        default :
            break;
    }
    update_tool_buttons(current_item());
}



QWidget* zcp_item_delegate::createEditor(QWidget* parent,
                                         const QStyleOptionViewItem& option,
                                         const QModelIndex& index) const
{
    auto edit_item = zcp && !zcp->is_select_mode() ? zcp->current_item() : Q_NULLPTR;
    if (edit_item)
    {
        if (edit_item->parent())
        {
            if (index.column() == 0)
            {
                QComboBox* cb = new QComboBox(parent);
                cb->addItem(zrm::ZrmConnectivity::zrm_work_mode_name(zrm::as_power), zrm::as_power);
                cb->addItem(zrm::ZrmConnectivity::zrm_work_mode_name(zrm::as_charger), zrm::as_charger);
                return cb;
            }
            else
            {
                QSpinBox* sb = new QSpinBox(parent);
                sb->setMinimum(1);
                sb->setMaximum(254);
                return     sb;
            }
        }
        else
        {
            // Edit connectivity name;
            if (index.column() == 0)
                return QItemDelegate::createEditor(parent, option, index);
        }
    }
    return Q_NULLPTR;
}


void zcp_item_delegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* cb = dynamic_cast<QComboBox*>(editor);
    if (cb)
    {
        int idx = cb->findData(index.data(Qt::UserRole), Qt::UserRole) ;
        cb->setCurrentIndex(idx);
        return;
    }

    QSpinBox* sb = dynamic_cast<QSpinBox*>(editor);
    if (sb)
    {
        sb->setValue(index.data(Qt::UserRole).toInt());
        return;
    }

    QItemDelegate::setEditorData(editor, index);

}

void zcp_item_delegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* cb = dynamic_cast<QComboBox*>(editor);
    if (cb)
    {
        model->setData(index, cb->currentData(Qt::UserRole), Qt::UserRole );
        model->setData(index, cb->currentText() );
        return;
    }

    QSpinBox* sb = dynamic_cast<QSpinBox*>(editor);
    if (sb)
    {
        //Сохранили предыдущий номер канала
        int value = sb->value();

        QVariant v = model->data(index, Qt::UserRole);
        model->setData(index, QString::number(value));
        model->setData(index, value, Qt::UserRole );
        model->setData(index, v, old_channel_number);
        return;
    }
    QItemDelegate::setModelData(editor, model, index);
}

void ZrmConnectivityParam::on_channel_type_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    auto current = tw_connectivity->currentItem();
    if (current && current->parent())
    {
        QVariant v = channel_type->currentData();
        current->setData(0, WorkMode, v);
        current->setText(0, zrm::ZrmConnectivity::zrm_work_mode_name(zrm::zrm_work_mode_t(v.toInt())));
    }
}

void ZrmConnectivityParam::on_spinBoxBoxNumber_valueChanged(int arg1)
{
    QTreeWidgetItem* current = tw_connectivity->currentItem();
    if (current && current->parent())
    {
        current->setData(0, BoxNumber, arg1);
        zrm::ZrmConnectivity* conn = connectivity(current);
        if (conn)
            conn->channel_set_box_number(channel_number(current), arg1);
    }
}

void ZrmConnectivityParam::on_spinBoxDeviceNumber_valueChanged(int arg1)
{
    QTreeWidgetItem* current = tw_connectivity->currentItem();
    if (current && current->parent())
    {
        current->setData(0, DeviceNumber, arg1);
        zrm::ZrmConnectivity* conn = connectivity(current);
        if (conn)
            conn->channel_set_device_number(channel_number(current), arg1);
    }
}

void ZrmConnectivityParam::on_pushButtonColor_clicked()
{
    QTreeWidgetItem* current = tw_connectivity->currentItem();
    if (current && current->parent())
    {
        QColor color = QColorDialog::getColor(pushButtonColor->property("color").toString());
        if (color.isValid())
        {
            QString strColor = color.name();
            setButtonColor(strColor);
            current->setData(0, Color, strColor);
            zrm::ZrmConnectivity* conn = connectivity(current);
            if (conn)
                conn->channel_set_color(channel_number(current), strColor);
        }
    }
}

void ZrmConnectivityParam::setButtonColor(QString color)
{
    pushButtonColor->setProperty("color", color);
    QString style = QString("background-color: %1 ;").arg(color);
    pushButtonColor->setStyleSheet(style);
}

void ZrmConnectivityParam::updateMonitor()
{
    QTreeWidgetItem* item = tw_connectivity->currentItem();
    uint16_t chan ;
    zrm::ZrmConnectivity* conn = nullptr;
    if (item)
    {
        conn = connectivity(item);
        chan = channel_number(item);
        if (item->childCount())
            chan = item->child(0)->data(1, Qt::UserRole).toUInt();
        else
            chan = item->data(1, Qt::UserRole).toUInt();
    }
    zrm_mon->bind(conn, item ? chan : 0);
}
