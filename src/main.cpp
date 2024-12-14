#include <wx/wx.h>
#include "Database.h"
#include "xlsxwriter.h"

#include <regex>
#include <string>
#include <iostream>

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);

private:
    void OnAddRecord(wxCommandEvent& event);
    void OnFindRecords(wxCommandEvent& event);
    void OnDeleteRecords(wxCommandEvent& event);
    void OnEditRecord(wxCommandEvent& event);
    void OnBackup(wxCommandEvent& event);
    void OnRestore(wxCommandEvent& event);
    void OnExportToXlsx(wxCommandEvent& event);

    Database db;
    wxListBox* recordList;
};

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame("Database GUI");
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)),
      db("database.txt") {

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxButton* addButton = new wxButton(this, wxID_ANY, "Add Record");
    wxButton* findButton = new wxButton(this, wxID_ANY, "Find Records");
    wxButton* deleteButton = new wxButton(this, wxID_ANY, "Delete Records");
    wxButton* editButton = new wxButton(this, wxID_ANY, "Edit Record");
    wxButton* backupButton = new wxButton(this, wxID_ANY, "Create Backup");
    wxButton* restoreButton = new wxButton(this, wxID_ANY, "Restore from Backup");
    wxButton* exportButton = new wxButton(this, wxID_ANY, "Export to XLSX");

    recordList = new wxListBox(this, wxID_ANY);

    sizer->Add(addButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(findButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(deleteButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(editButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(backupButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(restoreButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(exportButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(recordList, 1, wxEXPAND | wxALL, 5);

    SetSizer(sizer);

    addButton->Bind(wxEVT_BUTTON, &MyFrame::OnAddRecord, this);
    findButton->Bind(wxEVT_BUTTON, &MyFrame::OnFindRecords, this);
    deleteButton->Bind(wxEVT_BUTTON, &MyFrame::OnDeleteRecords, this);
    editButton->Bind(wxEVT_BUTTON, &MyFrame::OnEditRecord, this);
    backupButton->Bind(wxEVT_BUTTON, &MyFrame::OnBackup, this);
    restoreButton->Bind(wxEVT_BUTTON, &MyFrame::OnRestore, this);
    exportButton->Bind(wxEVT_BUTTON, &MyFrame::OnExportToXlsx, this);
}

void MyFrame::OnAddRecord(wxCommandEvent& event) {
    wxTextEntryDialog dialog(this, "Enter record as 'id,name,age,address':", "Add Record");
    if (dialog.ShowModal() == wxID_OK) {
        std::string input = dialog.GetValue().ToStdString();
        input.erase(std::remove(input.begin(), input.end(), ' '), input.end()); 

        std::regex recordPattern(R"((\d+),([^,]+),(\d+),([^,]+))"); 
        std::smatch match;

        if (std::regex_match(input, match, recordPattern)) {
            Record record;
            record.id = std::stoi(match[1].str());
            record.name = match[2].str();
            record.age = std::stoi(match[3].str());
            record.address = match[4].str();

            if (db.addRecord(record)) {
                wxMessageBox("Record added successfully!", "Success", wxOK | wxICON_INFORMATION);
            } else {
                wxMessageBox("Duplicate ID found!", "Error", wxOK | wxICON_ERROR);
            }
        } else {
            wxMessageBox("Invalid input format!", "Error", wxOK | wxICON_ERROR);
        }
    }
}

void MyFrame::OnFindRecords(wxCommandEvent& event) {
    wxTextEntryDialog dialog(this, "Enter field=value to search (e.g., name=John):", "Find Records");
    if (dialog.ShowModal() == wxID_OK) {
        std::string input = dialog.GetValue().ToStdString();
        size_t pos = input.find('=');
        if (pos != std::string::npos) {
            std::string field = input.substr(0, pos);
            std::string value = input.substr(pos + 1);
            auto results = db.findRecordsByField(field, value);
            recordList->Clear();  
            for (const auto& record : results) {
                recordList->Append(wxString::Format("%d, %s, %d, %s", record.id, record.name, record.age, record.address));
            }
            if (results.empty()) {
                wxMessageBox("No records found!", "Info", wxOK | wxICON_INFORMATION);
            }
        } else {
            wxMessageBox("Invalid input format!", "Error", wxOK | wxICON_ERROR);
        }
    }
}

void MyFrame::OnDeleteRecords(wxCommandEvent& event) {
    wxTextEntryDialog dialog(this, "Enter field=value to delete (e.g., id=123):", "Delete Records");
    if (dialog.ShowModal() == wxID_OK) {
        std::string input = dialog.GetValue().ToStdString();
        size_t pos = input.find('=');
        if (pos != std::string::npos) {
            std::string field = input.substr(0, pos);
            std::string value = input.substr(pos + 1);
            if (db.deleteRecordByField(field, value)) {
                wxMessageBox("Records deleted successfully!", "Success", wxOK | wxICON_INFORMATION);
            } else {
                wxMessageBox("No matching records found!", "Error", wxOK | wxICON_ERROR);
            }
        } else {
            wxMessageBox("Invalid input format!", "Error", wxOK | wxICON_ERROR);
        }
    }
}

void MyFrame::OnEditRecord(wxCommandEvent& event) {
    wxTextEntryDialog idDialog(this, "Enter the ID of the record to edit:", "Edit Record by ID");
    if (idDialog.ShowModal() == wxID_OK) {
        int id = std::stoi(idDialog.GetValue().ToStdString());
        
        wxString choices[] = {"name", "age", "address"};
        wxSingleChoiceDialog editDialog(this, "Select the field to edit:", "Edit Record", 3, choices);
        
        if (editDialog.ShowModal() == wxID_OK) {
            wxString fieldToEdit = editDialog.GetStringSelection();
            
            wxTextEntryDialog newValueDialog(this, wxString::Format("Enter new value for %s:", fieldToEdit), "Edit Record");
            if (newValueDialog.ShowModal() == wxID_OK) {
                wxString newValue = newValueDialog.GetValue();
                
                if (fieldToEdit == "id") {
                    wxMessageBox("Editing the ID is not allowed.", "Error", wxOK | wxICON_ERROR);
                } else {
                    if (db.editRecord(std::to_string(id), fieldToEdit.ToStdString(), newValue.ToStdString())) {
                        wxMessageBox("Record updated successfully!", "Success", wxOK | wxICON_INFORMATION);
                        OnFindRecords(event); 
                    } else {
                        wxMessageBox("Failed to update record.", "Error", wxOK | wxICON_ERROR);
                    }
                }
            }
        }
    }
}

void MyFrame::OnBackup(wxCommandEvent& event) {
    wxFileDialog saveFileDialog(this, "Save Backup", "", "backup.txt", "Text files (*.txt)|*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_OK) {
        if (db.createBackup(saveFileDialog.GetPath().ToStdString())) {
            wxMessageBox("Backup created successfully!", "Success", wxOK | wxICON_INFORMATION);
        } else {
            wxMessageBox("Failed to create backup!", "Error", wxOK | wxICON_ERROR);
        }
    }
}

void MyFrame::OnRestore(wxCommandEvent& event) {
    wxFileDialog openFileDialog(this, "Restore Backup", "", "", "Text files (*.txt)|*.txt", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_OK) {
        if (db.restoreFromBackup(openFileDialog.GetPath().ToStdString())) {
            wxMessageBox("Backup restored successfully!", "Success", wxOK | wxICON_INFORMATION);
        } else {
            wxMessageBox("Failed to restore backup!", "Error", wxOK | wxICON_ERROR);
        }
    }
}

void MyFrame::OnExportToXlsx(wxCommandEvent& event) {
    wxFileDialog saveFileDialog(this, "Export to XLSX", "", "database.xlsx", "Excel files (*.xlsx)|*.xlsx", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_OK) {
        if (db.exportToXlsx(saveFileDialog.GetPath().ToStdString())) {
            wxMessageBox("Data exported successfully to XLSX!", "Success", wxOK | wxICON_INFORMATION);
        } else {
            wxMessageBox("Failed to export data to XLSX.", "Error", wxOK | wxICON_ERROR);
        }
    }
}
