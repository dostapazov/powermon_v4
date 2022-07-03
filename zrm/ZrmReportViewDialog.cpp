#include "ZrmReportViewDialog.h"

#include "zrmreportdatabase.h"

#include <QSqlQuery>
#include <QDateTime>
#include <QFileDialog>
#include <QStandardPaths>
#include <QProcess>
#include <QPrinter>

#include <QLineSeries>
#include <QValueAxis>
#include <QDateTimeAxis>

#ifdef Q_OS_ANDROID
	#include <QDesktopServices>
#endif

ZrmReportViewDialog::ZrmReportViewDialog(QWidget* parent) :
	QDialog(parent)
{
	setupUi(this);

	init_chart();

	//setWindowFlags(Qt::Window);
	resize(800, 600);
	showMaximized();

	//cb_report_details->setChecked(true);

	connect(tbSaveHtml, &QAbstractButton::clicked, this, &ZrmReportViewDialog::save_report);
	connect(tbSavePdf, &QAbstractButton::clicked, this, &ZrmReportViewDialog::save_report);
	connect(cb_report_details, &QAbstractButton::clicked, this, &ZrmReportViewDialog::openReportFromBase);
}

void ZrmReportViewDialog::setReportId(qlonglong id)
{
	idReport = id;
	cb_report_details->setEnabled(true);
	cb_report_details->setVisible(true);
	openReportFromBase();
}

void ZrmReportViewDialog::setResultText(QString result)
{
	cb_report_details->setEnabled(false);
	cb_report_details->setVisible(false);
	result_text->setText(result);
	result_text->moveCursor(QTextCursor::MoveOperation::Start);
}

void ZrmReportViewDialog::save_report_html(const QString& file_name)
{
	QFile file(file_name);
	if (result_text && file.open(QFile::WriteOnly | QFile::Truncate))
		file.write(result_text->toHtml().toUtf8());
}

void ZrmReportViewDialog::save_report_pdf (const QString& file_name)
{
	QPrinter printer(QPrinter::ScreenResolution);
	printer.setOutputFormat(QPrinter::PdfFormat);
#if QT_VERSION_CHECK(5,15,0) > QT_VERSION
	printer.setOrientation(QPrinter::Orientation::Landscape);
#else
	printer.setPageOrientation(QPageLayout::Orientation::Landscape);
#endif
	printer.setOutputFileName(file_name);
	//result_text->print(&printer);

	QTextEdit te(result_text->toHtml());
	QTextDocument* doc = te.document();
	int mw = chartView->maximumWidth();
	chartView->setMaximumWidth(printer.width() - 50);
	QPixmap pix = chartView->grab();
	chartView->setMaximumWidth(mw);
	QImage image(pix.toImage());
	doc->addResource(QTextDocument::ImageResource, QUrl("image"), image);
	QTextCursor cursor = te.textCursor();
	cursor.movePosition(QTextCursor::End);
	cursor.insertImage("image");
	te.print(&printer);
}

void ZrmReportViewDialog::save_report()
{
#ifdef Q_OS_ANDROID
	QString doc_dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#else
	QString doc_dir = qApp->applicationDirPath();
#endif
	bool fmt_html = sender() == tbSaveHtml;
	QString filter = fmt_html ? tr("HTML (*.html *.htm)") : tr("PDF (*.pdf)");

	QString file_name = QFileDialog::getSaveFileName(this, tr("Сохранение результатов"), doc_dir, filter);
	if (!file_name.isEmpty())
	{
		if (fmt_html)
			save_report_html(file_name);
		else
			save_report_pdf(file_name);
#ifdef Q_OS_ANDROID
		QUrl url = QUrl::fromLocalFile(file_name);
		QDesktopServices::openUrl(url);
#else
		QProcess* console = new QProcess(this);
		console->start("cmd.exe", QStringList() << "/C" << file_name);
#endif
	}
}

void ZrmReportViewDialog::openReportFromBase()
{
	if (idReport <= 0)
		return;

	chartView->setVisible(cb_report_details->isChecked());

	ZrmReportDatabase rep_database;
	QString qtext =
		"SELECT r.id, bt.id AS id_akb_type, bt.name AS akb_type, bl.id AS id_akb_number, bl.serial_number AS akb_number, r.id_user, u.short_fio, "
		"CAST(r.dtm AS text) dtm, r.total_duration, r.total_energy, r.total_capacity "
		"FROM treport r "
		"LEFT JOIN tusers u ON u.id = r.id_user "
		"LEFT JOIN tbattery_list bl ON bl.id = r.id_battery "
		"LEFT JOIN tbattery_types bt ON bt.id = bl.id_type "
		"WHERE r.id = :id ; ";
	QSqlQuery query(*rep_database.database());
	if (!query.prepare(qtext))
		return;
	query.bindValue(":id", idReport);
	query.exec();
	query.next();
	QSqlRecord rec = query.record();
	if (rec.isEmpty())
		return;

	QString result;
	QStringList main_text ;
	QString doc_title  = tr("Отчет об обслуживании АКБ от %1").arg(rec.value("dtm").toDateTime().toString("dd-MM-yyyy"));

	QStringList  details_table;
	if (cb_report_details->isChecked())
	{
		QString qtextdetail =
			"SELECT stage_number, stage_duration, u_beg, i_beg, u_end, i_end, capacity "
			"FROM treport_details "
			"WHERE id_report = :id "
			"ORDER BY stage_number ";
		QSqlQuery querydetail(*rep_database.database());
		if (!querydetail.prepare(qtextdetail))
			return;
		querydetail.bindValue(":id", idReport);
		querydetail.exec();

		details_table += QString("<table width=600 border=1><caption><h3>%1</h3></caption> ").arg(tr("Результаты этапов"));
		details_table +=
			QString("<tr align=center><td>%1</td> <td>%2</td> <td>%3</td> <td>%4</td> <td>%5</td> <td>%6</td> <td>%7</td></tr> ")
			.arg(tr("Этап" ), tr("I нач"), tr("I кон"), tr("U нач"), tr("U кон"), tr("Ёмкость"), tr("Время"));

		QString detail_row = tr("<tr align=center><td>%1</td> "
								"<td style=\"text-align: right;\">%2 A</td> "
								"<td style=\"text-align: right;\">%3 А</td> "
								"<td style=\"text-align: right;\">%4 В</td> "
								"<td style=\"text-align: right;\">%5 В</td> "
								"<td style=\"text-align: right;\">%6 А*Ч</td>"
								"<td>%7</td></tr>");
		while (querydetail.next())
		{
			QSqlRecord recdetail = querydetail.record();
			double CAP  = recdetail.value("capacity").toDouble();
			double Ibeg = recdetail.value("i_beg").toDouble();
			double Iend = recdetail.value("i_end").toDouble();
			double Ubeg = recdetail.value("u_beg").toDouble();
			double Uend = recdetail.value("u_end").toDouble();
			int duration = recdetail.value("stage_duration").toInt();

			details_table += detail_row.arg(recdetail.value("stage_number").toInt())
							 .arg(Ibeg, 0, 'f', 2)
							 .arg(Iend, 0, 'f', 2)
							 .arg(Ubeg, 0, 'f', 2)
							 .arg(Uend, 0, 'f', 2)
							 .arg(CAP, 0, 'f', 2)
							 .arg(tr("%1:%2:%3").arg(duration / 3600, 2, 10, QLatin1Char('0'))
								  .arg((duration - duration / 3600) / 60, 2, 10, QLatin1Char('0'))
								  .arg(duration % 60, 2, 10, QLatin1Char('0'))
								 );
		}

		details_table += tr("</table>\n");

		// sensors
		zrm::method_exec_results_sensors_t results_sensor;
		int nStage = 0;
		zrm::stage_exec_result_sensors_t* stageSensors = nullptr;
		QString strSensors =
			"SELECT stage_number, sensor_number, t, u "
			"FROM treport_details_sensors "
			"WHERE id_report = :id "
			"ORDER BY stage_number, sensor_number;";
		QSqlQuery querySensors(*rep_database.database());
		if (!querySensors.prepare(strSensors))
			return;
		querySensors.bindValue(":id", idReport);
		querySensors.exec();
		while (querySensors.next())
		{
			QSqlRecord recSensor = querySensors.record();
			if (recSensor.value("stage_number").toInt() > nStage)
			{
				zrm::stage_exec_result_sensors_t ss;
				nStage = recSensor.value("stage_number").toInt();
				ss.stage = nStage;
				results_sensor.push_back(ss);
				stageSensors = &results_sensor[results_sensor.size() - 1];
			}
			zrm::stage_exec_result_sensor_t sensor;
			sensor.temp = recSensor.value("t").toDouble() * 1000;
			sensor.volt = recSensor.value("u").toDouble() * 1000;
			stageSensors->sensors.push_back(sensor);
		}

		if (results_sensor.size() > 0)
		{
			zrm::stage_exec_result_sensors_t headerSensor = results_sensor[0];
			int countSensors = static_cast<int>(headerSensor.sensors.size());

			auto table10t = [&details_table, results_sensor](int t)
			{
				// таблица из 10 датчиков


				int countSensors = static_cast<int>(results_sensor[0].sensors.size());
				int countColumn = 10;
				if (countSensors / 10 < t)
					return;
				if (countSensors / 10 == t)
					countColumn = countSensors % 10;
				if (0 == countColumn)
					return;
				details_table += QString("<table width=600 border=1><caption><h3>%1</h3></caption> ").arg(tr("Показания датчиков температуры %1 - %2").arg(t * 10 + 1).arg(t * 10 + countColumn));
				details_table += QString("<tr align=center><td>%1</td> ").arg(tr("Этап"));
				for (int i = 0; i < countColumn; i++)
					details_table += QString("<td>T%1 ℃</td>").arg(t * 10 + i + 1);
				details_table += "</tr> ";

				for (auto&& res : results_sensor)
				{
					QString detail_row = QString("<tr align=center><td>%1</td> ").arg(res.stage);
					for (int i = 0; i < countColumn; i++)
						detail_row += QString("<td style=\"text-align: right;\">%1</td> ").arg(res.sensors[t * 10 + i].temp / 1000.);
					detail_row += QString("</tr>");

					details_table += detail_row;
				}

				details_table += tr("</table>\n");
			};

			for (int t = 0; t <= (countSensors + 5) / 10; t++)
			{
				table10t(t);
			}

			auto table10v = [&details_table, results_sensor](int t)
			{
				// таблица из 10 датчиков
				int countSensors = static_cast<int>(results_sensor[0].sensors.size());
				int countColumn = 10;
				if (countSensors / 10 < t)
					return;
				if (countSensors / 10 == t)
					countColumn = countSensors % 10;
				if (0 == countColumn)
					return;
				details_table += QString("<table width=600 border=1><caption><h3>%1</h3></caption> ").arg(tr("Показания датчиков напряжения %1 - %2").arg(t * 10 + 1).arg(t * 10 + countColumn));
				details_table += QString("<tr align=center><td>%1</td> ").arg(tr("Этап"));
				for (int i = 0; i < countColumn; i++)
					details_table += QString("<td>U%2 В</td> ").arg(t * 10 + i + 1);
				details_table += "</tr> ";

				for (auto&& res : results_sensor)
				{
					QString detail_row = QString("<tr align=center><td>%1</td> ").arg(res.stage);
					for (int i = 0; i < countColumn; i++)
						detail_row += QString("<td style=\"text-align: right;\">%1</td> ").arg(res.sensors[t * 10 + i].volt / 1000.);
					detail_row += QString("</tr>");

					details_table += detail_row;
				}

				details_table += tr("</table>\n");
			};

			for (int t = 0; t <= (countSensors + 5) / 10; t++)
			{
				table10v(t);
			}
		}
		make_chart();
	}

	main_text += tr("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"\" http://www.w3.org\">");
	main_text += tr("<html><head>");
	main_text += tr("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
	main_text += tr("<title>%1</title>").arg(doc_title);
	//main_text += tr("<style type=\"text/css\"> td { white-space: nowrap; text-align: right; } </style>");
	main_text += tr("<style type=\"text/css\"> td { white-space: nowrap; } </style>");
	main_text += tr("</head><body><header><h2>%1</h2></header>").arg(doc_title);
	main_text += tr("<table cellpadding=\"8\">");
	main_text += tr("<tr><td>Тип АКБ</td> <td>%1 № %2</td></tr>").arg(rec.value("akb_type").toString(), rec.value("akb_number").toString());
	main_text += tr("<tr><td>Ответственный</td> <td>%1</td></tr>").arg(rec.value("short_fio").toString());

	double total_capacity = rec.value("total_capacity").toDouble();
	if (!qFuzzyIsNull(total_capacity))
		main_text += tr("<tr><td>Ёмкость АКБ</td> <td>%1 А*Ч</td></tr>").arg(total_capacity, 0, 'f', 2);

	double total_energy = rec.value("total_energy").toDouble();
	main_text += tr("<tr><td>%1</td> <td>%2 А*Ч</td></tr>").arg(total_energy < 0 ? tr("Из АКБ потреблено") : tr("в АКБ передано"))
				 .arg(fabs(total_energy), 0, 'f', 2);

	auto hms = pwm_utils::secunds2hms(uint32_t(rec.value("total_duration").toInt()));
	main_text += tr("<tr><td>Время выполнения</td> <td>%1:%2:%3</td></tr>")
				 .arg(std::get<0>(hms), 2, 10, QChar('0'))
				 .arg(std::get<1>(hms), 2, 10, QChar('0'))
				 .arg(std::get<2>(hms), 2, 10, QChar('0'));

	main_text += tr("</table>");

	if (cb_report_details->isChecked())
		main_text.append(details_table);
	main_text += tr("</body></html>");
	result = main_text.join(QChar('\n'));

	result_text->setText(result);
	result_text->moveCursor(QTextCursor::MoveOperation::Start);
}

// chart

void ZrmReportViewDialog::init_chart()
{
	chart = new QtCharts::QChart();
	chart->setFont(font());
	chartView->setChart(chart);

	chartView->setRenderHint(QPainter::Antialiasing);

	axisTime = new QtCharts::QDateTimeAxis;
	axisTime->setTitleText("Time");
	axisTime->setFormat("hh:mm:ss");
	chart->addAxis(axisTime, Qt::AlignBottom);

	auto addSeries = [this](QString name, QColor color)
	{
		QtCharts::QLineSeries* series  = new QtCharts::QLineSeries(chart);
		series->setName(name);
		series->setColor(color);
		chart->addSeries(series);
		map_series[name] = series;

		QtCharts::QValueAxis* axis = new QtCharts::QValueAxis;
		axis->setTitleText(name);
		chart->addAxis(axis, Qt::AlignLeft);
		series->attachAxis(axis);
		series->attachAxis(axisTime);
	};

	addSeries("I", Qt::red);
	addSeries("U", Qt::green);
	addSeries("C", Qt::blue);
}

void ZrmReportViewDialog::clear_chart()
{
	for (QtCharts::QLineSeries* series : qAsConst( map_series))
		series->clear();
}

void ZrmReportViewDialog::make_chart()
{
	clear_chart();

	QtCharts::QLineSeries* seriesI = map_series["I"];
	QtCharts::QLineSeries* seriesU = map_series["U"];
	QtCharts::QLineSeries* seriesC = map_series["C"];
	QDateTime zeroTime;
	zeroTime.setDate(QDate(2000, 1, 1));
	zeroTime.setTime(QTime(0, 0, 0, 0));
	uint64_t timeTotal = zeroTime.toMSecsSinceEpoch();

	ZrmReportDatabase rep_database;
	QString qtextdetail =
		"SELECT stage_number, stage_duration, u_beg, i_beg, u_end, i_end, capacity "
		"FROM treport_details "
		"WHERE id_report = :id "
		"ORDER BY stage_number ";
	QSqlQuery querydetail(*rep_database.database());
	if (!querydetail.prepare(qtextdetail))
		return;
	querydetail.bindValue(":id", idReport);
	querydetail.exec();

	if (!querydetail.next())
		return;
	QSqlRecord recdetail = querydetail.record();
	double CAP  = recdetail.value("capacity").toDouble();
	double Ibeg = recdetail.value("i_beg").toDouble();
	double Ubeg = recdetail.value("u_beg").toDouble();
	seriesI->append(timeTotal, Ibeg);
	seriesU->append(timeTotal, Ubeg);
	seriesC->append(timeTotal, CAP);
	double Iend = recdetail.value("i_end").toDouble();
	double Uend = recdetail.value("u_end").toDouble();
	int time = recdetail.value("stage_duration").toInt();
	timeTotal += time * 1000;
	seriesI->append(timeTotal, Iend);
	seriesU->append(timeTotal, Uend);
	seriesC->append(timeTotal, CAP);

	while (querydetail.next())
	{
		QSqlRecord recdetail = querydetail.record();
		double CAP  = recdetail.value("capacity").toDouble();
		double Iend = recdetail.value("i_end").toDouble();
		double Uend = recdetail.value("u_end").toDouble();
		int time = recdetail.value("stage_duration").toInt();
		timeTotal += time * 1000;

		seriesI->append(timeTotal, Iend);
		seriesU->append(timeTotal, Uend);
		seriesC->append(timeTotal, CAP);
	}

	for (QtCharts::QLineSeries* series : qAsConst(map_series))
	{
		qreal min = 2147483647.;
		qreal max = -2147483648.;
		for (QPointF& p : series->points())
		{
			qreal value = p.y();
			if (value < min)
				min = value;
			if (value > max)
				max = value;
		}
		if (max == min)
			max++;
		chart->axes(Qt::Vertical, series)[0]->setRange(min, max);
	};

	if (!seriesI->points().isEmpty())
	{
		axisTime->setMin(QDateTime::fromMSecsSinceEpoch(seriesI->points().first().x()));
		axisTime->setMax(QDateTime::fromMSecsSinceEpoch(seriesI->points().last().x()));
	}

	chart->update();
}
