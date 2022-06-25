#include "zrmmonitor.h"
#include <qdatetime.h>
#include <zrmproto.hpp>
#include <QGraphicsDropShadowEffect>

ZrmMonitor::ZrmMonitor(QWidget* parent) :
    ZrmChannelWidget(parent)
{
    setupUi(this);
    addShadow(monitor, 6, 6);
    monitor->set_scrollbars(this->mon_vsbar, mon_hsbar);
    m_enable_rx = tbMonRx->isChecked();
    m_enable_tx = tbMonTx->isChecked();
    m_paused    = tbMonPause->isChecked();
    monitor->set_update_time(300);
    initSignalConnections();
}


void   ZrmMonitor::initSignalConnections()
{
    connect(tbMonClear,  &QAbstractButton::clicked, monitor, &QTextViewer::clear);
    connect(tbMonRx, &QAbstractButton::toggled, this, [this](bool checked) {m_enable_rx = checked;});
    connect(tbMonTx, &QAbstractButton::toggled, this, [this](bool checked) {m_enable_tx = checked;});
    connect(tbMonPause, &QAbstractButton::toggled, this, [this](bool checked) {m_paused = checked;});
    connect(tbDetails, &QAbstractButton::toggled, this, [this](bool checked) {m_details = checked;});
}

void    ZrmMonitor::mon_line_add    (const QString& hdr, const QString& text, QColor color)
{
    if (!m_paused && (!hdr.isEmpty() || !text.isEmpty()) )
    {
        if (!hdr.isEmpty())
        {
            monitor->line_add
            (
                QString("%1 : %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"), hdr),
                color
            );
        }

        for ( auto&& line : text.split(QChar('\n')))
        {
            if (line.isEmpty())
                continue;
            monitor->line_add( line, color );
        }
    }
}

void    ZrmMonitor::on_connected         (bool con_state)
{
    mon_line_add(QString(), con_state ? tr("Связь установлена") : tr("Обрыв связи"), con_state ? Qt::darkGreen : Qt::darkRed);
}

void    ZrmMonitor::on_ioerror           (const QString& error_string)
{
    mon_line_add(error_string, QString(), Qt::darkRed);
}

QString getMonText(const QByteArray& data, bool detail)
{
    if (detail)
    {
        QString text;
        const zrm::recv_header_t* header = reinterpret_cast<const zrm::recv_header_t*>(data.constData());
        text += QString("Channel № %1, ").arg(static_cast<uint32_t>(header->proto_hdr.channel));
        text += QString("Packet Number № %1, ").arg(static_cast<uint32_t>(header->proto_hdr.packet_number));
        text += QString("SSID %1, ").arg(static_cast<uint32_t>(header->proto_hdr.session_id));
        text += QString("Type %1, ").arg(QString("%1").arg(static_cast<uint32_t>(header->proto_hdr.type), 2, 16, QChar('0')).toUpper());
        text += QString("Data size %1\n").arg(static_cast<uint32_t>(header->proto_hdr.data_size));

        text += QByteArray(reinterpret_cast<const char*>(header->data), static_cast<int>(header->proto_hdr.data_size)).toHex(' ');

        return text;
    }
    return QString::fromLocal8Bit(data.toHex(' '));

}

void    ZrmMonitor::channel_recv_packet  (unsigned channel, const zrm::recv_header_t* recv_data)
{
    if (m_source && channel == m_channel && m_enable_rx)
    {
        QByteArray recvData(reinterpret_cast<const char*>(recv_data), int(recv_data->size()));
        mon_line_add
        (
            QString("RX - size %1").arg(recvData.size()),
            getMonText(recvData, m_details)
            , m_details ? Qt::GlobalColor::blue : Qt::GlobalColor::darkBlue// monitor->palette().color(QPalette::Link)
        );
    }
}

void    ZrmMonitor::channel_send_packet  (unsigned channel, const zrm::send_header_t* send_data)
{
    if (m_source && channel == m_channel && m_enable_tx)
    {
        QByteArray sendData(reinterpret_cast<const char*>(send_data), int(send_data->size()));
        mon_line_add
        (
            QString("TX - size %1").arg(sendData.size()),
            getMonText(sendData, m_details),
            m_details ? Qt::GlobalColor::red : Qt::GlobalColor::darkRed // monitor->palette().color(QPalette::LinkVisited)
        );
    }
}

void    ZrmMonitor::update_controls      ()
{
    monitor->clear();
    if (m_source && m_channel )
    {
        QString hdr = QString("%1 : %2 %3 № %4")
                      .arg(tr("Подключено к "), m_source->name(), tr("канал")).arg( m_channel);
        mon_line_add(
            hdr
            , QString("%1").arg(m_source->is_connected() ? tr("соединение установлено") : tr("обрыв связи"))
            , monitor->palette().color(QPalette::Text)
        );
    }
}

void   ZrmMonitor::clear_controls       ()
{
    monitor->clear();
}


