#include "ZrmTCPSettings.h"

ZrmTCPSettings::ZrmTCPSettings(QWidget *parent) :
    ZrmChannelWidget(parent)
{
    setupUi(this);

    QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    QRegExp ipRegex ("^" + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange + "$");
    QRegExpValidator *ipValidator = new QRegExpValidator(ipRegex, this);
    leHost->setValidator(ipValidator);
    leMask->setValidator(ipValidator);

    connect(buttonSetSettings, &QAbstractButton::clicked, this, &ZrmTCPSettings::setSettings);
}

void ZrmTCPSettings::on_connected(bool con_state)
{
    Q_UNUSED(con_state)
    //buttonSetSettings->setEnabled(con_state);
}

void ZrmTCPSettings::update_controls()
{
    clear_controls();
    if(m_source && m_channel )
        channel_param_changed(m_channel, m_source->channel_params(m_channel));
}

void ZrmTCPSettings::clear_controls()
{
    leHost->clear();
    //sbPort->clear();
    sbPort->setValue(0);
    leMask->clear();
    //buttonSetSettings->setEnabled(false);
}

void ZrmTCPSettings::channel_param_changed(unsigned channel, const zrm::params_list_t & params_list)
{
    //SignalBlocker sb(findChildren<QWidget*>());
    if(channel == m_channel && m_source)
    {
        for(auto param : params_list)
        {
            QVariant value = m_source->param_get(m_channel, param.first);
            switch(param.first)
            {
            case zrm::PARAM_TCP_SETTINGS :
            {
                QString host;
                for (int i = 0; i < 4; i++)
                {
                    if (!host.isEmpty())
                        host += '.';
                    host += QString::number(param.second.puchar[i]);
                }
                leHost->setText(host);
                QString mask;
                for (int i = 0; i < 4; i++)
                {
                    if (!mask.isEmpty())
                        mask += '.';
                    mask += QString::number(param.second.puchar[i + 4]);
                }
                leMask->setText(mask);
                quint16 port;
                memcpy(&port, param.second.puchar + 8, 2);
                sbPort->setValue(port);
                break;
            }
            default: break;
            }
        }
    }
    ZrmChannelWidget::channel_param_changed(channel, params_list);
}

void ZrmTCPSettings::channel_session(unsigned channel)
{
    if (m_source && m_channel == channel && m_source->channel_session(m_channel).is_active())
    {
        zrm::params_t params;
        params.resize(zrm::params_t::size_type(1));
        params[0] = zrm::params_t::value_type(zrm::PARAM_TCP_SETTINGS);
        m_source->channel_subscribe_params(m_channel, params, true);
    }
}

void ZrmTCPSettings::setSettings()
{
    if (!m_source || leHost->text().isEmpty() || sbPort->value() <= 0 || leMask->text().isEmpty())
        return;
    zrm::zrm_param_t param = zrm::PARAM_TCP_SETTINGS;
    QByteArray data;
    data.resize(10);
    QStringList listHost = leHost->text().split('.');
    for (int i = 0; i < 4; i++)
    {
        data[i] = static_cast<char>(listHost[i].toInt());
    }
    QStringList listMask = leMask->text().split('.');
    for (int i = 0; i < 4; i++)
    {
        data[i + 4] = static_cast<char>(listMask[i].toInt());
    }
    quint16 port = static_cast<quint16>(sbPort->value());
    memcpy(data.data() + 8, &port, 2);
    m_source->channel_write_param(m_channel, zrm::WM_PROCESS_AND_WRITE, param, data.data(), 10);
}
