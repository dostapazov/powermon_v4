#include "zrmparamsview.h"

#include <QMessageBox>

ZrmParamsView::ZrmParamsView(QWidget *parent) :
    ZrmChannelWidget (parent)
{
    setupUi(this);
    init_params();
    QHeaderView * hdr = zrm_params->header();
    hdr->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    connect(&m_request_timer, &QTimer::timeout, this, &ZrmParamsView::request);
    zrm_params->sortItems(0,Qt::SortOrder::AscendingOrder);

    connect(pushButtonService, SIGNAL(clicked()), this, SLOT(serviceMode()));
}


void ZrmParamsView::init_params()
{
    m_orders[zrm::PARAM_VRDEV    ] = "Версия блока";
    m_orders[zrm::PARAM_RVDEV    ] = "Модификация блока";
    m_orders[zrm::PARAM_RVSW     ] = "Версия ПО";
    m_orders[zrm::PARAM_SOFT_REV ] = "Модификация ПО";

    m_orders[zrm::PARAM_CUR      ] = "Ток";
    m_orders[zrm::PARAM_LCUR     ] = "Ограничение тока";
    m_orders[zrm::PARAM_VOLT     ] = "Напряжение";
    m_orders[zrm::PARAM_LVOLT    ] = "Оганичение напряжения";
    m_orders[zrm::PARAM_CAP      ] = "Ёмкость";
    m_orders[zrm::PARAM_STG_NUM  ] = "Номер этапа";
    m_orders[zrm::PARAM_LOOP_NUM ] = "Номер цикла";
    m_orders[zrm::PARAM_TRECT    ] = "Температура";
    m_orders[zrm::PARAM_VOUT     ] = "Напряжение на выходе ЗРМ";
    m_orders[zrm::PARAM_MVOLT    ] = "Макс. напряжение";
    m_orders[zrm::PARAM_MCUR     ] = "Макс. ток";
    m_orders[zrm::PARAM_MCURD    ] = "Макс. ток разряда";
    m_orders[zrm::PARAM_DPOW     ] = "Макс. мощность разряда";
    m_orders[zrm::PARAM_MAX_CHP  ] = "Макс. мощность заряда";

    m_orders[zrm::PARAM_VOUT] = "Напряжение на электролитах";
    m_orders[zrm::PARAM_CUR_CONSUMPTION] = "Потребляемый ток";
    m_orders[zrm::PARAM_VOLT_SUPPLY] = "Напряжение питающей сети";
    m_orders[zrm::PARAM_VOLT_HIGH_VOLT_BUS] = "Напряжение высоковольтной шины";
    m_orders[zrm::PARAM_FAN_PERCENT] = "Вентиляторы";
}


void ZrmParamsView::channel_param_changed(unsigned channel, const zrm::params_list_t & params_list)
{
    if (m_source && m_channel == channel)
    {
        for (auto param : params_list)
        {
            params_items_t::iterator item = m_items.find(zrm::zrm_param_t(param.first));
            if (item == m_items.end() && is_viewed_param(param.first))
                item = m_items.insert(param.first, new QTreeWidgetItem(zrm_params, QStringList() << m_orders[param.first]));
            if (item!= m_items.end())
            {
                QVariant value = m_source->param_get(m_channel, param.first);
                item.value()->setText(column_value, value.toString());
            }
        }
    }
    ZrmChannelWidget::channel_param_changed(channel, params_list);
}


bool ZrmParamsView::is_viewed_param(zrm::zrm_param_t param)
{
   bool ret = m_orders.contains(param);
   return ret;
}


void    ZrmParamsView::update_controls      ()
{
   ZrmChannelWidget::update_controls();
   if(m_source && m_channel)
   {
    channel_session(m_channel);
    channel_param_changed(m_channel ,m_source->channel_params(m_channel));
   }
}

void    ZrmParamsView::clear_controls       ()
{
  zrm_params->clear();
  m_items.clear();
}

void    ZrmParamsView::channel_session      (unsigned channel)
{
    if(m_source && m_channel == channel && m_source->channel_session(m_channel).is_active())
    {
      m_request_timer.start(std::chrono::seconds(1));
      request();

    }
    else
    {
      m_request_timer.stop();
    }
}

void    ZrmParamsView::request()
{
  if(m_source && m_channel)
   {
    zrm::params_t req_params;
    req_params.resize(zrm::params_t::size_type( m_orders.size() ) );
    auto keys = m_orders.keys().toVector();
    std::copy(keys.begin(),keys.end(),req_params.begin());
    m_source->channel_query_params(m_channel,req_params);
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
