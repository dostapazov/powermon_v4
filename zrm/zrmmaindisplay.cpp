#include "zrmmaindisplay.h"
#include "zrmmethodchoose.h"
#include <signal_bloker.hpp>
#include <qdesktopwidget.h>
#include <qscreen.h>
#include <QGraphicsDropShadowEffect>
#include <powermon_utils.h>
#include "ui_constraints.hpp"


ZrmMainDisplay::ZrmMainDisplay(QWidget *parent) :
    ZrmChannelWidget(parent)
{
    setupUi(this);

//    int pixSize = 40;
//    QPixmap pixWorkTime(":zrm/icons/work_time.png");
//    pixWorkTime = pixWorkTime.scaled(pixSize, pixSize, Qt::KeepAspectRatio);
//    labelWorkTime->setPixmap(pixWorkTime);
//    QPixmap pixStageNum(":zrm/icons/stage_num.png");
//    pixStageNum = pixStageNum.scaled(pixSize, pixSize, Qt::KeepAspectRatio);
//    labelStageNum->setPixmap(pixStageNum);
//    QPixmap pixCycleNum(":zrm/icons/cycle_num.png");
//    pixCycleNum = pixCycleNum.scaled(pixSize, pixSize, Qt::KeepAspectRatio);
//    labelCycleNum->setPixmap(pixCycleNum);
//    QPixmap pixVolt(":zrm/icons/voltage.png");
//    pixVolt = pixVolt.scaled(pixSize, pixSize, Qt::KeepAspectRatio);
//    labelVolt->setPixmap(pixVolt);
//    QPixmap pixCurr(":zrm/icons/current.png");
//    pixCurr = pixCurr.scaled(pixSize, pixSize, Qt::KeepAspectRatio);
//    labelCurr->setPixmap(pixCurr);
//    QPixmap pixCapacity(":zrm/icons/capacity.png");
//    pixCapacity = pixCapacity.scaled(pixSize, pixSize, Qt::KeepAspectRatio);
//    labelCapacity->setPixmap(pixCapacity);
//    QPixmap pixT(":zrm/icons/temperature.png");
//    pixT = pixT.scaled(pixSize, pixSize, Qt::KeepAspectRatio);
//    labelTemperature->setPixmap(pixT);

    auto addShadow = [](QWidget* w)
    {
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
        shadow->setOffset(4);
        shadow->setBlurRadius(5);
        w->setGraphicsEffect(shadow);
    };
    addShadow(bMethodAuto);
    addShadow(bMethodAny);
    addShadow(bMethodManual);
    addShadow(bCharge);
    addShadow(bDischarge);

    bind(Q_NULLPTR,0);

    connectSlots();
}

void ZrmMainDisplay::connectSlots()
{
    connect(bMethodAuto, &QAbstractButton::clicked, this, [this]() { select_method(false); });
    connect(bMethodAny, &QAbstractButton::clicked, this, [this]() { select_method(true); });
    connect(bMethodManual, &QAbstractButton::clicked, this, &ZrmMainDisplay::manual_method);


    connect(edTimeLimit , SIGNAL(textChanged(QString)), this, SLOT(manual_method_changed()));
    connect(sbCurrLimit , SIGNAL(valueChanged(double)), this, SLOT(manual_method_changed()));
    connect(sbVoltLimit , SIGNAL(valueChanged(double)), this, SLOT(manual_method_changed()));
    connect(bCurrDec    , SIGNAL(clicked()), this, SLOT(currLimitChange()));
    connect(bCurrInc    , SIGNAL(clicked()), this, SLOT(currLimitChange()));
    connect(bVoltDec    , SIGNAL(clicked()), this, SLOT(voltLimitChange()));
    connect(bVoltInc    , SIGNAL(clicked()), this, SLOT(voltLimitChange()));
    connect(sbCycleTotal, SIGNAL(valueChanged(int)), this, SLOT(manual_method_changed()));
    connect(bCharge     , SIGNAL(clicked(bool)), this, SLOT(manual_method_changed()));
    connect(bDischarge  , SIGNAL(clicked(bool)), this, SLOT(manual_method_changed()));
    connect(bStart      , &QAbstractButton::clicked, this, &ZrmMainDisplay::start);
    connect(bStop       , &QAbstractButton::clicked, this, &ZrmMainDisplay::stop);
    connect(bPause      , &QAbstractButton::clicked, this, &ZrmMainDisplay::pause);
    connect(bResetError , &QAbstractButton::clicked, this, &ZrmMainDisplay::reset_error);
}


/**
 * @brief ZrmMainWidget::update_controls
 * Установка всех параметров из источника
 */

void  ZrmMainDisplay::update_controls()
{
    if(m_source && m_channel)
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
    setEditText(lb_work_time,no_value);
    setEditText (edTimeLimit,no_value);
    setEditText (edMode,no_value);
    lbStageNum  ->setValue(0);
    lbStageTotal->setValue(0);
    lbCycleNum  ->setValue(0);
    sbCycleTotal->setValue(0);
    setEditText (edMethodName,no_value,0);
    bMethodAuto->setEnabled(false);
    bMethodAny->setEnabled(false);
    bMethodManual->setEnabled(false);
    bPause->setEnabled(false);
    bStart->setEnabled(false);
    bStop->setEnabled(false);
    setEditText(edMode,tr("Не назначено устройство"),0);
    handle_error_state(0);
}

void  ZrmMainDisplay::handle_error_state (uint32_t err_code)
{
  auto p = error_state->palette();
  p.setColor(QPalette::Text, Qt::red);
  error_state->setPalette(p);
  setEditText(error_state,m_source->zrm_error_text(err_code),0)  ;
  bResetError->setVisible(err_code);
}

void  ZrmMainDisplay::channel_param_changed(unsigned channel, const zrm::params_list_t & params_list  )
{
    SignalBlocker sb(findChildren<QWidget*>());
    if(channel == m_channel && m_source)
    {
        for(auto param : params_list)
        {
            QVariant value = m_source->param_get(m_channel, param.first);
            switch(param.first)
            {
            case zrm::PARAM_STATE        : update_state(param.second.udword); break;
            case zrm::PARAM_WTIME        : setEditText(lb_work_time,value.toString(),0); break;
            case zrm::PARAM_LTIME        : setEditText(edTimeLimit,value.toString(),0); break;
            case zrm::PARAM_CUR          : lbCurr->setValue(value.toDouble()); break;
            case zrm::PARAM_LCUR         : sbCurrLimit->setValue(value.toDouble()); break;
            case zrm::PARAM_VOLT         : lbVolt->setValue(value.toDouble()); break;
            case zrm::PARAM_LVOLT        : sbVoltLimit->setValue(value.toDouble()); break;
            case zrm::PARAM_CAP          : set_number_value(edCapacity, value.toDouble(), 3); break;
            case zrm::PARAM_MAXTEMP      : sbTemperature->setValue(value.toDouble()); break;
            case zrm::PARAM_STG_NUM      : lbStageNum->setValue(int(param.second.sdword)); break;
            case zrm::PARAM_LOOP_NUM     : lbCycleNum->setValue(int(param.second.sdword)); break;
            case zrm::PARAM_ERROR_STATE  : handle_error_state(param.second.udword); break;

#ifdef DEF_RUPREHT
            case zrm::PARAM_DOUT         : if (param.second.uword & 0x0001)
                {
                    bRupreht = true;
                    setEditText(edMode,QString("Брак батареи, этап %1").arg(m_source->param_get(m_channel, zrm::PARAM_STG_NUM).toInt()),0);
                }
                break;
            case zrm::PARAM_ZRMMODE      : if (!bRupreht) edMode->setText(m_source->zrm_mode_text(param.second.udword)); break;
#else
            case zrm::PARAM_ZRMMODE      : setEditText(edMode,m_source->zrm_mode_text(param.second.udword),0); break;
#endif
            case zrm::PARAM_METHOD_STAGES: setup_method();break;

            default: break;
            }
        }
    }
    ZrmChannelWidget::channel_param_changed(channel, params_list);
}

void  ZrmMainDisplay::channel_session  (unsigned ch_num)
{
    bool wr_enabled = false;
    if(ch_num == m_channel)
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
 if(m_source)
   {
      zrm::params_t params;
      unsigned pv_first = zrm::PARAM_STATE;
      unsigned pv_last  = zrm::PARAM_ZRMMODE+1;
      params.resize(zrm::params_t::size_type(pv_last - pv_first));
      auto pb = params.begin();
      while(pb < params.end())
      {
           *pb = zrm::params_t::value_type(pv_first++);
           ++pb;
      }
    m_source->channel_subscribe_params(m_channel, params,true);
   }
}

void  ZrmMainDisplay::setup_method()
{

    SignalBlocker sb(findChildren<QWidget*>());
    auto method = m_source->channel_get_method(m_channel, false);
    if(method.m_method.m_id != m_method_id)
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
    if(m_model_name.isEmpty())
    {
        method_name = to_utf(method.m_method.m_name, sizeof(method.m_method.m_name));
    }
    else
    {
        method_name = QString("%1:%2").arg(m_model_name).arg(to_utf(method.m_method.m_name, sizeof(method.m_method.m_name)));
    }

    method_name = method_name.remove('\u0000');

    setEditText(edMethodName,method_name,0);


    //set_number_value(lbStageTotal, int(method.stages_count()), 2, infinity_symbol);
    lbStageTotal->setValue(int(method.stages_count()));
    sbCycleTotal->setValue(method.m_method.m_cycles_count);

    if(!m_manual_change)
    {
        QString time_limit_string = zrm_method_duration_text(method);
        setEditText(edTimeLimit,time_limit_string,0);
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
        setEditText(edMode,m_source->zrm_mode_text(m_source->param_get(m_channel, zrm::PARAM_ZRMMODE).toUInt()),0);
    }
    bLastStop = stopped;
#endif
  bMethodAuto->setEnabled(stopped);
  bMethodAny->setEnabled(stopped);
  bMethodManual->setEnabled(stopped);
  bStart->setEnabled(stopped);
  bStart->setProperty("start", !stopped);
  style()->polish(bStart);
  bStop->setEnabled(!stopped);
  bStop->setProperty("stop", stopped);
  style()->polish(bStop);

  bPause->setVisible(!stopped || paused);
  bPause->setProperty("pause", paused);
  //bPause->setText   ( (paused ) ? tr("Дальше") : tr("Пауза"));
  style()->polish(bPause);
  update_method_controls();
}


void  ZrmMainDisplay::set_current_limits( )
{
    double minLimit = 0,maxLimit = 0;
#ifndef QT_DEBUG
    if(is_manual() && m_source)
    {

        if (bCharge->isChecked())
        {
            maxLimit = m_source->param_get(m_channel, zrm::PARAM_MCUR).toDouble();

        }

        if(bDischarge->isChecked())
        {
            maxLimit = m_source->param_get(m_channel, zrm::PARAM_MCURD).toDouble();
        }

        minLimit = -maxLimit;

    }
    else
#endif
    {
        minLimit = -100000;
        maxLimit = 100000;
    }

   sbCurrLimit->setMinimum(minLimit);
   sbCurrLimit->setMaximum(maxLimit);
}



void  ZrmMainDisplay::set_volt_limits()
{
   double limit = 100000;
#ifndef QT_DEBUG
   if(is_manual() && m_source )
     limit =  m_source->param_get(m_channel, zrm::PARAM_MVOLT).toDouble() ;
#endif

    sbVoltLimit->setMaximum(limit);
}

void  ZrmMainDisplay::update_method_controls()
{
    bool is_stopped = m_source && m_source->channel_is_stopped(m_channel);
    bool enabled = is_stopped && is_manual();


    edTimeLimit->setEnabled(enabled);
    sbCycleTotal->setEnabled(enabled);
    sbVoltLimit->setEnabled(enabled);
    sbCurrLimit->setEnabled(enabled);

    bCurrDec->setEnabled(enabled);
    bCurrInc->setEnabled(enabled);
    bVoltDec->setEnabled(enabled);
    bVoltInc->setEnabled(enabled);


    edTimeLimit->setReadOnly(!is_manual());
    sbCurrLimit->setReadOnly(!is_manual());
    sbVoltLimit->setReadOnly(!is_manual());
    sbCycleTotal->setReadOnly(!is_manual());

    bCurrDec->setVisible(is_manual());
    bCurrInc->setVisible(is_manual());

    bVoltDec->setVisible(is_manual());
    bVoltInc->setVisible(is_manual());
    bMethodManual->setChecked(is_manual());
    manualButtonsFrame->setVisible(is_manual());
    if(!is_manual())
    {
        manualButtons->setExclusive(false);
        bCharge->setChecked(false);
        bDischarge->setChecked(false);
        manualButtons->setExclusive(true);
    }

    set_current_limits();
    set_volt_limits();

//    if (!is_manual())
//    {

//        if (bCharge->isChecked())
//        {
//            double value = sbCurrLimit->value();
//            sbCurrLimit->setMinimum(-m_source->param_get(m_channel, zrm::PARAM_MCUR).toDouble());
//            sbCurrLimit->setMaximum(m_source->param_get(m_channel, zrm::PARAM_MCUR).toDouble());
//            if (value < 0)
//                sbCurrLimit->setValue(-value);
//        }
//        else
//        {
//            double value = sbCurrLimit->value();
//            sbCurrLimit->setMinimum(-m_source->param_get(m_channel, zrm::PARAM_MCURD).toDouble());
//            sbCurrLimit->setMaximum(m_source->param_get(m_channel, zrm::PARAM_MCURD).toDouble());
//            if (value > 0)
//                sbCurrLimit->setValue(-value);
//        }
//    }
//    else
//    {
//        sbVoltLimit->setMaximum(100000.);
//        sbCurrLimit->setMaximum(100000.);
//        sbCurrLimit->setMinimum(-100000.);
//    }
}


void ZrmMainDisplay::set_method_duration(zrm::zrm_method_t &method, const QString & str)
{
    method.m_method.m_hours = method.m_method.m_minutes = method.m_method.m_secs = 0;
    QStringList sl = str.split(':');
    int i = 0;
    for(QString & text : sl)
    {
      switch(i)
      {
       case 0: method.m_method.m_hours   = uint8_t(text.trimmed().toUInt());break;
       case 1: method.m_method.m_minutes = uint8_t(text.trimmed().toUInt());break;
       case 2: method.m_method.m_secs    = uint8_t(text.trimmed().toUInt());break;
       default : return;
      }
      ++i;
    }

}

void ZrmMainDisplay::currLimitChange()
{
    QDoubleSpinBox * sb = sbCurrLimit;
    double newValue = sb->value();
        if(sender() == bCurrDec)
        newValue -=  sb->singleStep();
    else
        newValue +=  sb->singleStep();
    qDebug()<< newValue <<"Limit "<< sb->maximum();
    sb->setValue(newValue );

}

void ZrmMainDisplay::voltLimitChange()
{
    QDoubleSpinBox * sb = sbVoltLimit;
    double newValue = sb->value();
    if(sender() == bVoltDec)
        newValue -=  sb->singleStep();
    else
        newValue +=  sb->singleStep();

    sb->setValue(newValue );

}

void ZrmMainDisplay::manual_method()
{
    bMethodManual->setChecked(true);

    double  currLimit = m_source->param_get(m_channel, zrm::PARAM_MCUR).toDouble();
    double  voltLimit = m_source->param_get(m_channel, zrm::PARAM_MVOLT).toDouble();

    sbCurrLimit->setMinimum(-currLimit);
    sbCurrLimit->setMaximum(currLimit);


    sbVoltLimit->setMinimum(0);
    sbVoltLimit->setMaximum(voltLimit);
    m_method_id = zrm::METHOD_MANUAL_ID;
    update_method_controls();
    bMethodAuto->setChecked(false);
    bMethodAny->setChecked(false);


    //if(bCharge->isChecked() || bDischarge->isChecked())
        manual_method_changed();

}


void ZrmMainDisplay::manual_method_changed()
{
    QSignalBlocker b1(sbVoltLimit);
    QSignalBlocker b2(sbCurrLimit);

    zrm::zrm_method_t method;
    QString text = tr("Ручной ");
    if(bCharge->isChecked())
        text +=bCharge->text() ;
    if(bDischarge->isChecked())
        text +=bDischarge->text() ;

    QByteArray name = codec() ? codec()->fromUnicode(text) : text.toLocal8Bit();
    zrm::method_t  & met = method.m_method ;
    met = zrm::method_t();
    met.m_id = 0;

    memcpy(met.m_name, name.constData(), std::min(sizeof(met.m_name), size_t(name.size())));

    set_method_duration(method, edTimeLimit->text());
    met.set_cycles(sbCycleTotal->value());

//    met.set_capacity(sbCurrLimit->value());
//    met.set_voltage(sbVoltLimit->value());
//    met.set_current(sbCurrLimit->value());
    double base_value =  100.0;
    met.set_capacity(base_value);
    met.set_voltage(base_value);
    met.set_current(base_value);


    zrm::stage_t st;
    st.m_number = 1;

    if(bCharge->isChecked() )
    {
        st.m_type   =  zrm::stage_type_t::STT_CHARGE ;
        st.set_charge_volt   (sbVoltLimit->value(), met);
        st.set_charge_curr   (sbCurrLimit->value(), met);
    }

    if(bDischarge->isChecked())
    {
        st.m_type   = zrm::stage_type_t::STT_DISCHARGE;
        st.set_discharge_volt(sbVoltLimit->value(), met);
        st.set_discharge_curr(sbCurrLimit->value(), met);
    }

    if(st.m_type)
    {
        method+= st;
    }

    m_manual_change = true;
    m_source->channel_set_method(m_channel, method);
}


void    ZrmMainDisplay::on_connected         (bool con_state)
{
  setEditText(edMode,con_state ? QString() : tr("Нет связи"),0);
}

void    ZrmMainDisplay::on_ioerror           (const QString & error_string)
{
   if(error_string.length())
      setEditText(edMode,error_string,0);
}

void ZrmMainDisplay::start()
{
    if(is_stopped())
        m_source->channel_start(m_channel);
}

void ZrmMainDisplay::stop()
{
    if(!is_stopped())
        m_source->channel_stop(m_channel);
}

void  ZrmMainDisplay::pause        ()
{
  if(is_paused())
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
      for (auto && btn : this->ctrlButtonFrame->findChildren<QPushButton*>())
      {
          QSize size(64,64);
          btn->setMinimumSize(size);
          btn->setMaximumSize(size);
          btn->setIconSize(size);
      }

   QSize icon_size(MAIN_DISPLAT_ICON_WIDTH,MAIN_DISPLAT_ICON_HEIGHT);

   for (auto && lbl : paramFrame->findChildren<QLabel*>() )
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
