#include "zrmdisplaywidget.h"
#include <qdatetime.h>
#include <qtextcodec.h>

QString ZrmDisplayWidget::make_report(const QString & a_maker_name, const QString & a_akb_type, const QString & a_akb_number
                                      , bool details, const zrm::method_exec_results_t & results)
{
    QString result;
    int32_t total_duration    = 0;
    qreal   total_capacity    = 0;
    qreal   total_energy      = 0;

    QStringList  main_text ;
    QString      doc_title  = tr("Отчет об обслуживании АКБ от %1").arg(QDateTime::currentDateTime().toString("dd-MM-yyyy"));
    QString      maker_name = a_maker_name.isEmpty() ? tr("__________________________________") : a_maker_name;
    QString      akb_type   = a_akb_type.isEmpty  () ? tr("________________") : a_akb_type;
    QString      akb_number = a_akb_number.isEmpty() ? tr("________________") : a_akb_number;

    QStringList  details_table;
    details_table += QString("<table width=600 border=1><caption><h3>%1</h3></caption> ").arg(tr("Результаты этапов"));
    details_table +=
            QString("<tr align=center><td>%1</td> <td>%2</td> <td>%3</td> <td>%4</td> <td>%5</td> <td>%6</td> <td>%7</td></tr> ")
            .arg(tr("Этап" )).arg(tr("I нач")).arg(tr("I кон")).arg(tr("U нач")).arg(tr("U кон")).arg(tr("Ёмкость")).arg(tr("Время"));


    QString detail_row = tr("<tr align=center><td>%1</td> <td>%2 A</td> <td>%3 А</td>  <td>%4 В</td> <td>%5 В</td> <td>%6 А*Ч</td><td>%7</td></tr>");
    for(auto res : results)
    {

        qreal CAP  = qreal(res.capcacity )/1000.0;
        total_duration += uint32_t(res.duration[0])*3600 + uint32_t(res.duration[1])*60 + uint32_t(res.duration[2]);
        total_energy   += CAP;
        if(res.state & zrm::STFL_CAPACITY_MEASHURE)
           total_capacity = qMax(total_capacity,fabs(CAP));

        if(details)
        {
            qreal Ibeg = qreal(res.curr_begin)/1000.0;
            qreal Iend = qreal(res.curr_end  )/1000.0;
            qreal Ubeg = qreal(res.volt_begin)/1000.0;
            qreal Uend = qreal(res.volt_end  )/1000.0;

            details_table += detail_row.arg(uint32_t(res.stage)).arg(Ibeg,0,'f',2).arg(Iend,0,'f',2)
                                       .arg(Ubeg,0,'f',2).arg(Uend,0,'f',2).arg(CAP,0,'f',2)
                                       .arg(tr("%1:%2:%3").arg(res.duration[0],2,10,QLatin1Char('0'))
                                                          .arg(res.duration[1],2,10,QLatin1Char('0'))
                                                          .arg(res.duration[2],2,10,QLatin1Char('0'))
                                           );
        }
    }

    details_table+= tr("</table>\n");

    main_text += tr("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"\" http://www.w3.org\">");
    main_text += tr("<html><head>");
    main_text += tr("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
    main_text += tr("<title>%1</title></head><body><header><h2>%1</h2></header>").arg(doc_title);
    main_text += tr("<table cellpadding=\"8\">");
    main_text += tr("<tr><td>Тип АКБ</td> <td>%1 № %2</td></tr>").arg(akb_type).arg(akb_number);
    main_text += tr("<tr><td>Ответственный</td> <td>%1</td></tr>").arg(maker_name);

    if(!qFuzzyIsNull(total_capacity))
        main_text += tr("<tr><td>Ёмкость АКБ</td> <td>%1 А*Ч</td></tr>").arg(total_capacity,0,'f',2);

    main_text += tr("<tr><td>%1</td> <td>%2 А*Ч</td></tr>").arg(total_energy<0 ? tr("Из АКБ потреблено") : tr("в АКБ передано"))
                                                           .arg(fabs(total_energy),0, 'f' ,2);

    auto hms = zrm::method_t::secunds2hms(uint32_t(total_duration));
    main_text += tr("<tr><td>Время выполнения</td> <td>%1:%2:%3</td></tr>")
                   .arg(std::get<0>(hms),2,10,QChar('0'))
                   .arg(std::get<1>(hms),2,10,QChar('0'))
                   .arg(std::get<2>(hms),2,10,QChar('0'));

    main_text += tr("</table>");

    if(details)
       main_text.append(details_table);
   main_text += tr("</body></html>");
   result = main_text.join(QChar('\n'));
   return result;
}



void ZrmDisplayWidget::setup_method    ()
{
  auto method = m_source->channel_get_method(m_channel);
  set_number_value(lbStageTotal,int(method.stages_count()), 2, infinity_symbol );
  set_number_value(lbCycleTotal,int(quint32(method.m_method.m_cycles_count)), 2, infinity_symbol );

  stages_clear();
  auto met_volt = method.m_method.voltage();
  auto met_curr = method.m_method.current();
  auto met_cap  = method.m_method.capacity();
  QString time_limit_string = tr("Неограничено");
  if(method.m_method.duration())
  {
     char text[32] ;
     snprintf (text,sizeof(text),"%02u:%02u:%02u",unsigned(method.m_method.m_hours),unsigned(method.m_method.m_minutes),unsigned(method.m_method.m_secs));
     time_limit_string = text;
  }

  this->lbTimeLimit->setText(time_limit_string);

  lb_method_name->setText(to_utf(method.m_method.m_name,sizeof(method.m_method.m_name)));

  for(auto stage : method.m_stages)
  {
    stage_add(stage, met_volt, met_curr, met_cap );
  }
  auto param = m_source->param_get(m_channel, zrm::PARAM_STG_NUM);
  this->set_current_stage(param.toInt());
}

void ZrmDisplayWidget::stages_clear()
{
  while(tw_stages->topLevelItemCount())
        delete tw_stages->takeTopLevelItem(0);
}

void ZrmDisplayWidget::stage_add(const zrm::stage_t & stage, qreal met_volt, qreal met_current, qreal met_cap)
{
  QTreeWidgetItem * item =  new QTreeWidgetItem;
  item->setData(0, Qt::UserRole, uint32_t(stage.m_number));
  item->setText(0, QString::number(uint32_t(stage.m_number)));
  item->setText(1, m_source->get_stage_type_name(m_channel,zrm::stage_type_t(stage.m_type)));
  QList<QTreeWidgetItem*> sub_items;


  if(stage.m_type & zrm::STT_CHARGE)
    {
      QTreeWidgetItem * sitem = new QTreeWidgetItem;
      sitem->setText(1,m_source->get_stage_type_name(m_channel,zrm::STT_CHARGE));
      QString text = tr("U=%1 В, I=%2 А").arg(stage.charge_volt(met_volt),0,'f', 2).arg(stage.charge_curr(met_current), 0,'f', 2);
      sitem->setText(2,text);
      sub_items<<sitem;

      if(stage.m_type == zrm::STT_IMPULSE)
        {
          sitem = new QTreeWidgetItem;
          sitem->setText(1,tr("Время "));
          sitem->setText(2,tr("%1 сек").arg(uint32_t(stage.m_char_time)));
          sub_items<<sitem;
        }

    }

  if(stage.m_type & zrm::STT_DISCHARGE)
    {
      QTreeWidgetItem * sitem = new QTreeWidgetItem;
      sitem->setText(1, m_source->get_stage_type_name(m_channel,zrm::STT_DISCHARGE));
      QString text = tr("U=%1 В, I=%2 А").arg(stage.discharge_volt(met_volt),0,'f', 2).arg(stage.discharge_curr(met_current), 0,'f', 2);
      sitem->setText(2,text);
      sub_items<<sitem;

      if(stage.m_type == zrm::STT_IMPULSE)
        {
          sitem = new QTreeWidgetItem;
          sitem->setText(1,tr("Время "));
          sitem->setText(2,tr("%1 сек").arg(uint32_t(stage.m_dis_time)));
          sub_items<<sitem;
        }

    }

  if(stage.m_end_flag & zrm::stage_end_time)
    {
      QTreeWidgetItem * sitem = new QTreeWidgetItem;
      sitem->setText(1,tr("Завершение по времени"));
      QString text = tr("%1:%2:%3").arg( uint32_t(stage.m_hours  ),2,10,QLatin1Char('0'))
                                   .arg( uint32_t(stage.m_minutes),2,10,QLatin1Char('0'))
                                   .arg( uint32_t(stage.m_secs   ),2,10,QLatin1Char('0'));
      sitem->setText(2,text);
      sub_items<<sitem;
    }

  if(stage.m_end_flag & zrm::stage_end_current)
    {
      QTreeWidgetItem * sitem = new QTreeWidgetItem;
      sitem->setText(1,tr("Завершение по I"));
      sitem->setText(2, tr("%1 А").arg(QString::number(stage.end_curr(met_current),'f',2 )));
      sub_items<<sitem;
    }

  if(stage.m_end_flag & zrm::stage_end_voltage)
    {
      QTreeWidgetItem * sitem = new QTreeWidgetItem;
      sitem->setText(1,tr("Завершение по U"));
      sitem->setText(2, tr("%1 В").arg(QString::number(stage.end_volt(met_volt),'f',2 )));
      sub_items<<sitem;
    }

  if(stage.m_end_flag & zrm::stage_end_delta_voltage)
    {
      QTreeWidgetItem * sitem = new QTreeWidgetItem;
      sitem->setText(1,tr("Завершение по ΔU"));
      sitem->setText(2, tr("%1 В").arg(QString::number(stage.end_delta_volt(met_volt),'f',2 )));
      sub_items<<sitem;
    }

  if(stage.m_end_flag & zrm::stage_end_cell_voltage)
    {
      QTreeWidgetItem * sitem = new QTreeWidgetItem;
      sitem->setText(1,tr("Завершение по U-элемента"));
      sitem->setText(2, tr("%1 В").arg(QString::number(stage.end_cell_volt(),'f',2 )));
      sub_items<<sitem;
    }


  if(stage.m_end_flag & zrm::stage_end_temper)
    {
      QTreeWidgetItem * sitem = new QTreeWidgetItem;
      sitem->setText(1,tr("Завершение T"));
      sitem->setText(2, tr("%1 C").arg(QString::number(stage.end_temp(),'f',2 )));
      sub_items<<sitem;
    }


  if(stage.m_end_flag & zrm::stage_end_capacity)
    {
      QTreeWidgetItem * sitem = new QTreeWidgetItem;
      sitem->setText(1,tr("Завершение по емкости"));
      sitem->setText(2, tr("%1 А*Ч").arg(QString::number(stage.end_capacity(met_cap),'f',2 )));
      sub_items<<sitem;
    }

  if(sub_items.count()) item->addChildren(sub_items);

  tw_stages->addTopLevelItem(item);
  m_item_def_bkgnd =  item->background(0);

}


