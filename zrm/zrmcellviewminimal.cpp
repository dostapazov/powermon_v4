#include "zrmcellviewminimal.h"
#include <algorithm>
#include <QPalette>
#ifdef QT_DEBUG
    #include <QRandomGenerator>
#endif

ZrmCellViewMinimal::ZrmCellViewMinimal(QWidget* parent) :
    ZrmChannelWidget(parent)
{
    setupUi(this);
    initCellsTree();
}

void ZrmCellViewMinimal::channel_session(unsigned ch_num)
{
    if (m_source && m_channel == ch_num && m_source->channel_session(m_channel).is_active())
    {
        zrm::params_t params;
        params.push_back(zrm::PARAM_CCNT);
        params.push_back(zrm::PARAM_CELL);
        m_source->channel_subscribe_params(m_channel, params, true);
        cellsCount(20);
    }
}

void ZrmCellViewMinimal::channel_param_changed(unsigned channel, const zrm::params_list_t& params_list)
{
    if (channel == m_channel)
    {

        zrm::params_list_t::const_iterator end = params_list.end();
        zrm::params_list_t::const_iterator ptr;

        if ( (ptr = params_list.find(zrm::PARAM_CCNT)) != end )
        {
            cellsCount(ptr->second.value<uint16_t>(false));
        }

        if ((ptr = params_list.find(zrm::PARAM_CELL)) != end )
        {
            cellsParam();
        }
    }
    ZrmChannelWidget::channel_param_changed(channel, params_list);
}

//class MinMaxValues
//{
//    double min_volt = std::numeric_limits<double>::max();
//    double max_volt = std::numeric_limits<double>::min();
//    double sum_volt = 0;
//    double min_temp = std::numeric_limits<double>::max();
//    double max_temp = std::numeric_limits<double>::min();
//    double sum_temp = 0;
//    int    count = 0;
//    double dU ;
//    double dT ;
//public:
//    MinMaxValues(double du, double dt): dU(du), dT(dt) {};
//    void   appendValue(double volt, double temp);
//    double minVolt() {return min_volt;}
//    double maxVolt() {return max_volt;}
//    double midVolt() {return sum_volt / double((count < 2) ? 1 : count);}
//    double minTemp() {return min_temp;}
//    double maxTemp() {return max_temp;}
//    double midTemp() {return sum_temp / double((count < 2) ? 1 : count);}
//    void   checkOutBounds(QTreeWidgetItem* item,
//                          double volt, double temp,
//                          const QColor& bk, const QColor& text,
//                          const QColor& outBk, const QColor& outText
//                         );

//};

//void   MinMaxValues::appendValue(double volt, double temp)
//{
//    ++count;
//    min_volt = qMin(volt, min_volt);
//    max_volt = qMax(volt, max_volt);
//    sum_volt += volt;
//    min_temp = qMin(temp, min_temp);
//    max_temp = qMax(temp, max_temp);
//    sum_temp += temp;
//};


//void   MinMaxValues::checkOutBounds(QTreeWidgetItem* item,
//                                    double volt, double temp,
//                                    const QColor& bk, const QColor& text,
//                                    const QColor& outBk, const QColor& outText
//                                   )
//{
//    if (!item)
//        return;
//    auto cond = [](double value, double midValue, double delta)->bool
//    {
//        return !qFuzzyIsNull(delta) && fabs(midValue - value) > fabs(delta);
//    };

//    if (!qFuzzyIsNull(dU) && cond(volt, midVolt(), dU))
//    {
//        item->setData(ZrmCellViewMinimal::ColumnRoles::volt, Qt::BackgroundColorRole, outBk);
//        item->setData(ZrmCellViewMinimal::ColumnRoles::volt, Qt::TextColorRole, outText);
//    }
//    else
//    {
//        item->setData(ZrmCellViewMinimal::ColumnRoles::volt, Qt::BackgroundColorRole, bk);
//        item->setData(ZrmCellViewMinimal::ColumnRoles::volt, Qt::TextColorRole, text);
//    }

//    if (!qFuzzyIsNull(dT) && cond(temp, midTemp(), dT))
//    {
//        item->setData(ZrmCellViewMinimal::ColumnRoles::temp, Qt::BackgroundColorRole, outBk);
//        item->setData(ZrmCellViewMinimal::ColumnRoles::temp, Qt::TextColorRole, outText);
//    }
//    else
//    {
//        item->setData(ZrmCellViewMinimal::ColumnRoles::temp, Qt::BackgroundColorRole, bk);
//        item->setData(ZrmCellViewMinimal::ColumnRoles::temp, Qt::TextColorRole, text);
//    }

//}

#ifdef QT_DEBUG
namespace {
zrm::zrm_cells_t fakeCells(size_t sz)
{
    zrm::zrm_cells_t cells;
    cells.resize(sz);
    for (zrm::zrm_cell_t& cell : cells)
    {
        cell.m_volt = QRandomGenerator::global()->bounded(2000, 100000);
        cell.m_temp = QRandomGenerator::global()->bounded(5000, 30000);
    }
    return cells;
}
}
#endif


void ZrmCellViewMinimal::cellsParam()
{
    cellsTree->setUpdatesEnabled(false);
#ifdef QT_DEBUG
    zrm::zrm_cells_t cells = fakeCells(20);
#else
    zrm::zrm_cells_t cells =  m_source->channel_cell_info(m_channel);
#endif

    cellsCount(cells.size());

    int row = 0;
    for (const zrm::zrm_cell_t& cell : cells)
    {
        QTreeWidgetItem* item = cellsTree->topLevelItem(row++);
        item->setText(ColumnRoles::volt, QString::number(cell.volt(), 'f', 2));
        item->setText(ColumnRoles::temp, QString::number(cell.temp(), 'f', 2));
    }

//    row = 0;
//    for (const zrm::zrm_cell_t& cell : cells)
//    {
//        QTreeWidgetItem* item = cellsTree->topLevelItem(row++);
//        mmv.checkOutBounds(item, cell.volt(), cell.temp(), normalBackground, normalText, outBoundBackground, outBoundText);

//    }
    cellsTree->setUpdatesEnabled(true);
}


void ZrmCellViewMinimal::update_controls()
{
    ZrmChannelWidget::update_controls();
    if (m_source && m_channel)
    {
        cellsParam();
        channel_session(m_channel);
        channel_param_changed(m_channel, m_source->channel_params(m_channel));
    }
}

void ZrmCellViewMinimal::clear_controls()
{
    cellsCount(0);
}

void ZrmCellViewMinimal::resizeEvent(QResizeEvent* re)
{
    ZrmChannelWidget::resizeEvent(re);
    updateColumnWidth();
}

void ZrmCellViewMinimal::showEvent(QShowEvent* se)
{
    ZrmChannelWidget::showEvent(se);
    updateColumnWidth();
}


void ZrmCellViewMinimal::initCellsTree()
{
//    normalBackground = cellsTree->palette().color(QPalette::Background);
//    normalText = cellsTree->palette().color(QPalette::Text);
//    outBoundBackground = Qt::GlobalColor::darkRed;
//    outBoundText = Qt::GlobalColor::white;

    QHeaderView* hv = cellsTree->header();
    hv->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    hv->setDefaultAlignment(Qt::AlignmentFlag::AlignCenter);
    updateColumnWidth();
}

void ZrmCellViewMinimal::updateColumnWidth()
{
    QHeaderView* hv = cellsTree->header();
    int colWidth = hv->width() / hv->count();

    for (int index = 0; index < hv->count(); index++)
    {
        hv->resizeSection(index, colWidth);

    }

}

void ZrmCellViewMinimal::cellsCount(uint16_t ccnt)
{
    uint16_t itemsCount = cellsTree->topLevelItemCount();
    while (itemsCount < ccnt  )
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(cellsTree);
        item->setText(ColumnRoles::number, QString::number(++itemsCount));
        item->setText(ColumnRoles::volt, QString("--,--"));
        item->setText(ColumnRoles::temp, QString("--,--"));
        for (int i = 0; i <= ColumnRoles::temp; i++)
        {
            item->setTextAlignment(i, Qt::AlignmentFlag::AlignCenter );
        }
    }

    while (itemsCount > ccnt)
    {
        delete cellsTree->topLevelItem(--itemsCount);
    }
}

int  ZrmCellViewMinimal::getCellsCount() const
{
    return cellsTree->topLevelItemCount();
}

QSize ZrmCellViewMinimal::sizeHint() const
{
    QSize sz = ZrmChannelWidget::sizeHint();
    qDebug() << "ZrmCellViewMinimal size hint " << sz;
    qDebug() << "Cells Minimun size hint " << cellsTree->minimumSizeHint();
    qDebug() << "Cells size hint " << cellsTree->sizeHint();
    if (getCellsCount())
    {
        QTreeWidgetItem* item = cellsTree->topLevelItem(0);
        QSize itemSize =  cellsTree->visualItemRect(item).size();
        qDebug() << "Item visual size" << itemSize;
        itemSize.setHeight(getCellsCount() * (itemSize.height() + 3));
        qDebug() << "Item new visual size" << itemSize;
        sz.setHeight(itemSize.height());
    }

    return sz;
}


