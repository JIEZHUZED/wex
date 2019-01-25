#ifndef __pltext_h
#define __pltext_h

#include <vector>

#include <wx/string.h>
#include <wx/window.h>

#include <wex/plot/ploutdev.h>

class wxPLTextLayout
{
public:
	enum TextAlignment { LEFT, CENTER, RIGHT };	

	wxPLTextLayout( wxPLOutputDevice &dc, const wxString &text, TextAlignment ta = LEFT );
	
	void Align( TextAlignment ta );
	void Render( wxPLOutputDevice &dc, double x, double y, double rotationDegrees = 0.0, bool drawBounds = false ); 
	
	inline double Width() { return m_bounds.x; }
	inline double Height() { return m_bounds.y; }
	
	static wxString Escape( const wxString &text );

protected:
	struct text_piece
	{
		text_piece() : text(wxEmptyString), state(NORMAL), origin(0, 0), size(0, 0), aligned_x(0) {  }

		enum TextState { NORMAL, SUPERSCRIPT, SUBSCRIPT }; 
		wxString text;
		TextState state;
		wxRealPoint origin;
		wxRealPoint size;
		double aligned_x;
	};

	std::vector< std::vector<text_piece> > m_lines;
	wxRealPoint m_bounds;
	
	
	std::vector<text_piece> Parse( const wxString &text );
};



class wxPLTextLayoutDemo : public wxWindow
{
public:
	wxPLTextLayoutDemo( wxWindow *parent );

	std::vector<double> Draw( wxPLOutputDevice &dc, const wxPLRealRect &geom );

	void OnPaint( wxPaintEvent & );
	void OnSize( wxSizeEvent & );

	DECLARE_EVENT_TABLE();
};

#define wxFT_FONT_DEFAULT_INDEX 0
#define wxFT_FONT_DEFAULT_FAMILY wxFreeTypeFontFamily(wxFT_FONT_DEFAULT_INDEX)


int wxFreeTypeLoadAllFontFiles( const wxString &path = wxEmptyString );
int wxFreeTypeLoadFontFile( const wxString &font_file );
wxArrayString wxFreeTypeListFontFaces();
wxArrayString wxFreeTypeListFontFaceVariants();
bool wxFreeTypeWriteBuiltinFontData( int ifnt, const wxString &file );
wxString wxFreeTypeFontDataFile( int ifnt );
bool wxFreeTypeFontStyle( int ifnt, bool *bold, bool *italic );
wxString wxFreeTypeFontFamily( int ifnt );

int wxFreeTypeFindClosestFont( const wxString &family, bool bold, bool italic );

unsigned char *wxFreeTypeFontData( int ifnt, size_t *len );

wxRealPoint wxFreeTypeMeasure( int ifnt, double points, unsigned int dpi, const wxString &text );
void wxFreeTypeDraw( wxImage *img, bool init_img, const wxPoint &pos, 
	int ifnt, double points, unsigned int dpi, 
	const wxString &text, const wxColour &c=*wxBLACK, double angle = 0.0 );
wxImage wxFreeTypeDraw( wxRealPoint *pos, int ifnt, double points, unsigned int dpi, 
	const wxString &text, const wxColour &c=*wxBLACK, double angle=0.0 );
void wxFreeTypeDraw( wxDC &dc, const wxPoint &pos, int ifnt, double points, unsigned int dpi,
	const wxString &text, const wxColour &c=*wxBLACK, double angle=0.0 );
void wxFreeTypeDraw( wxGraphicsContext &gc, const wxRealPoint &pos, int ifnt, double points, unsigned int dpi,
	const wxString &text, const wxColour &c=*wxBLACK, double angle=0.0 );

class wxFreeTypeDemo : public wxWindow
{
	std::vector<int> faces;
	int Face(int idx);
public:
	wxFreeTypeDemo( wxWindow *parent );
	static void GenerateTTFBinaryFontData();
	void OnPaint( wxPaintEvent & );
	void OnSize( wxSizeEvent & );
	DECLARE_EVENT_TABLE();
};

#endif
