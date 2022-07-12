/*
 *  The short representation of the all  channels
 *
*/

#include "ZrmReadySlaveWidget.h"

#include "ZrmChannelView.h"
#include "zrmreadylayout.h"

#include <QScrollArea>

ZrmReadySlaveWidget::ZrmReadySlaveWidget(QWidget* parent) :
    ZrmBaseWidget(parent)
{
    ready_area = new QScrollArea(this);
    ready_area->setWidgetResizable(true);
    ready_widget = new QWidget(ready_area);
    ready_area->setWidget(ready_widget);
    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(ready_area);
    setLayout(l);

    m_ready_layout = new ZrmReadyLayout(ready_widget);
    m_ready_layout->setOrientation(Qt::Horizontal);
    m_ready_layout->set_scroll_area(ready_area);
    ready_widget->setLayout(m_ready_layout);
}

ZrmReadySlaveWidget::~ZrmReadySlaveWidget()
{
    set_layout_count(0);
}

ZrmChannelView* ZrmReadySlaveWidget::create_channel_widget()
{
    auto w = new ZrmChannelView(ready_widget);
    w->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(w, &ZrmChannelView::clicked, this, &ZrmReadySlaveWidget::zrm_clicked);
    return w;
}

void  ZrmReadySlaveWidget::set_layout_count(int count)
{
    int ready_count = m_ready_layout->count();
    while (count != ready_count)
    {
        if (ready_count < count)
        {
            m_ready_layout->addWidget(create_channel_widget());
            ++ready_count;
        }

        if (ready_count > count)
        {
            --ready_count;
            QLayoutItem* litem = m_ready_layout->takeAt(ready_count);
            QScopedPointer<QWidget> w (litem->widget());
            if (m_current == w.get())
            {
                m_current = nullptr;
            }

            w->disconnect();
            w.reset(nullptr);
            delete litem;
        }
    }
}

void ZrmReadySlaveWidget::bind(zrm::ZrmConnectivity* src, uint16_t chan, bool _connect_signals)
{
    ZrmBaseWidget::bind(src, chan, _connect_signals);

    // ищем виджет с текущим каналом и активируем его
    zrm_chanhel_activate(nullptr);
    if (m_ready_layout)
    {
        for (int i = 0; i < m_ready_layout->count(); i++)
        {
            ZrmChannelView* w = dynamic_cast<ZrmChannelView*>(m_ready_layout->itemAt(i)->widget());
            if (w && w->connectivity() == m_source && w->channel() == m_channel)
            {
                zrm_chanhel_activate(w);
                break;
            }
        }
    }
}

void ZrmReadySlaveWidget::update_ready()
{
    int ch_count = zrm::ZrmConnectivity::channels_total();
    set_layout_count(ch_count);
    int idx = 0;
    for (auto&& conn : zrm::ZrmConnectivity::connectivities())
    {
        for (auto&& chan : conn->channels())
        {
            auto litem = m_ready_layout->itemAt(idx++);
            ZrmChannelView* w = dynamic_cast<ZrmChannelView*>(litem->widget());
            if (w)
                w->bind(conn, chan);
        }
    }

    m_current = !m_current && m_ready_layout->count() ? dynamic_cast<ZrmChannelView*>(m_ready_layout->itemAt(0)->widget()) : m_current;
    zrm_chanhel_activate(m_current);
    m_ready_layout->set_size(ready_area->size());
    ready_widget->adjustSize();
}

void ZrmReadySlaveWidget::zrm_chanhel_activate(ZrmChannelView* w)
{
    if (m_current != w)
    {
        if (m_current)
            m_current->set_active(false);
        m_current = w;
    }
    if (m_current)
        m_current->set_active(true);
}

void ZrmReadySlaveWidget::zrm_clicked()
{
    auto w = dynamic_cast<ZrmChannelView*>(sender());
    if (w)
        emit channel_activated(w->connectivity(), w->channel());
}

QSize ZrmReadySlaveWidget::sizeHint() const
{
    return m_ready_layout->sizeHint();
}
