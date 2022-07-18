#include "powermon_utils.h"
#include <QLineEdit>


namespace pwm_utils {

void setEditText(QLineEdit* edit, const QString& text, int cursorPos )
{
    if (edit)
    {
        edit->setText(text);
        if (cursorPos >= 0)
            edit->setCursorPosition(cursorPos);
    }
}

QString    hms2string(const method_hms& hms)
{
    QString str = QString("%1:%2:%3")
                  .arg(std::get<0>(hms), 3, 10, QLatin1Char('0'))
                  .arg(std::get<1>(hms), 2, 10, QLatin1Char('0'))
                  .arg(std::get<2>(hms), 2, 10, QLatin1Char('0'));
    return str;
}

method_hms string2hms(const QString& str)
{
    uint8_t hms[3] = {0};
    QStringList sl = str.split(QChar(':'));
    int cnt = qMin(3, sl.count());
    int i = 0;
    for (auto&& s : sl)
    {
        if (i < cnt)
        {
            hms[i++] = uint8_t(s.trimmed().toUInt());
        }
        else
            break;
    }
    return std::make_tuple(hms[0], hms[1], hms[2]);
}

method_hms secunds2hms(uint32_t duration)
{
    div_t h  = div(int(duration), SECUNDS_IN_HOUR);
    div_t ms = div(h.rem, SECUNDS_IN_MINUTE  );
    return std::make_tuple(uint8_t(h.quot), uint8_t(ms.quot), uint8_t(ms.rem));
}

uint32_t  hms2secunds(uint8_t h, uint8_t m, uint8_t s)
{
    return h * SECUNDS_IN_HOUR + m * SECUNDS_IN_MINUTE + s;
}

uint32_t  hms2secunds(const method_hms& hms)
{
    return hms2secunds(std::get<0>(hms), std::get<1>(hms), std::get<2>(hms) );
}



}
