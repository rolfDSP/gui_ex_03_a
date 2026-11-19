# gui_ex_03_a – Documentation

A small wxWidgets desktop application written in C++20. It is developed in
WSL (Linux) and builds two executables from the same source:

- a native **Linux** (GTK) executable
- a **Windows** executable, cross-compiled with MinGW-w64

The application has a main window with a name input and a greeting output, a
menu bar, and a separate child window (`SubWin`) with its own colour theme and
a drawn border.

---

## Contents

1. [Project layout](#1-project-layout)
2. [Features overview](#2-features-overview)
3. [Source code](#3-source-code)
   - [3.1 main.cpp – App and MainFrame](#31-maincpp--app-and-mainframe)
   - [3.2 SubWin.h / SubWin.cpp – the child window](#32-subwinh--subwincpp--the-child-window)
4. [Build system](#4-build-system)
   - [4.1 CMakeLists.txt](#41-cmakeliststxt)
   - [4.2 CMakePresets.json](#42-cmakepresetsjson)
   - [4.3 MinGW toolchain file](#43-mingw-toolchain-file)
   - [4.4 Prebuilt wxWidgets for Windows](#44-prebuilt-wxwidgets-for-windows)
5. [Building and running](#5-building-and-running)
6. [Extending the application](#6-extending-the-application)
7. [Troubleshooting](#7-troubleshooting)

---

## 1. Project layout

```
gui_ex_03_a/
├── main.cpp                    Application entry point, App and MainFrame classes
├── SubWin.h                    Declaration of the SubWin child window
├── SubWin.cpp                  Implementation of SubWin (theme, border, text)
├── CMakeLists.txt              Build description (wxWidgets lookup, executable)
├── CMakePresets.json           Configure/build presets for Linux and Windows
├── cmake/
│   └── mingw-w64-x86_64.cmake  Toolchain file for the Windows cross-build
├── build/<preset>/             Build directories (generated, one per preset)
└── exe/
    ├── linux/gui_ex_03_a       Linux executable (generated)
    └── windows/gui_ex_03_a.exe Windows executable (generated)
```

The `*:Zone.Identifier` files are metadata that Windows adds to files copied
from a Windows drive. They are not used by the build and can be deleted.

---

## 2. Features overview

### Main window ("GUI Example 03", 400 × 300)

| Element                 | Behaviour                                                      |
|-------------------------|----------------------------------------------------------------|
| **Your name:** input    | Text field with hint; pressing **Enter** triggers *Greet*      |
| **Greet** button (green)| Writes `Hello, <name>!` to the output box and clears the input |
| **Clear** button (red)  | Empties the output box                                         |
| Output box              | Multi-line, read-only, fills the remaining window space        |

If the input is empty (or only spaces), *Greet* writes `Please enter a name.`

### Menu bar

| Menu        | Item       | Shortcut | Action                                              |
|-------------|------------|----------|-----------------------------------------------------|
| **File**    | Test       | Ctrl+T   | Writes `File -> Test was called.` to the output box |
| **File**    | Quit       | Ctrl+Q   | Closes the application                              |
| **Objects** | SubWin     | –        | Opens the SubWin window, or brings it to the front  |

### SubWin window ("SubWin", 300 × 150)

- Shows the text **Hello from SubWin**, centred.
- Very dark red background (RGB 64, 0, 0) with light text (RGB 255, 220, 220).
- White border, **2 px wide**, **5 px** away from the edges of the inner area;
  it follows the window when it is resized.
- Only **one** instance exists at a time.
- Closes automatically when the main window (the application) closes.

---

## 3. Source code

### 3.1 main.cpp – App and MainFrame

#### `class App : public wxApp`

The application object. wxWidgets creates it through the macro
`wxIMPLEMENT_APP(App)`, which also provides the platform-specific `main()` /
`WinMain()` function.

```cpp
bool OnInit() override {
    (new MainFrame())->Show();
    return true;
}
```

`OnInit()` creates and shows the main window. Returning `true` starts the
event loop. The application ends when its last top-level window is closed.

> **Memory management:** wxWidgets windows are created with `new` and are
> never deleted manually. A top-level window destroys itself when it is closed,
> and every window destroys its child windows.

#### `class MainFrame : public wxFrame`

The main window. Everything is set up in the constructor.

**Menu bar**

```cpp
auto *fileMenu = new wxMenu;
fileMenu->Append(ID_TEST, "&Test\tCtrl+T", "Run the test function");
fileMenu->AppendSeparator();
fileMenu->Append(wxID_EXIT, "&Quit\tCtrl+Q", "Quit the application");

auto *objectsMenu = new wxMenu;
objectsMenu->Append(ID_SUBWIN, "&SubWin", "Show the SubWin window");
```

- `&` marks the keyboard mnemonic (Alt+F, Alt+O, …).
- `\tCtrl+T` after the label defines the keyboard shortcut.
- The third argument is the help text shown in a status bar (if one exists).
- `wxID_EXIT` is a standard wxWidgets ID; on some platforms it gets special
  handling (e.g. the application menu on macOS).

**Menu IDs**

```cpp
enum { ID_TEST = wxID_HIGHEST + 1, ID_SUBWIN };
```

Own IDs start above `wxID_HIGHEST` so they never clash with predefined
wxWidgets IDs. New menu items get new enum entries here.

**Event binding**

```cpp
Bind(wxEVT_MENU, &MainFrame::onTest,   this, ID_TEST);
Bind(wxEVT_MENU, &MainFrame::onSubWin, this, ID_SUBWIN);
Bind(wxEVT_MENU, [this](wxCommandEvent &) { Close(); }, wxID_EXIT);
```

Events are connected dynamically with `Bind()` – either to member functions
or to lambdas. The last argument selects which menu ID the handler is for.

**Controls and layout**

All controls are children of a `wxPanel`, which provides the correct
background colour and keyboard navigation (Tab) on all platforms.

The layout uses sizers so the window can be resized:

```
layout (vertical)
├── inputRow (horizontal):  [label] [input_ ──────── stretches]
├── buttonRow (horizontal): [── stretch spacer ──] [Greet] [Clear]
└── output_                 (stretches in both directions)
```

- `Add(window, proportion, flags, border)`: proportion `1` means "take the
  extra space", `wxEXPAND` means "fill the space in the other direction".
- `AddStretchSpacer()` pushes the buttons to the right.

**Button colours**

```cpp
greetBtn->SetBackgroundColour(wxColour(46, 160, 67));   // green
clearBtn->SetBackgroundColour(wxColour(207, 34, 46));   // red
...->SetForegroundColour(*wxWHITE);                     // white text
```

Note: with some GTK themes on Linux, button colours may be ignored or only
partly applied. On Windows they work.

**Member functions**

| Function        | Purpose                                                                 |
|-----------------|-------------------------------------------------------------------------|
| `onTest()`      | Handler for *File → Test*; appends a line to the output box             |
| `onSubWin()`    | Handler for *Objects → SubWin*; creates/shows/raises the SubWin window  |
| `Greet()`       | Reads and trims the input, writes the greeting, clears and refocuses input |

**Member variables**

| Variable   | Type                | Purpose                                          |
|------------|---------------------|--------------------------------------------------|
| `input_`   | `wxTextCtrl *`      | Name input field                                 |
| `output_`  | `wxTextCtrl *`      | Read-only output box                             |
| `subWin_`  | `wxWeakRef<SubWin>` | The SubWin window if open, otherwise `nullptr`   |

#### How SubWin is opened and closed

```cpp
void onSubWin(wxCommandEvent &) {
    if (!subWin_)
        subWin_ = new SubWin(this);   // parent = this: closed with main window
    subWin_->Show();
    subWin_->Raise();
}
```

- **Single instance:** a new SubWin is only created if none exists. If it is
  already open, `Raise()` brings it to the front.
- **`wxWeakRef`:** a "weak" pointer that is reset to `nullptr` automatically
  when the window it points to is destroyed. When the user closes SubWin,
  `subWin_` becomes `nullptr`, so the next menu click creates a fresh window.
  A plain pointer would keep pointing at the destroyed window (a dangling
  pointer) and crash the program.
- **Closing with the application:** SubWin is created with `this` (the main
  window) as **parent**. When a window is destroyed, wxWidgets destroys all its
  children – including child top-level windows such as SubWin. So no extra code
  is needed to close SubWin when the application exits.
- A side effect of the parent relationship: on Windows, SubWin always stays
  in front of the main window and has no separate taskbar button.

### 3.2 SubWin.h / SubWin.cpp – the child window

#### Declaration (`SubWin.h`)

```cpp
class SubWin : public wxFrame {
public:
    explicit SubWin(wxWindow *parent);
};
```

SubWin is a normal frame window (with title bar, resizable). `explicit`
prevents accidental implicit conversion from a `wxWindow*`.

#### Implementation (`SubWin.cpp`)

The constructor builds the whole window in four steps.

**1. Panel and text**

```cpp
auto *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                          wxTAB_TRAVERSAL | wxFULL_REPAINT_ON_RESIZE);
auto *text = new wxStaticText(panel, wxID_ANY, "Hello from SubWin");
```

`wxTAB_TRAVERSAL` is the default panel style. `wxFULL_REPAINT_ON_RESIZE` is
added so the **whole** panel is redrawn after a resize; without it only newly
exposed areas would be redrawn and old pieces of the border would remain
visible.

**2. Colour theme**

```cpp
const wxColour background(64, 0, 0);       // very dark red
const wxColour foreground(255, 220, 220);  // light pinkish white
SetBackgroundColour(background);
panel->SetBackgroundColour(background);
text->SetBackgroundColour(background);
text->SetForegroundColour(foreground);
```

The background is set on the frame, the panel and the text label, so no
default grey shows through anywhere. The title bar is drawn by the operating
system / window manager and keeps its normal look.

**3. White border (paint handler)**

```cpp
panel->Bind(wxEVT_PAINT, [panel](wxPaintEvent &) {
    constexpr int distance = 5;
    constexpr int width    = 2;
    wxPaintDC dc(panel);
    dc.SetPen(wxPen(*wxWHITE, 1));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    wxRect rect(panel->GetClientSize());
    rect.Deflate(distance);
    for (int i = 0; i < width; ++i) {
        dc.DrawRectangle(rect);
        rect.Deflate(1);
    }
});
```

- `wxEVT_PAINT` is sent whenever the panel must be redrawn (shown, uncovered,
  resized). Inside a paint handler a `wxPaintDC` **must** be created.
- `wxTRANSPARENT_BRUSH` means the rectangles are not filled – only the outline
  is drawn, so the text stays visible.
- `GetClientSize()` is the inner area of the window (without title bar and
  frame). `Deflate(distance)` shrinks it by 5 px on every side.
- The border is drawn as two nested 1-pixel rectangles instead of one rectangle
  with a 2-pixel pen. A wide pen is centred on the geometric line, so its exact
  pixel position can differ between platforms; nested 1-pixel rectangles
  always cover exactly the pixels 5 and 6 from each edge.

To change the border, edit `distance` (gap to the edge) and `width`
(thickness); to change its colour, edit `*wxWHITE` in `SetPen()`.

**4. Layout**

```cpp
auto *layout = new wxBoxSizer(wxVERTICAL);
layout->AddStretchSpacer();
layout->Add(text, 0, wxALIGN_CENTER_HORIZONTAL);
layout->AddStretchSpacer();
panel->SetSizer(layout);
```

Two stretch spacers above and below centre the text vertically;
`wxALIGN_CENTER_HORIZONTAL` centres it horizontally.

---

## 4. Build system

The project uses **CMake** (≥ 3.28) with the **Ninja** generator and
**C++20**.

### 4.1 CMakeLists.txt

**Finding wxWidgets** – in this order:

1. **Windows (cross-build):** `find_package(wxWidgets 3.2 CONFIG ...)` looks
   for the prebuilt MinGW wxWidgets (see [4.4](#44-prebuilt-wxwidgets-for-windows)).
   It provides the targets `wx::core` and `wx::base`.
2. **Linux:** `find_package(wxWidgets 3.2 ...)` uses CMake's FindwxWidgets
   module, which finds the system wxWidgets via `wx-config`
   (package `libwxgtk3.2-dev`).
3. **Fallback:** if neither is found, `FetchContent` downloads wxWidgets
   v3.2.6 from GitHub and builds it as part of the project (static, without
   samples/tests/demos). This is slow and happens per build directory, so it
   is only a safety net.

The chosen libraries end up in `WX_LIBS`, which is linked to the executable.

**Executable**

```cmake
add_executable(gui_ex_03_a main.cpp SubWin.cpp SubWin.h)
target_link_libraries(gui_ex_03_a PRIVATE ${WX_LIBS})
```

New source files must be added to `add_executable`.

**Output directory** – the executable is written to `exe/<platform>`:

| Platform | Directory      |
|----------|----------------|
| Linux    | `exe/linux`    |
| Windows  | `exe/windows`  |
| macOS    | `exe/macos`    |

**Windows-specific settings**

- `WIN32_EXECUTABLE TRUE` – a GUI program without a console window.
- MinGW: `-static -static-libgcc -static-libstdc++` – the GCC runtime is
  linked statically, so the `.exe` runs on any Windows machine without extra
  DLLs. Together with the static wxWidgets the `.exe` is self-contained
  (about 14 MB).

### 4.2 CMakePresets.json

| Preset            | Platform | Build type | Build directory          |
|-------------------|----------|------------|--------------------------|
| `linux-debug`     | Linux    | Debug      | `build/linux-debug`      |
| `linux-release`   | Linux    | Release    | `build/linux-release`    |
| `windows-debug`   | Windows  | Debug      | `build/windows-debug`    |
| `windows-release` | Windows  | Release    | `build/windows-release`  |

All presets use Ninja. The Windows presets use the MinGW toolchain file.
Each configure preset has a build preset with the same name.

### 4.3 MinGW toolchain file

`cmake/mingw-w64-x86_64.cmake` tells CMake to cross-compile for 64-bit
Windows:

- Compilers: `x86_64-w64-mingw32-gcc-posix` / `-g++-posix` (the *posix*
  thread model supports `std::thread` and `std::mutex`), resource compiler
  `x86_64-w64-mingw32-windres`.
- Libraries, headers and CMake packages are searched **only** in the MinGW
  sysroot (`/usr/x86_64-w64-mingw32`) and in the prebuilt wxWidgets folder –
  never in the Linux system directories, so the Linux wxWidgets is never
  picked up by mistake.
- `WX_MINGW_ROOT` – location of the prebuilt wxWidgets. Default:
  `~/opt/wxWidgets-3.2.6-mingw64`. Can be overridden with
  `-DWX_MINGW_ROOT=/path` or the environment variable `WX_MINGW_ROOT`.

Required package: `sudo apt install mingw-w64`.

### 4.4 Prebuilt wxWidgets for Windows

To avoid downloading and compiling wxWidgets for every new project, it was
built **once** and installed to a shared location:

| Path                                   | Content                              |
|----------------------------------------|--------------------------------------|
| `~/opt/wxWidgets-3.2.6-mingw64/`       | Installed libraries, headers, CMake config (used by projects) |
| `~/opt/src/wxWidgets-3.2.6/`           | Source code (with all git submodules) |
| `~/opt/src/wx-build-mingw64/`          | Build directory                      |

It is a static Release build with the bundled third-party libraries
(zlib, libpng, libjpeg, libtiff, expat, PCRE2). It is also used by the
`windows-debug` preset; mixing a Release wxWidgets with a Debug application is
fine with MinGW.

**Rebuilding it** (e.g. after a compiler update or for another wx version):

```bash
cd ~/opt/src/wxWidgets-3.2.6
cmake -S . -B ~/opt/src/wx-build-mingw64 -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=/home/rolf/CProgs/gui_ex_03_a/cmake/mingw-w64-x86_64.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$HOME/opt/wxWidgets-3.2.6-mingw64 \
  -DwxBUILD_SHARED=OFF -DwxBUILD_SAMPLES=OFF -DwxBUILD_TESTS=OFF -DwxBUILD_DEMOS=OFF \
  -DwxUSE_ZLIB=builtin -DwxUSE_LIBPNG=builtin -DwxUSE_LIBJPEG=builtin \
  -DwxUSE_LIBTIFF=builtin -DwxUSE_EXPAT=builtin -DwxUSE_REGEX=builtin
cmake --build ~/opt/src/wx-build-mingw64 -j$(nproc)
cmake --install ~/opt/src/wx-build-mingw64
```

**New projects:** copy `CMakeLists.txt`, `CMakePresets.json` and the `cmake/`
folder from this project; they will then find the prebuilt wxWidgets
automatically.

---

## 5. Building and running

### Prerequisites (Ubuntu/WSL)

```bash
sudo apt install build-essential cmake ninja-build libwxgtk3.2-dev mingw-w64
```

### Linux build

```bash
cmake --preset linux-release
cmake --build --preset linux-release
./exe/linux/gui_ex_03_a
```

Running a GUI program in WSL requires WSLg (included in current Windows 10/11
WSL versions).

### Windows build

```bash
cmake --preset windows-release
cmake --build --preset windows-release
```

Start the result from Windows, e.g. from WSL:

```bash
./exe/windows/gui_ex_03_a.exe
```

or by double-clicking it in Explorer
(`\\wsl$\<distro>\home\rolf\CProgs\gui_ex_03_a\exe\windows`).

For debug builds, use the `*-debug` presets instead.

### Clean rebuild

```bash
rm -rf build/<preset>
cmake --preset <preset>
cmake --build --preset <preset>
```

---

## 6. Extending the application

**Add a menu item**

1. Add an ID to the enum in `MainFrame`:
   `enum { ID_TEST = wxID_HIGHEST + 1, ID_SUBWIN, ID_NEW_ITEM };`
2. Append it to a menu: `objectsMenu->Append(ID_NEW_ITEM, "&New item");`
3. Bind a handler: `Bind(wxEVT_MENU, &MainFrame::onNewItem, this, ID_NEW_ITEM);`
4. Implement `void onNewItem(wxCommandEvent &)`.

**Add another child window**

1. Create `MyWin.h` / `MyWin.cpp` following the `SubWin` pattern.
2. Add both files to `add_executable(...)` in `CMakeLists.txt`.
3. In `MainFrame`, add a `wxWeakRef<MyWin>` member and an `onMyWin()` handler
   like `onSubWin()`. Pass `this` as parent so the window closes with the
   application.

**Change the SubWin appearance**

| What              | Where (SubWin.cpp)                                 |
|-------------------|----------------------------------------------------|
| Background colour | `const wxColour background(64, 0, 0);`             |
| Text colour       | `const wxColour foreground(255, 220, 220);`        |
| Border gap/width  | `distance` / `width` in the paint handler          |
| Border colour     | `wxPen(*wxWHITE, 1)` in the paint handler          |
| Window size       | `wxSize(300, 150)` in the constructor              |
| Text              | `"Hello from SubWin"`                              |

---

## 7. Troubleshooting

| Symptom | Cause / solution |
|---------|------------------|
| Configure prints `wxWidgets not found - fetching it from GitHub` for the Windows build | The prebuilt wxWidgets is missing or `WX_MINGW_ROOT` points to the wrong place. Check that `~/opt/wxWidgets-3.2.6-mingw64/lib/cmake/wxWidgets/wxWidgetsConfig.cmake` exists, or rebuild it (see [4.4](#44-prebuilt-wxwidgets-for-windows)). |
| Configure prints the same for the Linux build | `libwxgtk3.2-dev` is not installed (`wx-config` missing). |
| Changes to the toolchain file have no effect | The toolchain file is only read when a build directory is first configured. Delete `build/<preset>` and configure again. |
| `x86_64-w64-mingw32-g++-posix: not found` | Install MinGW: `sudo apt install mingw-w64`. |
| Button colours not shown on Linux | Some GTK themes ignore custom button colours; this is a theme limitation. |
| Remnants of the SubWin border after resizing | The panel needs the style `wxFULL_REPAINT_ON_RESIZE` (already set). |
| The Windows `.exe` asks for `libstdc++-6.dll` or similar | The static link options were removed; keep the `MINGW` block in `CMakeLists.txt`. |
