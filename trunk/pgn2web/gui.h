#include <wx/wx.h>
#include <wx/image.h>

//include icon resource
#include "pgn2web.xpm"

//define constant for system dependent file seperator
#ifdef WINDOWS
const wxChar SEPERATOR = wxT('\\');
#else
const wxChar SEPERATOR = wxT('/');
#endif

class PiecesView : public wxWindow {

 public:
  PiecesView(wxWindow* parent);

  void onPaint(wxPaintEvent& event);

  void setPieceSet(int set);

 protected:
  static const wxChar* pieceSets[16];

  int pieceSet;
  wxBitmap* pieceBitmaps[16][6];

  DECLARE_EVENT_TABLE()
};


class p2wFrame: public wxFrame {

 public:
  p2wFrame(wxWindow* parent, int id, const wxString& title, const wxPoint& pos=wxDefaultPosition,
	   const wxSize& size=wxDefaultSize, long style=wxDEFAULT_FRAME_STYLE);

  void browsePGN(wxCommandEvent& event);
  void browseHTML(wxCommandEvent& event);
  void choosePieceSet(wxCommandEvent& event);
  void convert(wxCommandEvent& event);
  void quit(wxCommandEvent& event);

 private:
  void set_properties();
  void do_layout();
  
 protected:
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

  enum {ID_BROWSEPGN = (wxID_HIGHEST + 1), ID_BROWSEHTML, ID_CHOOSE, ID_CONVERT };

  DECLARE_EVENT_TABLE()
};
