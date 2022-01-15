/*Реализация дисплея зарядного устройства
 *
 *
 */

#include "zrmdisplaywidget.h"
#include <qfiledialog.h>
#include <qstandardpaths.h>
#include <qdatetime.h>
#include <zrmmethodchoose.h>


QIcon             ZrmDisplayWidget::network_rx;
QIcon             ZrmDisplayWidget::network_tx;
QIcon             ZrmDisplayWidget::network_idle;
QIcon             ZrmDisplayWidget::network_offline;

QIcon             ZrmDisplayWidget::relay_off;
QIcon             ZrmDisplayWidget::relay_on;
QIcon             ZrmDisplayWidget::current_out;
QIcon             ZrmDisplayWidget::current_in;
QIcon             ZrmDisplayWidget::modeU;
QIcon             ZrmDisplayWidget::modeUstab;
QIcon             ZrmDisplayWidget::modeI;
QIcon             ZrmDisplayWidget::modeIstab;
QIcon             ZrmDisplayWidget::pause_icon;
QIcon             ZrmDisplayWidget::empty_icon;



ZrmDisplayWidget::ZrmDisplayWidget(QWidget *parent) :
    ZrmBaseWidget(parent)
{
    setupUi(this);
#ifdef Q_OS_ANDROID
   setup_android_ui();
#endif
    init_icons();

    auto header = tw_stages->header();
    header->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    header->setSectionResizeMode(2,QHeaderView::ResizeMode::Stretch);

    connect(cb_report_details ,&QAbstractButton::clicked  , this, &ZrmDisplayWidget::gen_result_report);
    connect(ed_report_maker   ,&QLineEdit::textChanged    , this, &ZrmDisplayWidget::gen_result_report);
    connect(ed_akb_type       ,&QLineEdit::textChanged    , this, &ZrmDisplayWidget::gen_result_report);
    connect(ed_akb_number     ,&QLineEdit::textChanged    , this, &ZrmDisplayWidget::gen_result_report);
    connect(&watchdog         ,&QTimer::timeout           , this, &ZrmDisplayWidget::on_watchdog_timeout);

    bMinimize->setDefaultAction(actMinimize);
    on_actMinimize_toggled(false);

    monitor->set_update_time(2000);
    monitor->set_scrollbars(mon_vsbar,mon_hsbar);
    tabWidget->setCurrentIndex(0);
}

#ifdef Q_OS_ANDROID
void ZrmDisplayWidget::setup_android_ui()
{
    qDebug()<<"Setup android UI";
    QSize sz(96,96);
    auto tb_list =  findChildren<QToolButton*>();
    for(auto tb: tb_list)
    {
        tb->setIconSize(sz);
    }

    auto lb_list =  fr_state->findChildren<QLabel*>();
    for(auto lb: lb_list)
    {
      if(lb->minimumSize().width())
      {
        lb->setMinimumSize(sz);
        lb->setMaximumSize(sz);
      }
    }
}
#endif


ZrmDisplayWidget::~ZrmDisplayWidget()
{}

void  ZrmDisplayWidget::bind(zrm::ZrmConnectivity   * src, bool conn_signals)
{
   if(m_source != src)
   {
     ZrmBaseWidget::bind(src, conn_signals );
   }
}

void ZrmDisplayWidget::init_icons()
{
  if(relay_on.isNull())
  {
    QString icon_path = QLatin1String(":/zrm/icons/");
    relay_off  = QIcon(icon_path+QLatin1String("relay_off.png"  ));
    relay_on   = QIcon(icon_path+QLatin1String("relay_on.png"   ));
    current_out= QIcon(icon_path+QLatin1String("current_out.png"));
    current_in = QIcon(icon_path+QLatin1String("current_in.png" ));

    modeU      = QIcon(icon_path+QLatin1String("modeU.png"    ));
    modeUstab  = QIcon(icon_path+QLatin1String("modeUstab.png"));
    modeI      = QIcon(icon_path+QLatin1String("modeI.png"    ));
    modeIstab  = QIcon(icon_path+QLatin1String("modeIstab.png"));
    pause_icon = QIcon(icon_path+QLatin1String("control_pause.png"));

    network_rx = QIcon(icon_path+QLatin1String("network-receive.png"));
    network_tx = QIcon(icon_path+QLatin1String("network-transmit.png"));
    network_idle    = QIcon(icon_path+QLatin1String("network-idle.png"));
    network_offline = QIcon(icon_path+QLatin1String("network-offline.png"));
  }
}


void     ZrmDisplayWidget::set_channel(uint16_t value)
{
   if(m_channel != value)
   {
      m_channel = value;
      update_controls();
   }
}



/**
 * @brief ZrmDisplayWidget::update_controls
 * Установка всех параметров из источника
 */

void  ZrmDisplayWidget::update_controls()
{
  if(m_source)
  {
    channel_param_changed(m_channel,m_source->channel_params(m_channel));
    bool value = m_source->channel_work_mode(m_channel) == zrm::zrm_work_mode_t::as_charger;
    QString cap_name = value ? tr("Ёмкость"):tr("Мощьность");
    QString cap_unit = value ? tr("Ач"):tr("Вт");
    lb_cap_name->setText (cap_name);
    lb_cap_unit1->setText(cap_unit);
    edManCap->setSuffix(QString("  %1").arg(cap_unit));
  }
  else
    clear_controls();
  lb_dev_addr->setText(QString("%1").arg(m_channel,4,10,QLatin1Char('0')));
}


void ZrmDisplayWidget::clear_controls()
{
  lbVolt      ->setText(no_value);
  lbVoltLimit ->setText(no_value);
  lbCurr      ->setText(no_value);
  lbCurrLimit ->setText(no_value);
  lbCapacity  ->setText(no_value);
  lbWorkTime  ->setText(no_value);
  lbTimeLimit ->setText(no_value);
  lbMode      ->setText(no_value);
  lbStageNum  ->setText(no_value);
  lbStageTotal->setText(no_value);
  lbCycleNum  ->setText(no_value);
  lbCycleTotal->setText(no_value);
}


void    ZrmDisplayWidget::set_label_icon(QLabel * label,const QIcon * icon)
{
    label->setPixmap(icon ? icon->pixmap(label->size()) : QPixmap());
}

void ZrmDisplayWidget::set_message_text(const QString & msg, ZrmDisplayWidget::msg_type_t msg_type)
{
  QString style_sheet;
  QColor color = Qt::black;
  if(msg_type != msg_info)
  {
   color    = msg_type == msg_error ? Qt::darkRed : Qt::darkGreen;
   QRgb rgb = color.rgb();
   style_sheet = tr("color:rgb(%1,%2,%3);").arg(qRed(rgb)).arg(qGreen(rgb)).arg(qBlue(rgb));
  }
  lb_message->setStyleSheet(style_sheet);
  lb_message->setText(msg);
  if(!msg.isEmpty())
  {
   monitor->line_add("Message",color);
   monitor->line_add(msg,color);
  }
}

void    ZrmDisplayWidget::on_watchdog_timeout()
{
  if(--m_watchdog_value < 0 )
  {
   on_connected(false);
   set_message_text(tr("Устройство не отвечает"),msg_error);
  }
}



void    ZrmDisplayWidget::on_connected     ( bool       conn_state)
{
 if(m_connected != conn_state)
 {
    if(!conn_state)
    {
     set_label_icon(lb_connect_status, &network_offline);
     set_message_text(tr("Обрыв связи с устройством"),msg_error);
     watchdog.stop();
    }
    else
    {
     watchdog.start(std::chrono::seconds(1));
     m_watchdog_value = m_watchdog_period;
    }
  m_connected = conn_state;
 }
}

void    ZrmDisplayWidget::channel_send_packet  (unsigned channel, const zrm::send_header_t * send_data)
{
  if(channel == m_channel)
  {
    set_label_icon(lb_connect_status, &network_tx);
    monitor_add(false,QByteArray(reinterpret_cast<const char*>(send_data), int(send_data->size())));
    on_connected(true);
  }
}

void  ZrmDisplayWidget::channel_recv_packet  (unsigned channel, const zrm::recv_header_t * recv_data)
{
  if( channel == m_channel)
  {
    monitor_add(true, QByteArray(reinterpret_cast<const char*>(recv_data), int(recv_data->size())));
   if(!(++m_rcv_counter&3))
    {
      set_label_icon(lb_connect_status, m_switch_led ? &network_rx : &network_idle);
      m_switch_led = !m_switch_led;
    }

   m_watchdog_value = m_watchdog_period;
  }
}

void  ZrmDisplayWidget::on_ioerror           (const QString & error_string)
{
 set_message_text(error_string, msg_error);
}

void  ZrmDisplayWidget::channel_session  ()
{
  bool wr_enabled = !m_source->channel_session(m_channel).is_read_only();

  zrm::params_t params;
  make_request_params(params);
  m_source->channel_request_params(m_channel, params,wr_enabled);

  bStartStop->setEnabled(wr_enabled);
  bPause->setEnabled(wr_enabled);
}


void  ZrmDisplayWidget::channel_param_changed(unsigned channel, const zrm::params_list_t & params_list  )
{
  if(channel == m_channel)
  {
     bool is_minimized = !tabWidget->isVisible();
     bool capt_changed = false;
     for(auto param : params_list)
     {
       QVariant value = m_source->param_get(m_channel, param.first);
       switch(param.first)
       {
         case zrm::PARAM_CON          : channel_session(); break;
         case zrm::PARAM_STATE        : update_state(param.second.udword);break;
         case zrm::PARAM_WTIME        : lbWorkTime ->setText(value.toString()); capt_changed = true;break;
         case zrm::PARAM_LTIME        : lbTimeLimit->setText(value.toString()); break;
         case zrm::PARAM_CUR          : set_number_value(lbCurr     , value.toDouble(), 1);capt_changed = true; break;
         case zrm::PARAM_LCUR         : set_number_value(lbCurrLimit, value.toDouble(), 1); break;
         case zrm::PARAM_VOLT         : set_number_value(lbVolt     , value.toDouble(), 1);capt_changed = true; break;
         case zrm::PARAM_LVOLT        : set_number_value(lbVoltLimit, value.toDouble(), 1); break;
         case zrm::PARAM_CAP          : set_number_value(lbCapacity , value.toDouble(), 3);capt_changed = true; break;
         case zrm::PARAM_STG_NUM      : set_current_stage(int(param.second.sdword));break;
         case zrm::PARAM_LOOP_NUM     : set_number_value(lbCycleNum, int(param.second.sdword), 2);break;
         case zrm::PARAM_ERROR_STATE  : set_message_text(this->m_source->zrm_error_text(param.second.udword),msg_error); break;
         case zrm::PARAM_FAULTL_DEV   :break;
         case zrm::PARAM_MID          :break;
         case zrm::PARAM_TRECT        :break;
         case zrm::PARAM_TCONV        :break;
         case zrm::PARAM_VOUT         :break;
         case zrm::PARAM_DOUT         :break;
         case zrm::PARAM_DIN          :break;
         case zrm::PARAM_CCNT         :break;
         case zrm::PARAM_ZRMMODE      :lbMode->setText(m_source->zrm_mode_text(param.second.udword)); break;
         case zrm::PARAM_TEMP         :break;
         case zrm::PARAM_METH_EXEC_RESULT: gen_result_report();break;
         case zrm::PARAM_METHOD_STAGES   : setup_method()     ;break;
         default: break;
      }
     }
    if(is_minimized && capt_changed) update_minimized_info();

  }
}

void ZrmDisplayWidget::update_minimized_info()
{
 QString str = tr("U:%1 I:%2 C:%3 [%4]").arg(lbVolt->text())
                                         .arg(lbCurr->text())
                                         .arg(lbCapacity->text())
                                         .arg(lbWorkTime->text())
         ;

 set_message_text(str,msg_info);
}

static void set_item_bkgnd(QTreeWidgetItem * item , const QBrush & bkgnd)
{
    item->setBackground(1, bkgnd);
    item->setBackground(2, bkgnd);
    for(int i = 0 ; i< item->childCount();i++)
    {
       set_item_bkgnd(item->child(i), bkgnd);
    }

}



void ZrmDisplayWidget::set_current_stage(int stage_num)
{
  set_number_value(lbStageNum,stage_num, 2, QString());
  QTreeWidgetItem * curr_item =  Q_NULLPTR;
  if(stage_num > 0 && stage_num <= tw_stages->topLevelItemCount())
  {
     curr_item  = tw_stages->topLevelItem(int(stage_num-1));
     set_item_bkgnd( curr_item, QBrush(Qt::darkGreen));
     curr_item->setExpanded(true);

  }

  for(int i = 0;curr_item &&  i< tw_stages->topLevelItemCount();i++)
  {
    auto  item = tw_stages->topLevelItem(i);
    if( item && item != curr_item)
     {
       set_item_bkgnd( item, this->m_item_def_bkgnd);
     }
  }

}



void ZrmDisplayWidget::update_state    (uint32_t state)
{
  zrm::oper_state_t oper_state;
  oper_state.state = uint16_t(state);

  bool is_stopped = m_source->channel_is_stopped(m_channel);
  bool is_paused  = m_source->channel_is_paused (m_channel);
  qDebug()<<QString("Update state %1").arg(state,4,16,QLatin1Char('0'));
  qDebug()<< tr(" auto on %1  is_stopped  %2 ").arg(oper_state.state_bits.auto_on).arg(is_stopped);
  bMethod->setEnabled(is_stopped);

  bStartStop->setText( (is_stopped) ? tr("Пуск")       : tr("Стоп"));
  bPause->setText    ( (is_paused ) ? tr("Продолжить") : tr("Пауза"));

  set_label_icon     ( lb_relay_state, oper_state.state_bits.relay_on ? &relay_on : &relay_off );

  if(oper_state.state_bits.start_rectifier || oper_state.state_bits.start_load)
  {
     set_label_icon(lb_curr_dir,oper_state.state_bits.start_rectifier ? &current_out : &current_in);
     QIcon * icon = Q_NULLPTR;

     if(oper_state.state_bits.ctr_stab)
        icon =  oper_state.state_bits.start_rectifier ? &modeUstab : &modeIstab;
     else
       icon = oper_state.state_bits.start_rectifier ? &modeU : &modeI;

     set_label_icon(lb_mode,icon);

  }
  else
  {
      set_label_icon(lb_mode, Q_NULLPTR);
      set_label_icon(lb_curr_dir,oper_state.state_bits.start_pause ? &pause_icon : Q_NULLPTR);
  }

}

void ZrmDisplayWidget::make_request_params(zrm::params_t & req_param)
{
      req_param.clear();
      req_param.push_back(zrm::PARAM_STATE      );
      req_param.push_back(zrm::PARAM_WTIME      );    //Время работы 2
      req_param.push_back(zrm::PARAM_LTIME      );    //Ограничение времени 3
      req_param.push_back(zrm::PARAM_CUR        );    //Ток 4
      req_param.push_back(zrm::PARAM_LCUR       );    //Ограничение тока 5
      req_param.push_back(zrm::PARAM_VOLT       );    //Напряжение 6
      req_param.push_back(zrm::PARAM_LVOLT      );    //Оганичение напряжения 7
      req_param.push_back(zrm::PARAM_CAP        );    //Ёмкость 8
      req_param.push_back(zrm::PARAM_TEMP       );    //Температура 9
      req_param.push_back(zrm::PARAM_STG_NUM    );    //Номер этапа 10
      req_param.push_back(zrm::PARAM_LOOP_NUM   );    //Номер цикла 11
      req_param.push_back(zrm::PARAM_ERROR_STATE);    //Код ошибки состояния ЗРМ 12
      req_param.push_back(zrm::PARAM_FAULTL_DEV );    //Номер неисправного ЗРМ 13
      req_param.push_back(zrm::PARAM_MID        );    //ID выполняемого метода 14
      req_param.push_back(zrm::PARAM_TRECT      );    //Температура выпрямителя ЗРМ
      req_param.push_back(zrm::PARAM_TCONV      );    //Температура преобразователя
      req_param.push_back(zrm::PARAM_VOUT       );    //Напряжение на выходе ЗРМ
      req_param.push_back(zrm::PARAM_DOUT       );    //Дискретные выходы
      req_param.push_back(zrm::PARAM_DIN        );    //Дискретные входы
      req_param.push_back(zrm::PARAM_CCNT       );
      req_param.push_back(zrm::PARAM_ZRMMODE    );

}

//    zrm::params_t params;

void ZrmDisplayWidget::on_bStartStop_clicked()
{
  if(m_source)
  {
    if(m_source->channel_is_stopped(m_channel))
    {
       m_source->channel_start(m_channel);
    }
    else
      m_source->channel_stop(m_channel);
  }
}

void ZrmDisplayWidget::on_bPause_clicked()
{
  if(m_source->channel_is_paused(m_channel))
      m_source->channel_start(m_channel);
  else
     m_source->channel_pause(m_channel);
}





void ZrmDisplayWidget::on_actMinimize_toggled(bool checked)
{
    QIcon icon(QString(":/zrm/icons/%1").arg(checked ? "maximize.png" : "minimize.png"));
    actMinimize->setIcon(icon);
    tabWidget->setVisible(!checked);

    if(!checked)
     set_message_text(QString(),msg_info);
    else
      update_minimized_info();


    auto vert_policy = checked ? QSizePolicy::Maximum : QSizePolicy::Expanding;
    auto horz_policy = checked ? QSizePolicy::Maximum : QSizePolicy::Expanding;
    gb_display->setSizePolicy(horz_policy, vert_policy);
}

QTextEdit* ZrmDisplayWidget::get_result_text_edit()
{
   if(!m_result_text)
   {
     m_result_text = new  QTextEdit(tabResults);
     m_result_text->setVisible(true);
     m_result_text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
     tabResults->layout()->addWidget(m_result_text);
   }
 return m_result_text;
}


void ZrmDisplayWidget::gen_result_report()
{
    auto method_result = m_source->results_get(m_channel);

    if(method_result.size())
    {
     QString rep_text = make_report(ed_report_maker->text(),ed_akb_type->text(), ed_akb_number->text(),cb_report_details->isChecked(),method_result);
     auto result_text = get_result_text_edit();
     result_text->setText(rep_text);
     result_text->moveCursor(QTextCursor::MoveOperation::Start);
     tabWidget->setCurrentIndex( tabWidget->indexOf(tabResults));
    }
    else
    {
     if(m_result_text)  m_result_text->clear();
    }
}


void ZrmDisplayWidget::on_tbSave_clicked()
{
    QString doc_dir  ;
    QString file_name;
    doc_dir   = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    file_name = QFileDialog::getSaveFileName(this, tr("Сохранение результатов"),doc_dir,tr("HTML (*.html *.htm)"));
    if(!file_name.isEmpty())
    {
      QFile file(file_name);
      if(m_result_text && file.open(QFile::WriteOnly|QFile::Truncate))
      {
         file.write(m_result_text->toHtml().toUtf8());
      }
    }
}

void ZrmDisplayWidget::on_tabWidget_currentChanged(int index)
{
   if(tabWidget->indexOf( tabMon ) == index)
   {
    monitor->repaint();
    monitor->set_update_time(200);
   }
   else
    monitor->set_update_time(20000);
}



void ZrmDisplayWidget::on_bMethod_clicked()
{
  ZrmMethodChoose choose_dlg(this);
#ifdef Q_OS_ANDROID
   choose_dlg.setGeometry(geometry());
#endif
  choose_dlg.set_as_charger(m_source->channel_work_mode(m_channel));
  zrm::zrm_method_t method;
  if(choose_dlg.exec() == QDialog::Accepted
    &&
    choose_dlg.get_method(method, codec() )
    )
  {
    m_source->channel_set_method(m_channel, method);
  }
}



