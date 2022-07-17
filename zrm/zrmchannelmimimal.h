#ifndef ZRMCHANNELMIMIMAL_H
#define ZRMCHANNELMIMIMAL_H

#include "ui_zrmchannelmimimal.h"
#include <zrmbasewidget.h>
#include <qicon.h>

class ZrmChannelMimimal : public ZrmGroupWidget, private Ui::ZrmChannelMimimal
{
    Q_OBJECT

public:
    explicit ZrmChannelMimimal(QWidget* parent = nullptr);
    zrm::zrm_work_mode_t work_mode();
    virtual void update_ui() override;
    void set_active(bool active);
    void set_method(const zrm::zrm_method_t& method);
    void bind(zrm::ZrmConnectivity* src, uint16_t chan, bool _connect_signals = true) override;

signals:
    void clicked();

public slots:
    void start();
    void stop();
    void setColor(unsigned channel, QString color);

private slots:
    void expand(bool checked);

protected:
    void channel_session(unsigned ch_num) override;
    void mousePressEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* target, QEvent* event) override;
    void update_controls() override;
    void clear_controls() override;
    void channel_param_changed(unsigned channel, const zrm::params_list_t& params_list) override;
    void handle_error_state(unsigned err_code);
    void update_state(uint32_t state);

};

#endif // ZRMCHANNELMIMIMAL_H
