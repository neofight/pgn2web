#include <wx/wx.h>
#include <wx/config.h>
#include <wx/filename.h>
#include <wx/image.h>

#include "pgn2web.h"

//include icon resource
#include "pgn2web.xpm"

//define constant for system dependent file seperator
#ifdef WINDOWS
const wxChar SEPERATOR = wxT('\\');
#else
const wxChar SEPERATOR = wxT('/');
#endif

// default installation path (*nix)
#ifndef INSTALL_PATH
#define INSTALL_PATH "/usr/local/pgn2web/"
#endif

//define event ids
enum { ID_BROWSEPGN = (wxID_HIGHEST + 1), ID_BROWSEHTML, ID_CHOOSE, ID_CONVERT,
       ID_UPDATE_PROGRESS };

DECLARE_EVENT_TYPE(wxEVT_UPDATE_PROGRESS, -1) //custom event type for progress updates

class PiecesView : public wxWindow {

 public:
  PiecesView(wxWindow* parent, const wxString& resourcePath);
  ~PiecesView();

  void onPaint(wxPaintEvent& event);

  void setPieceSet(int set);

 protected:
  static const wxChar* pieceSets[16];

  int pieceSet;
  wxBitmap* pieceBitmaps[16][6];

  DECLARE_EVENT_TABLE()
};

class pgn2webThread : public wxThread {

 public:
  pgn2webThread(wxEvtHandler *listener, const wxString& resourcePath,
		const wxString& PGNFilename, const wxString& HTMLFilename, bool credit,
		const wxString& pieces, STRUCTURE layout);

  ExitCode Entry();

 protected:
  // parameters for pgn2web function
  wxString     m_resourcePath;
  wxEvtHandler *m_listener;
  wxString     m_PGNFilename;
  wxString     m_HTMLFilename;
  bool         m_credit;
  wxString     m_pieces;
  STRUCTURE    m_layout;
};

class ProgressDialog : public wxDialog {

 public:
  ProgressDialog(wxWindow* parent);

  void updateProgress(wxCommandEvent& event);

 private:
  void set_properties();
  void do_layout();
  
 protected:
  wxStaticText* progressText;
  wxGauge*      progressGauge;
  wxButton*     progressOk;

  DECLARE_EVENT_TABLE()
};

class p2wFrame: public wxFrame {

 public:
  p2wFrame(const wxString& installPath);

  void browsePGN(wxCommandEvent& event);
  void browseHTML(wxCommandEvent& event);
  void choosePieceSet(wxCommandEvent& event);
  void convert(wxCommandEvent& event);
  void quit(wxCommandEvent& event);

 private:
  void set_properties();
  void do_layout();
  
 protected:
  wxString       m_installPath;

  wxPanel*       rootPanel;
  wxStaticBox*   optionsBox;
  wxStaticText*  pgnLabel;
  wxTextCtrl*    pgnText;
  wxButton*      pgnButton;
  wxStaticText*  htmlLabel;
  wxTextCtrl*    htmlText;
  wxButton*      htmlButton;
  wxCheckBox*    linkCheckBox;
  PiecesView*    piecesView;
  wxChoice*      piecesChoice;
  wxStaticText*  layoutLabel;
  wxRadioButton* framesetRadio;
  wxRadioButton* linkedRadio;
  wxRadioButton* individuaRadio;
  wxButton*      convertButton;
  wxButton*      quitButton;

  DECLARE_EVENT_TABLE()
};
