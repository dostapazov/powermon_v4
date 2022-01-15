#include "zrmdisplaywidget.h"
#include <qdatetime.h>

void ZrmDisplayWidget::on_tbMonClear_clicked()
{
    monitor->clear();
}

QByteArray   to_hex(const QByteArray & data)
{
    QByteArray ret;
    ret.resize((data.size()<<1)+data.size());
    bool empty = true;
    char * temp = ret.data();
    for(auto value : data)
    {
      quint8 byte = quint8(value);
      temp+=sprintf(temp,"%s%02X",empty ? "":" ",quint32(byte));
      empty = false;
    }
    return ret;
}

void  ZrmDisplayWidget::monitor_add      (bool rx, const QByteArray & packet)
{
  if(!tbMonPause->isChecked() && ((rx && tbMonRx->isChecked()) || (!rx && tbMonTx->isChecked())))
  {
     QColor color  = rx ? Qt::darkBlue : Qt::darkGreen;
     monitor->line_add(QString("%1 - %2 : ").arg(rx ? "RX":"TX").arg(QDateTime().currentDateTime().toString("hh:mm:ss.zzz")), color);
     monitor->line_add(to_hex( packet),color);
  }
}
