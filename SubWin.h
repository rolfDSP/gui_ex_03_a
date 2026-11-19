#pragma once

#include <wx/wx.h>

// Child window that simply shows a greeting text.
// Created with the main frame as parent, so wxWidgets closes it
// automatically when the main frame (the application) is closed.
class SubWin : public wxFrame {
public:
    explicit SubWin(wxWindow *parent);
};
