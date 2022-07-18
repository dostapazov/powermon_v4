#ifndef ZRMLOGERCHART_H
#define ZRMLOGERCHART_H

#include <zrmbasewidget.h>
#include "ui_ZrmLogerChart.h"

#include "zrmproto.hpp"
#include <QtCharts>

namespace QtCharts {
class QLineSeries;
class QDateTimeAxis;
class QValueAxis;
}

class ZrmLogerChart : public ZrmChannelWidget, private Ui::ZrmLogerChart
{
	Q_OBJECT

public:
	explicit ZrmLogerChart(QWidget* parent = nullptr);

protected:
	virtual void on_connected(bool con_state) override;
	virtual void update_controls() override;
	virtual void clear_controls() override;
	virtual void channel_recv_packet(unsigned channel, const zrm::recv_header_t* recv_data) override;
	virtual void channel_param_changed(unsigned channel, const zrm::params_list_t& params_list) override;
	virtual void channel_session(unsigned channel) override;

	void init_chart();
	void update_chart_legend_position();

private slots:
	void getLog();
	void getPackPoints();
	void load();
	void stop();
	void setLogCount(const zrm::param_variant& pv);
	void setLogID(const zrm::param_variant& pv);
	void addPoint(const zrm::param_variant& pv);
	void addPoint(const uint8_t* data, uint16_t size);

private:
	QtCharts::QChart* m_chart = nullptr;
	QMap<uint16_t, QtCharts::QLineSeries*> map_series;
	QMap<uint16_t, int32_t> map_min;
	QMap<uint16_t, int32_t> map_max;
	QtCharts::QDateTimeAxis* axisTime;
	//QtCharts::QValueAxis *axisTime;
	//uint32_t timeResolutions = 0;
	//uint16_t pointsNumber = 0;  // Количество точек в пакете
	uint32_t packNumber = 0;
	QTimer timerPack;
};

#endif // ZRMLOGERCHART_H
