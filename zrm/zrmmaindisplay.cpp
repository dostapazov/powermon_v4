#include "zrmmaindisplay.h"
#include "zrmmethodchoose.h"
#include <signal_bloker.hpp>
#include <qdesktopwidget.h>
#include <qscreen.h>
#include <QGraphicsDropShadowEffect>
#include <powermon_utils.h>
#include "ui_constraints.hpp"



ZrmMainDisplay::ZrmMainDisplay(QWidget* parent) :
    ZrmChannelWidget(parent)
{
    setupUi(this);
    setupIcons();

    addShadow(bMethodAuto, 4, 5);
    addShadow(bMethodAny, 4, 5);
    addShadow(bMethodManual, 4, 5);
    addShadow(bCharge, 4, 5);
    addShadow(bDischarge, 4, 5);

    style()->polish(bStart);
    style()->polish(bStop);
    style()->polish(bPause);
    style()->polish(bMethodManual);

    bind(Q_NULLPTR, 0);
    connectSlots();
}

void ZrmMainDisplay::setupIcons()
{
    int pixSize = 40;
    QPixmap pixWorkTime(":zrm/icons/work_time.png");
    pixWorkTime = pixWorkTime.scaled(pixSize, pixSize, Qt::KeepAspectRatio);
    labelWorkTime->setPixmap(pixWorkTime);
    QPixmap pixStageNum(":zrm/icons/stage_num.png");
    pixStageNum = pixStageNum.scaled(pixSize, pixSize, Qt::KeepAspectRatio);
    labelStageNum->setPixmap(pixStageNum);
    QPixmap pixCycleNum(":zrm/icons/cycle_num.png");
    pixCycleNum = pixCycleNum.scaled(pixSize, pixSize, Qt::KeepAspectRatio);
    labelCycleNum->setPixmap(pixCycleNum);
    QPixmap pixVolt(":zrm/icons/voltage.png");
    pixVolt = pixVolt.scaled(pixSize, pixSize, Qt::KeepAspectRatio);
    labelVolt->setPixmap(pixVolt);
    QPixmap pixCurr(":zrm/icons/current.png");
    pixCurr = pixCurr.scaled(pixSize, pixSize, Qt::KeepAspectRatio);
    labelCurr->setPixmap(pixCurr);
    QPixmap pixCapacity(":zrm/icons/capacity.png");
    pixCapacity = pixCapacity.scaled(pixSize, pixSize, Qt::KeepAspectRatio);
    labelCapacity->setPixmap(pixCapacity);
    QPixmap pixT(":zrm/icons/temperature.png");
    pixT = pixT.scaled(pixSize, pixSize, Qt::KeepAspectRatio);
    labelTemperature->setPixmap(pixT);
}


void ZrmMainDisplay::connectSlots()
{
    connect(bMethodAuto, &QAbstractButton::clicked, this, [this]() { select_method(false); });
    connect(bMethodAny, &QAbstractButton::clicked, this, [this]() { select_method(true); });
    connect(bMethodManual, &QAbstractButton::clicked, this, &ZrmMainDisplay::manual_method);
    connect(bCharge, &QAbstractButton::toggled, this,  &ZrmMainDisplay::manual_method_changed);
    connect(bDischarge, &QAbstractButton::toggled, this,  &ZrmMainDisplay::manual_method_changed);
    connect(sbCurrLimit, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ZrmMainDisplay::manual_method_changed);
    connect(sbVoltLimit, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ZrmMainDisplay::manual_method_changed);


    connect(bCurrDec, &QAbstractButton::clicked, this, &ZrmMainDisplay::currLimitChange);
    connect(bCurrInc, &QAbstractButton::clicked, this, &ZrmMainDisplay::currLimitChange);
    connect(bVoltDec, &QAbstractButton::clicked, this, &ZrmMainDisplay::voltLimitChange);
    connect(bVoltInc, &QAbstractButton::clicked, this, &ZrmMainDisplay::voltLimitChange);


    connect(bStart, &QAbstractButton::clicked, this, &ZrmMainDisplay::start);
    connect(bStop, &QAbstractButton::clicked, this, &ZrmMainDisplay::stop);
    connect(bPause, &QAbstractButton::clicked, this, &ZrmMainDisplay::pause);
    connect(bResetError, &QAbstractButton::clicked, this, &ZrmMainDisplay::reset_error);
}


/**
 * @brief ZrmMainWidget::update_controls
 * Установка всех параметров из источника
 */

void  ZrmMainDisplay::update_controls()
{
    if (m_source && m_channel)
    {
        channel_param_changed(m_channel, m_source->channel_params(m_channel));
        setup_method();
        bool value = m_source->channel_work_mode(m_channel) == zrm::zrm_work_mode_t::as_charger;
        QString cap_name = value ? tr("Ёмкость") : tr("Мощность");
        QString cap_unit = value ? tr("   Ач") : tr("   Вт");
        //labelCapacity->setText(cap_name);
        labelCapacity->setToolTip(cap_name);
        edCapacity->setSuffix(cap_unit);
        bMethodAuto->setEnabled(true);
        bMethodAny->setEnabled(true);
        bMethodManual->setEnabled(true);
    }
}

void ZrmMainDisplay::clear_controls()
{
    SignalBlocker sb(findChildren<QWidget*>());
    m_method_id = zrm::METHOD_UNKNOWN_ID;
    m_model_name = QString();
    lbVolt      ->setValue(0);
    sbVoltLimit ->setValue(0.0);
    lbCurr      ->setValue(0);
    sbCurrLimit ->setValue(0.0);
    edCapacity  ->setValue(0.0);
    sbTemperature->setValue(0.0);
    setEditText(lb_work_time, no_value);
    setEditText (edTimeLimit, no_value);
    setEditText (edMode, no_value);
    lbStageNum  ->setValue(0);
    lbStageTotal->setValue(0);
    lbCycleNum  ->setValue(0);
    sbCycleTotal->setValue(0);
    setEditText (edMethodName, no_value, 0);
    bMethodAuto->setEnabled(false);
    bMethodAny->setEnabled(false);
    bMethodManual->setEnabled(false);
    bPause->setEnabled(false);
    bStart->setEnabled(false);
    bStop->setEnabled(false);
    setEditText(edMode, tr("Не назначено устройство"), 0);
    handle_error_state(0);
}

void  ZrmMainDisplay::handle_error_state (uint32_t err_code)
{
    auto p = error_state->palette();
    p.setColor(QPalette::Text, Qt::red);
    error_state->setPalette(p);
    setEditText(error_state, m_source->zrm_error_text(err_code), 0)  ;
    bResetError->setVisible(err_code);
}

void  ZrmMainDisplay::channel_param_changed(unsigned channel, const zrm::params_list_t& params_list  )
{
    SignalBlocker sb(findChildren<QWidget*>());
    if (channel == m_channel && m_source)
    {
        for (auto param : params_list)
        {
            QVariant value = m_source->param_get(m_channel, param.first);
            switch (param.first)
            {
                case zrm::PARAM_STATE        :
                    update_state(param.second.udword);
                    break;
                case zrm::PARAM_WTIME        :
                    setEditText(lb_work_time, value.toString(), 0);
                    break;
                case zrm::PARAM_LTIME        :
                    setEditText(edTimeLimit, value.toString(), 0);
                    break;
                case zrm::PARAM_CUR          :
                    lbCurr->setValue(value.toDouble());
                    break;
                case zrm::PARAM_LCUR         :
                    sbCurrLimit->setValue(value.toDouble());
                    break;
                case zrm::PARAM_VOLT         :
                    lbVolt->setValue(value.toDouble());
                    break;
                case zrm::PARAM_LVOLT        :
                    sbVoltLimit->setValue(value.toDouble());
                    break;
                case zrm::PARAM_CAP          :
                    set_number_value(edCapacity, value.toDouble(), 3);
                    break;
                case zrm::PARAM_MAXTEMP      :
                    sbTemperature->setValue(value.toDouble());
                    break;
                case zrm::PARAM_STG_NUM      :
                    lbStageNum->setValue(int(param.second.sdword));
                    break;
                case zrm::PARAM_LOOP_NUM     :
                    lbCycleNum->setValue(int(param.second.sdword));
                    break;
                case zrm::PARAM_ERROR_STATE  :
                    handle_error_state(param.second.udword);
                    break;

#ifdef DEF_RUPREHT
                case zrm::PARAM_DOUT         :
                    if (param.second.uword & 0x0001)
                    {
                        bRupreht = true;
                        setEditText(edMode, QString("Брак батареи, этап %1").arg(m_source->param_get(m_channel, zrm::PARAM_STG_NUM).toInt()), 0);
                    }
                    break;
                case zrm::PARAM_ZRMMODE      :
                    if (!bRupreht)
                        edMode->setText(m_source->zrm_mode_text(param.second.udword));
                    break;
#else
                case zrm::PARAM_ZRMMODE      :
                    setEditText(edMode, m_source->zrm_mode_text(param.second.udword), 0);
                    break;
#endif
                case zrm::PARAM_METHOD_STAGES:
                    setup_method();
                    break;

                default:
                    break;
            }
        }
    }
    ZrmChannelWidget::channel_param_changed(channel, params_list);
}

void  ZrmMainDisplay::channel_session  (unsigned ch_num)
{
    bool wr_enabled = false;
    if (ch_num == m_channel)
    {
        wr_enabled = m_source && !m_source->channel_session(m_channel).is_read_only();
        make_request();
    }

    bStart->setEnabled(wr_enabled);
    bStop->setEnabled(wr_enabled);
    bPause->setEnabled(wr_enabled);
}

void  ZrmMainDisplay::make_request  ()
{
    if (m_source)
    {
        zrm::params_t params;

        constexpr size_t PARAMS_SIZE = 1 + zrm::PARAM_ZRMMODE  - zrm::PARAM_STATE;
        params.resize(zrm::params_t::size_type(PARAMS_SIZE));
        unsigned pv_first = zrm::PARAM_STATE;
        auto pb = params.begin();
        while (pb < params.end())
        {
            *pb = zrm::params_t::value_type(pv_first++);
            ++pb;
        }
        m_source->channel_subscribe_params(m_channel, params, true);
    }
}

void  ZrmMainDisplay::setup_method()
{

    SignalBlocker sb(findChildren<QWidget*>());
    auto method = m_source->channel_get_method(m_channel, false);
    if (method.m_method.m_id != m_method_id)
    {
        m_auto_method = method.m_method.method_kind() == zrm::method_kind_automatic;
        m_model_name = QString();
        m_method_id = method.m_method.m_id;

        manualButtonsFrame->setVisible(is_manual());
        if (is_manual())
        {
            bool charge = true;
            if (method.stages_count() > 0)
                charge = (zrm::stage_type_t::STT_CHARGE == method.m_stages[0].m_type);
            if (charge)
            {
                bCharge->setChecked(true);
                bDischarge->setChecked(false);
            }
            else
            {
                bCharge->setChecked(false);
                bDischarge->setChecked(true);
            }
            bMethodManual->setChecked(is_manual());
        }

        //update_method_controls();
    }
    else
    {
        m_model_name = QString();
    }
    //qDebug() << Q_FUNC_INFO  <<" method_id " << m_method_id;

    QString method_name;
    if (m_model_name.isEmpty())
    {
        method_name = to_utf(method.m_method.m_name, sizeof(method.m_method.m_name));
    }
    else
    {
        method_name = QString("%1:%2").arg(m_model_name, to_utf(method.m_method.m_name, sizeof(method.m_method.m_name)));
    }

    method_name = method_name.remove(QChar('\u0000'));

    setEditText(edMethodName, method_name, 0);


    //set_number_value(lbStageTotal, int(method.stages_count()), 2, infinity_symbol);
    lbStageTotal->setValue(int(method.stages_count()));
    sbCycleTotal->setValue(method.m_method.m_cycles_count);

    if (!m_manual_change)
    {
        QString time_limit_string = zrm_method_duration_text(method);
        setEditText(edTimeLimit, time_limit_string, 0);
    }

    auto param = m_source->param_get(m_channel, zrm::PARAM_STG_NUM);
    //set_number_value(lbStageNum, param.toInt(), 2);
    lbStageNum->setValue(param.toInt());
    update_method_controls();
    m_manual_change = false;

}

void ZrmMainDisplay::update_state    (uint32_t state)
{
    zrm::oper_state_t oper_state;
    oper_state.state = uint16_t(state);

    bool stopped = is_stopped();
    bool paused  = is_paused ();
//  qDebug()<<QString("Update state %1").arg(state,4,16,QLatin1Char('0'));
//  qDebug()<< tr(" auto on %1  is_stopped  %2 ").arg(oper_state.state_bits.auto_on).arg(stopped);
#ifdef DEF_RUPREHT
    static bool bLastStop = false;
    if (bRupreht && !stopped && bLastStop)
    {
        bRupreht = false;
        setEditText(edMode, m_source->zrm_mode_text(m_source->param_get(m_channel, zrm::PARAM_ZRMMODE).toUInt()), 0);
    }
    bLastStop = stopped;
#endif
    bMethodAuto->setEnabled(stopped);
    bMethodAny->setEnabled(stopped);
    bMethodManual->setEnabled(stopped);
    bStart->setEnabled(stopped);
    bStart->setProperty("start", !stopped);
    bStop->setEnabled(!stopped);
    bStop->setProperty("stop", stopped);

    bPause->setEnabled(!stopped);
    bPause->setProperty("pause", paused);
    //bPause->setText   ( (paused ) ? tr("Дальше") : tr("Пауза"));
    update_method_controls();
}

void  ZrmMainDisplay::update_method_controls()
{
    bool is_stopped = m_source && m_source->channel_is_stopped(m_channel);
    bool enabled = is_stopped && is_manual();


    sbCycleTotal->setEnabled(enabled);
    sbVoltLimit->setEnabled(enabled);
    sbCurrLimit->setEnabled(enabled);

    bCurrDec->setEnabled(enabled);
    bCurrInc->setEnabled(enabled);
    bVoltDec->setEnabled(enabled);
    bVoltInc->setEnabled(enabled);


    edTimeLimit->setReadOnly(!enabled);
    //edTimeLimit->setEnabled(enabled);

    sbCurrLimit->setReadOnly(!enabled);
    sbVoltLimit->setReadOnly(!enabled);
    sbCycleTotal->setReadOnly(!enabled);


    bCurrDec->setVisible(is_manual());
    bCurrInc->setVisible(is_manual());

    bVoltDec->setVisible(is_manual());
    bVoltInc->setVisible(is_manual());

    bCharge->setEnabled(enabled);
    bDischarge->setEnabled(enabled);

    bMethodManual->setChecked(is_manual());
    manualButtonsFrame->setVisible(is_manual());

    if (!is_manual())
    {
        manualButtons->setExclusive(false);
        bCharge->setChecked(false);
        bDischarge->setChecked(false);
        manualButtons->setExclusive(true);
    }

}


zrm::method_hms String2Duration(const QString& str)
{
    uint8_t hours = 0, minutes = 0, secunds = 0;
    QStringList sl = str.split(':');
    int i = 0;
    for (QString& text : sl)
    {
        switch (i)
        {
            case 0:
                hours   = uint8_t(text.trimmed().toUInt());
                break;
            case 1:
                minutes = uint8_t(text.trimmed().toUInt());
                break;
            case 2:
                secunds    = uint8_t(text.trimmed().toUInt());
                break;
            default :
                break;
        }
        ++i;
    }
    return std::make_tuple(hours, minutes, secunds);
}

void ZrmMainDisplay::set_method_duration(zrm::zrm_method_t& method, const QString& str)
{
    zrm::method_hms hms = String2Duration(str);
    method.m_method.m_hours = std::get<0>(hms);
    method.m_method.m_minutes = std::get<1>(hms);
    method.m_method.m_secs = std::get<2>(hms);

}

void ZrmMainDisplay::currLimitChange()
{
    QDoubleSpinBox* sb = sbCurrLimit;
    double newValue = sb->value();
    if (sender() == bCurrDec)
        newValue -=  sb->singleStep();
    else
        newValue +=  sb->singleStep();
    sb->setValue(newValue );
}

void ZrmMainDisplay::voltLimitChange()
{
    QDoubleSpinBox* sb = sbVoltLimit;
    double newValue = sb->value();
    if (sender() == bVoltDec)
        newValue -=  sb->singleStep();
    else
        newValue +=  sb->singleStep();
    sb->setValue(newValue );
}

double ZrmMainDisplay::getManualVoltage()
{
    double voltage = sbVoltLimit->value();
    double voltLimit = m_source->param_get(m_channel, zrm::zrm_param_t::PARAM_MVOLT).toDouble();
    if (!qFuzzyIsNull(voltLimit))
        voltage = qMin(sbVoltLimit->value(), voltLimit);
    return voltage;
}

double ZrmMainDisplay::getManualCurrent(bool charge)
{
    double currLimit = m_source->param_get(m_channel, charge ? zrm::zrm_param_t::PARAM_MCUR : zrm::zrm_param_t::PARAM_MCURD ).toDouble();
    double current = sbCurrLimit->value();
    if (!qFuzzyIsNull(currLimit))
        current = qMin(sbCurrLimit->value(), currLimit);
    return current;
}

zrm::zrm_method_t ZrmMainDisplay::create_manual_method(bool charge)
{
    zrm::zrm_method_t method;
    set_method_duration(method, edTimeLimit->text());

    QString text = tr("Ручной ") + (charge ? bCharge : bDischarge)->text();
    QByteArray name = codec() ? codec()->fromUnicode(text) : text.toLocal8Bit();
    method.m_method.m_id = zrm::METHOD_MANUAL_ID;
    memcpy(method.m_method.m_name, name.constData(), std::min(sizeof(method.m_method.m_name), size_t(name.size())));
    method.m_method.set_cycles(sbCycleTotal->value());

    double voltage = getManualVoltage();
    double current = getManualCurrent(charge);

    method.m_method.set_voltage(voltage);
    method.m_method.set_capacity(current);
    method.m_method.set_current(current);

    zrm::stage_t st;
    st.m_number = 1;

    if (charge)
    {
        st.m_type = zrm::stage_type_t::STT_CHARGE ;
        st.set_charge_volt   (voltage, method.m_method);
        st.set_charge_curr   (current, method.m_method);
    }
    else
    {
        st.m_type = zrm::stage_type_t::STT_DISCHARGE;
        st.set_discharge_volt(voltage, method.m_method);
        st.set_discharge_curr(current, method.m_method);
    }

    method += st;
    return method;

}

void ZrmMainDisplay::manual_method_changed()
{
    if (!is_manual())
        return;

    zrm::zrm_method_t method;

    if (bCharge->isChecked())
    {
        method = create_manual_method(true);
    }

    if (bDischarge->isChecked())
    {
        method = create_manual_method(false);
    }

    if (method.stages_count())
    {
        m_source->channel_set_method(m_channel, method);
    }
}


void ZrmMainDisplay::manual_method()
{
    bMethodManual->setChecked(true);

    m_method_id = zrm::METHOD_MANUAL_ID;
    update_method_controls();
    bMethodAuto->setChecked(false);
    bMethodAny->setChecked(false);

    manual_method_changed();
}

void    ZrmMainDisplay::on_connected         (bool con_state)
{
    setEditText(edMode, con_state ? QString() : tr("Нет связи"), 0);
}

void    ZrmMainDisplay::on_ioerror           (const QString& error_string)
{
    if (error_string.length())
        setEditText(edMode, error_string, 0);
}

void ZrmMainDisplay::start()
{
    if (is_stopped())
    {
        if (is_manual())
        {
            manual_method_changed();
        }
        m_source->channel_start(m_channel);
    }
}

void ZrmMainDisplay::stop()
{
    if (!is_stopped())
        m_source->channel_stop(m_channel);
}

void  ZrmMainDisplay::pause        ()
{
    if (is_paused())
        m_source->channel_start(m_channel);
    else
        m_source->channel_pause(m_channel);
}

void  ZrmMainDisplay::reset_error  ()
{
    m_source->channel_reset_error(m_channel);
}

void ZrmMainDisplay::select_method(bool bAbstract)
{
    ZrmMethodChoose dlg(this);
    zrm::zrm_work_mode_t wm = m_source->channel_work_mode(m_channel);
    dlg.set_mode(wm);
    dlg.setAbstract(bAbstract);
    dlg.adjustSize();
    if (QDialog::Accepted == dlg.exec())
    {
        bMethodManual->setChecked(false);
        bMethodAuto->setChecked(!bAbstract);
        bMethodAny->setChecked(bAbstract);

        zrm::zrm_method_t method;
        if (dlg.get_method(method, codec(), nullptr))
        {
            m_source->channel_set_method(m_channel, method);
        }
    }
}

#ifdef Q_OS_ANDROID
void ZrmMainDisplay::update_android_ui()
{
    for (auto&& btn : this->ctrlButtonFrame->findChildren<QPushButton*>())
    {
        QSize size(64, 64);
        btn->setMinimumSize(size);
        btn->setMaximumSize(size);
        btn->setIconSize(size);
    }

    QSize icon_size(MAIN_DISPLAT_ICON_WIDTH, MAIN_DISPLAT_ICON_HEIGHT);

    for (auto&& lbl : paramFrame->findChildren<QLabel*>() )
    {

        lbl->setMaximumSize(icon_size);
        lbl->setMinimumSize(icon_size);
    }


    bVoltDec->setMinimumWidth(MAIN_DISPLAT_ICON_WIDTH);
    bVoltInc->setMinimumWidth(MAIN_DISPLAT_ICON_WIDTH);
    bCurrDec->setMinimumWidth(MAIN_DISPLAT_ICON_WIDTH);
    bCurrInc->setMinimumWidth(MAIN_DISPLAT_ICON_WIDTH);
    tempButton->setMinimumSize(icon_size);
    tempButton->setIconSize(icon_size);

}
#endif


void ZrmMainDisplay::update_ui()
{
    ZrmChannelWidget::update_ui();
#ifdef Q_OS_ANDROID
    update_android_ui();
#endif
}
