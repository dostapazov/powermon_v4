#include "ZrmChannel.h"

#include <QPainter>

ZrmChannel::ZrmChannel(QWidget* parent) :
    ZrmBaseWidget(parent)
{
    setMinimumSize(184, 82);
}

void ZrmChannel::set_active(bool active)
{
    bActive = active;
    repaint();
}

void ZrmChannel::bind(zrm::ZrmConnectivity* src, uint16_t chan, bool _connect_signals)
{
    if (src == m_source && m_channel == chan)
        update_controls();
    ZrmBaseWidget::bind(src, chan, _connect_signals);
}

void ZrmChannel::clear_controls()
{
    handle_error_state(0);
    volt = 0.;
    curr = 0.;
    repaint();
}

void ZrmChannel::update_controls()
{
    if (m_source && m_channel)
        channel_param_changed(m_channel, m_source->channel_params(m_channel));
}

void ZrmChannel::channel_param_changed(unsigned channel, const zrm::params_list_t& params_list)
{
    if (channel == m_channel && m_source)
    {
        for (auto param : params_list)
        {
            QVariant value = m_source->param_get(m_channel, param.first);
            switch (param.first)
            {
                case zrm::PARAM_VOLT         :
                    volt = value.toDouble();
                    repaint();
                    break;
                case zrm::PARAM_CUR          :
                    curr = value.toDouble();
                    repaint();
                    break;
                case zrm::PARAM_ERROR_STATE  :
                    handle_error_state(param.second.udword);
                    break;
                default :
                    break;
            }
        }
    }
    ZrmBaseWidget::channel_param_changed(channel, params_list);
}

void ZrmChannel::channel_session(unsigned ch_num)
{
    if (m_source && ch_num == m_channel)
    {
        if (m_source->channel_session(m_channel).is_active())
        {
            zrm::params_t params;
            params.push_back(zrm::PARAM_CUR);
            params.push_back(zrm::PARAM_VOLT);
            params.push_back(zrm::PARAM_ERROR_STATE);
            m_source->channel_subscribe_params(m_channel, params, true);
        }
    }
}

void ZrmChannel::handle_error_state(unsigned err_code)
{
    setToolTip(m_source->zrm_error_text(err_code));
}

void ZrmChannel::mousePressEvent(QMouseEvent* event)
{
    emit clicked();
    ZrmBaseWidget::mousePressEvent(event);
}

void ZrmChannel::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    QFont f = p.font();
    f.setPointSize(10);
    p.setFont(f);

    // фон
    p.drawPixmap(0, 0, width(), height(), QPixmap(bActive ? ":zrm/icons/active.png" : ":zrm/icons/no_active.png"));

    // напряжение
    // длинна текста
    QString strValue = QString::number(volt, 'f', 1);
    QFontMetrics fm(p.font());
    QRect textRect = fm.boundingRect(strValue);
    // значение напряжения
    p.drawText(31 - textRect.width() / 2, 35 - textRect.height() / 2, textRect.width(), textRect.height(), Qt::AlignVCenter | Qt::AlignHCenter, strValue);

    // ток
    // длинна текста
    strValue = QString::number(curr, 'f', 1);
    textRect = fm.boundingRect(strValue);
    // значение тока
    p.drawText(142 - textRect.width() / 2, 35 - textRect.height() / 2, textRect.width(), textRect.height(), Qt::AlignVCenter | Qt::AlignHCenter, strValue);

    // шкаф / устройство
    // длинна текста
    zrm::ZrmChannelAttributes attrs = m_source->channelAttributes(m_channel);

    int box = attrs.box_number;
    int device = attrs.device_number;

    strValue = (box > 0) ? QString::number(box) : "";
    if (box > 0 && device > 0)
        strValue += " : ";
    if (device > 0)
        strValue += QString::number(device);
    textRect = fm.boundingRect(strValue);
    // шкаф / устройство
    p.drawText(87 - textRect.width() / 2, 42 - textRect.height(), textRect.width(), textRect.height(), Qt::AlignVCenter | Qt::AlignHCenter, strValue);

    ZrmBaseWidget::paintEvent(event);
}
