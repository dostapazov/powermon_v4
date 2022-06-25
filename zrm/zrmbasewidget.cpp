
#include "zrmbasewidget.h"
#include <qcoreapplication.h>
#include <QGraphicsDropShadowEffect>

QString           ZrmBaseWidget::infinity_symbol = QString("∞");
QString           ZrmBaseWidget::no_value        = QString("--");

QLatin1String     ZrmBaseWidget::m_codec_name  ;
QTextCodec*       ZrmBaseWidget::m_text_codec = Q_NULLPTR;

ZrmFlashTimer     ZrmBaseWidget::flash_timer;


ZrmFlashTimer::ZrmFlashTimer(QObject* parent ): QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &ZrmFlashTimer::on_timeout);
}

bool ZrmFlashTimer::is_flash_on()
{
    return m_flash_state & 1;
}

void  ZrmFlashTimer::on_timeout()
{
    ++m_flash_state;
    emit flash(is_flash_on());
}

void ZrmFlashTimer::start_flash()
{
    if (!m_timer.isActive())
        m_timer.start(std::chrono::milliseconds(333));
}

void ZrmFlashTimer::stop_flash ()
{
    if (m_timer.isActive())
        m_timer.stop();
}



ZrmBaseWidget::ZrmBaseWidget(QWidget* parent):
    QWidget(parent)
{
}

ZrmBaseWidget::~ZrmBaseWidget()
{
    ZrmBaseWidget::bind(Q_NULLPTR, 0);
}



QString       ZrmBaseWidget::channel_name(uint16_t chan)
{
    QString cname;
    if (m_source)
    {
        cname = m_source->name();
        if (chan)
            cname = QString("%1 : %2").arg(cname).arg(chan);
    }
    return cname;
}


void     ZrmBaseWidget::set_codec_name(const QLatin1String& str)
{
    m_codec_name = str;
    QByteArray array (m_codec_name.latin1());
    m_text_codec = QTextCodec::codecForName(array);
}


void    ZrmBaseWidget::connect_signals      (zrm::ZrmConnectivity* conn_obj, bool conn)
{
    if (conn_obj)
    {
        QSignalBlocker sb(conn_obj);
        if (!conn)
            conn_obj->disconnect(this);
        else
        {
            connect(conn_obj, &zrm::ZrmConnectivity::sig_connect, this, &ZrmBaseWidget::slot_connected       );
            connect(conn_obj, &zrm::ZrmConnectivity::sig_recv_packet, this, &ZrmBaseWidget::slot_recv_packet     );
            connect(conn_obj, &zrm::ZrmConnectivity::sig_send_packet, this, &ZrmBaseWidget::slot_send_packet     );
            connect(conn_obj, &zrm::ZrmConnectivity::sig_device_error, this, &ZrmBaseWidget::slot_ioerror         );
            connect(conn_obj, &zrm::ZrmConnectivity::sig_channel_change, this, &ZrmBaseWidget::slot_param_changes   );
            connect(conn_obj, &zrm::ZrmConnectivity::destroyed, this, &ZrmBaseWidget::slot_source_destroyed);
        }
    }
}

void     ZrmBaseWidget::bind(zrm::ZrmConnectivity*    src, uint16_t chan, bool _connect_signals)
{
    if (m_source != src || m_channel != chan)
    {

        on_connected(false);
        clear_controls();
        if (m_source != src)
        {
            connect_signals(m_source, false);
            m_source = src;
            connect_signals(src, _connect_signals);
        }
        qApp->processEvents();
        m_channel = chan;
        if (chan )
        {
            on_connected   (src->is_connected());
            //qDebug()<<this->objectName()<<" update_controls begin";
            update_controls();
            //qDebug()<<this->objectName()<<" update_controls end";
        }
    }
}

void    ZrmBaseWidget::slot_source_destroyed (QObject* obj)
{
    source_destroyed(reinterpret_cast<zrm::ZrmConnectivity*>(obj));
}

void ZrmBaseWidget::slot_connected     ( bool       conn_state)
{
    on_connected(conn_state);
}

void ZrmBaseWidget::slot_recv_packet   ( QByteArray packet    )
{
    if (packet.size())
    {
        auto  hdr = reinterpret_cast<const zrm::recv_header_t*>(packet.constData());
        channel_recv_packet(hdr->proto_hdr.channel, hdr);
    }
}

void ZrmBaseWidget::slot_send_packet   ( QByteArray packet    )
{
    if (packet.size())
    {
        auto  hdr = reinterpret_cast<const zrm::send_header_t*>(packet.constData());
        channel_send_packet(hdr->proto_hdr.channel, hdr);
    }
}

void ZrmBaseWidget::slot_ioerror       ( QString    error_string )
{
    on_ioerror(error_string);
}

void ZrmBaseWidget::slot_param_changes (unsigned    channel,  zrm::params_list_t params_list)
{
    channel_param_changed(channel, params_list);
}

void ZrmBaseWidget::on_connected  (bool con_state)
{
    Q_UNUSED(con_state)

}

void    ZrmBaseWidget::on_ioerror (const QString& error_string)
{
    Q_UNUSED(error_string) ;
}

void ZrmBaseWidget::channel_recv_packet  (unsigned channel, const zrm::recv_header_t* recv_data)
{
    Q_UNUSED(channel);
    Q_UNUSED(recv_data);
}

void ZrmBaseWidget::channel_send_packet  (unsigned channel, const zrm::send_header_t* send_data)
{
    Q_UNUSED(channel);
    Q_UNUSED(send_data);
}

void ZrmBaseWidget::channel_param_changed(unsigned channel, const zrm::params_list_t& params_list  )
{
    Q_UNUSED(channel);
    auto p = params_list.find(zrm::PARAM_CON);
    if (p != params_list.end())
    {
        channel_session(channel);
    }
    Q_UNUSED(params_list);
}


void    ZrmBaseWidget::channel_session      (unsigned ch_num)
{
    Q_UNUSED(ch_num);
}

void    ZrmBaseWidget::source_destroyed     (zrm::ZrmConnectivity* src)
{
    if (src == m_source)
    {
        m_source = Q_NULLPTR;
        clear_controls();
    }
}

void    ZrmBaseWidget::update_controls      ()
{
}

QString ZrmBaseWidget::number_text( double value, int precision)
{
    double multiplier = pow(10, precision) ;
    qint64 n_value = qRound(value * multiplier);
    value  = double(n_value) / multiplier;
    if (qFuzzyIsNull(value))
        value = .0;
    QString ret = QString::number(value, 'f', precision);
    return ret;

}


QString ZrmBaseWidget::zrm_method_duration_text(const zrm::zrm_method_t& method)
{
    QString method_time;
    if (method.m_method.duration())
    {
        char text[32] ;
        snprintf (text, sizeof(text), "%02u:%02u:%02u", unsigned(method.m_method.m_hours), unsigned(method.m_method.m_minutes), unsigned(method.m_method.m_secs));
        method_time = QString::fromLocal8Bit(text);
    }
    else
    {
        method_time = infinity_symbol;
    }
    return    method_time;

}

QString ZrmBaseWidget::number_text(int value, int width, int base)
{
    return QString("%1").arg(value, width, base) ;
}

bool  ZrmBaseWidget::load_ui(const QString& ui_file)
{
#ifdef QUILOADER_H
    QUiLoader loader;
    QFile file(ui_file);
    if (file.open(QFile::ReadOnly | QFile::ExistingOnly))
    {
        QWidget* widget =   loader.load(&file, this);
        init_ui(widget);
        return true;
    }
#else
    Q_UNUSED(ui_file)
#endif
    return false;
}

void    ZrmBaseWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    onActivate();
}

void    ZrmBaseWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    onDeactivate();
}

void ZrmBaseWidget::addShadow (QWidget* w, qreal offset, qreal blurRadius)
{
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setOffset(offset);
    shadow->setBlurRadius(blurRadius);
    w->setGraphicsEffect(shadow);
};


void    ZrmGroupWidget::update_ui()
{
    for (auto&& c : m_widgets)
        c->update_ui();
}

void    ZrmGroupWidget::zrm_widgets_clear    ()
{
    for (auto&& c : m_widgets)
        c->bind(Q_NULLPTR, 0);
    m_widgets.clear();
}

void    ZrmGroupWidget::zrm_widgets_make     ()
{
    m_widgets.clear();
    m_widgets = findChildren<ZrmBaseWidget*>();
}

void  ZrmGroupWidget::bind(zrm::ZrmConnectivity*    src, uint16_t chan, bool _connect_signals )
{
    ZrmBaseWidget::bind(src, chan, _connect_signals);
    zrm_widgets_make();
    for (auto&& c : findChildren<ZrmBaseWidget*>())
    {
        c->bind(src, chan, false);
    }
}


void ZrmGroupWidget::channel_recv_packet  (unsigned channel, const zrm::recv_header_t* recv_data)
{
    for (auto&& widget : m_widgets)
        widget->channel_recv_packet(channel, recv_data);
}

void ZrmGroupWidget::channel_send_packet  (unsigned channel, const zrm::send_header_t* send_data)
{
    for (auto&& widget : m_widgets)
        widget->channel_send_packet(channel, send_data);
}

void ZrmGroupWidget::channel_param_changed(unsigned channel, const zrm::params_list_t& params_list  )
{
    for (auto&& widget : m_widgets)
        widget->channel_param_changed(channel, params_list);
    ZrmBaseWidget::channel_param_changed(channel, params_list);
}


void  ZrmGroupWidget::on_connected         (bool con_state)
{
    for (auto&& widget : m_widgets)
        widget->on_connected(con_state);
}

void  ZrmGroupWidget::on_ioerror           (const QString& error_string)
{
    for (auto&& widget : m_widgets)
        widget->on_ioerror(error_string);
}

void  ZrmGroupWidget::source_destroyed     (zrm::ZrmConnectivity* src)
{
    for (auto&& widget : m_widgets)
        widget->source_destroyed(src);
    ZrmBaseWidget::source_destroyed(src);

}

void  ZrmGroupWidget::clear_controls       ()
{
    for (auto&& widget : m_widgets)
        widget->clear_controls();
    ZrmBaseWidget::clear_controls();
}

void  ZrmGroupWidget::update_controls ()
{
    for (auto&& widget : m_widgets)
        widget->update_controls();
    ZrmBaseWidget::update_controls();
}

