#include "zrmmethodchoose.h"

ZrmMethodChoose::ZrmMethodChoose(QWidget* parent) :
    QDialog(parent)
{
    setupUi(this);

    connect(methods, &ZrmMethodsTree::method_selected, this, &ZrmMethodChoose::sl_method_selected);
    connect(doubleSpinBoxVolt, SIGNAL(valueChanged(double)), this, SLOT(editVoltage()));
    connect(doubleSpinBoxCap, SIGNAL(valueChanged(double)), this, SLOT(editCapacity()));
}

ZrmMethodChoose::~ZrmMethodChoose()
{
    close_database();
}

bool ZrmMethodChoose::open_database(bool remake_tree)
{
    if (remake_tree)
        methods->clear();
    return methods->open_database(m_work_mode, bAbstract);
}

void ZrmMethodChoose::close_database()
{
    methods->close_database();
}

int ZrmMethodChoose::exec()
{
    showMaximized();
    return QDialog::exec();
}

void ZrmMethodChoose::set_mode(zrm::zrm_work_mode_t value)
{
    if (m_work_mode != zrm::zrm_work_mode_t(value))
        m_work_mode = zrm::zrm_work_mode_t(value);
}

void ZrmMethodChoose::sl_method_selected(QTreeWidgetItem* item)
{
    m_current_method = item;
    bool sel_enabled = (item && methods->method_valid(item));
    zrm::zrm_method_t method;
    get_method(method, nullptr);
    doubleSpinBoxVolt->setValue(method.m_method.voltage());
    doubleSpinBoxCap->setValue(method.m_method.capacity());
    gbVC->setEnabled(bAbstract);
    gbVC->setVisible(bAbstract);
    labelWarning->setVisible(sel_enabled && bAbstract);
    method_minmax->method_minmax(method);
    bOk->setEnabled(sel_enabled);
}

bool  ZrmMethodChoose::get_method(zrm::zrm_method_t&   zrm_method, QTextCodec* codec, QString* model_name)
{
    if (!m_current_method)
        return false;

    return methods->get_method(zrm_method, codec, model_name);
}

void ZrmMethodChoose::setAbstract(bool a)
{
    bAbstract = a;
    m_current_method = nullptr;
    gbVC->setVisible(a);
    open_database(true);
    methods->setFocus(Qt::OtherFocusReason);
    auto item = methods->currentItem();
    if (item)
    {
        item->setSelected(true);
        sl_method_selected(item);
    }
    else
        method_minmax->clear_controls();
}

void ZrmMethodChoose::editVoltage()
{
    if (!m_current_method)
        return;
    zrm::zrm_method_t method;
    get_method(method, nullptr);
    double volt = doubleSpinBoxVolt->value();
    method.m_method.set_voltage(volt);
    m_current_method->setData(0, ZrmMethodsTree::tree_roles_t::role_voltage, volt);
    QString text;
    text = methods->number(m_current_method->data(0, ZrmMethodsTree::tree_roles_t::role_voltage).toDouble(), 1);
    m_current_method->setText(ZrmMethodsTree::column_type_t::column_voltage, text);
    labelWarning->setVisible(false);
}

void ZrmMethodChoose::editCapacity()
{
    if (!m_current_method)
        return;
    zrm::zrm_method_t method;
    get_method(method, nullptr);
    double cap = doubleSpinBoxCap->value();
    method.m_method.set_capacity(cap);
    m_current_method->setData(0, ZrmMethodsTree::tree_roles_t::role_capacity, cap);
    QString text;
    text = methods->number(m_current_method->data(0, ZrmMethodsTree::tree_roles_t::role_capacity).toDouble(), 1);
    m_current_method->setText(ZrmMethodsTree::column_type_t::column_capacity, text);
    labelWarning->setVisible(false);
}
