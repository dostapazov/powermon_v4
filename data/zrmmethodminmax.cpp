#include "zrmmethodminmax.h"
#include <vector>
#include <limits>
#include <tuple>

ZrmMethodMinmax::ZrmMethodMinmax(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
}

void     ZrmMethodMinmax::clear_controls()
{
    for(auto sb : findChildren<QSpinBox*>())
    {
        sb->setValue(0);
        sb->setSpecialValueText("--");
    }

    for(auto dsb : findChildren<QDoubleSpinBox*>())
     {
        dsb->setValue(.0);
        dsb->setSpecialValueText("--");
     }
}


ZrmMethodMinmax::mmidx ZrmMethodMinmax::get_minmax_idx(const qvarvect &vec)
{
   int      min_idx  = -1;
   QVariant minValue ;
   int      max_idx  = -1;
   QVariant maxValue ;
   int      number = 0;
   for(auto val : vec)
   {

     if(val.isValid() && !val.isNull())
     {
         if(min_idx<0)  { min_idx = number; minValue = val; }
         else
         {
           if(minValue.toDouble() > val.toDouble()){ minValue = val; min_idx = number; }
         }
         if(max_idx<0)  { max_idx = number; maxValue = val; }
         else
         {
             if(maxValue.toDouble() < val.toDouble()){ maxValue = val; max_idx = number; }
         }
     }
    ++number;
   }
   return std::make_tuple(min_idx, max_idx);
}

void ZrmMethodMinmax::set_minmax_values(const zrm::zrm_method_t & met,const qvarvect & vecU, const qvarvect & vecI )
{
    auto minmaxU =  get_minmax_idx(vecU);
    auto minmaxI =  get_minmax_idx(vecI);
    int idx,stage,stages_count = int(met.stages_count());

    idx   = std::get<0>(minmaxI);
    stage = idx < stages_count ? idx : idx-stages_count;
    if(idx>=0)
    {
      //Минимальное значение тока
        Imin  ->setValue (vecI[qvarvect::size_type(idx)].toDouble());
        IminU ->setValue (vecU[qvarvect::size_type(idx)].toDouble());
        stImin->setValue(stage+1);
    }

    idx   = std::get<1>(minmaxI);
    stage = idx < stages_count ? idx : idx-stages_count;
    if(idx>=0)
    {
      //Максимальное значение тока
        Imax  ->setValue (vecI[qvarvect::size_type(idx)].toDouble());
        ImaxU ->setValue (vecU[qvarvect::size_type(idx)].toDouble());
        stImax->setValue(stage+1);
    }

    idx   = std::get<0>(minmaxU);
    stage = idx < stages_count ? idx : idx-stages_count;
    if(idx>=0)
    {
      //Минимальное значение Напряжения

        Umin  ->setValue(vecU[qvarvect::size_type(idx)].toDouble());
        UminI ->setValue(vecI[qvarvect::size_type(idx)].toDouble());
        stUmin->setValue(stage+1);
    }

    idx   = std::get<1>(minmaxU);
    stage = idx < stages_count ? idx : idx-stages_count;
    if(idx>=0)
    {
      //Максимальное значение напряжения
        Umax  ->setValue(vecU[qvarvect::size_type(idx)].toDouble());
        UmaxI ->setValue(vecI[qvarvect::size_type(idx)].toDouble());
        stUmax->setValue(stage+1);
    }

    for(auto sb : findChildren<QAbstractSpinBox*>())
        sb->setSpecialValueText(QString());

}


void     ZrmMethodMinmax::method_minmax(const zrm::zrm_method_t & method)
{

 clear_controls();
 qvarvect::size_type  stages_count = method.stages_count();
 if(stages_count)
 {

     qvarvect vecU(stages_count<<1);
     qvarvect vecI(stages_count<<1);
    // double met_volt = method.m_method.voltage();
    // double met_curr = method.m_method.current();
     qvarvect::size_type i = 0;
     for(auto & stage : method.m_stages)
     {
       if(!stage.is_pause())
       {
         if(stage.is_charge())
         {
            vecU[i] = stage.charge_volt(method.m_method);
            vecI[i] = stage.charge_curr(method.m_method);
         }
        if(stage.is_discharge())
        {
            vecU[i+stages_count] = stage.discharge_volt(method.m_method);
            vecI[i+stages_count] = stage.discharge_curr(method.m_method);
        }
       }
      ++i;
     }
   set_minmax_values(method, vecU, vecI);
  }
}
