#include "powermon_utils.h"



void setEditText(QLineEdit * edit, const QString & text, int cursorPos )
{
    if(edit)
    {
        edit->setText(text);
        if(cursorPos>=0)
            edit->setCursorPosition(cursorPos);
    }
}
