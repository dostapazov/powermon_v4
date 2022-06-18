#include "zrmreadylayout.h"

#include "zrmchannelmimimal.h"

#include <QScrollArea>

ZrmReadyLayout::ZrmReadyLayout(QWidget* parent): QLayout (parent)
{
    setSizeConstraint(QLayout::SetMinAndMaxSize);
}

ZrmReadyLayout::~ZrmReadyLayout()
{
    clear();
}

void         ZrmReadyLayout::clear()
{
    while (ZrmReadyLayout::count())
    {
        auto li = ZrmReadyLayout::takeAt(ZrmReadyLayout::count() - 1);
        delete li->widget();
        delete li;
    }

}


void ZrmReadyLayout::addWidget(QWidget* widget)
{
    addItem(new QWidgetItem(widget));
}

void ZrmReadyLayout::addItem  (QLayoutItem* item)
{
    if (!m_items.count(item) && item->widget())
    {
        m_items.append(item);

        item->widget()->setParent(parentWidget());
        item->widget()->setVisible(true);
    }
}


void   ZrmReadyLayout::set_size(const QSize size)
{
    QRect r = geometry();
    r.setSize(size);
    setGeometry(r);
}


bool  ZrmReadyLayout::hasHeightForWidth() const
{
    return m_scroll_area;
}

int   ZrmReadyLayout::heightForWidth(int width) const
{
    Q_UNUSED(width)
    int sp = 3 * spacing();
    QLayout* layout = m_scroll_area->layout();
    if (layout)
        sp = 2 * layout->margin() + 2 * layout->spacing();
    return m_scroll_area ? m_scroll_area->size().height() - sp : minimumSize().height();
}

QSize ZrmReadyLayout::sizeHint() const
{
    QSize sz(m_min_width, m_min_height);
    sz = sz.expandedTo(QSize(m_hint_width, m_hint_height));
    return sz;
}

QSize ZrmReadyLayout::minimumSize() const
{
    QSize sz(100, 50);
    for (auto i : m_items)
        sz = sz.expandedTo(i->widget()->minimumSize());
    m_min_width  = sz.width();
    m_min_height = sz.height();
    return sz;
}

void  ZrmReadyLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    do_layout(rect);
}

void ZrmReadyLayout::do_layout(const QRect& rect) const
{
    int sp = spacing();
    QRect r = this->alignmentRect(rect);
    int x = r.left() + sp;
    int y = r.top () + sp;
    int max_x = 0;
    for (auto litem : m_items)
    {
        auto* w = dynamic_cast<ZrmBaseWidget*>(litem->widget());
        w->setGeometry(QRect(x, y, m_min_width, m_min_height));
        x += sp + m_min_width;
        max_x = qMax(max_x, x);
        if (x + m_min_width + sp > r.right() && x > r.left() + sp)
        {
            x = r.left() + sp;
            y += m_min_height + sp;
        }
    }
    m_hint_width = max_x;
    m_hint_height = (x == r.left() + sp) ? y : y + m_min_height + sp;

    parentWidget()->setMinimumHeight(m_hint_height);
}
