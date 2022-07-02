#include "zrmparamsview.h"

#include <QMessageBox>

ZrmParamsView::ZrmParamsView(QWidget* parent) :
    ZrmChannelWidget (parent)
{
    setupUi(this);
    QHeaderView* hdr = zrm_params->header();
    hdr->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    connect(&m_request_timer, &QTimer::timeout, this, &ZrmParamsView::request);
    connect(pushButtonService, &QAbstractButton::clicked, this, &ZrmParamsView::serviceMode);
    init_params();
}

void ZrmParamsView::appendParam(zrm::zrm_param_t param, const QString& text, bool ordered)
{
    if (ordered)
        m_orders.push_back( param );
    else
        m_query_parms.push_back(param);

    QTreeWidgetItem* item =   new QTreeWidgetItem(zrm_params, QStringList() << text);

    m_items.insert(param, item );
}


void ZrmParamsView::init_params()
{
    appendParam(zrm::PARAM_VRDEV, "Версия блока", false);
    appendParam(zrm::PARAM_RVDEV, "Модификация блока", false);
    appendParam(zrm::PARAM_RVSW, "Версия ПО", false);
    appendParam(zrm::PARAM_SOFT_REV, "Модификация ПО", false);

    appendParam(zrm::PARAM_CUR, "Ток", true);
    appendParam(zrm::PARAM_LCUR, "Ограничение тока", true);
    appendParam(zrm::PARAM_VOLT, "Напряжение", true);
    appendParam(zrm::PARAM_LVOLT, "Оганичение напряжения", true);
    appendParam(zrm::PARAM_CAP, "Ёмкость", true);
    appendParam(zrm::PARAM_STG_NUM, "Номер этапа", true);
    appendParam(zrm::PARAM_LOOP_NUM, "Номер цикла", true);
    appendParam(zrm::PARAM_TRECT, "Температура", true);
    appendParam(zrm::PARAM_VOUT, "Напряжение на выходе ЗРМ", true);
    appendParam(zrm::PARAM_MVOLT, "Макс. напряжение", false);

    appendParam(zrm::PARAM_MCUR, "Макс. ток", true);
    appendParam(zrm::PARAM_MAX_CHP, "Макс. мощность заряда", false); //
    appendParam(zrm::PARAM_MCURD, "Макс. ток разряда", true); //
    appendParam(zrm::PARAM_DPOW, "Макс. мощность разряда", false); //

    appendParam(zrm::PARAM_CUR_CONSUMPTION, "Потребляемый ток", true);
    appendParam(zrm::PARAM_VOLT_SUPPLY, "Напряжение питающей сети", true);
    appendParam(zrm::PARAM_VOLT_HIGH_VOLT_BUS, "Напряжение высоковольтной шины", true);
    appendParam(zrm::PARAM_FAN_PERCENT, "Вентиляторы", true);
    respond =   new QTreeWidgetItem(zrm_params, QStringList() << "Время ответа канала");
}


void ZrmParamsView::channel_param_changed(unsigned channel, const zrm::params_list_t& params_list)
{
    if (m_source && m_channel == channel)
    {
        zrm_params->setUpdatesEnabled(false);
        for (auto param : params_list)
        {
            params_items_t::iterator item = m_items.find(zrm::zrm_param_t(param.first));

            if (item != m_items.end())
            {
                QVariant value = m_source->param_get(m_channel, param.first);
                QString str = value.toString();
                item.value()->setText(column_value, str);
            }
        }
        zrm_params->setUpdatesEnabled(true);
    }
    ZrmChannelWidget::channel_param_changed(channel, params_list);
}

void    ZrmParamsView::clear_controls()
{
    for (auto&& item : m_items)
    {
        item->setText(column_value, QString());
    }
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
    //m_request_timer.stop();
}

void    ZrmParamsView::request()
{
    if (m_source && m_channel)
    {
        m_source->channel_query_params(m_channel, m_orders);
        if (respond)
            respond->setText(column_value, QString("%1 ms").arg( m_source->channelRespondTime(m_channel))) ;
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
