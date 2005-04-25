#include "gui.h"

/*** Custom progress event ***/

DEFINE_EVENT_TYPE(wxEVT_UPDATE_PROGRESS)

/*** Progress Callback ***/

void progress_callback(float percentage, void *context)
{
  //post event with integer precentage
  wxCommandEvent progressEvent(wxEVT_UPDATE_PROGRESS, ID_UPDATE_PROGRESS);
  progressEvent.SetInt((int)percentage);
  ((wxEvtHandler*)context)->AddPendingEvent(progressEvent);
} 

/*** PiecesView ***/

const wxChar* PiecesView::pieceSets[16] = { wxT("adventurer"), wxT("alfonso-x"), wxT("cases"),
					    wxT("condal"), wxT("harlequin"), wxT("kingdom"),
					    wxT("leipzig"), wxT("line"), wxT("lucena"),
					    wxT("magnetic"), wxT("mark"), wxT("marroquin"),
					    wxT("maya"), wxT("mediaeval"), wxT("merida"), 
					    wxT("motif") };

PiecesView::PiecesView(wxWindow* parent, const wxString& resourcePath)
  : wxWindow(parent, -1, wxDefaultPosition, wxSize(196,36))
{
  //Set background colour
  SetBackgroundColour(wxColour(wxT("black")));

  //Derive path for images
  wxFileName imagePath(resourcePath);
  imagePath.AppendDir(wxT("images"));
  wxFileName setPath;
  wxFileName fileName;

  //Load pieces bitmaps
  pieceSet = 14;

  for(int set = 0; set < 16; set++) {

    setPath = imagePath;
    setPath.AppendDir(pieceSets[set]);

    fileName = setPath;
    fileName.SetName(wxT("wpws.png"));
    pieceBitmaps[set][0] = new wxBitmap(fileName.GetFullPath(), wxBITMAP_TYPE_PNG);

    fileName = setPath;
    fileName.SetName(wxT("wnbs.png"));
    pieceBitmaps[set][1] = new wxBitmap(fileName.GetFullPath(), wxBITMAP_TYPE_PNG);

    fileName = setPath;
    fileName.SetName(wxT("wbws.png"));
    pieceBitmaps[set][2] = new wxBitmap(fileName.GetFullPath(), wxBITMAP_TYPE_PNG);
    
    fileName = setPath;
    fileName.SetName(wxT("wrbs.png"));
    pieceBitmaps[set][3] = new wxBitmap(fileName.GetFullPath(), wxBITMAP_TYPE_PNG);

    fileName = setPath;
    fileName.SetName(wxT("wqws.png"));
    pieceBitmaps[set][4] = new wxBitmap(fileName.GetFullPath(), wxBITMAP_TYPE_PNG);
    
    fileName = setPath;
    fileName.SetName(wxT("wkbs.png"));
    pieceBitmaps[set][5] = new wxBitmap(fileName.GetFullPath(), wxBITMAP_TYPE_PNG);

  }
}

PiecesView::~PiecesView()
{
  //free the images allocated in the constructor
  for(int set = 0; set < 16; set++) {
    for(int piece = 0; piece < 6; piece++) {
      delete pieceBitmaps[set][piece];
    }
  }   
}

void PiecesView::onPaint(wxPaintEvent &event)
{
  //draw pieces
  wxPaintDC dc(this);

  for(int piece = 0, x= 2; piece < 6; piece++, x += 32) {
    dc.DrawBitmap(*pieceBitmaps[pieceSet][piece], x, 2, false);
  }
}

void PiecesView::setPieceSet(int set)
{
  //set piece set and redraw
  pieceSet = set;
  Refresh();
}

BEGIN_EVENT_TABLE(PiecesView, wxWindow)
     EVT_PAINT(PiecesView::onPaint)
END_EVENT_TABLE()

/*** pgn2webThread ***/

pgn2webThread::pgn2webThread(wxEvtHandler *listener, const wxString& resourcePath,
			     const wxString& PGNFilename, const wxString& HTMLFilename,
			     bool credit, const wxString& pieces, STRUCTURE layout) : wxThread()
{
  //store parameters for pgn2web function
  m_resourcePath = resourcePath;
  m_listener = listener;
  m_PGNFilename = PGNFilename;
  m_HTMLFilename = HTMLFilename;
  m_credit = credit;
  m_pieces = pieces;
  m_layout = layout;
}

wxThread::ExitCode pgn2webThread::Entry()
{
  //simply call pgn2web function with stored parameters
  pgn2web(m_resourcePath.mb_str(), m_PGNFilename.mb_str(), m_HTMLFilename.mb_str(), m_credit,
	  m_pieces.mb_str(), m_layout, progress_callback, m_listener);
  
  return NULL;
}

/*** ProgressDialog ***/

ProgressDialog::ProgressDialog(wxWindow* parent) : wxDialog(parent, wxID_ANY, wxT("pgn2web"),
							    wxDefaultPosition, wxDefaultSize,
							    wxDEFAULT_DIALOG_STYLE &
							    ~wxCLOSE_BOX)
{
  progressText = new wxStaticText(this, -1, wxT("Converting PGN to HTML..."), wxDefaultPosition,
				  wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
  progressGauge = new wxGauge(this, -1, 100, wxDefaultPosition, wxDefaultSize,
			      wxGA_HORIZONTAL|wxGA_SMOOTH);
  progressOk = new wxButton(this, wxID_OK, wxT("OK"));

  set_properties();
  do_layout();
}

void ProgressDialog::updateProgress(wxCommandEvent& event)
{
  //update progress bar with new percentage
  int percentage = event.GetInt();
  progressGauge->SetValue(percentage);

  //if complete, display message and enable ok button
  if(percentage == 100) {
    progressText->SetLabel(wxT("Conversion Complete"));
    progressOk->Enable();
  }
}

void ProgressDialog::set_properties()
{
  SetTitle(wxT("pgn2web"));
  progressGauge->SetSize(wxSize(400,28));
  progressOk->Enable(false);
}

void ProgressDialog::do_layout()
{
  wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
  rootSizer->Add(progressText, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
  rootSizer->Add(progressGauge, 0, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
  rootSizer->Add(progressOk, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxFIXED_MINSIZE, 5);
  SetAutoLayout(true);
  SetSizer(rootSizer);
  rootSizer->Fit(this);
  rootSizer->SetSizeHints(this);
  Layout();
  Centre();
}

BEGIN_EVENT_TABLE(ProgressDialog, wxDialog)
  EVT_COMMAND(ID_UPDATE_PROGRESS, wxEVT_UPDATE_PROGRESS, ProgressDialog::updateProgress)
END_EVENT_TABLE()


/*** p2wFrame ***/

p2wFrame::p2wFrame(const wxString& installPath)
  : wxFrame(0, wxID_ANY, wxT("pgn2web"), wxDefaultPosition, wxDefaultSize,
	    wxDEFAULT_FRAME_STYLE & ~wxMAXIMIZE_BOX & ~wxRESIZE_BORDER)
{
  m_installPath = installPath;

  rootPanel = new wxPanel(this, -1);
  optionsBox = new wxStaticBox(rootPanel, -1, wxT("Options"));
  pgnLabel = new wxStaticText(rootPanel, -1, wxT("PGN file"), wxDefaultPosition, wxDefaultSize,
			      wxALIGN_RIGHT);
  pgnText = new wxTextCtrl(rootPanel, -1, wxT(""));
  pgnButton = new wxButton(rootPanel, ID_BROWSEPGN, wxT("Browse..."));
  htmlLabel = new wxStaticText(rootPanel, -1, wxT("HTML file(s)"));
  htmlText = new wxTextCtrl(rootPanel, -1, wxT(""));
  htmlButton = new wxButton(rootPanel, ID_BROWSEHTML, wxT("Browse..."));
  linkCheckBox = new wxCheckBox(rootPanel, -1, wxT("Include link to pgn2web homepage"));
  piecesView = new PiecesView(rootPanel, m_installPath);
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
  piecesChoice = new wxChoice(rootPanel, ID_CHOOSE, wxDefaultPosition, wxDefaultSize, 16,
			      piecesChoice_choices, 0);
  layoutLabel = new wxStaticText(rootPanel, -1, wxT("Layout:"));
  framesetRadio = new wxRadioButton(rootPanel, -1, wxT("Frameset"), wxDefaultPosition,
				    wxDefaultSize, wxRB_GROUP);
  linkedRadio = new wxRadioButton(rootPanel, -1, wxT("Linked"));
  individuaRadio = new wxRadioButton(rootPanel, -1, wxT("Individual"));
  convertButton = new wxButton(rootPanel, ID_CONVERT, wxT("Convert"));
  quitButton = new wxButton(rootPanel, wxID_EXIT, wxT("Quit"));

  set_properties();
  do_layout();
}

void p2wFrame::browsePGN(wxCommandEvent& event)
{
  wxFileDialog *cDialog = new wxFileDialog(this, wxT("Select a PGN file..."), wxT(""), wxT(""),
					   wxT("PGN files(*.pgn;*.PGN)|*.pgn;*.PGN"),
					   wxOPEN | wxFILE_MUST_EXIST);
  if(wxID_OK == cDialog->ShowModal()) {
    pgnText->SetValue(cDialog->GetPath());
  }
  cDialog->Destroy();
}
 
void p2wFrame::browseHTML(wxCommandEvent& event)
{
  wxFileDialog *cDialog = new wxFileDialog(this, wxT("Name HTML file(s)..."), wxT(""), wxT(""),
					   wxT("HTML files(*.html)|*.html"), wxSAVE);
  if(wxID_OK == cDialog->ShowModal()) {
    htmlText->SetValue(cDialog->GetPath());
  }
  cDialog->Destroy();
}

void p2wFrame::choosePieceSet(wxCommandEvent& event)
{
  //pass the selected piece set on to the piece view
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

  //get the selected layout
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

  //create the progress dialog
  ProgressDialog *dialog = new ProgressDialog(this);

  //run the conversion in a seperate thread
  pgn2webThread *thread = new pgn2webThread(dialog, m_installPath, pgnText->GetValue(),
					    htmlText->GetValue(), linkCheckBox->GetValue(),
					    piecesChoice->GetStringSelection().Lower(), layout);
  thread->Create();
  thread->Run();

  //show the dialog
  dialog->ShowModal();
  dialog->Destroy();
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
  wxBoxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
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
  optionsSizer->Add(linkCheckBox, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
  piecesSizer->Add(piecesView, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  piecesSizer->Add(piecesChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  optionsSizer->Add(piecesSizer, 0, wxALIGN_CENTER_HORIZONTAL, 0);
  layoutSizer->Add(layoutLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  layoutSizer->Add(framesetRadio, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  layoutSizer->Add(linkedRadio, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  layoutSizer->Add(individuaRadio, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
  optionsSizer->Add(layoutSizer, 0, wxALIGN_CENTER_HORIZONTAL, 0);
  rootSizer->Add(optionsSizer, 1, wxALL|wxEXPAND, 10);
  buttonsSizer->Add(convertButton, 0, wxALL, 5);
  buttonsSizer->Add(quitButton, 0, wxALL, 5);
  rootSizer->Add(buttonsSizer, 0,
		 wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 5);
  rootPanel->SetAutoLayout(true);
  rootPanel->SetSizer(rootSizer);
  panelSizer->Add(rootPanel, 1, wxEXPAND, 0);
  SetAutoLayout(true);
  SetSizer(panelSizer);
  panelSizer->Fit(this);
  panelSizer->SetSizeHints(this);
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
  //Get installation path for resources

  wxString installPath;

#ifdef WINDOWS
  /* read the path from the registry */
  wxConfig *config = new wxConfig("pgn2web");

  if (!config->Read("installPath", &installPath)) {
    //if not found use default
    installPath = wxT("C:\\Program Files\\pgn2web\\");
  }

  delete config;
#else
  installPath = wxT(INSTALL_PATH);
#endif
  
  wxInitAllImageHandlers();
  p2wFrame* mainFrame = new p2wFrame(installPath);
  SetTopWindow(mainFrame);
  mainFrame->Show();
  return true;
}
