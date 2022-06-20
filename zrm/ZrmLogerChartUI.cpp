#include "ZrmLogerChartUI.h"

#include <signal_bloker.hpp>

#include <QtCharts>
#include <QtCharts/qlineseries.h>
#include <QtCharts/qvalueaxis.h>

#include <QDateTimeAxis>
#include <QValueAxis>
#include <QDateTime>

constexpr int CHART_UPDATE_PREIOD = 200;

ZrmLogerChartUI::ZrmLogerChartUI(QWidget* parent) :
	ZrmChannelWidget(parent)
{
	setupUi(this);

	timerChart.setInterval(CHART_UPDATE_PREIOD);
	connect(&timerChart, &QTimer::timeout, this, &ZrmLogerChartUI::updateChart);
	init_chart();
}

void ZrmLogerChartUI::update_controls()
{
	clear_controls();

	if (m_source && m_channel )
		channel_param_changed(m_channel, m_source->channel_params(m_channel));

}

void ZrmLogerChartUI::clear_controls()
{
	timerChart.stop();
	if (u_series)
		u_series->clear();
	if (i_series)
		i_series->clear();
}

void  ZrmLogerChartUI::channel_param_changed(unsigned channel, const zrm::params_list_t& params_list  )
{
	SignalBlocker sb(findChildren<QWidget*>());
	if (channel == m_channel && m_source)
	{
		for (auto param : params_list)
		{
			//QVariant value = m_source->param_get(m_channel, param.first);
			switch (param.first)
			{
				case zrm::PARAM_STATE        :
					if (is_stopped())
						timerChart.stop();
					else
					{
						if (!timerChart.isActive())
							timerChart.start();
					}
					break;
				case zrm::PARAM_MCUR :
				case zrm::PARAM_MCURD :
					m_chart->axes(Qt::Vertical, i_series)[0]->setRange(-m_source->param_get(m_channel, zrm::PARAM_MCURD).toDouble(), m_source->param_get(m_channel, zrm::PARAM_MCUR).toDouble());
					break;
				case zrm::PARAM_MVOLT :
					m_chart->axes(Qt::Vertical, u_series)[0]->setRange(0, m_source->param_get(m_channel, zrm::PARAM_MVOLT).toDouble());
					break;
				default:
					break;
			}
		}
	}
	ZrmChannelWidget::channel_param_changed(channel, params_list);
}

void ZrmLogerChartUI::init_chart()
{
	m_chart = new QtCharts::QChart();
	m_chart->setFont(font());
	chart_view->setChart(m_chart);

	chart_view->setRenderHint(QPainter::Antialiasing);

	u_series = new QtCharts::QLineSeries(m_chart);
	u_series->setColor(Qt::green);
	u_series->setName(tr("Напряжение"));
	m_chart->addSeries(u_series);

	i_series = new QtCharts::QLineSeries(m_chart);
	i_series->setColor(Qt::red);
	i_series->setName(tr("Ток"));
	m_chart->addSeries(i_series);

	//add axis to the chart
	QtCharts::QDateTimeAxis* axisX = new QtCharts::QDateTimeAxis;

	axisX->setTickCount(10);
	axisX->setFormat("hh:mm:ss");
	axisX->setTitleText("Time");
	m_chart->addAxis(axisX, Qt::AlignBottom);
	u_series->attachAxis(axisX);

	QtCharts::QValueAxis* axisY = new QtCharts::QValueAxis;
	axisY->setLabelFormat("%.1f");
	axisY->setTitleText("U");
	m_chart->addAxis(axisY, Qt::AlignLeft);
	u_series->attachAxis(axisY);

	i_series->attachAxis(axisX);

	QtCharts::QValueAxis* axisYI = new QtCharts::QValueAxis;
	axisYI->setLabelFormat("%.1f");
	axisYI->setTitleText("I");
	m_chart->addAxis(axisYI, Qt::AlignLeft);
	i_series->attachAxis(axisYI);
}

void ZrmLogerChartUI::updateChart()
{
	this->setUpdatesEnabled(false);
	qint64 t = QDateTime::currentDateTime().toMSecsSinceEpoch();
	i_series->append(t, m_source->param_get(m_channel, zrm::PARAM_CUR).toDouble());
	u_series->append(t, m_source->param_get(m_channel, zrm::PARAM_VOLT).toDouble());


	constexpr qint64 TIME_LENGTH = 1000 * 60;
	constexpr int    MAX_SERIES_COUNT = TIME_LENGTH / CHART_UPDATE_PREIOD;
	QDateTime endRange = QDateTime::currentDateTime();
	QDateTime begRange = endRange.addMSecs(-TIME_LENGTH);
	int count_to_remove = i_series->count() - MAX_SERIES_COUNT;
	if (count_to_remove > 0)
	{
		i_series->removePoints(0, count_to_remove);
		u_series->removePoints(0, count_to_remove);
	}

	QtCharts::QAbstractAxis* axis = m_chart->axes(Qt::Horizontal, u_series).at(0);
	axis->setRange(begRange, endRange);
	this->setUpdatesEnabled(true);
}
