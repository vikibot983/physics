#include <wx/wx.h>
#include <wx/grid.h>
#include <chrono>
#include "my_header.h"
#include <iostream>

// Global pointer to the main frame
wxFrame* g_mainFrame = nullptr;

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

    void OnRun(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);

private:
    wxGrid* grid;
    wxPanel* panel;
    wxStaticText* timerLabel;
    wxTimer* timer;

    int bulletX;
    int bulletStartX;
    int bulletEndX;
    int elapsedSeconds;

    void OnPaint(wxPaintEvent& event);
};

enum
{
    ID_Run = 1,
    ID_Timer = 2
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    MyFrame* frame = new MyFrame("Timing Example with Grid", wxPoint(50, 50), wxSize(600, 400));
    frame->Show(true);
    g_mainFrame = frame; // Initialize the global pointer
    return true;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxFrame(nullptr, wxID_ANY, title, pos, size), elapsedSeconds(0), bulletX(10)
{
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Run, "&Run...\tCtrl-R", "Run the example function and measure time");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");

    Bind(wxEVT_MENU, &MyFrame::OnRun, this, ID_Run);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);

    // Create the panel for custom drawing
    panel = new wxPanel(this, wxID_ANY);

    // Create the grid for displaying float values
    grid = new wxGrid(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    grid->CreateGrid(1, 10);  // Create a grid with 1 row and 10 columns
    grid->HideRowLabels();    // Hide row labels
    // Set column labels
    for (int i = 0; i < 10; ++i) {
        grid->SetColLabelValue(i, wxString::Format("x%d", i + 1));
    }

    float values[] = {101.53, 100.87, 98.97, 99.95, 101.50, 102.01, 97.37, 100.01, 99.56, 101.33};

    // Update the grid with the float values
    for (int i = 0; i < 10; ++i) {
        grid->SetCellValue(0, i, wxString::Format("%.2f", values[i]));
    }

    panel->Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);

    // Create the timer label
    timerLabel = new wxStaticText(panel, wxID_ANY, "0 s", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

    // Set sizers to layout the grid and the panel properly
    wxBoxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
    panelSizer->Add(grid, 0, wxALL | wxEXPAND, 10);
    panelSizer->AddSpacer(30); // Add space for the bullet below the grid
    panelSizer->Add(timerLabel, 0, wxALL, 10); // Add timer label
    panel->SetSizerAndFit(panelSizer);

    // Adjust frame size to fit the panel and grid
    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(panel, 1, wxALL | wxEXPAND, 10);
    this->SetSizerAndFit(frameSizer);

    // Initialize timer
    timer = new wxTimer(this, ID_Timer);
    Bind(wxEVT_TIMER, &MyFrame::OnTimer, this, ID_Timer);
}

void MyFrame::OnRun(wxCommandEvent& event)
{
    auto start = std::chrono::high_resolution_clock::now();

    printMessage();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "Execution time: " << duration.count() << "seconds" << std::endl;

    elapsedSeconds = 0;
    bulletX = 10; // Reset bullet position
    timerLabel->SetLabel("0 s"); // Reset timer label

    wxRect gridRect = grid->GetRect();
    bulletStartX = gridRect.GetLeft() + 10;
    bulletEndX = gridRect.GetRight() - 10;

    timer->Start(1000);  // Start the timer to update every second
    panel->Refresh();    // Refresh the panel to trigger a paint event
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets example with timing and grid", "About Timing Example", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnTimer(wxTimerEvent& event)
{
    elapsedSeconds++;
    if (elapsedSeconds > 10)
    {
        timer->Stop();
        return;
    }

    timerLabel->SetLabel(wxString::Format("%d s", elapsedSeconds));

    // Calculate new bullet position
    int totalSteps = 10;
    int stepSize = (bulletEndX - bulletStartX) / totalSteps;
    bulletX = bulletStartX + stepSize * elapsedSeconds;

    panel->Refresh();  // Refresh the panel to trigger a paint event
}

void MyFrame::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(panel);

    // Get the size and position of the grid
    wxRect gridRect = grid->GetRect();
    int gridBottom = gridRect.GetBottom();

    // Draw a bullet at the current position
    int bulletRadius = 5;
    int bulletY = gridBottom + 20; // Adjust this value as needed

    dc.SetBrush(*wxBLACK_BRUSH);
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawCircle(bulletX, bulletY, bulletRadius);
}

