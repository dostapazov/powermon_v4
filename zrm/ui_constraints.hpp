#ifndef UI_CONSTRAINTS_HPP
#define UI_CONSTRAINTS_HPP
#include <qglobal.h>

#ifdef Q_OS_ANDROID
    constexpr int MAIN_WIDOW_ICON_WIDTH = 96;
    constexpr int MAIN_WIDOW_ICON_HEIGHT = 96;
    constexpr int MAIN_WIDOW_BUTTON_WIDTH = 96;
    constexpr int MAIN_WIDOW_BUTTON_HEIGHT = 96;
    constexpr int SPLITTER_HADNLE_WIDTH = 32;
    constexpr int DEFAULT_FONT_SIZE = 32;

    constexpr int MAIN_DISPLAT_ICON_HEIGHT = 48;
    constexpr int MAIN_DISPLAT_ICON_WIDTH = 48;

#else
    constexpr int DESKTOP_MAIN_WIDOW_ICON_WIDTH = 50;
    constexpr int DESKTOP_MAIN_WIDOW_ICON_HEIGTH = 50;
    constexpr int DESKTOP_MAIN_WIDOW_BUTTON_WIDTH = 60;
    constexpr int DESKTOP_MAIN_WIDOW_BUTTON_HEIGTH = 60;
    constexpr int SPLITTER_HADNLE_WIDTH = 8;
    constexpr int DEFAULT_FONT_SIZE = 16;
#endif



#endif // UI_CONSTRAINTS_HPP
