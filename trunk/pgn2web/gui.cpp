/*
  p2wgui - GUI frontend to pgn2web

  Copyright (C) 2004, 2005 William Hoggarth <email: me@whoggarth.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "pgn2web.h"

//include headers for wxWidgets
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

//include icon resource
#include "pgn2web.xpm"

//define constant for system dependent file seperator
#ifdef WINDOWS
const char SEPERATOR = '\\';
#else
const char SEPERATOR = '/';
#endif

//panel containing all the controls
class p2wPanel : public wxPanel {
public:
  p2wPanel(wxWindow *pcParent);

  void BrowsePGN(wxCommandEvent cEvent);
  void BrowseHTML(wxCommandEvent cEvent);
  void Convert(wxCommandEvent cEvent);
  void Quit(wxCommandEvent cEvent);

protected:
  wxStaticText *m_pcPGNLabel;
  wxTextCtrl *m_pcPGNTextCtrl;
  wxButton *m_pcPGNButton;

  wxStaticText *m_pcHTMLLabel;
  wxTextCtrl *m_pcHTMLTextCtrl;
  wxButton *m_pcHTMLButton;

  wxButton *m_pcConvert;
  wxButton *m_pcQuit;

  enum {ID_BROWSEPGN, ID_BROWSEHTML, ID_CONVERT};

  DECLARE_EVENT_TABLE()
};

//simple app class
class p2wApp : public wxApp {
public:
  virtual bool OnInit();
};

DECLARE_APP(p2wApp)

BEGIN_EVENT_TABLE(p2wPanel, wxPanel)
  EVT_BUTTON(ID_BROWSEPGN, p2wPanel::BrowsePGN)
  EVT_BUTTON(ID_BROWSEHTML, p2wPanel::BrowseHTML)
  EVT_BUTTON(ID_CONVERT, p2wPanel::Convert)
  EVT_BUTTON(wxID_EXIT, p2wPanel::Quit)
END_EVENT_TABLE()

IMPLEMENT_APP(p2wApp)

p2wPanel::p2wPanel(wxWindow *pcParent) : wxPanel(pcParent) {
  //add and layout gui components
  wxBoxSizer *pcRootSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *pcFileSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *pcPGNSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *pcPGNLabelSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *pcHTMLSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer *pcHTMLLabelSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *pcButtonSizer = new wxBoxSizer(wxHORIZONTAL);

  m_pcPGNLabel = new wxStaticText(this, -1, "PGN File");
  m_pcPGNTextCtrl = new wxTextCtrl(this, -1);
  m_pcPGNButton = new wxButton(this, ID_BROWSEPGN, "Browse...");

  pcPGNSizer->Add(m_pcPGNTextCtrl, 1, wxALIGN_CENTER | wxALL, 5);
  pcPGNSizer->Add(m_pcPGNButton, 0, wxALIGN_CENTER | wxALL, 5);
  pcPGNLabelSizer->Add(m_pcPGNLabel, 0, wxALIGN_CENTER | wxALL, 5);
  pcPGNLabelSizer->Add(pcPGNSizer, 0, wxEXPAND);

  m_pcHTMLLabel = new wxStaticText(this, -1, "HTML File(s)");
  m_pcHTMLTextCtrl = new wxTextCtrl(this, -1);
  m_pcHTMLButton = new wxButton(this, ID_BROWSEHTML, "Browse...");

  pcHTMLSizer->Add(m_pcHTMLTextCtrl, 1, wxALIGN_CENTER | wxALL, 5);
  pcHTMLSizer->Add(m_pcHTMLButton, 0, wxALIGN_CENTER | wxALL, 5);
  pcHTMLLabelSizer->Add(m_pcHTMLLabel, 0, wxALIGN_CENTER | wxALL, 5);
  pcHTMLLabelSizer->Add(pcHTMLSizer, 0, wxEXPAND);

  pcFileSizer->Add(pcPGNLabelSizer, 1, wxALIGN_CENTER | wxEXPAND);
  pcFileSizer->Add(pcHTMLLabelSizer, 1, wxALIGN_CENTER | wxEXPAND);

  m_pcConvert = new wxButton(this, ID_CONVERT, "Convert");
  m_pcQuit = new wxButton(this, wxID_EXIT, "Quit");

  pcButtonSizer->Add(m_pcConvert, 0, wxALIGN_CENTER | wxALL, 5);
  pcButtonSizer->Add(m_pcQuit, 0, wxALIGN_CENTER | wxALL, 5);

  pcRootSizer->Add(pcFileSizer, 1, wxALIGN_CENTER | wxEXPAND);
  pcRootSizer->Add(pcButtonSizer, 1, wxALIGN_CENTER);

  SetSizer(pcRootSizer); 
  SetAutoLayout(true);
  Layout();
  Fit();
}

//do the conversion by using pgn2web
void p2wPanel::Convert(wxCommandEvent cEvent) {
  wxString cCommand;
  wxString cPath = m_pcHTMLTextCtrl->GetValue();

  //ensure there are filenames entered
  if(m_pcPGNTextCtrl->GetValue() == "") {
    wxMessageDialog* cDialog = new wxMessageDialog(this, "Please choose a PGN file to convert", "pgn2web", wxOK | wxICON_INFORMATION);
    cDialog->ShowModal();
    cDialog->Destroy();
    return;
  }

  if(cPath == "") {
    wxMessageDialog* cDialog = new wxMessageDialog(this, "Please enter a name for the HTML file(s)", "pgn2web", wxOK | wxICON_INFORMATION);
    cDialog->ShowModal();
    cDialog->Destroy();
    return;
  }

  //if the html filename has no path use the PGN file's path
  if(cPath.Find(SEPERATOR, true) == -1) {
    cPath = m_pcPGNTextCtrl->GetValue();

    if(cPath.Find(SEPERATOR, true) != -1) {
      cPath = cPath.Left(cPath.Find(SEPERATOR, true) + 1);
      m_pcHTMLTextCtrl->SetValue(cPath + m_pcHTMLTextCtrl->GetValue());
    }
  }

  pgn2web(m_pcPGNTextCtrl->GetValue(), m_pcHTMLTextCtrl->GetValue());
}

//bring up filebrowser to select a file
void p2wPanel::BrowsePGN(wxCommandEvent cEvent) {
  wxFileDialog *cDialog = new wxFileDialog(this, "Select a PGN file...", "", "", "PGN files(*.pgn)|*.pgn|", wxOPEN);
  if(wxID_OK == cDialog->ShowModal()) {
    m_pcPGNTextCtrl->SetValue(cDialog->GetPath());
  }
  cDialog->Destroy();
}

//bring up filebrowser to select a file
void p2wPanel::BrowseHTML(wxCommandEvent cEvent) {
  wxFileDialog *cDialog = new wxFileDialog(this, "Name HTML file(s)...", "", "", "HTML files(*.html)|*.html|", wxSAVE);
  if(wxID_OK == cDialog->ShowModal()) {
    m_pcHTMLTextCtrl->SetValue(cDialog->GetPath());
  }
  cDialog->Destroy();
}

//quit
void p2wPanel::Quit(wxCommandEvent cEvent) {
  GetParent()->Close();
}

//setup main window, add panel and size it
bool p2wApp::OnInit() {
  wxFrame *pcFrame = new wxFrame(NULL, -1, "pgn2web", wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~wxRESIZE_BORDER & ~wxMAXIMIZE_BOX);
  wxPanel *pcPanel = new p2wPanel(pcFrame);
  pcFrame->SetIcon(wxIcon(pgn2web_xpm));
  pcFrame->Fit();
  pcFrame->Show();
  SetTopWindow(pcFrame);
  return true;
}
