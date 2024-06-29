#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/filedlg.h>
#include <chrono>
#include "my_header.h"
#include <fstream>

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
    void OnSave(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);

private:
    wxGrid* grid;
    wxPanel* panel;
    wxStaticText* timeLabel;
    wxStaticText* timerLabel;
    wxStaticText* speedLabel;
    wxTextCtrl* speedText;
    wxTimer* timer;

    int bulletX;
    int bulletStartX;
    int bulletEndX;
    int elapsedMilliseconds;
    int cellValue;

    void OnPaint(wxPaintEvent& event);
};

enum
{
    ID_Run = 1,
    ID_Timer = 2,
    ID_Save = 3,
    ID_Open = 4
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
    : wxFrame(nullptr, wxID_ANY, title, pos, size), elapsedMilliseconds(0), bulletX(10)
{
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Run, "&Run...\tCtrl-R", "Run the example function and measure time");
    menuFile->Append(ID_Save, "&Save...\tCtrl-S", "Save the grid data to a file");
    menuFile->Append(ID_Open, "&Open...\tCtrl-O", "Open a file and load the grid data");
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
    Bind(wxEVT_MENU, &MyFrame::OnSave, this, ID_Save);
    Bind(wxEVT_MENU, &MyFrame::OnOpen, this, ID_Open);

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
    
    cellValue = 0;
    
    // Update the grid with the float values
    for (int i = 0; i < 10; ++i) {
        grid->SetCellValue(0, i, wxString::Format("%.2f", values[i]));
    }

    panel->Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);

    // Create labels and text control
    timeLabel = new wxStaticText(panel, wxID_ANY, "Time:", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    timerLabel = new wxStaticText(panel, wxID_ANY, "0 s", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    speedLabel = new wxStaticText(panel, wxID_ANY, "Speed:", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    speedText = new wxTextCtrl(panel, wxID_ANY, "10 m/s", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

    // Set sizers to layout the grid and the panel properly
    wxBoxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
    panelSizer->Add(grid, 0, wxALL | wxEXPAND, 10);
    panelSizer->AddSpacer(30); // Add space for the bullet below the grid

    // Create a horizontal box sizer for time labels
    wxBoxSizer* timeSizer = new wxBoxSizer(wxHORIZONTAL);
    timeSizer->Add(timeLabel, 0, wxALL, 10);
    timeSizer->Add(timerLabel, 0, wxALL, 10);
    panelSizer->Add(timeSizer, 0, wxALIGN_LEFT);

    panelSizer->Add(speedLabel, 0, wxALL, 10); // Add speed label
    panelSizer->Add(speedText, 0, wxALL, 10);

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

    // Print to the terminal instead of showing a message box
    std::cout << "Execution time: " << duration.count() << " seconds" << std::endl;

    for(int i = 0; i < 10; i++){
        grid->SetCellValue(0, i, "");
    }

    elapsedMilliseconds = 0;
    bulletX = 10; // Reset bullet position
    timerLabel->SetLabel("0 s"); // Reset timer label

    wxRect gridRect = grid->GetRect();
    bulletStartX = gridRect.GetLeft() + 10;
    bulletEndX = gridRect.GetRight() - 10;

    timer->Start(33);  // Start the timer to update every 33 milliseconds (approx. 30 fps)
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

void MyFrame::OnSave(wxCommandEvent& event)
{
    wxFileDialog saveFileDialog(this, _("Save CSV file"), "", "",
                                "CSV files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;

    std::ofstream file;
    file.open(saveFileDialog.GetPath().ToStdString());

    for (int i = 0; i < 10; ++i) {
        file << grid->GetColLabelValue(i).ToStdString();
        if (i < 9) file << ",";
    }
    file << "\n";

    for (int i = 0; i < 10; ++i) {
        file << grid->GetCellValue(0, i).ToStdString();
        if (i < 9) file << ",";
    }
    file << "\n";

    file.close();
}

void MyFrame::OnOpen(wxCommandEvent& event)
{
    wxFileDialog openFileDialog(this, _("Open CSV file"), "", "",
                                "CSV files (*.csv)|*.csv", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    std::ifstream file;
    file.open(openFileDialog.GetPath().ToStdString());

    std::string line;
    if (std::getline(file, line)) {
        // Skip the header line
    }
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
    
    // Draw x-coordinate line under the bullet
    int lineY = gridBottom + 20 + bulletRadius + 1;
    dc.DrawLine(bulletStartX, lineY, bulletEndX, lineY);
    dc.DrawLine(bulletStartX, lineY - 2, bulletStartX, lineY + 2);
    dc.DrawLine(bulletEndX, lineY - 2, bulletEndX, lineY + 2);
    dc.DrawText("0 m", bulletStartX, lineY + 5);
    dc.DrawText("100 m", bulletEndX - 20, lineY + 5);
}

void MyFrame::OnTimer(wxTimerEvent& event)
{
    elapsedMilliseconds += 33;
    int elapsedSeconds = elapsedMilliseconds / 1000;

    if (elapsedSeconds >= 10)
    {
        timer->Stop();
        return;
    }

    // Update grid values
    if (elapsedSeconds > 0 && grid->GetCellValue(0, elapsedSeconds - 1).IsEmpty())
    {
        float min = 10 * elapsedSeconds * 0.97f;
        float max = 10 * elapsedSeconds * 1.03f;
        float randomF = min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
        grid->SetCellValue(0, elapsedSeconds - 1, wxString::Format("%.2f", randomF));
        cellValue = elapsedSeconds;
    }

    // Update timer label
    timerLabel->SetLabel(wxString::Format("%d s", elapsedSeconds));

    // Calculate new bullet position
    float progress = elapsedMilliseconds / 10000.0f; // 10000 milliseconds for 10 seconds
    bulletX = bulletStartX + progress * (bulletEndX - bulletStartX);

    // Refresh the panel to trigger a paint event
    panel->Refresh();
}

       

