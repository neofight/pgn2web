#include "gui.h"

#include "pgn2web.h"

/*** PiecesView ***/

const wchar_t* PiecesView::pieceSets[16] = { wxT("adventurer"), wxT("alfonso-x"), wxT("cases"),
					     wxT("condal"), wxT("harlequin"), wxT("kingdom"),
					     wxT("leipzig"), wxT("line"), wxT("lucena"),
					     wxT("magnetic"), wxT("mark"), wxT("marroquin"),
					     wxT("maya"), wxT("mediaeval"), wxT("merida"), 
					     wxT("motif") };

PiecesView::PiecesView(wxWindow* parent) : wxWindow(parent, -1, wxDefaultPosition,
						    wxSize(196,36))
{
  SetBackgroundColour(wxColour(wxT("black")));

  //Load pieces bitmaps
  pieceSet = 14;

  for(int set = 0; set < 16; set++) {

    pieceBitmaps[set][0] = new wxBitmap(wxString(wxT("images/")) + pieceSets[set] + 
					wxT("/wpws.png"), wxBITMAP_TYPE_PNG);

    pieceBitmaps[set][1] = new wxBitmap(wxString(wxT("images/")) + pieceSets[set] +
					wxT("/wnbs.png"), wxBITMAP_TYPE_PNG);

    pieceBitmaps[set][2] = new wxBitmap(wxString(wxT("images/")) + pieceSets[set] +
					wxT("/wbws.png"),   wxBITMAP_TYPE_PNG);

    pieceBitmaps[set][3] = new wxBitmap(wxString(wxT("images/")) + pieceSets[set] +
					wxT("/wrbs.png"), wxBITMAP_TYPE_PNG);

    pieceBitmaps[set][4] = new wxBitmap(wxString(wxT("images/")) + pieceSets[set] +
					wxT("/wqws.png"), wxBITMAP_TYPE_PNG);

    pieceBitmaps[set][5] = new wxBitmap(wxString(wxT("images/")) + pieceSets[set] +
					wxT("/wkbs.png"), wxBITMAP_TYPE_PNG);
  }
}

void PiecesView::onPaint(wxPaintEvent &event)
{
  wxPaintDC dc(this);

  for(int piece = 0, x= 2; piece < 6; piece++, x += 32) {
    dc.DrawBitmap(*pieceBitmaps[pieceSet][piece], x, 2, false);
  }
}

void PiecesView::setPieceSet(int set)
{
  pieceSet = set;
  Refresh();
}

BEGIN_EVENT_TABLE(PiecesView, wxWindow)
     EVT_PAINT(PiecesView::onPaint)
END_EVENT_TABLE()

/*** p2wFrame ***/

p2wFrame::p2wFrame(wxWindow* parent, int id, const wxString& title, const wxPoint& pos,
		   const wxSize& size, long style)
  : wxFrame(parent, id, title, pos, size,
	    wxDEFAULT_FRAME_STYLE & ~wxMAXIMIZE_BOX & ~wxRESIZE_BORDER)
{
  optionsBox = new wxStaticBox(this, -1, wxT("Options"));
  pgnLabel = new wxStaticText(this, -1, wxT("PGN file"), wxDefaultPosition, wxDefaultSize,
			      wxALIGN_RIGHT);
  pgnText = new wxTextCtrl(this, -1, wxT(""));
  pgnButton = new wxButton(this, ID_BROWSEPGN, wxT("Browse..."));
  htmlLabel = new wxStaticText(this, -1, wxT("HTML file(s)"));
  htmlText = new wxTextCtrl(this, -1, wxT(""));
  htmlButton = new wxButton(this, ID_BROWSEHTML, wxT("Browse..."));
  linkCheckBox = new wxCheckBox(this, -1, wxT("Include link to pgn2web homepage"));
  piecesView = new PiecesView(this);
  const wxString piecesChoice_choices[] = {
    wxT("Adventurer"),
    wxT("Alfonso-X"),
    wxT("Cases"),
    wxT("Condal"),
    wxT("Harlequin"),
    wxT("Kingdom"),
    wxT("Leipzig"),
    wxT("Line"),
    wxT("Lucena"),
    wxT("Magnetic"),
    wxT("Mark"),
    wxT("Marroquin"),
    wxT("Maya"),
    wxT("Mediaeval"),
    wxT("Merida"),
    wxT("Motif")
  };
  piecesChoice = new wxChoice(this, ID_CHOOSE, wxDefaultPosition, wxDefaultSize, 16,
			      piecesChoice_choices, 0);
  layoutLabel = new wxStaticText(this, -1, wxT("Layout:"));
  framesetRadio = new wxRadioButton(this, -1, wxT("Frameset"));
  linkedRadio = new wxRadioButton(this, -1, wxT("Linked"));
  individuaRadio = new wxRadioButton(this, -1, wxT("Individual"));
  convertButton = new wxButton(this, ID_CONVERT, wxT("Convert"));
  quitButton = new wxButton(this, wxID_EXIT, wxT("Quit"));

  set_properties();
  do_layout();
}

void p2wFrame::browsePGN(wxCommandEvent& event)
{
  wxFileDialog *cDialog = new wxFileDialog(this, wxT("Select a PGN file..."), wxT(""), wxT(""),
					   wxT("PGN files(*.pgn)|*.pgn|"), wxOPEN);
  if(wxID_OK == cDialog->ShowModal()) {
    pgnText->SetValue(cDialog->GetPath());
  }
  cDialog->Destroy();
}
 
void p2wFrame::browseHTML(wxCommandEvent& event)
{
  wxFileDialog *cDialog = new wxFileDialog(this, wxT("Name HTML file(s)..."), wxT(""), wxT(""),
					   wxT("HTML files(*.html)|*.html|"), wxSAVE);
  if(wxID_OK == cDialog->ShowModal()) {
    htmlText->SetValue(cDialog->GetPath());
  }
  cDialog->Destroy();
}

void p2wFrame::choosePieceSet(wxCommandEvent& event)
{
  piecesView->setPieceSet(event.GetInt());
}

void p2wFrame::convert(wxCommandEvent& event)
{
  STRUCTURE layout;

  //ensure there are filenames entered
  if(pgnText->GetValue() == wxT("")) {
    wxMessageDialog* cDialog = new wxMessageDialog(this, 
						   wxT("Please choose a PGN file to convert"),
						   wxT("pgn2web"), wxOK | wxICON_INFORMATION);
    cDialog->ShowModal();
    cDialog->Destroy();
    return;
  }

  if(htmlText->GetValue() == wxT("")) {
    wxMessageDialog* cDialog = new wxMessageDialog(this,
						 wxT("Please enter a name for the HTML file(s)"),
						 wxT("pgn2web"), wxOK | wxICON_INFORMATION);
    cDialog->ShowModal();
    cDialog->Destroy();
    return;
  }

  //if the html filename has no path use the PGN file's path
  wxString path = htmlText->GetValue();
  
  if(path.Find(SEPERATOR, true) == -1) {
    path = pgnText->GetValue();

    if(path.Find(SEPERATOR, true) != -1) {
      path = path.Left(path.Find(SEPERATOR, true) + 1);
      htmlText->SetValue(path + htmlText->GetValue());
    }
  }

  if(framesetRadio->GetValue()) {
    layout = FRAMESET;
  }
  else {
    if(linkedRadio->GetValue()) {
      layout = LINKED;
    }
    else {
      layout = INDIVIDUAL;
    }
  }

  pgn2web(pgnText->GetValue().mb_str(), htmlText->GetValue().mb_str(), linkCheckBox->GetValue(),
	  piecesChoice->GetStringSelection().Lower().mb_str(), layout);
}

void p2wFrame::quit(wxCommandEvent& event)
{
  Close();
}

void p2wFrame::set_properties()
{
  SetTitle(wxT("pgn2web"));
  SetIcon(wxIcon(pgn2web_xpm));
  linkCheckBox->SetValue(1);
  piecesView->SetSize(wxSize(196, 36));
  piecesChoice->SetSelection(14);
}

void p2wFrame::do_layout()
{
  wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
  wxStaticBoxSizer* optionsSizer = new wxStaticBoxSizer(optionsBox, wxVERTICAL);
  wxBoxSizer* layoutSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer* piecesSizer = new wxBoxSizer(wxHORIZONTAL);
  wxFlexGridSizer* filesSizer = new wxFlexGridSizer(2, 3, 0, 0);
  filesSizer->Add(pgnLabel, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  filesSizer->Add(pgnText, 1, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5);
  filesSizer->Add(pgnButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  filesSizer->Add(htmlLabel, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
  filesSizer->Add(htmlText, 1, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 5);
  filesSizer->Add(htmlButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  filesSizer->AddGrowableCol(1);
  rootSizer->Add(filesSizer, 0, wxLEFT|wxRIGHT|wxTOP|wxEXPAND, 5);
  optionsSizer->Add(linkCheckBox, 0, wxALIGN_CENTER_HORIZONTAL, 0);
  piecesSizer->Add(piecesView, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  piecesSizer->Add(piecesChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  optionsSizer->Add(piecesSizer, 1, wxEXPAND, 0);
  layoutSizer->Add(layoutLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  layoutSizer->Add(framesetRadio, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  layoutSizer->Add(linkedRadio, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  layoutSizer->Add(individuaRadio, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  optionsSizer->Add(layoutSizer, 1, wxALIGN_CENTER_HORIZONTAL, 0);
  rootSizer->Add(optionsSizer, 1, wxALL|wxEXPAND, 10);
  buttonsSizer->Add(convertButton, 0, wxALL, 5);
  buttonsSizer->Add(quitButton, 0, wxALL, 5);
  rootSizer->Add(buttonsSizer, 0,
		 wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 5);
  SetAutoLayout(true);
  SetSizer(rootSizer);
  rootSizer->Fit(this);
  rootSizer->SetSizeHints(this);
  Layout();
  Centre();
}

BEGIN_EVENT_TABLE(p2wFrame, wxFrame)
  EVT_BUTTON(ID_BROWSEPGN, p2wFrame::browsePGN)
  EVT_BUTTON(ID_BROWSEHTML, p2wFrame::browseHTML)
  EVT_CHOICE(ID_CHOOSE, p2wFrame::choosePieceSet)
  EVT_BUTTON(ID_CONVERT, p2wFrame::convert)
  EVT_BUTTON(wxID_EXIT, p2wFrame::quit)
END_EVENT_TABLE()

/*** p2wApp ***/

class p2wApp: public wxApp {
public:
  bool OnInit();
};

IMPLEMENT_APP(p2wApp)
  
bool p2wApp::OnInit()
{
  wxInitAllImageHandlers();
  p2wFrame* mainFrame = new p2wFrame(0, -1, wxT(""));
  SetTopWindow(mainFrame);
  mainFrame->Show();
  return true;
}
