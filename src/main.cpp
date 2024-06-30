#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/filedlg.h>
#include <chrono>
#include "my_header.h"
#include <fstream>
#include <fstream>
#include <sstream>


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
    void OnEdit(wxCommandEvent& event);
    int GetIntFromInput(const wxString& input);

private:
    wxGrid* grid;
    wxPanel* panel;
    wxStaticText* timeLabel;
    wxStaticText* timerLabel;
    wxStaticText* speedLabel;
    wxStaticText* durationLabel;
    wxStaticText* durationValueLabel;
    wxStaticText* speedValue;
    wxTimer* timer;

    int bulletX;
    int bulletStartX;
    int bulletEndX;
    int elapsedMilliseconds;
    int rowIndex;
    int animationDuration;
    float speed;

    void OnPaint(wxPaintEvent& event);
};


enum
{
    ID_Run = 1,
    ID_Timer = 2,
    ID_Open = 3,
    ID_Save = 4,
    ID_Edit = 5
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
    : wxFrame(nullptr, wxID_ANY, title, pos, size), elapsedMilliseconds(0), bulletX(10), animationDuration(10), speed(10.0f)  // Default duration and speed
{
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Run, "&Run...\tCtrl-R", "Run the example function and measure time");
    menuFile->Append(ID_Save, "&Save...\tCtrl-S", "Save the grid data to a file");
    menuFile->Append(ID_Open, "&Open...\tCtrl-O", "Open a file and load the grid data");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    
    wxMenu* menuEdit = new wxMenu;
    menuEdit->Append(ID_Edit, "&Edit...\tCtrl-E", "Edit experiment variables");

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuEdit, "&Edit");
    menuBar->Append(menuHelp, "&Help");

    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");

    Bind(wxEVT_MENU, &MyFrame::OnRun, this, ID_Run);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnSave, this, ID_Save);
    Bind(wxEVT_MENU, &MyFrame::OnOpen, this, ID_Open);
    Bind(wxEVT_MENU, &MyFrame::OnEdit, this, ID_Edit);

    // Create the panel for custom drawing
    panel = new wxPanel(this, wxID_ANY);
    
    // Set sizers to layout the panel properly
    wxBoxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);

    // Add space for the bullet animation at the top
    panelSizer->AddSpacer(50);  // Increase the space for the bullet animation

    // Create the grid for displaying float values
    grid = new wxGrid(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    grid->CreateGrid(0, 10);  // Create a grid with 1 row and 10 columns
    grid->HideRowLabels();    // Hide row labels
    // Set column labels
    for (int i = 0; i < 10; ++i) {
        grid->SetColLabelValue(i, wxString::Format("x%d", i + 1));
    }

    panel->Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);

    // Create labels and text control
    timeLabel = new wxStaticText(panel, wxID_ANY, "Time:", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    timerLabel = new wxStaticText(panel, wxID_ANY, "0 s", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    durationLabel = new wxStaticText(panel, wxID_ANY, "duration", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    durationValueLabel = new wxStaticText(panel, wxID_ANY, "10 s", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    speedLabel = new wxStaticText(panel, wxID_ANY, "Speed:", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    speedValue = new wxStaticText(panel, wxID_ANY, "10 m/s", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);

    // Create a horizontal box sizer for time labels
    wxBoxSizer* timeSizer = new wxBoxSizer(wxHORIZONTAL);
    timeSizer->Add(timeLabel, 0, wxALL, 10);
    timeSizer->Add(timerLabel, 0, wxALL, 10);
    timeSizer->Add(durationLabel, 0, wxALL, 10);
    timeSizer->Add(durationValueLabel, 0, wxALL, 10);
    panelSizer->Add(timeSizer, 0, wxALIGN_LEFT);

    wxBoxSizer* speedSizer = new wxBoxSizer(wxHORIZONTAL);
    speedSizer->Add(speedLabel, 0, wxALL, 10);
    speedSizer->Add(speedValue, 0, wxALL, 10);
    panelSizer->Add(speedSizer, 0, wxALL, wxALIGN_LEFT);

    // Add the grid below the other panels and make it expand
    panelSizer->Add(grid, 1, wxALL | wxEXPAND, 10);

    panel->SetSizer(panelSizer);

    // Adjust frame size to fit the panel and grid
    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(panel, 1, wxALL | wxEXPAND, 10);
    this->SetSizerAndFit(frameSizer);

    // Initialize timer
    timer = new wxTimer(this, ID_Timer);
    Bind(wxEVT_TIMER, &MyFrame::OnTimer, this, ID_Timer);
    
    // Initial drawing of the line
    bulletStartX = 10;
    bulletEndX = GetSize().GetWidth() - 30;
    panel->Refresh();
    rowIndex = 0;
}

void MyFrame::OnRun(wxCommandEvent& event)
{
    auto start = std::chrono::high_resolution_clock::now();

    printMessage();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    // Print to the terminal instead of showing a message box
    std::cout << "Execution time: " << duration.count() << " seconds" << std::endl;

    // Add a new row to the grid
    int newRow = grid->GetNumberRows();
    grid->AppendRows(1);

    // Adjust the panel and window size to fit the new row
    panel->Layout();
    this->Fit();

    elapsedMilliseconds = 0;
    bulletX = 10; // Reset bullet position
    timerLabel->SetLabel("0 s"); // Reset timer label

    wxRect gridRect = grid->GetRect();
    bulletStartX = gridRect.GetLeft();
    bulletEndX = gridRect.GetRight();
    
    animationDuration = GetIntFromInput(durationValueLabel->GetLabel());
    speed = GetIntFromInput(speedValue->GetLabel());

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

    grid->ClearGrid();

    int row = 0;
    while (std::getline(file, line)) {
        if (grid->GetNumberRows() <= row) {
            grid->AppendRows(1);
        }
        std::istringstream ss(line);
        std::string cell;
        int col = 0;
        while (std::getline(ss, cell, ',') && col < 10) {
            grid->SetCellValue(row, col, cell);
            ++col;
        }
        ++row;
    }

    file.close();
    
    rowIndex = row;
    
    // Adjust the panel and window size to fit the new row
    panel->Layout();
    this->Fit();
}

void MyFrame::OnSave(wxCommandEvent& event)
{
    wxFileDialog saveFileDialog(this, _("Save CSV file"), "", "",
                                "CSV files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;

    std::ofstream file;
    file.open(saveFileDialog.GetPath().ToStdString());

    // Write column headers
    for (int i = 0; i < 10; ++i) {
        file << grid->GetColLabelValue(i).ToStdString();
        if (i < 9) file << ",";
    }
    file << "\n";

    // Write all rows of cell values
    int numRows = grid->GetNumberRows();
    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < 10; ++col) {
            file << grid->GetCellValue(row, col).ToStdString();
            if (col < 9) file << ",";
        }
        file << "\n";
    }

    file.close();
}

void MyFrame::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(panel);

    // Draw a bullet at the current position
    int bulletRadius = 5;
    int bulletY = 20; // Position the bullet at the top

    dc.SetBrush(*wxBLACK_BRUSH);
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawCircle(bulletX, bulletY, bulletRadius);
    
    // Draw x-coordinate line under the bullet
    int lineY = bulletY + bulletRadius + 1;
    dc.DrawLine(bulletStartX, lineY, bulletEndX, lineY);
    dc.DrawLine(bulletStartX, lineY - 2, bulletStartX, lineY + 2);
    dc.DrawLine(bulletEndX, lineY - 2, bulletEndX, lineY + 2);
    dc.DrawText("0 m", bulletStartX, lineY + 5);
    dc.DrawText("100 m", bulletEndX - 20, lineY + 5);
}


void MyFrame::OnTimer(wxTimerEvent& event)
{
    elapsedMilliseconds += 33;
    int elapsedSeconds = (elapsedMilliseconds / 1000);
	
    // Update grid values
    if (elapsedSeconds > 0 && elapsedSeconds % (animationDuration / 10) == 0 && grid->GetCellValue(rowIndex, elapsedSeconds / (animationDuration / 10) - 1).IsEmpty())
    {
        float min = speed * elapsedSeconds * 0.97f;
        float max = speed * elapsedSeconds * 1.03f;
        float randomF = min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
        std::cout << grid->GetNumberRows() << std::endl;
        grid->SetCellValue(rowIndex, elapsedSeconds / (animationDuration / 10) - 1, wxString::Format("%.2f", randomF));
    }
    
    // Update timer label
    timerLabel->SetLabel(wxString::Format("%d s", elapsedSeconds));

    if (elapsedSeconds >= animationDuration)
    {
        rowIndex++;
        timer->Stop();
        return;
    }

    // Calculate new bullet position
    float progress = static_cast<float>(elapsedMilliseconds) / (animationDuration * 1000.0f); // duration in milliseconds
    bulletX = bulletStartX + progress * (bulletEndX - bulletStartX);

    // Refresh the panel to trigger a paint event
    panel->Refresh();
}


int MyFrame::GetIntFromInput(const wxString& input)
{
    wxString digits;
    for (auto c : input)
    {
        if (wxIsdigit(c))
        {
            digits += c;
        }
    }
    long result;
    digits.ToLong(&result);
    return static_cast<int>(result);
}

void MyFrame::OnEdit(wxCommandEvent& event)
{
    wxDialog dialog(this, wxID_ANY, "Edit Experiment Variables", wxDefaultPosition, wxDefaultSize);
    wxBoxSizer* dialogSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* durationLabel = new wxStaticText(&dialog, wxID_ANY, "Duration (s):");
    wxTextCtrl* durationInput = new wxTextCtrl(&dialog, wxID_ANY, durationValueLabel->GetLabel(), wxDefaultPosition, wxSize(100, -1));
    dialogSizer->Add(durationLabel, 0, wxALL, 5);
    dialogSizer->Add(durationInput, 0, wxALL, 5);

    wxStaticText* speedLabel = new wxStaticText(&dialog, wxID_ANY, "Speed (m/s):");
    wxTextCtrl* speedInput = new wxTextCtrl(&dialog, wxID_ANY, speedValue->GetLabel(), wxDefaultPosition, wxSize(100, -1));
    dialogSizer->Add(speedLabel, 0, wxALL, 5);
    dialogSizer->Add(speedInput, 0, wxALL, 5);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* okButton = new wxButton(&dialog, wxID_OK, "OK");
    wxButton* cancelButton = new wxButton(&dialog, wxID_CANCEL, "Cancel");
    buttonSizer->Add(okButton, 0, wxALL, 5);
    buttonSizer->Add(cancelButton, 0, wxALL, 5);

    dialogSizer->Add(buttonSizer, 0, wxALIGN_CENTER);

    dialog.SetSizerAndFit(dialogSizer);

    if (dialog.ShowModal() == wxID_OK)
    {
        int duration = GetIntFromInput(durationInput->GetValue());
        int speed = GetIntFromInput(speedInput->GetValue());

        durationValueLabel->SetLabel(wxString::Format("%d s", duration));
        speedValue->SetLabel(wxString::Format("%d m/s", speed));
    }
}

