#include "zrmmonitor.h"
#include <qdatetime.h>

ZrmMonitor::ZrmMonitor(QWidget *parent) :
    ZrmChannelWidget(parent)
{
    setupUi(this);
    monitor->set_scrollbars(this->mon_vsbar,mon_hsbar);
    m_enable_rx = tbMonRx->isChecked();
    m_enable_tx = tbMonTx->isChecked();
    m_paused    = tbMonPause->isChecked();
    for(auto bt : btn_frame->findChildren<QAbstractButton*>())
    {
      if(bt == tbMonClear)
        connect(bt,  &QAbstractButton::clicked, monitor, &QTextViewer::clear);
        else
        connect(bt,  &QAbstractButton::toggled, this, &ZrmMonitor::btn_toggled);
    }

    monitor->set_update_time(200);

}


void   ZrmMonitor::btn_toggled(bool checked)
{
  auto src = sender();
  if(src == tbMonRx)
      m_enable_rx = checked;
  if(src == tbMonRx)
      m_enable_tx = checked;
  if(src == tbMonPause)
      m_paused    = checked;

}


void    ZrmMonitor::mon_line_add    (const QString & hdr, const QString & text, QColor color)
{
  if(!m_paused && (!hdr.isEmpty() || !text.isEmpty()) )
  {
    monitor->line_add( QString("%1 : %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")).arg(hdr),color );
    if(!text.isEmpty())
      monitor->line_add( text,color );
  }
}

void    ZrmMonitor::on_connected         (bool con_state)
{
  mon_line_add(QString(), con_state ? tr("Связь установлена") : tr("Обрыв связи") , con_state ? Qt::darkGreen : Qt::darkRed);
}

void    ZrmMonitor::on_ioerror           (const QString & error_string)
{
  mon_line_add(error_string, QString(), Qt::darkRed);
}

void    ZrmMonitor::channel_recv_packet  (unsigned channel, const zrm::recv_header_t *recv_data)
{
  if(m_source && channel == m_channel && m_enable_rx)
  {
    //auto hex_data = QByteArray(reinterpret_cast<const char*>(recv_data->data),int(recv_data->size())).toHex(' ');
    auto hex_data = QByteArray(reinterpret_cast<const char*>(recv_data),int(recv_data->size())).toHex(' ');
    mon_line_add(QString("RX"), QString::fromLocal8Bit(hex_data.toUpper()), monitor->palette().color(QPalette::Link));
  }
}

void    ZrmMonitor::channel_send_packet  (unsigned channel, const zrm::send_header_t * send_data)
{
  if(m_source && channel == m_channel)
  {
      //auto hex_data = QByteArray(reinterpret_cast<const char*>(send_data->data),int(send_data->size())).toHex(' ');
      auto hex_data = QByteArray(reinterpret_cast<const char*>(send_data),int(send_data->size())).toHex(' ');
      mon_line_add(QString("TX"), QString::fromLocal8Bit(hex_data.toUpper()), monitor->palette().color(QPalette::LinkVisited));
  }
}

void    ZrmMonitor::update_controls      ()
{
  if(m_source && m_channel )
  {
    monitor->clear();
    mon_line_add(
                QString("%1 : %2 %3 № %4").arg(tr("Подключено")).arg(m_source->name()).arg(tr("канал")).arg(m_channel)
               ,QString()
               ,monitor->palette().color(QPalette::Text)
               );
  }
}

void   ZrmMonitor::clear_controls       ()
{
  monitor->clear();
}


