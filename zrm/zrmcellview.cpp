#include "zrmcellview.h"
#include <algorithm>

ZrmCellView::ZrmCellView(QWidget* parent) :
    ZrmChannelWidget(parent)
{
    setupUi(this);
    QHeaderView* hv = cell_table->horizontalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    cell_table->setRowCount(2);
    cell_table->setColumnCount(0);
    QStringList sl;
    sl << "U" << "T";
    cell_table->setVerticalHeaderLabels(sl);

    connect(cell_dU, SIGNAL(valueChanged(double)), this, SLOT(saveCell()));
    connect(cell_dT, SIGNAL(valueChanged(double)), this, SLOT(saveCell()));
    initCellsTree();

}


void ZrmCellView::channel_session(unsigned ch_num)
{
    if (m_source && m_channel == ch_num && m_source->channel_session(m_channel).is_active())
    {
        zrm::params_t params;
        params.push_back(zrm::PARAM_CCNT);
        params.push_back(zrm::PARAM_CELL);
        m_source->channel_subscribe_params(m_channel, params, true);
    }
}

void ZrmCellView::channel_param_changed(unsigned channel, const zrm::params_list_t& params_list)
{
    if (channel == m_channel)
    {

        zrm::params_list_t::const_iterator end = params_list.end();
        zrm::params_list_t::const_iterator ptr;

        if ( (ptr = params_list.find(zrm::PARAM_CCNT)) != end )
        {
            cell_count(ptr->second.value<uint16_t>(false));
            cellsCount(ptr->second.value<uint16_t>(false));
        }

        if ((ptr = params_list.find(zrm::PARAM_CELL)) != end )
        {
            cell_params(ptr->second.uword);
            cellParams(ptr->second.uword);
        }
    }
    ZrmChannelWidget::channel_param_changed(channel, params_list);
}

void ZrmCellView::update_column_width()
{
    int ccnt = cell_table->columnCount();
    QHeaderView* hv = cell_table->horizontalHeader();
    if (hv && ccnt)
        hv->setDefaultSectionSize((cell_table->width() - cell_table->verticalHeader()->width()) / ccnt);
}

void ZrmCellView::create_cell_item(int row, int col)
{
    QTableWidgetItem* item;
    item = cell_table->item(row, col);
    if (!item)
    {
        item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignCenter);
        cell_table->setItem(row, col, item);
    }
}


void ZrmCellView::cell_count(uint16_t ccnt)
{
    qDebug() << Q_FUNC_INFO;


    if (ccnt == cell_table->columnCount())
        return;

    cell_table->clearContents();
    cell_table->setColumnCount(ccnt);
    int row_count = cell_table->rowCount();

    if (ccnt)
    {
        QStringList sl;
        for (int i = 0; i < ccnt; i++)
        {
            sl << QString::number(i + 1);
            for (int row = 0; row < row_count; row++)
                create_cell_item(row, i);
        }
        cell_table->setHorizontalHeaderLabels(sl);
        update_column_width();
    }
}

void check_out_bounds(QTableWidgetItem* item, double value, double mid, double delta, QColor bk_color, QColor text_color )
{
    if (!qFuzzyIsNull(delta) && fabs(mid - value) > fabs(delta))
        std::swap(bk_color, text_color);
    item->setBackground(bk_color);
    item->setForeground(text_color);
}

void ZrmCellView::cell_params(uint16_t value)
{
    qDebug() << Q_FUNC_INFO;
    cell_count(value);
    zrm::zrm_cells_t cells =  m_source->channel_cell_info(m_channel);

    double min_volt = std::numeric_limits<double>::max();// 10000;
    double max_volt = std::numeric_limits<double>::min();
    double mid_volt = 0;
    double min_temp = 10000;
    double max_temp = -10000;
    double mid_temp = 0;

    zrm::zrm_cells_t::iterator beg = cells.begin();
    zrm::zrm_cells_t::iterator end = cells.end();
    zrm::zrm_cells_t::iterator ptr = beg;

    while (ptr < end)
    {
        double volt = ptr->volt();
        double temp = ptr->temp();

        min_volt = qMin(min_volt, volt);
        max_volt = qMax(max_volt, volt);
        mid_volt += volt;
        min_temp = qMin(min_temp, temp);
        max_temp = qMax(max_temp, temp);
        mid_temp += temp;
        ++ptr;
    }

    if (value > 2 && !qFuzzyCompare(min_volt, max_volt) )
    {
        mid_volt -= min_volt;
        mid_volt -= max_volt;
        mid_volt /= double(value - 2);
    }
    else
    {
        mid_volt /= double(value);
    }

    if (value > 2 && !qFuzzyCompare(min_temp, max_temp) )
    {
        mid_temp -= min_temp;
        mid_temp -= max_temp;
        mid_temp /= double(value - 2);
    }
    else
    {
        mid_temp /= double(value);
    }


    int col = 0;
    auto dU = cell_dU->value();
    auto dT = cell_dT->value();
    QColor bk_color = cell_table->palette().color(QPalette::Background);
    QColor txt_color = cell_table->palette().color(QPalette::Text);


    ptr = beg;
    while (ptr < end)
    {
        QTableWidgetItem* item_u = cell_table->item(0, col);
        QTableWidgetItem* item_t = cell_table->item(1, col);

        if (item_u)
        {
            double volt = ptr->volt();
            item_u->setText(number_text(volt, 2));
            check_out_bounds(item_u, volt, mid_volt, dU, bk_color, txt_color);
        }

        if (item_t)
        {
            double temp = ptr->temp();
            item_t->setText(number_text(temp, 1));
            check_out_bounds(item_t, temp, mid_temp, dT, bk_color, txt_color);
        }
        ++col;
        ++ptr;
    }
}

class MinMaxValues
{
    double min_volt = std::numeric_limits<double>::max();
    double max_volt = std::numeric_limits<double>::min();
    double sum_volt = 0;
    double min_temp = std::numeric_limits<double>::max();
    double max_temp = std::numeric_limits<double>::min();
    double sum_temp = 0;
    int    count = 0;
public:
    void   appendValue(double volt, double temp);
    double minVolt() {return min_volt;}
    double maxVolt() {return max_volt;}
    double midVolt() {return sum_volt / double((count < 2) ? 1 : count);}
    double minTemp() {return min_temp;}
    double maxTemp() {return max_temp;}
    double midTemp() {return sum_temp / double((count < 2) ? 1 : count);}

};

void   MinMaxValues::appendValue(double volt, double temp)
{
    ++count;
    min_volt = qMin(volt, min_volt);
    max_volt = qMax(volt, max_volt);
    sum_volt += volt;
    min_temp = qMin(temp, min_temp);
    max_temp = qMax(temp, max_temp);
    sum_temp += temp;
};

void ZrmCellView::cellParams(uint16_t value)
{

    cellsCount(value);
    zrm::zrm_cells_t cells =  m_source->channel_cell_info(m_channel);
    MinMaxValues mmv;

    for (const zrm::zrm_cell_t& cell : cells)
    {
        mmv.appendValue(cell.volt(), cell.temp());
    }

    int row = 0;
    for (const zrm::zrm_cell_t& cell : cells)
    {
        QTreeWidgetItem* item = cellsTree->topLevelItem(row);
        item->setText(ColumnRoles::volt, QString::number(cell.volt(), 'f', 2));
        item->setText(ColumnRoles::temp, QString::number(cell.temp(), 'f', 2));
    }
}


void ZrmCellView::update_controls()
{
    ZrmChannelWidget::update_controls();
    if (m_source && m_channel)
    {
        uint16_t ccnt = param_get(zrm::PARAM_CCNT).value<uint16_t>(false);
        cell_params(ccnt);
        channel_session(m_channel);
        channel_param_changed(m_channel, m_source->channel_params(m_channel));
    }
}

void ZrmCellView::clear_controls()
{
    cell_count(0);
}

void ZrmCellView::resizeEvent(QResizeEvent* re)
{
    ZrmChannelWidget::resizeEvent(re);
    update_column_width();
    updateColumnWidth();
}

void ZrmCellView::showEvent(QShowEvent* se)
{
    ZrmChannelWidget::showEvent(se);
    update_column_width();
    updateColumnWidth();
}

void ZrmCellView::update_params()
{
    if (m_source && m_channel)
    {
        auto p = m_source->channel_masakb_param(m_channel);
        cell_dU->setValue(p.dU);
        cell_dT->setValue(p.dT);
    }
}

void ZrmCellView::saveCell()
{
    zrm::zrm_maskab_param_t _map;
    _map.dU = cell_dU->value();
    _map.dT = cell_dT->value();
    m_source->channel_set_masakb_param(m_channel, _map);
}

void ZrmCellView::initCellsTree()
{
    QHeaderView* hv = cellsTree->header();
    hv->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    hv->setDefaultAlignment(Qt::AlignmentFlag::AlignCenter);
    updateColumnWidth();
}

void ZrmCellView::updateColumnWidth()
{
    QHeaderView* hv = cellsTree->header();
    int colWidth = hv->width() / hv->count();

    for (int index = 0; index < hv->count(); index++)
    {
        hv->resizeSection(index, colWidth);

    }

}

void ZrmCellView::cellsCount(uint16_t ccnt)
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




