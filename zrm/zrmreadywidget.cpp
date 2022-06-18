/*
 *  The short representation of the all  channels
 *
*/

#include "zrmreadywidget.h"
#include <zrmchannelmimimal.h>

ZrmReadyWidget::ZrmReadyWidget(QWidget* parent) :
    ZrmBaseWidget (parent)
{
    setupUi(this);
    m_ready_layout = new ZrmReadyLayout(ready_widget);
    m_ready_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_ready_layout->set_scroll_area(ready_area);
    ready_widget->setLayout(m_ready_layout);

}


ZrmReadyWidget::~ZrmReadyWidget()
{
    set_layout_count(0);
    if (m_ready_accum)
    {
        delete m_ready_accum;
        m_ready_accum = Q_NULLPTR;
    }
}


ZrmReadyAccum*      ZrmReadyWidget::ready_accum()
{
    if (!m_ready_accum)
    {
        m_ready_accum = new ZrmReadyAccum;
        m_ready_accum->update_connectivities();
    }
    return  m_ready_accum;
}

ZrmChannelMimimal* ZrmReadyWidget::create_channel_widget()
{
    auto w = new ZrmChannelMimimal(ready_widget);
    w->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(w, &ZrmChannelMimimal::clicked, this, &ZrmReadyWidget::zrm_clicked);
    return w;
}

void  ZrmReadyWidget::set_layout_count(int count)
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
            auto litem = m_ready_layout->takeAt(ready_count);
            auto w = litem->widget();
            if (m_current == w)
                m_current = Q_NULLPTR;
            delete w;
            delete litem;
        }
    }
}

void ZrmReadyWidget::update_ready()
{
    int ch_count = zrm::ZrmConnectivity::channels_total();
    set_layout_count(ch_count);
    int idx = 0;
    for (auto&& conn : zrm::ZrmConnectivity::connectivities())
    {
        for (auto&& chan : conn->channels())
        {
            auto litem = m_ready_layout->itemAt(idx++);
            ZrmChannelMimimal* w = dynamic_cast<ZrmChannelMimimal*>(litem->widget());
            if (w)
            {
                w->bind(conn, chan);
            }
        }
    }

    m_current = !m_current && m_ready_layout->count() ? dynamic_cast<ZrmChannelMimimal*>(m_ready_layout->itemAt(0)->widget()) : m_current;
    zrm_chanhel_activate(m_current, false);
    m_ready_layout->set_size(ready_area->size());
    ready_widget->adjustSize();
    ready_accum()->update_connectivities();
}


void ZrmReadyWidget::zrm_chanhel_activate (ZrmChannelMimimal* w, bool bSelect)
{
    if (m_current != w )
    {
        if (m_current)
            m_current->set_active(false);
        m_current = w;
    }
    if (m_current)
        m_current->set_active(true);
    emit channel_activated(m_current, bSelect);
}

void ZrmReadyWidget::zrm_clicked()
{
    auto w = dynamic_cast<ZrmChannelMimimal*>(sender());
    zrm_chanhel_activate(w, true);
}

QSize ZrmReadyWidget::sizeHint() const
{
    return m_ready_layout->sizeHint();
}

void  ZrmReadyWidget::resizeEvent(QResizeEvent* event)
{
    ZrmBaseWidget::resizeEvent(event);
    ready_widget->adjustSize();
}

int ZrmReadyWidget::ready_count()
{
    return m_ready_layout->count();
}

ZrmChannelMimimal* ZrmReadyWidget::ready_at(int idx)
{
    return idx < m_ready_layout->count() ? dynamic_cast<ZrmChannelMimimal*>(m_ready_layout->itemAt(idx)->widget()) : Q_NULLPTR;
}

void ZrmReadyWidget::next_channel    ()
{
    if (m_ready_layout)
    {
        int idx = m_current ? m_ready_layout->indexOf(m_current) : 0;
        if (++idx >= m_ready_layout->count()   )
            idx = 0 ;
        zrm_chanhel_activate(dynamic_cast<ZrmChannelMimimal*>(m_ready_layout->itemAt(idx)->widget()), false);
    }
}

void ZrmReadyWidget::prev_channel    ()
{
    if (m_ready_layout)
    {
        int idx = m_current ? m_ready_layout->indexOf(m_current) : 0;
        if ( --idx < 0   )
            idx = m_ready_layout->count() - 1;
        zrm_chanhel_activate(dynamic_cast<ZrmChannelMimimal*>(m_ready_layout->itemAt(idx)->widget()), false);
    }
}

void ZrmReadyWidget::selectChannel(zrm::ZrmConnectivity* conn, unsigned channel)
{
    // ищем виджет с текущим каналом и активируем его
    if (m_ready_layout)
    {
        for (int i = 0; i < m_ready_layout->count(); i++)
        {
            ZrmChannelMimimal* w = dynamic_cast<ZrmChannelMimimal*>(m_ready_layout->itemAt(i)->widget());
            if (w && w->connectivity() == conn && w->channel() == channel)
            {
                zrm_chanhel_activate(w, false);
                break;
            }
        }
    }
}
