#include <wx/wx.h>
#include <wx/weakref.h>

#include "SubWin.h"

// Main window: input field, two buttons, read-only output box
class MainFrame : public wxFrame {
public:
    MainFrame() : wxFrame(nullptr, wxID_ANY, "GUI Example 03",
                          wxDefaultPosition, wxSize(400, 300)) {
        // Main menu: File -> Test (Ctrl+T), File -> Quit (Ctrl+Q)
        auto *fileMenu = new wxMenu;
        fileMenu->Append(ID_TEST, "&Test\tCtrl+T", "Run the test function");
        fileMenu->AppendSeparator();
        fileMenu->Append(wxID_EXIT, "&Quit\tCtrl+Q", "Quit the application");
        // Objects -> SubWin: open (or bring to front) the sub window
        auto *objectsMenu = new wxMenu;
        objectsMenu->Append(ID_SUBWIN, "&SubWin", "Show the SubWin window");
        auto *menuBar = new wxMenuBar;
        menuBar->Append(fileMenu, "&File");
        menuBar->Append(objectsMenu, "&Objects");
        SetMenuBar(menuBar);
        Bind(wxEVT_MENU, &MainFrame::onTest, this, ID_TEST);
        Bind(wxEVT_MENU, &MainFrame::onSubWin, this, ID_SUBWIN);
        Bind(wxEVT_MENU, [this](wxCommandEvent &) { Close(); }, wxID_EXIT);

        auto *panel = new wxPanel(this);

        // Input controls
        auto *label = new wxStaticText(panel, wxID_ANY, "Your name:");
        input_ = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition,
                                wxDefaultSize, wxTE_PROCESS_ENTER);
        input_->SetHint("Type a name and press Enter or click Greet");
        auto *greetBtn = new wxButton(panel, wxID_ANY, "Greet");
        auto *clearBtn = new wxButton(panel, wxID_ANY, "Clear");

        // Button colours: green "Greet", red "Clear", white text for contrast
        greetBtn->SetBackgroundColour(wxColour(46, 160, 67));
        greetBtn->SetForegroundColour(*wxWHITE);
        clearBtn->SetBackgroundColour(wxColour(207, 34, 46));
        clearBtn->SetForegroundColour(*wxWHITE);

        // Output control
        output_ = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition,
                                 wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);

        // Layout: [label][input] / buttons right-aligned / output fills rest
        auto *inputRow = new wxBoxSizer(wxHORIZONTAL);
        inputRow->Add(label, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
        inputRow->Add(input_, 1);

        auto *buttonRow = new wxBoxSizer(wxHORIZONTAL);
        buttonRow->AddStretchSpacer();
        buttonRow->Add(greetBtn, 0, wxRIGHT, 8);
        buttonRow->Add(clearBtn);

        auto *layout = new wxBoxSizer(wxVERTICAL);
        layout->Add(inputRow, 0, wxEXPAND | wxALL, 8);
        layout->Add(buttonRow, 0, wxEXPAND | wxLEFT | wxRIGHT, 8);
        layout->Add(output_, 1, wxEXPAND | wxALL, 8);
        panel->SetSizer(layout);

        // Behaviour: bind events to member functions / lambdas
        greetBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) { Greet(); });
        input_->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent &) { Greet(); });
        clearBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) { output_->Clear(); });

        input_->SetFocus();
    }

private:
    // IDs for own menu items (wxID_HIGHEST + n avoids clashes with wx IDs)
    enum { ID_TEST = wxID_HIGHEST + 1, ID_SUBWIN };

    // File -> Test: print a text into the output box
    void onTest(wxCommandEvent &) {
        output_->AppendText("File -> Test was called.\n");
    }

    // Objects -> SubWin: create the sub window once; if it is already
    // open, just bring it to the front instead of opening a second one
    void onSubWin(wxCommandEvent &) {
        if (!subWin_)
            subWin_ = new SubWin(this);   // parent = this: closed with main window
        subWin_->Show();
        subWin_->Raise();
    }

    void Greet() {
        const wxString name = input_->GetValue().Trim().Trim(false);
        output_->AppendText(name.IsEmpty() ? wxString("Please enter a name.\n")
                                           : "Hello, " + name + "!\n");
        input_->Clear();
        input_->SetFocus();
    }

    wxTextCtrl *input_  = nullptr;
    wxTextCtrl *output_ = nullptr;
    wxWeakRef<SubWin> subWin_;   // becomes nullptr automatically when SubWin closes
};

class App : public wxApp {
public:
    bool OnInit() override {
        (new MainFrame())->Show();
        return true;
    }
};

wxIMPLEMENT_APP(App);
