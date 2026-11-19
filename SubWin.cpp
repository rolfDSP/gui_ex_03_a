#include "SubWin.h"

#include <wx/dcclient.h>

SubWin::SubWin(wxWindow *parent)
    : wxFrame(parent, wxID_ANY, "SubWin", wxDefaultPosition, wxSize(300, 150)) {
    // wxFULL_REPAINT_ON_RESIZE: redraw the whole panel when the window
    // is resized, so the border always follows the window edges
    auto *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                              wxTAB_TRAVERSAL | wxFULL_REPAINT_ON_RESIZE);
    auto *text = new wxStaticText(panel, wxID_ANY, "Hello from SubWin");

    // Colour theme: very dark red background with light text for contrast
    const wxColour background(64, 0, 0);
    const wxColour foreground(255, 220, 220);
    SetBackgroundColour(background);
    panel->SetBackgroundColour(background);
    text->SetBackgroundColour(background);
    text->SetForegroundColour(foreground);

    // White border: 5 px away from the inner edges, 2 px wide
    panel->Bind(wxEVT_PAINT, [panel](wxPaintEvent &) {
        constexpr int distance = 5;
        constexpr int width    = 2;
        wxPaintDC dc(panel);
        dc.SetPen(wxPen(*wxWHITE, 1));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        // Draw 'width' nested 1-px rectangles: exact pixel placement on
        // every platform (a single 2-px pen would be centred on the line)
        wxRect rect(panel->GetClientSize());
        rect.Deflate(distance);
        for (int i = 0; i < width; ++i) {
            dc.DrawRectangle(rect);
            rect.Deflate(1);
        }
    });

    // Centre the text in the window
    auto *layout = new wxBoxSizer(wxVERTICAL);
    layout->AddStretchSpacer();
    layout->Add(text, 0, wxALIGN_CENTER_HORIZONTAL);
    layout->AddStretchSpacer();
    panel->SetSizer(layout);
}
