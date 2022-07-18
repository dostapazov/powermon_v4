#include "ZrmLogerChart.h"

#include <signal_bloker.hpp>

#include <QtCharts/qlineseries.h>
#include <QtCharts/qvalueaxis.h>

#include <QDateTimeAxis>
#include <QValueAxis>
#include <QDateTime>

ZrmLogerChart::ZrmLogerChart(QWidget* parent) :
    ZrmChannelWidget(parent)
{
    setupUi(this);

    connect(buttonLoad, &QAbstractButton::clicked, this, &ZrmLogerChart::getLog);
    connect(buttonStop, &QAbstractButton::clicked, this, &ZrmLogerChart::stop);

    init_chart();

    timerPack.setInterval(50);
}

void ZrmLogerChart::on_connected(bool con_state)
{
    Q_UNUSED(con_state);
    //buttonSetSettings->setEnabled(con_state);
}

void ZrmLogerChart::update_controls()
{
    clear_controls();

    if (m_source && m_channel )
        channel_param_changed(m_channel, m_source->channel_params(m_channel));
}

void ZrmLogerChart::clear_controls()
{
    stop();
    m_chart->removeAllSeries();
    map_series.clear();
    map_min.clear();
    map_max.clear();
}

void ZrmLogerChart::channel_recv_packet(unsigned channel, const zrm::recv_header_t* recv_data)
{
    Q_UNUSED(channel)
    Q_UNUSED(recv_data)
    /*if (m_source && channel == m_channel)
    {
        if (zrm::packet_types_t::PT_DATAREAD == recv_data->proto_hdr.type)
        {
            const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(recv_data->data);
            size_t data_size = recv_data->proto_hdr.data_size;
            if (data_ptr && data_size)
            {
                const uint8_t* data_end = data_ptr + data_size;
                uint8_t state = *data_ptr++;
                (void)(state);

                while (data_ptr < data_end)
                {
                    zrm::zrm_param_t param = zrm::zrm_param_t(*data_ptr++);
                    uint16_t param_size = *data_ptr++;
                    if (!param_size)
                        continue;
                    if (zrm::PARAM_LOG_POINT == param)
                    {
                        //qDebug() << "!!!" << param_size << QByteArray(reinterpret_cast<const char*>(data_ptr), int(param_size)).toHex(' ');
                        addPoint(data_ptr, data_size);
                    }
                    data_ptr += param_size;
                }
            }
        }
    }*/
}

void  ZrmLogerChart::channel_param_changed(unsigned channel, const zrm::params_list_t& params_list  )
{
    SignalBlocker sb(findChildren<QWidget*>());
    if (channel == m_channel && m_source)
    {
        for (auto param : params_list)
        {
            switch (param.first)
            {
                case zrm::PARAM_LOG_COUNT :
                    setLogCount(param.second);
                    break;
                case zrm::PARAM_LOG_ID :
                    setLogID(param.second);
                    break;
                case zrm::PARAM_LOG_POINT :
                    addPoint(param.second);
                    break;
                default:
                    break;
            }
        }
    }
    ZrmChannelWidget::channel_param_changed(channel, params_list);
}

void ZrmLogerChart::channel_session(unsigned channel)
{
    if (m_source && m_channel == channel && m_source->channel_session(m_channel).is_active())
    {
        zrm::params_t params;
        params.push_back(zrm::PARAM_LOG_COUNT);
        m_source->channel_subscribe_params(m_channel, params, true);
    }
}

void ZrmLogerChart::getLog()
{
    // получить время лога
    if (m_source && channel() == m_channel && m_source->channel_session(m_channel).is_active())
    {
        //quint8 wr_value = static_cast<quint8>(spinBoxLogCount->value());
        //m_source->channel_write_param(m_channel, zrm::WM_PROCESS_AND_WRITE, zrm::PARAM_LOG_ID, &wr_value, sizeof(wr_value));
        QByteArray ba(7, 0x0000);
        *ba.data() = static_cast<QByteArray::value_type>(spinBoxLogCount->value());
        m_source->channel_write_param(m_channel, zrm::WM_PROCESS_AND_WRITE, zrm::PARAM_LOG_ID, ba.constData(), ba.size());
    }
}

void ZrmLogerChart::getPackPoints()
{
    qDebug() << "ask number" << packNumber << (m_source && channel() == m_channel && m_source->channel_session(m_channel).is_active());
    // запросить следующую точку лога
    if (m_source && channel() == m_channel && m_source->channel_session(m_channel).is_active())
        m_source->channel_write_param(m_channel, zrm::WM_PROCESS_AND_WRITE, zrm::PARAM_LOG_POINT, &packNumber, sizeof(packNumber));

}

void ZrmLogerChart::load()
{
    getLog();

    connect(&timerPack, SIGNAL(timeout()), this, SLOT(getLog()));
    timerPack.start();
}

void ZrmLogerChart::stop()
{
    timerPack.stop();
    timerPack.disconnect();

    buttonLoad->setEnabled(true);

    // сброс параметров
    /*if (m_source && channel() == m_channel && m_source->channel_session(m_channel).is_active())
    {
        zrm::params_t params;
        params.push_back(zrm::PARAM_LOG_POINT);
        m_source->channel_subscribe_params(m_channel, params, false);
    }*/
    packNumber = 0;
}

void ZrmLogerChart::init_chart()
{
    m_chart = new QtCharts::QChart();
    m_chart->setFont(font());
    chart_view->setChart(m_chart);

    chart_view->setRenderHint(QPainter::Antialiasing);
    //chart->legend()->hide();

    axisTime = new QtCharts::QDateTimeAxis;
    axisTime->setTickCount(10);
    axisTime->setFormat("hh:mm:ss");
    //axisTime = new QtCharts::QValueAxis;
    axisTime->setTitleText("Time");
    m_chart->addAxis(axisTime, Qt::AlignBottom);

    auto legend = m_chart->legend();
    legend->detachFromChart();
    legend->setAlignment(Qt::AlignTop);
    legend->setBackgroundVisible(true);
    legend->setBrush(QBrush(QColor(128, 128, 128, 128)));
    legend->setPen(QPen(QColor(192, 192, 192, 192)));
    QFont font = legend->font();
    font.setPointSizeF(10.0);
    legend->setFont(font);
    legend->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    update_chart_legend_position();
}

void ZrmLogerChart::update_chart_legend_position()
{
    if (m_chart)
    {
        QtCharts::QLegend* legend = m_chart->legend();
        QRectF r = chart_view->geometry();
        QSizeF legend_size = legend->size();
        legend_size = legend_size.expandedTo(QSizeF(300, 50));
        QPointF tl(r.width() - 320, 20);
        QRectF lr(tl, QSizeF(300, 50));
        legend->setGeometry(lr);
        legend->update();
    }
}

void ZrmLogerChart::setLogCount(const zrm::param_variant& pv)
{
    qDebug() << "count!!!!!";

    if (pv.is_valid())
    {
        uint8_t countLog = pv.value<uint8_t>(true);
        qDebug() << "log count" << countLog;
        spinBoxLogCount->setMaximum(countLog);
        spinBoxLogCount->setValue(countLog > 0 ? 1 : 0);
    }
}

void ZrmLogerChart::setLogID(const zrm::param_variant& pv)
{
    qDebug() << "time!!!!!";

    clear_controls();

    /*if (m_source && channel() == m_channel && m_source->channel_session(m_channel).is_active())
    {
        // подписываемся на точки
        zrm::params_t params;
        params.push_back(zrm::PARAM_LOG_POINT);
        m_source->channel_subscribe_params(m_channel, params, true);
    }*/
    //uint8_t idLog;
    if (pv.is_valid())
    {
        //memcpy(&idLog, pv.puchar, 1);
        //memcpy(&timeResolutions, pv.puchar + 1, 4);
        //memcpy(&pointsNumber, pv.puchar + 5, 2);
        //qDebug() << "time resolution" << idLog << timeResolutions << pointsNumber;
        uint8_t idLog = pv.ubyte;
        qDebug() << "id log " << idLog;
        getPackPoints();
    }

    timerPack.stop();
    timerPack.disconnect();
    connect(&timerPack, SIGNAL(timeout()), this, SLOT(getPackPoints()));
    timerPack.start();

    buttonLoad->setEnabled(false);
}

void ZrmLogerChart::addPoint(const zrm::param_variant& pv)
{
    qDebug() << "point!!!!!" << pv.is_valid();
    uint16_t id;
    uint32_t time;
    int32_t value;
    //pointsNumber = 10;
    uint packLength = 104;
    //qDebug() << "length" << packLength;
    //if (pv.is_valid())
    {
        QByteArray ba;
        ba.resize(packLength);
        memcpy(ba.data(), pv.puchar, packLength);
        qDebug() << "hex" << ba.length() << ba.toHex();

        uint32_t pn = pv.udword;
        if (packNumber + 1 != pn)
        {
            qDebug() << "wrong number! " << packNumber << pn;
            return;
        }
        qDebug() << "pack number" << pn;
        packNumber++;
        for (int i = 0; i < 10; i++)
        {
            memcpy(&id, pv.puchar + 10 * i + 4, 2);
            memcpy(&time, pv.puchar + 10 * i + 6, 4);
            memcpy(&value, pv.puchar + 10 * i + 10, 4);
            qDebug() << "point" << id << time << value;

            // признак конца все единицы
            if (0xffff == id && 0xffffffff == time && (int32_t)0xffffffff == value)
            {
                qDebug() << "stop";
                stop();
                for (auto&& u : map_series[6]->points())
                    qDebug() << u;
                m_chart->update();
                return;
            }

            if (!map_series.contains(id))
            {
                if (map_series.isEmpty())
                {
                    //axisTime->setMax(time + 1);
                    //axisTime->setMin(time);
                    axisTime->setMax(QDateTime::fromMSecsSinceEpoch(time + 1));
                    axisTime->setMin(QDateTime::fromMSecsSinceEpoch(time));
                }
                QtCharts::QLineSeries* series  = new QtCharts::QLineSeries(m_chart);
                QVector<Qt::GlobalColor> vec_color;
                vec_color << Qt::darkRed << Qt::darkRed << Qt::darkBlue << Qt::darkCyan << Qt::darkMagenta << Qt::darkYellow << Qt::black << Qt::darkGray;
                //series->setColor(Qt::darkBlue);
                series->setColor(vec_color[id]);
                series->setName(QString::number(id));
                m_chart->addSeries(series);
                map_series[id] = series;
                map_min[id] = 2147483647;
                map_max[id] = -2147483648;

                QtCharts::QValueAxis* axis = new QtCharts::QValueAxis;
                axis->setTitleText(QString::number(id));
                m_chart->addAxis(axis, Qt::AlignLeft);
                series->attachAxis(axis);
                series->attachAxis(axisTime);
            }
            QtCharts::QLineSeries* lseries = map_series[id];

            lseries->append(time, value);
            if (value < map_min[id])
                map_min[id] = value;
            if (value > map_max[id])
                map_max[id] = value;
            /*if (axisTime->min() > time)
                axisTime->setMin(time);
            if (axisTime->max() < time)
                axisTime->setMax(time);*/
            if (axisTime->min().toMSecsSinceEpoch() > time)
                axisTime->setMin(QDateTime::fromMSecsSinceEpoch(time));
            if (axisTime->max().toMSecsSinceEpoch() < time)
                axisTime->setMax(QDateTime::fromMSecsSinceEpoch(time));

            //m_chart->axes(Qt::Horizontal, m_series)[0]->setRange(time-10, time);
            m_chart->axes(Qt::Vertical, lseries)[0]->setRange(map_min[id] - 1, map_max[id] + 1);
            //axisTime->setRange(QDateTime::fromMSecsSinceEpoch(1995), QDateTime::fromMSecsSinceEpoch(3939));
            //axisTime->setRange(1995, 3939);
        }
        qDebug() << map_series[6];
        m_chart->update();
    }
    getPackPoints();

    /*if (m_source && channel() == m_channel)
    {
        if (m_source->channel_session(m_channel).is_active())
        {
            zrm::params_t params;
            //params.push_back(zrm::PARAM_LOG_TIME);
            params.push_back(zrm::PARAM_LOG_POINT);
            m_source->channel_subscribe_params(m_channel, params, true);
        }
    }*/
}

void ZrmLogerChart::addPoint(const uint8_t* data, uint16_t size)
{
    qDebug() << "add point!!!!!";
    uint16_t id;
    uint32_t time;
    int32_t value;

    qDebug() << "!!!" << size << QByteArray(reinterpret_cast<const char*>(data), int(size)).toHex(' ');

    uint32_t pn;
    int count = (size - 4) / 10;
    memcpy(&pn, data, 4);
    if (packNumber + 1 != pn)
    {
        qDebug() << "wrong number! " << packNumber << pn;
        return;
    }
    qDebug() << "pack number" << pn;
    packNumber++;
    for (int i = 0; i < count; i++)
    {
        memcpy(&id, data + 10 * i + 4, 2);
        memcpy(&time, data + 10 * i + 6, 4);
        memcpy(&value, data + 10 * i + 10, 4);
        qDebug() << "point" << id << time << value;

        // признак конца все единицы
        if (0xffff == id && 0xffffffff == time && (int32_t)0xffffffff == value)
        {
            qDebug() << "stop";
            stop();
            /*for (auto u : map_series[6]->points())
                qDebug() << u;*/
            m_chart->update();
            return;
        }

        if (!map_series.contains(id))
        {
            if (map_series.isEmpty())
            {
                //axisTime->setMax(time + 1);
                //axisTime->setMin(time);
                axisTime->setMax(QDateTime::fromMSecsSinceEpoch(time + 1));
                axisTime->setMin(QDateTime::fromMSecsSinceEpoch(time));
            }
            QtCharts::QLineSeries* series  = new QtCharts::QLineSeries(m_chart);
            QVector<Qt::GlobalColor> vec_color;
            vec_color << Qt::darkRed << Qt::darkRed << Qt::darkBlue << Qt::darkCyan << Qt::darkMagenta << Qt::darkYellow << Qt::black << Qt::darkGray;
            //series->setColor(Qt::darkBlue);
            series->setColor(vec_color[id]);
            series->setName(QString::number(id));
            m_chart->addSeries(series);
            map_series[id] = series;
            map_min[id] = 2147483647;
            map_max[id] = -2147483648;

            QtCharts::QValueAxis* axis = new QtCharts::QValueAxis;
            axis->setTitleText(QString::number(id));
            m_chart->addAxis(axis, Qt::AlignLeft);
            series->attachAxis(axis);
            series->attachAxis(axisTime);
        }
        QtCharts::QLineSeries* lseries = map_series[id];

        lseries->append(time, value);
        if (value < map_min[id])
            map_min[id] = value;
        if (value > map_max[id])
            map_max[id] = value;
        /*if (axisTime->min() > time)
            axisTime->setMin(time);
        if (axisTime->max() < time)
            axisTime->setMax(time);*/
        if (axisTime->min().toMSecsSinceEpoch() > time)
            axisTime->setMin(QDateTime::fromMSecsSinceEpoch(time));
        if (axisTime->max().toMSecsSinceEpoch() < time)
            axisTime->setMax(QDateTime::fromMSecsSinceEpoch(time));

        //m_chart->axes(Qt::Horizontal, m_series)[0]->setRange(time-10, time);
        m_chart->axes(Qt::Vertical, lseries)[0]->setRange(map_min[id] - 1, map_max[id] + 1);
        //axisTime->setRange(QDateTime::fromMSecsSinceEpoch(1995), QDateTime::fromMSecsSinceEpoch(3939));
        //axisTime->setRange(1995, 3939);
    }
    //qDebug() << map_series[6];
    m_chart->update();

    getPackPoints();

    /*if (m_source && channel() == m_channel)
    {
        if (m_source->channel_session(m_channel).is_active())
        {
            zrm::params_t params;
            //params.push_back(zrm::PARAM_LOG_TIME);
            params.push_back(zrm::PARAM_LOG_POINT);
            m_source->channel_subscribe_params(m_channel, params, true);
        }
    }*/
}
