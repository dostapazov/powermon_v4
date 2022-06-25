#ifndef ZRMMONITOR_H
#define ZRMMONITOR_H

#include "ui_zrmmonitor.h"
#include <zrmbasewidget.h>

class ZrmMonitor : public ZrmChannelWidget, private Ui::ZrmMonitor
{
    Q_OBJECT

public:
    explicit ZrmMonitor(QWidget* parent = nullptr);

    void    mon_line_add    (const QString& hdr, const QString& text, QColor color);


protected:
    virtual void    on_connected         (bool con_state) override;
    virtual void    on_ioerror           (const QString& error_string) override;
    virtual void    channel_recv_packet  (unsigned channel, const zrm::recv_header_t* recv_data) override;
    virtual void    channel_send_packet  (unsigned channel, const zrm::send_header_t* send_data) override;
    virtual void    update_controls      () override;
    virtual void    clear_controls       () override;

private:
    void   initSignalConnections();

    bool    m_paused    = false;
    bool    m_enable_tx = true;
    bool    m_enable_rx = true;
    bool    m_details = false;
    QColor  m_color_tx;
    QColor  m_color_rx;
};

#endif // ZRMMONITOR_H
