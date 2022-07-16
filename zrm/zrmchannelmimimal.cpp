#include "zrmchannelmimimal.h"
#include <QGraphicsDropShadowEffect>
#include <zrmparamcvt.h>

ZrmChannelMimimal::ZrmChannelMimimal(QWidget* parent) :
    ZrmGroupWidget(parent)
{
    setupUi(this);
    set_active(false);
    extraPanel->setVisible(false);
    for (auto&& w : frame->findChildren<QWidget*>())
    {
        if (w == btStop || w == bExpand)
            continue;
        w->installEventFilter(this);
        w->setMouseTracking  (true);
    }

    connect(btStop, &QAbstractButton::clicked, this, &ZrmChannelMimimal::stop);
    connect(bExpand, &QAbstractButton::toggled, this, &ZrmChannelMimimal::expand);
}

void ZrmChannelMimimal::set_active(bool active)
{
    QWidget* shadowWidget = frame;
    if (active)
    {
        frame->setFrameShadow(QFrame::Shadow::Sunken);
        addShadow(shadowWidget, 4, 5, Qt::GlobalColor::darkBlue);
    }
    else
    {
        frame->setFrameShadow(QFrame::Shadow::Raised);
        QScopedPointerDeleteLater::cleanup(shadowWidget->graphicsEffect());
        shadowWidget->setGraphicsEffect(nullptr);
    }
}

QString getDevText(int box, int dev)
{

    if (box || dev)
        return QString::asprintf("%02d-%02d", box, dev);

    return QString("     ");
}

void ZrmChannelMimimal::bind(zrm::ZrmConnectivity* src, uint16_t chan, bool _connect_signals)
{
    if (m_source)
        disconnect(m_source, SIGNAL(sig_change_color(unsigned, QString)), this, SLOT(setColor(unsigned, QString)));
    if (src == m_source && m_channel == chan)
        update_controls();
    if (m_source && m_channel)
    {
        connect(m_source, &zrm::ZrmConnectivity::sig_change_color, this, &ZrmChannelMimimal::setColor);
    }

    ZrmGroupWidget::bind(src, chan, _connect_signals);

}

void ZrmChannelMimimal::setColor(unsigned channel, QString color)
{
    if (m_channel == channel)
    {
        QString style = QString("QFrame { background-color: %1 ; }").arg(color);
        frame->setStyleSheet(style);
    }
}

void  ZrmChannelMimimal::clear_controls  ()
{
    handle_error_state(0);
    volt->setValue(.0);
    curr->setValue(.0);
    volt->setSpecialValueText(ZrmBaseWidget::no_value);
    curr->setSpecialValueText(ZrmBaseWidget::no_value);
    ed_time->setText(no_value);

    btStop->setEnabled(false);
}

void  ZrmChannelMimimal::update_controls()
{
    QString str = "Нет связи";
    if (m_source && m_channel)
    {
        channel_param_changed(m_channel, m_source->channel_params(m_channel));
        auto sess = m_source->channel_session(m_channel);
        str = channel_name(m_channel);
        str += sess.is_active() ? QString(" [ ID %1 ]").arg(sess.session_param.ssID, 4, 16, QLatin1Char('0')).toUpper() : tr(" [ нет соединения ]");

        zrm::ZrmChannelAttributes attrs = m_source->channelAttributes(m_channel);

        QColor color(QRgb(attrs.color));
        setColor(m_channel, color.name());
        name->setText(getDevText(attrs.box_number, attrs.device_number));

    }
    setToolTip(str);

}

void ZrmChannelMimimal::update_state    (uint32_t state)
{
    zrm::oper_state_t oper_state;
    oper_state.state = uint16_t(state);
    bool stop_enable =  oper_state.state_bits.auto_on || oper_state.state_bits.start_pause;
    btStop->setEnabled(stop_enable);

}


void  ZrmChannelMimimal::channel_param_changed(unsigned channel, const zrm::params_list_t& params_list  )
{

    if (channel == m_channel && m_source)
    {
        for (auto param : params_list)
        {

            switch (param.first)
            {
                case zrm::PARAM_STATE        :
                    update_state(param.second.udword);
                    break;
                case zrm::PARAM_VOLT         :
                    volt->setValue( ZrmParamCvt::toDouble(param.second).toDouble());
                    break;
                case zrm::PARAM_CUR          :
                    curr->setValue(ZrmParamCvt::toDouble(param.second).toDouble());
                    break;
                case zrm::PARAM_WTIME        :
                    ed_time ->setText(ZrmParamCvt::toTime(param.second).toString());
                    break;
                case zrm::PARAM_STG_NUM      :
                    set_number_value(lbStageNum, param.second.value<int>(false), 2, "--");
                    break;
                case zrm::PARAM_ERROR_STATE  :
                    handle_error_state(param.second.udword);
                    break;
                //case zrm::PARAM_ZRMMODE      : edMode->setText(m_source->zrm_mode_text(param.second.udword)); break;
                default:
                    break;
            }
        }
    }
    ZrmGroupWidget::channel_param_changed(channel, params_list);
}

void  ZrmChannelMimimal::channel_session(unsigned ch_num)
{
    if (m_source && ch_num == m_channel)
    {
        if (m_source->channel_session(m_channel).is_active())
        {
            zrm::params_t params;
            params.push_back( zrm::PARAM_STATE       ) ;
            params.push_back( zrm::PARAM_CUR         ) ;
            params.push_back( zrm::PARAM_VOLT        ) ;
            params.push_back( zrm::PARAM_WTIME       ) ;
            params.push_back( zrm::PARAM_ERROR_STATE ) ;
            params.push_back( zrm::PARAM_MAXTEMP     ) ;
            m_source->channel_subscribe_params(m_channel, params, true);
            volt->setSpecialValueText( QString() );
            curr->setSpecialValueText( QString() );
        }
    }
}

void ZrmChannelMimimal::handle_error_state(unsigned err_code)
{
    setToolTip(m_source->zrm_error_text(err_code));
}

void ZrmChannelMimimal::mousePressEvent(QMouseEvent* event)
{
    emit clicked();
    ZrmGroupWidget::mousePressEvent(event);
};

void     ZrmChannelMimimal::start()
{
    if (m_source && m_channel)
        m_source->channel_start(m_channel);
}

void     ZrmChannelMimimal::stop ()
{
    if (m_source && m_channel)
        m_source->channel_stop(m_channel);
}

void     ZrmChannelMimimal::set_method(const zrm::zrm_method_t& method)
{
    if (m_source && m_channel)
        m_source->channel_set_method(m_channel, method);
}

zrm::zrm_work_mode_t ZrmChannelMimimal::work_mode()
{
    return (m_source && m_channel) ? m_source->channel_work_mode(m_channel) : zrm::zrm_work_mode_t::as_charger;
}

bool  ZrmChannelMimimal::eventFilter(QObject* target, QEvent* event)
{
//    switch (event->type())
//    {
//        case QEvent::MouseButtonRelease :
//            emit clicked();
//            break;
//        case QEvent::MouseButtonPress   :
//            emit clicked();
//            break;
//        default :
//            break;
//    }
    return ZrmBaseWidget::eventFilter(target, event);
}

void ZrmChannelMimimal::update_ui()
{
#ifdef Q_OS_ANDROID
    QSize icon_size(42, 42);
    btStop->setIconSize(icon_size);
    btStop->setMaximumHeight(48);
#endif
}

void ZrmChannelMimimal::expand(bool checked)
{
    extraPanel->setVisible(checked);
    bExpand->setArrowType(checked ? Qt::ArrowType::UpArrow : Qt::ArrowType::DownArrow);
}

