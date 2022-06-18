#include "zrmmethodbase.h"

constexpr int STAGE_NUMBER_COLUMN = 0;
constexpr int STAGE_TYPE_COLUMN   = 1;
constexpr int STAGE_FINISH_COLUMN = 2;


ZrmMethodBase::ZrmMethodBase(QWidget* parent) :
    ZrmChannelWidget (parent)
{
    setupUi(this);
    auto hdr = tw_stages->header();
    hdr->setSectionResizeMode(QHeaderView::ResizeToContents);
    hdr->setSectionResizeMode(STAGE_FINISH_COLUMN, QHeaderView::Stretch);

}

void  ZrmMethodBase::channel_param_changed(unsigned channel, const zrm::params_list_t& params_list  )
{
    if (channel == m_channel)
    {
        for (auto param : params_list)
        {
            switch (param.first)
            {
                case zrm::PARAM_STG_NUM       :
                    set_current_stage(param.second.sword);
                    break;
                case zrm::PARAM_WTIME         :
                    lb_time_work->setText(param_get(param.first).toString());
                    break;
                case zrm::PARAM_LOOP_NUM      :
                    set_number_value(lb_cycle_num, param.second.sword, 2, no_value);
                    break;
                case zrm::PARAM_METHOD_STAGES :
                    setup_method();
                    break;
                default :
                    break;
            }
        }
    }
    ZrmChannelWidget::channel_param_changed(channel, params_list);
}

bool     ZrmMethodBase::is_details_enabled()
{
    return m_details_visible;
}

void     ZrmMethodBase::set_details_enable(bool value)
{
    if (m_details_visible != value)
    {
        m_details_visible = value;
        details_box->setVisible(m_details_visible);
        lb_method_name->setVisible(m_details_visible);
    }
}

void  ZrmMethodBase::showEvent            (QShowEvent* event)
{
    ZrmChannelWidget::showEvent(event);
    details_box->setVisible(m_details_visible);
    lb_method_name->setVisible(m_details_visible);
}


void  ZrmMethodBase::update_controls()
{
    if (m_source && m_channel)
    {
        setup_method();
        channel_param_changed(m_channel, m_source->channel_params(m_channel));
    }
}

void  ZrmMethodBase::clear_controls       ()
{
    stages_clear();
    m_method_id  = uint16_t(-1);
    lb_stage_num  ->setText(no_value);
    lb_stage_total->setText(no_value);
    lb_cycle_num  ->setText(no_value);
    lb_cycle_total->setText(no_value);
    lb_time_work  ->setText(no_value);
    lb_time_limit ->setText(no_value);
    lb_method_name->setText(no_value);
}

void  ZrmMethodBase::timerEvent           (QTimerEvent* ev)
{
    if (m_timer_id && ev->timerId() == m_timer_id)
    {
        m_source->channel_get_method(m_channel, false);
    }
    ZrmChannelWidget::timerEvent(ev);
}


void ZrmMethodBase::setup_method    ()
{

    auto method = m_source->channel_get_method(m_channel, false);

    set_number_value(lb_stage_total, int(method.stages_count()), 2, infinity_symbol );
    set_number_value(lb_cycle_total, int(quint32(method.m_method.m_cycles_count)), 2, infinity_symbol );

    stages_clear();
    auto met_volt = method.m_method.voltage();
    auto met_curr = method.m_method.current();
    auto met_cap  = method.m_method.capacity();


    QString time_limit_string = zrm_method_duration_text(method);
    //qDebug()<< "<Method time limit "<< time_limit_string;
    this->lb_time_limit->setText(time_limit_string);

    lb_method_name->setText(to_utf(method.m_method.m_name, sizeof(method.m_method.m_name)));

    for (auto stage : method.m_stages)
    {
        stage_add(stage, met_volt, met_curr, met_cap );
    }
    auto param = m_source->param_get(m_channel, zrm::PARAM_STG_NUM);
    set_current_stage(param.toInt());
    m_method_id = method.m_method.m_id;
}

void ZrmMethodBase::stages_clear()
{
    while (tw_stages->topLevelItemCount())
        delete tw_stages->takeTopLevelItem(0);
}

void ZrmMethodBase::stage_add(const zrm::stage_t& stage, qreal met_volt, qreal met_current, qreal met_cap)
{
    QTreeWidgetItem* item =  new QTreeWidgetItem;

    item->setData(STAGE_NUMBER_COLUMN, Qt::UserRole, uint32_t(stage.m_number));
    item->setText(STAGE_NUMBER_COLUMN, QString::number(uint32_t(stage.m_number)));
    item->setText(STAGE_TYPE_COLUMN, m_source->get_stage_type_name(m_channel, zrm::stage_type_t(stage.m_type)));
    QList<QTreeWidgetItem*> sub_items;


    if (stage.m_type & zrm::STT_CHARGE)
    {
        QTreeWidgetItem* sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, m_source->get_stage_type_name(m_channel, zrm::STT_CHARGE));
        QString text = tr("U=%1 В, I=%2 А").arg(stage.charge_volt(met_volt), 0, 'f', 2).arg(stage.charge_curr(met_current), 0, 'f', 2);
        sitem->setText(STAGE_FINISH_COLUMN, text);
        sub_items << sitem;

        if (stage.m_type == zrm::STT_IMPULSE)
        {
            sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr("Время "));
            sitem->setText(STAGE_FINISH_COLUMN, tr("%1 сек").arg(uint32_t(stage.m_char_time)));
            sub_items << sitem;
        }

    }

    if (stage.m_type & zrm::STT_DISCHARGE)
    {
        QTreeWidgetItem* sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, m_source->get_stage_type_name(m_channel, zrm::STT_DISCHARGE));
        QString text = tr("U=%1 В, I=%2 А").arg(stage.discharge_volt(met_volt), 0, 'f', 2).arg(stage.discharge_curr(met_current), 0, 'f', 2);
        sitem->setText(STAGE_FINISH_COLUMN, text);
        sub_items << sitem;

        if (stage.m_type == zrm::STT_IMPULSE)
        {
            sitem = new QTreeWidgetItem;
            sitem->setText(STAGE_TYPE_COLUMN, tr("Время "));
            sitem->setText(STAGE_FINISH_COLUMN, tr("%1 сек").arg(uint32_t(stage.m_dis_time)));
            sub_items << sitem;
        }

    }

    if (stage.m_finish_flags & zrm::stage_end_time)
    {
        QTreeWidgetItem* sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по времени"));
        QString text = tr("%1:%2:%3").arg( uint32_t(stage.m_hours  ), 2, 10, QLatin1Char('0'))
                       .arg( uint32_t(stage.m_minutes), 2, 10, QLatin1Char('0'))
                       .arg( uint32_t(stage.m_secs   ), 2, 10, QLatin1Char('0'));
        sitem->setText(STAGE_FINISH_COLUMN, text);
        sub_items << sitem;
    }

    if (stage.m_finish_flags & zrm::stage_end_current)
    {
        QTreeWidgetItem* sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по I"));
        sitem->setText(STAGE_FINISH_COLUMN,  tr("%1 А").arg(QString::number(stage.end_curr(met_current), 'f', 2 )));
        sub_items << sitem;
    }

    if (stage.m_finish_flags & zrm::stage_end_voltage)
    {
        QTreeWidgetItem* sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по U"));
        sitem->setText(STAGE_FINISH_COLUMN,  tr("%1 В").arg(QString::number(stage.end_volt(met_volt), 'f', 2 )));
        sub_items << sitem;
    }

    if (stage.m_finish_flags & zrm::stage_end_delta_voltage)
    {
        QTreeWidgetItem* sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по ΔU"));
        sitem->setText(STAGE_FINISH_COLUMN,  tr("%1 В").arg(QString::number(stage.end_delta_volt(met_volt), 'f', 2 )));
        sub_items << sitem;
    }

    if (stage.m_finish_flags & zrm::stage_end_cell_voltage)
    {
        QTreeWidgetItem* sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по U-элемента"));
        sitem->setText(STAGE_FINISH_COLUMN,  tr("%1 В").arg(QString::number(stage.end_cell_volt(), 'f', 2 )));
        sub_items << sitem;
    }


    if (stage.m_finish_flags & zrm::stage_end_temper)
    {
        QTreeWidgetItem* sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение T"));
        sitem->setText(STAGE_FINISH_COLUMN,  tr("%1 C").arg(QString::number(stage.end_temp(), 'f', 2 )));
        sub_items << sitem;
    }


    if (stage.m_finish_flags & zrm::stage_end_capacity)
    {
        QTreeWidgetItem* sitem = new QTreeWidgetItem;
        sitem->setText(STAGE_TYPE_COLUMN, tr("Завершение по емкости"));
        sitem->setText(STAGE_FINISH_COLUMN,  tr("%1 А*Ч").arg(QString::number(stage.end_capacity(met_cap), 'f', 2 )));
        sub_items << sitem;
    }

    if (sub_items.count())
        item->addChildren(sub_items);

    tw_stages->addTopLevelItem(item);
    m_item_def_bkgnd =  item->background(0);

}

static void set_item_bkgnd(QTreeWidgetItem* item, const QBrush& bkgnd)
{
    item->setBackground(1, bkgnd);
    item->setBackground(2, bkgnd);
    for (int i = 0 ; i < item->childCount(); i++)
    {
        set_item_bkgnd(item->child(i), bkgnd);
    }

}



void ZrmMethodBase::set_current_stage(int stage_num)
{
    set_number_value(lb_stage_num, stage_num, 2, QString());
    QTreeWidgetItem* curr_item =  Q_NULLPTR;
    if (stage_num > 0 && stage_num <= tw_stages->topLevelItemCount())
    {
        curr_item  = tw_stages->topLevelItem(int(stage_num - 1));
        set_item_bkgnd( curr_item, QBrush(Qt::green));
        curr_item->setExpanded(true);
    }

    for (int i = 0; curr_item &&  i < tw_stages->topLevelItemCount(); i++)
    {
        auto  item = tw_stages->topLevelItem(i);
        if ( item && item != curr_item)
        {
            set_item_bkgnd( item, this->m_item_def_bkgnd);
        }
    }

    if (curr_item )
    {
        curr_item->treeWidget()->scrollToItem( curr_item->childCount() ? curr_item->child(curr_item->childCount() - 1) : curr_item);
    }

}



