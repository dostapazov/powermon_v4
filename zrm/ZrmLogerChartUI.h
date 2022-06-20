#ifndef ZRMLOGERCHARTUI_H
#define ZRMLOGERCHARTUI_H

#include <zrmbasewidget.h>
#include "ui_ZrmLogerChartUI.h"
#include "zrmproto.hpp"

namespace QtCharts {
class QLineSeries;
}

class ZrmLogerChartUI : public ZrmChannelWidget, private Ui::ZrmLogerChartUI
{
	Q_OBJECT

public:
	explicit ZrmLogerChartUI(QWidget* parent = nullptr);

protected:
	virtual void update_controls() override;
	virtual void clear_controls() override;
	virtual void channel_param_changed(unsigned channel, const zrm::params_list_t& params_list) override;

	void init_chart();

private slots:
	void updateChart();

private:
	QtCharts::QChart* m_chart = nullptr;
	QtCharts::QLineSeries* u_series = nullptr;
	QtCharts::QLineSeries* i_series = nullptr;
	QTimer timerChart;
};

#endif // ZRMLOGERCHARTUI_H
