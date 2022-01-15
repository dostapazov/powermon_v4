#include "reportcommon.h"
#include <qdebug.h>
#include <qsqlquery.h>
#include <qitemdelegate.h>
#include <zrm_connectivity.hpp>
#include <qpainter.h>
#include <QMessageBox>

#include "ZrmReportViewDialog.h"

ReportDetailsModel::ReportDetailsModel(QObject *parent) : QSqlQueryModel(parent)
{
}

QVariant ReportDetailsModel::data(const QModelIndex &index, int role) const
{
    QVariant value = QSqlQueryModel::data(index, role);
    if(index.column() == 1 && role == Qt::DisplayRole)
        value = QString("%1:%2:%3").arg(value.toInt() / 60 / 60, 2, 10, QLatin1Char('0'))
                .arg(value.toInt() / 60 % 60, 2, 10, QLatin1Char('0'))
                .arg(value.toInt() % 60, 2, 10, QLatin1Char('0'));
    return value;
}

class ReportItemDelegate:public QItemDelegate
{
   public:
    ReportItemDelegate(QObject * parent = Q_NULLPTR):QItemDelegate(parent){}
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    private:
    void custom_paint(QPainter *painter,
                      const QStyleOptionViewItem &option,
                      const QString & str
                      ) const;
};

//ZrmReportDatabase ReportCommon::rep_database;

ReportCommon::ReportCommon(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
    init_actions();
    QDate cdt = QDate::currentDate();
    dtm_beg->setDate(cdt.addMonths(-1));
    dtm_end->setDate(cdt);

    connect(tbUsersPage , &QAbstractButton::toggled, this, &ReportCommon::switch_pages);
    connect(bAllTimes   , &QAbstractButton::clicked, this, &ReportCommon::read_reports);
    connect(bReadReports, &QAbstractButton::clicked, this, &ReportCommon::read_reports);
    connect(bShowReport, &QAbstractButton::clicked, this, &ReportCommon::showReport);
    connect(bDelReport, &QAbstractButton::clicked, this, &ReportCommon::deleteReport);
    ReportTable->setItemDelegate( new ReportItemDelegate(ReportTable));

    open_types();
    open_numbers();
    splitter->setStretchFactor  (0, 1);
    splitter->setStretchFactor  (1, 3);
    splitter_2->setStretchFactor(0, 2);
    splitter_2->setStretchFactor(1, 1);
    switch_pages(true);

    updateReportButtons();
}

ReportCommon::~ReportCommon()
{
    UsersTable->setModel(Q_NULLPTR);
    TypesTable->setModel(Q_NULLPTR);
}

void ReportCommon::init_actions()
{
   bUserNew    ->setDefaultAction(actUserNew);
   bUserApply  ->setDefaultAction(actUserApply);
   bUserRevert ->setDefaultAction(actUserRevert);
   bUserMarkDel->setDefaultAction(actUserMarkDel);
   for(auto tb : fr_users_btn->findChildren<QAbstractButton*>())
           connect(tb, &QAbstractButton::clicked, this, &ReportCommon::users_btn_clicked);


   bTypesNew      ->setDefaultAction(actTypeNew);
   bTypesApply    ->setDefaultAction(actTypeApply);
   bTypesRevert   ->setDefaultAction(actTypeRevert);
   bTypesMarkDel  ->setDefaultAction(actTypeMarkDel);

   for(auto tb : fr_types_btn->findChildren<QAbstractButton*>())
           connect(tb, &QAbstractButton::clicked, this, &ReportCommon::types_btn_clicked);


   bNumbersNew    ->setDefaultAction(actNumberNew);
   bNumbersApply  ->setDefaultAction(actNumberApply);
   bNumbersRevert ->setDefaultAction(actNumberRevert);
   bNumbersMarkDel->setDefaultAction(actNumberMarkDel);

   for(auto tb : fr_numbers_btn->findChildren<QAbstractButton*>())
           connect(tb, &QAbstractButton::clicked, this, &ReportCommon::numbers_btn_clicked);
}


void ReportCommon::switch_pages(bool checked)
{
    QWidget * page = checked ? analize_page : users_page;
    if(page == users_page && !UsersTable->model())
        open_users();
    fr_users_btn->setVisible(!checked);
    stackedWidget->setCurrentWidget(page);
}

void ReportCommon::open_users  ()
{
    if(!UsersTable->model())
    {
#ifdef Q_OS_ANDROID
        UsersTable->setEditTriggers(QAbstractItemView::EditTrigger::AllEditTriggers);
#endif
        rep_database.assign_users_model(UsersTable);
        UsersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        connect(UsersTable->model(), &QSqlTableModel::dataChanged, this, [&]() { UsersTable->model()->submit(); });
    }
}

void ReportCommon::open_types  ()
{
    if(!TypesTable->model())
    {
#ifdef Q_OS_ANDROID
        TypesTable->setEditTriggers(QAbstractItemView::EditTrigger::AllEditTriggers);
#endif
        rep_database.assign_types_model(TypesTable);
        auto hdr = TypesTable->horizontalHeader();
        hdr->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
        hdr->setSectionResizeMode(rep_database.field_index(TypesTable->model(),"name"), QHeaderView::ResizeMode::Stretch);

        connect(TypesTable->model(), &QSqlTableModel::dataChanged, this, [&]() { TypesTable->model()->submit(); });
    }
}

void ReportCommon::user_new()
{
    auto model = UsersTable->model();
    int row_num = model->rowCount();
    model->insertRow(row_num);
}

void ReportCommon::user_revert()
{
    UsersTable->model()->revert();
}

void ReportCommon::user_apply()
{
   UsersTable->model()->submit();
}

void ReportCommon::user_mark_del()
{
    rep_database.mark_del(UsersTable->model(), UsersTable->currentIndex().row());
}


void ReportCommon::users_btn_clicked()
{
   QObject * src = sender();
   auto model  = UsersTable->model();
   int row     = UsersTable->currentIndex().row();
   if(src == bUserNew    ) rep_database.new_record(model);
   if(src == bUserApply  ) rep_database.submit    (model);
   if(src == bUserRevert ) rep_database.revert    (model);
   if(src == bUserMarkDel) rep_database.mark_del  (model, row);

}

void ReportCommon::types_btn_clicked()
{
    QObject * src = sender();
    auto model  = TypesTable->model();
    int row     = TypesTable->currentIndex().row();
    if(src == bTypesNew    ) rep_database.new_record(model);
    if(src == bTypesApply  ) rep_database.submit    (model);
    if(src == bTypesRevert ) rep_database.revert    (model);
    if(src == bTypesMarkDel) rep_database.mark_del  (model, row);
}

void ReportCommon::numbers_btn_clicked()
{
    QObject * src = sender();
    auto model  = NumbersTable->model();
    int row     = NumbersTable->currentIndex().row();
    if(src == bNumbersNew    )
    {
       int row = rep_database.new_record(model);
       if(row>=0)
       {
         QAbstractItemModel * type_model = TypesTable->model();
         QModelIndex   type_index = type_model->index(TypesTable->currentIndex().row(), rep_database.field_id(type_model));
         int type_id = type_index.data().toInt();
         model->setData(model->index(row,rep_database.field_index(model,"id_type")),type_id );
       }
       else
        rep_database.revert(model);

    }

    if(src == bNumbersApply  ) rep_database.submit    (model);
    if(src == bNumbersRevert ) rep_database.revert    (model);
    if(src == bNumbersMarkDel) rep_database.mark_del  (model, row);

}

void ReportCommon::open_numbers()
{
 if(!NumbersTable->model())
 {
  rep_database.assign_numbers_model(NumbersTable);
#ifdef Q_OS_ANDROID
  NumbersTable->setEditTriggers(QAbstractItemView::EditTrigger::AllEditTriggers);
#endif
  NumbersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  QItemSelectionModel * selection_model = TypesTable->selectionModel();
  if(selection_model)
  {
    connect(selection_model, &QItemSelectionModel::currentRowChanged, this, &ReportCommon::type_row_changed);
  }
  selection_model = NumbersTable->selectionModel();
  if(selection_model)
  {
    connect(selection_model, &QItemSelectionModel::currentRowChanged, this, &ReportCommon::number_row_changed);
  }
 }
}

void ReportCommon::type_row_changed(const QModelIndex &current)
{
    auto model = current.model();
    int type_id = model->index(current.row(),rep_database.field_id(model)).data().toInt();
    rep_database.numbers_select(type_id);
    model = rep_database.numbers_model();
    if(model->rowCount())
       NumbersTable->setCurrentIndex(model->index(0,2));
    else
    read_reports();
}

void ReportCommon::number_row_changed(const QModelIndex &current)
{
  Q_UNUSED( current )
  read_reports();
  //Выбран другой аккумулятор
}


void ReportCommon::report_query_bind_values(QSqlQuery & query)
{
    int number_id = rep_database.get_record_id(NumbersTable->model(), NumbersTable->currentIndex().row());
    query.bindValue(":id_battery",number_id);
    if(!bAllTimes->isChecked())
    {
     QDateTime dtm(dtm_beg->date());
     query.bindValue(":dtm_beg",dtm);
     dtm = QDateTime(dtm_end->date()).addDays(1);
     query.bindValue(":dtm_end", dtm);
    }

}

QSqlQuery ReportCommon::report_query_get()
{
    QString qtext =
    "select "
    "r.id, r.id_battery, r.id_user, cast(r.dtm as text) dtm ,u.short_fio, r.total_duration, r.total_energy, r.total_capacity "
    "from treport r "
    "left join tusers u on u.id = r.id_user "
    "where r.id_battery = :id_battery ";
    if(!bAllTimes->isChecked())
    qtext += QString("and (r.dtm >= :dtm_beg and r.dtm <=  :dtm_end )");
    qtext +=  QLatin1String("order by r.dtm ");
            ;
    QSqlQuery query(*rep_database.database());
    if(query.prepare(qtext))
    {
      report_query_bind_values( query);
      query.exec();
    }
    return  query;
}

void ReportCommon::read_reports    ()
{
    if(!m_reports_model)
    {
      m_reports_model = new QSqlQueryModel(this);
      ReportTable->setModel(m_reports_model);
      m_reports_model->setQuery(report_query_get());

      auto hdr = ReportTable->horizontalHeader();
      hdr->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
      hdr->setSectionResizeMode(4, QHeaderView::ResizeMode::Stretch);
      hdr->hideSection(0);
      hdr->hideSection(1);
      hdr->hideSection(2);

      m_reports_model->setHeaderData(3,Qt::Horizontal,QObject::tr("Дата"));
      m_reports_model->setHeaderData(4,Qt::Horizontal,QObject::tr("Пользователь"));
      m_reports_model->setHeaderData(5,Qt::Horizontal,QObject::tr("Длительность"));
      m_reports_model->setHeaderData(6,Qt::Horizontal,QObject::tr("Ёмкость"));
      m_reports_model->setHeaderData(7,Qt::Horizontal,QObject::tr("Ёмкость АКБ"));
      auto selection_model = ReportTable->selectionModel();
      if(selection_model)
      {
        connect(selection_model, &QItemSelectionModel::currentRowChanged, this, &ReportCommon::report_row_changed);
        connect(selection_model, &QItemSelectionModel::currentRowChanged, this, &ReportCommon::updateReportButtons);
      }
      updateReportButtons();

    }
    else
    {
       m_reports_model->setQuery(report_query_get());
    }
   report_row_changed(m_reports_model->index(0,0));

}

void ReportCommon::showReport()
{
    int row = ReportTable->currentIndex().row();
    if (row < 0 || !m_reports_model)
        return;
    qlonglong rep_id =  m_reports_model->index(row, 0).data().toLongLong();
    ZrmReportViewDialog report(this);
    report.setReportId(rep_id);
    report.exec();
}

void ReportCommon::deleteReport()
{

    int row = ReportTable->currentIndex().row();
    if (row < 0 || !m_reports_model)
        return;
    if (QMessageBox::Yes != QMessageBox::question(this, "Удаление", "Удалить выбранный отчет?"))
            return;
    QVariant rep_id =  m_reports_model->index(row, 0).data();
    rep_database.reportDelete(rep_id);
    read_reports();
    updateReportButtons();
}

void ReportCommon::updateReportButtons()
{
    bool bEnable = ReportTable->currentIndex().isValid();
    bShowReport->setEnabled(bEnable);
    bDelReport->setEnabled(bEnable);
}

void ReportItemDelegate::custom_paint(QPainter *painter,const QStyleOptionViewItem &option, const QString & str  ) const
{
    painter->save();
    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setBrush(option.palette.highlightedText());
        painter->setPen(option.palette.highlightedText().color());
    }
    painter->drawText(painter->boundingRect(option.rect,Qt::AlignHCenter|Qt::AlignVCenter,str) ,str);
    painter->restore();

}


void ReportItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
  /*Отрисовка элементов отчета*/
  switch(index.column())
  {

   case 3 :
        {

         QString str = index.data().toDateTime().toString(("dd-MM-yyyy hh:mm:ss"));
         custom_paint(painter, option, str);

        }
      break;
   case 5 :
         {
          auto hms = zrm::method_t::secunds2hms(index.data().toUInt());
          QString str = zrm::ZrmConnectivity::hms2string(hms);
          custom_paint(painter, option, str);
         }
          break;
   default:
   QItemDelegate::paint(painter, option, index);
   break;
  }
}


void ReportCommon::report_row_changed(const QModelIndex &current)
{
  QString id_report = ":id_report";
  QVariant rep_id =  m_reports_model->index(current.row(),0).data();

  if(!m_report_details_model)
  {

      QString query_text  = QString
      (
        " SELECT stage_number,stage_duration, u_beg, i_beg, u_end, i_end, capacity "
        " FROM treport_details  where id_report = %1 "
        " order by stage_number"
      ).arg(id_report);
      QSqlQuery query(*rep_database.database());
      m_report_details_model = new ReportDetailsModel(ReportDetailTable);
      query.prepare(query_text);
      query.bindValue(id_report,rep_id);
      query.exec();
      m_report_details_model->setQuery(query);

      m_report_details_model->setHeaderData(0, Qt::Horizontal,QObject::tr("Этап"));
      m_report_details_model->setHeaderData(1, Qt::Horizontal,QObject::tr("Длительность"));
      m_report_details_model->setHeaderData(2, Qt::Horizontal,QObject::tr("U нач"));
      m_report_details_model->setHeaderData(3, Qt::Horizontal,QObject::tr("I нач"));
      m_report_details_model->setHeaderData(4, Qt::Horizontal,QObject::tr("U кон"));
      m_report_details_model->setHeaderData(5, Qt::Horizontal,QObject::tr("I кон"));
      m_report_details_model->setHeaderData(6, Qt::Horizontal,QObject::tr("Емкость"));
      auto hdr = ReportDetailTable->horizontalHeader();
      hdr->setSectionResizeMode(QHeaderView::ResizeToContents);
      hdr->setStretchLastSection(true);
      ReportDetailTable->setModel(m_report_details_model);

  }
  else
  {
   QSqlQuery query =  m_report_details_model->query();
   query.bindValue(id_report,rep_id);
   query.exec();
   m_report_details_model->setQuery(query);
  }
}
