#ifndef POWERMON_UTILS_H
#define POWERMON_UTILS_H
#include <tuple>
#include <QString>

class QLineEdit;
namespace pwm_utils {

constexpr int SECUNDS_IN_MINUTE = 60;
constexpr int SECUNDS_IN_HOUR = 60 * SECUNDS_IN_MINUTE;

using method_hms = std::tuple<uint8_t, uint8_t, uint8_t>;
QString        hms2string(const method_hms& hms);
method_hms     string2hms(const QString& str);
method_hms     secunds2hms(uint32_t duration);
uint32_t       hms2secunds(const method_hms& hms);
uint32_t       hms2secunds(uint8_t h, uint8_t m, uint8_t s);


void setEditText(QLineEdit* edit, const QString& text, int cursorPos = -1);

}

#endif // POWERMON_UTILS_H
