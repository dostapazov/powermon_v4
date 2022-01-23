#ifndef UI_CONSTRAINTS_HPP
#define UI_CONSTRAINTS_HPP
#include <qglobal.h>

#ifdef Q_OS_ANDROID
constexpr int MAIN_WIDOW_ICON_WIDTH = 42;
constexpr int MAIN_WIDOW_ICON_HEIGHT = 42;
constexpr int MAIN_WIDOW_BUTTON_WIDTH = 42;
constexpr int MAIN_WIDOW_BUTTON_HEIGHT = 42;
constexpr int SPLITTER_HADNLE_WIDTH = 16;
constexpr int DEFAULT_FONT_SIZE = 32;

#else
constexpr int DESKTOP_MAIN_WIDOW_ICON_WIDTH = 42;
constexpr int DESKTOP_MAIN_WIDOW_ICON_HEIGTH = 42;
constexpr int DESKTOP_MAIN_WIDOW_BUTTON_WIDTH = 42;
constexpr int DESKTOP_MAIN_WIDOW_BUTTON_HEIGTH = 42;
constexpr int SPLITTER_HADNLE_WIDTH = 8;
constexpr int DEFAULT_FONT_SIZE = 16;
#endif



#endif // UI_CONSTRAINTS_HPP
