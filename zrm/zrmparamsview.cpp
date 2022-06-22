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

void ZrmParamsView::appendParam(zrm::zrm_param_t param, const QString& text)
{
    m_orders.push_back( param );
    QTreeWidgetItem* item =   new QTreeWidgetItem(zrm_params, QStringList() << text);
    m_items.insert(param, item );
}


void ZrmParamsView::init_params()
{
    appendParam(zrm::PARAM_VRDEV, "Версия блока");
    appendParam(zrm::PARAM_RVDEV, "Модификация блока");
    appendParam(zrm::PARAM_RVSW, "Версия ПО");
    appendParam(zrm::PARAM_SOFT_REV, "Модификация ПО");

    appendParam(zrm::PARAM_CUR, "Ток");
    appendParam(zrm::PARAM_LCUR, "Ограничение тока");
    appendParam(zrm::PARAM_VOLT, "Напряжение");
    appendParam(zrm::PARAM_LVOLT, "Оганичение напряжения");
    appendParam(zrm::PARAM_CAP, "Ёмкость");
    appendParam(zrm::PARAM_STG_NUM, "Номер этапа");
    appendParam(zrm::PARAM_LOOP_NUM, "Номер цикла");
    appendParam(zrm::PARAM_TRECT, "Температура");
    appendParam(zrm::PARAM_VOUT, "Напряжение на выходе ЗРМ");
    appendParam(zrm::PARAM_MVOLT, "Макс. напряжение");
    appendParam(zrm::PARAM_MCUR, "Макс. ток");
    appendParam(zrm::PARAM_MCURD, "Макс. ток разряда");
    appendParam(zrm::PARAM_DPOW, "Макс. мощность разряда");
    appendParam(zrm::PARAM_MAX_CHP, "Макс. мощность заряда");

    appendParam(zrm::PARAM_VOUT, "Напряжение на электролитах");
    appendParam(zrm::PARAM_CUR_CONSUMPTION, "Потребляемый ток");
    appendParam(zrm::PARAM_VOLT_SUPPLY, "Напряжение питающей сети");
    appendParam(zrm::PARAM_VOLT_HIGH_VOLT_BUS, "Напряжение высоковольтной шины");
    appendParam(zrm::PARAM_FAN_PERCENT, "Вентиляторы");
}


void ZrmParamsView::channel_param_changed(unsigned channel, const zrm::params_list_t& params_list)
{
    if (m_source && m_channel == channel)
    {
        for (auto param : params_list)
        {
            params_items_t::iterator item = m_items.find(zrm::zrm_param_t(param.first));
            if (item != m_items.end())
            {
                QVariant value = m_source->param_get(m_channel, param.first);
                item.value()->setText(column_value, value.toString());
            }
        }
    }
    ZrmChannelWidget::channel_param_changed(channel, params_list);
}


void    ZrmParamsView::update_controls      ()
{
    ZrmChannelWidget::update_controls();
    if (m_source && m_channel)
    {
        channel_session(m_channel);
        channel_param_changed(m_channel, m_source->channel_params(m_channel));
    }
}

void ZrmParamsView::showEvent(QShowEvent* event)
{
    ZrmChannelWidget::showEvent(event);
    if (m_source && m_source->channel_session(m_channel).is_active())
    {
        m_request_timer.start(std::chrono::milliseconds(1666));
        request();
    }

}

void ZrmParamsView::hideEvent(QHideEvent* event)
{
    ZrmChannelWidget::hideEvent(event);
    m_request_timer.stop();
}


void    ZrmParamsView::clear_controls       ()
{
}

void    ZrmParamsView::channel_session      (unsigned channel)
{
    if (m_source && m_channel == channel && m_source->channel_session(m_channel).is_active())
    {
        qDebug() << Q_FUNC_INFO << " channel : " << channel;
    }
}

void    ZrmParamsView::request()
{
    if (m_source && m_channel)
    {
        m_source->channel_query_params(m_channel, m_orders);
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
