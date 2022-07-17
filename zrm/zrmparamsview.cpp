#include "zrmparamsview.h"
#include <zrmparamcvt.h>
#include <QMessageBox>
#include <QInputDialog>
#include <QStyledItemDelegate>

namespace {

class ItemDelegate: public QStyledItemDelegate
{
public:
    ItemDelegate() = default;
    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

};

QWidget* ItemDelegate::createEditor(QWidget* parent,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const
{
    if (index.column() != ZrmParamsView::column_new_value)
        return nullptr;
    return QStyledItemDelegate::createEditor(parent, option, index);
}

}

ZrmParamsView::ZrmParamsView(QWidget* parent) :
    ZrmChannelWidget (parent)
{
    setupUi(this);
    m_EditableIcon = QIcon(":/zrm/icons/edit_2.png");

    QHeaderView* hdr = zrm_params->header();
    hdr->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

    connect(&m_request_timer, &QTimer::timeout, this, &ZrmParamsView::request);

    connect(actServiceMode, &QAction::triggered, this, &ZrmParamsView::serviceMode);
    connect(actWriteParameters, &QAction::triggered, this, &ZrmParamsView::writeParameters);
    connect(passwd, &QLineEdit::textChanged, this, &ZrmParamsView::passwdChanged);


    tbWriteParams->setDefaultAction(actWriteParameters);
    tbServiceMode->setDefaultAction(actServiceMode);

    connect(zrm_params, &QTreeWidget::itemChanged, this, &ZrmParamsView::paramChanged);
    zrm_params->setItemDelegate(new ItemDelegate);
    init_params();
}

void ZrmParamsView::appendParam(zrm::zrm_param_t param, const QString& text, bool ordered, bool editable)
{
    if (ordered)
        m_orders.push_back( param );
    else
        m_query_parms.push_back(param);

    QTreeWidget* parent = zrm_params;


    QTreeWidgetItem* item =   new QTreeWidgetItem(parent, QStringList() << text);
    if (editable)
    {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setIcon(column_new_value, m_EditableIcon);
        m_EditableItems.append(item);
    }

    m_items.insert(param, item );
}


void ZrmParamsView::init_params()
{
    appendParam(zrm::PARAM_VRDEV, tr("Версия блока"), false);
    appendParam(zrm::PARAM_RVDEV, tr("Модификация блока"), false);
    appendParam(zrm::PARAM_RVSW, tr("Версия ПО"), false);
    appendParam(zrm::PARAM_SOFT_REV, tr("Модификация ПО"), false);
    appendParam(zrm::PARAM_SERNM, tr("Заводской номер"), false);
    appendParam(zrm::PARAM_MEMFR, tr("Свободная память"), true);

    appendParam(zrm::PARAM_CUR, tr("Ток"), true);
    appendParam(zrm::PARAM_LCUR, tr("Ограничение тока"), true);
    appendParam(zrm::PARAM_VOLT, tr("Напряжение"), true);
    appendParam(zrm::PARAM_LVOLT, tr("Оганичение напряжения"), true);
    appendParam(zrm::PARAM_CAP, tr("Ёмкость"), true);
    appendParam(zrm::PARAM_STG_NUM, tr("Номер этапа"), true);
    appendParam(zrm::PARAM_LOOP_NUM, tr("Номер цикла"), true);
    appendParam(zrm::PARAM_TRECT, tr("Температура"), true);
    appendParam(zrm::PARAM_VOUT, tr("Напряжение на выходе ЗРМ"), true);
    appendParam(zrm::PARAM_MVOLT, tr("Макс. напряжение"), false);

    appendParam(zrm::PARAM_MAX_CHP, tr("Макс. мощность заряда"), false); //
    appendParam(zrm::PARAM_MCUR, tr("Макс. ток"), true);
    appendParam(zrm::PARAM_MCURD, tr("Макс. ток разряда"), true); //

    appendParam(zrm::PARAM_CUR_CONSUMPTION, tr("Потребляемый ток"), true);
    appendParam(zrm::PARAM_VOLT_SUPPLY, tr("Напряжение питающей сети"), true);
    appendParam(zrm::PARAM_VOLT_HIGH_VOLT_BUS, tr("Напряжение высоковольтной шины"), true);
    appendParam(zrm::PARAM_FAN_PERCENT, tr("Вентиляторы"), true);


    respond =   new QTreeWidgetItem(zrm_params, QStringList() << tr("Время ответа канала"));

    appendParam(zrm::PARAM_TRYCT, tr("Кол-во попыток запуска"), false, true);
    appendParam(zrm::PARAM_RSTTO, tr("Таймаут между перезапусками"), false, true);
    appendParam(zrm::PARAM_VSTRT, tr("Напряжение автозапуска"), false, true);
    gbWriteParams->setVisible(false);
}


void ZrmParamsView::channel_param_changed(unsigned channel, const zrm::params_list_t& params_list)
{
    if (m_source && m_channel == channel)
    {
        setUpdatesEnabled(false);
        for (auto param : params_list)
        {
            params_items_t::iterator item = m_items.find(zrm::zrm_param_t(param.first));

            if (item != m_items.end())
            {
                QString str = ZrmParamCvt::toVariant(param.first, param.second).toString();
                item.value()->setText(column_value, str);
            }
        }
        setUpdatesEnabled(true);
    }
    ZrmChannelWidget::channel_param_changed(channel, params_list);
}

void    ZrmParamsView::clear_controls()
{
    for (auto&& item : m_items)
    {
        item->setText(column_value, QString());
        item->setText(column_new_value, QString());
    }
    if (respond)
        respond->setText(column_value, QString());
}

void ZrmParamsView::onActivate()
{
    ZrmChannelWidget::onActivate();

    if (m_source && m_source->channel_session(m_channel).is_active())
    {
        channel_param_changed(m_channel, m_source->channel_params(m_channel));
        m_request_timer.start(std::chrono::milliseconds(333));
        m_source->channel_query_params(m_channel, m_query_parms);
        request();
    }

}

void ZrmParamsView::onDeactivate()
{
    ZrmChannelWidget::onDeactivate();
    m_request_timer.stop();
}

void    ZrmParamsView::request()
{
    if (m_source && m_channel)
    {
        m_source->channel_query_params(m_channel, m_orders);
        if (respond)
        {
            qint64  tm = m_source->channelRespondTime(m_channel);
            respond->setText(column_value, tm ?  QString("%1 ms").arg( tm ) : QString()) ;
        }
    }
}

void ZrmParamsView::serviceMode()
{
    if (m_source && m_channel && m_source->channel_session(m_channel).is_active())
    {
        if (m_source && m_channel && QMessageBox::Yes != QMessageBox::question(this, "Сервисный режим", "Перевести устройство в сервисный режим ?"))
            return;
        quint8 wr_value = 0xAA;
        m_source->channel_write_param(m_channel, zrm::WM_PROCESS_AND_WRITE, zrm::PARAM_BOOT_LOADER, &wr_value, sizeof(wr_value));
    }
    else
        QMessageBox::information(this, "Внимание!", "Нет канала для передачи");
}

void ZrmParamsView::paramChanged(QTreeWidgetItem*, int column)
{

    if (column != column_new_value)
        return;

    for (const QTreeWidgetItem* testItem : qAsConst(m_EditableItems))
    {
        if (testItem->text(column_new_value).trimmed().isEmpty())
            continue;
        gbWriteParams->setVisible(true);
        return;
    }
    gbWriteParams->setVisible(false);
}

void ZrmParamsView::writeParameters()
{
    for (QTreeWidgetItem* item : qAsConst(m_EditableItems))
    {
        QString newValue = item->text(column_new_value).trimmed();
        if (newValue.isEmpty())
            continue;
        zrm::zrm_param_t param = m_items.key(item);
        qDebug() << "param is " << quint32(param);
        item->setText(column_value, newValue);
        item->setText(column_new_value, QString());
    }
    passwd->clear();
}

void ZrmParamsView::passwdChanged(const QString& text)
{
    actWriteParameters->setDisabled(text.trimmed().isEmpty());
}

