#include "zrmreadyaccum.h"

#include <QToolButton>
#include <QStyle>
#include <QAction>
#include <QSound>

ZrmReadyAccum::ZrmReadyAccum(QWidget* parent) : ZrmBaseWidget (parent)
{
    setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    setGeometry(0, 0, 48, 48);
    connect (&ZrmBaseWidget::flash_timer, &ZrmFlashTimer::flash, this, &ZrmReadyAccum::update_view);
}

void ZrmReadyAccum::channel_param_changed(unsigned channel, const zrm::params_list_t& params_list)
{
    zrm::ZrmConnectivity* conn = dynamic_cast<zrm::ZrmConnectivity*>(sender());
    if (!conn)
        conn = m_source;

    auto p = params_list.find(zrm::PARAM_STATE);
    if (p != params_list.end())
        handle_state(conn, channel, p->second.udword);

    p = params_list.find(zrm::PARAM_ERROR_STATE);
    if (p != params_list.end())
        handle_error_state(conn, channel, p->second.udword);
    ZrmBaseWidget::channel_param_changed(channel, params_list);
}

void ZrmReadyAccum::source_destroyed(zrm::ZrmConnectivity* src)
{
    Q_UNUSED(src)
}

void ZrmReadyAccum::handle_state(zrm::ZrmConnectivity* __conn, unsigned channel, uint32_t state)
{
    Q_UNUSED(__conn)
    Q_UNUSED(channel)
    Q_UNUSED(state)
    int exec_count = 0;
    for (auto&& con : zrm::ZrmConnectivity::connectivities())
    {
        m_source = con;
        for (auto&& chan : con->channels())
            if (con->channel_is_executing(chan))
                ++exec_count;
    }
    m_source = nullptr;
    if (m_exec_count != exec_count)
    {
        m_exec_count = exec_count;
        qDebug() << QString(" Running count %1").arg(exec_count);
        update_view(true);
    }
}

void ZrmReadyAccum::channel_session(unsigned ch_num)
{
    if (m_source)
        m_source->channel_subscribe_param(uint16_t(ch_num), zrm::PARAM_ERROR_STATE, true);
}

void ZrmReadyAccum::handle_error_state(zrm::ZrmConnectivity* __conn, unsigned channel, uint32_t error_code)
{
    Q_UNUSED(__conn)
    Q_UNUSED(channel)
    Q_UNUSED(error_code)

    int error_count = 0;
    for (auto&& con : zrm::ZrmConnectivity::connectivities())
    {
        m_source = con;
        for (auto&& chan : con->channels())
            if (param_get(chan, zrm::PARAM_ERROR_STATE).toUInt())
                ++error_count;
    }
    m_source = nullptr;

    if (m_error_count != error_count)
    {
        if (m_error_count < error_count)
            QSound::play(":/zrm/sounds/alarm.wav");

        qDebug() << QString(" Error count %1").arg(error_count);
        m_error_count = error_count;
        update_view(true);
        if (error_code)
            ZrmBaseWidget::flash_timer.start_flash();
    }
}

void ZrmReadyAccum::update_connectivities()
{
    for (auto&& con : zrm::ZrmConnectivity::connectivities())
    {
        connect_signals(con, true);
        for (auto&& ch : con->channels())
        {
            m_source = con;
            channel_param_changed(ch, con->channel_params(ch));
        }
    }
    m_source = nullptr;
}

QString ZrmReadyAccum::get_current_tip ()
{
    QString ret = m_action_tip;
    if (m_error_count)
        ret = tr("Ошибка при выполнении");
    else if (m_exec_count)
        ret = m_exec_count == zrm::ZrmConnectivity::channels_total() ? tr("Все каналы работают") : tr("Не все каналы стартовали");
    return ret;
}

void ZrmReadyAccum::update_view(bool flash_on)
{
    if (m_action)
        m_action->setToolTip(get_current_tip());
    if (button)
    {
        QString color = "blue";
        if (m_error_count)
            color = (flash_on) ? "red" : "yellow";
        else if (m_exec_count)
            color = m_exec_count == zrm::ZrmConnectivity::channels_total() ? "green" : "yellow";
        button->setProperty("error-state", color);
        style()->polish(button);
    }
}

void ZrmReadyAccum::setButton(QToolButton* tb)
{
    if (button == tb)
        return;

    button = tb;
    if (!button)
    {
        m_action_tip = QString();
        m_action = nullptr;
        return;
    }

    m_action_tip  =  tb->toolTip();
    m_action = tb->defaultAction();
}
