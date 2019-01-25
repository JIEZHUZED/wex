#ifndef __ploutdev_h
#define __ploutdev_h

#include <wx/dc.h>
#include <wx/gdicmn.h>
#include <wx/string.h>
#include <wx/graphics.h>

#include <wex/matrix.h>
#include <wex/pdf/pdfdoc.h>
#include <wex/pdf/pdfshape.h>


class wxPLRealRect
{
public:
	double x, y, width, height;

	wxPLRealRect() { x=y=width=height=0.0; }
	wxPLRealRect( const wxPLRealRect &rhs )
		: x(rhs.x), y(rhs.y), width(rhs.width), height(rhs.height) { }
	wxPLRealRect( double _x, double _y, double _w, double _h )
		: x(_x), y(_y), width(_w), height(_h) { }

	bool Contains( double xx, double yy ) const {
		return ( xx >= x && xx <= x+width
			&& yy >= y && yy <= y+height );
	}

	bool Contains( const wxRealPoint &pos ) const { return Contains(pos.x,pos.y); }
};

class wxPLOutputDevice
{
public:
	wxPLOutputDevice();
	virtual ~wxPLOutputDevice() { }

	enum Style { NONE, SOLID, DOT, DASH, DOTDASH, MITER, BEVEL, ROUND, BUTT, HATCH };
	enum FillRule { ODD_EVEN_RULE, WINDING_RULE };

	virtual void SetAntiAliasing( bool b ) = 0;
	virtual bool GetAntiAliasing() const = 0;

	// Pure virtuals, to be implemented
	virtual void Clip( double x, double y, double width, double height ) = 0;
	virtual void Unclip() = 0;
	virtual void Pen( const wxColour &c, double size=1, Style line = SOLID, Style join = MITER, Style cap = BUTT ) = 0;
	virtual void Brush( const wxColour &c, Style sty = SOLID ) = 0;

	virtual void Line( double x1, double y1, double x2, double y2 ) = 0;
	virtual void Lines( size_t n, const wxRealPoint *pts ) = 0;
	virtual void Polygon( size_t n, const wxRealPoint *pts, FillRule rule = ODD_EVEN_RULE ) = 0;
	virtual void Rect( double x, double y, double width, double height )  = 0;	
	virtual void Circle( double x, double y, double radius ) = 0;
	virtual void Sector( double x, double y, double radius, double angle1, double angle2 ) = 0;
	virtual void Arc( double x, double y, double width, double height, double angle1, double angle2 ) = 0;
	virtual void Image( const wxImage &img, const wxPLRealRect &rect ) = 0;
	
	virtual void MoveTo( double x, double y ) = 0;
	virtual void LineTo( double x, double y ) = 0;
	virtual void CloseSubPath() = 0;
	virtual void Path( FillRule rule = WINDING_RULE ) = 0;
	

	virtual void TextSize( double points ) = 0;
	virtual bool TextFace( const wxString &face, bool bold=false, bool italic=false ) = 0;
	virtual void TextAdjust( double relpt ) = 0;
	virtual double TextSize() const = 0;
	virtual void TextColour( const wxColour &col ) = 0;

	virtual void Text( const wxString &text, double x, double y,  double angle=0 ) = 0;
	virtual void Measure( const wxString &text, double *width, double *height ) = 0;


	// API variants and helpers;
	virtual void NoPen() { Pen( *wxBLACK, 1.0, NONE ); }
	virtual void NoBrush() { Brush( *wxBLACK, NONE ); }
	virtual void Line( const wxRealPoint &p1, const wxRealPoint &p2 ) { Line( p1.x, p1.y, p2.x, p2.y ); }
	virtual void Rect( const wxPLRealRect &r ) { Rect( r.x, r.y, r.width, r.height ); }
	virtual void Circle( const wxRealPoint &p, double radius ) { Circle(p.x, p.y, radius); }
	virtual void Text( const wxString &text, const wxRealPoint &p, double angle=0 ) { Text( text, p.x, p.y, angle ); }
	virtual void Arc( double x, double y, double r, double angle1, double angle2 ) { Arc(x-r,y-r,r+r,r+r,angle1,angle2); }

	double WrappedText( const wxString &str, double x, double y, 
		double xlimit, wxArrayString *lines=0 );
};

class wxPLPdfOutputDevice : public wxPLOutputDevice
{
	double m_textSize;
	double m_textSizeAdjust;
	bool m_pen, m_brush;
	wxPdfDocument &m_pdf;
	wxPdfShape m_shape;

public:
	wxPLPdfOutputDevice( wxPdfDocument &doc );
	virtual void SetAntiAliasing( bool );
	virtual bool GetAntiAliasing() const { return true; }
	virtual void Clip( double x, double y, double width, double height );
	virtual void Unclip();
	virtual void Pen( const wxColour &c, double size=1, Style line = SOLID, Style join = MITER, Style cap = BUTT );
	virtual void Brush( const wxColour &c, Style sty = SOLID );
	virtual void Line( double x1, double y1, double x2, double y2 );
	virtual void Lines( size_t n, const wxRealPoint *pts );
	virtual void Polygon( size_t n, const wxRealPoint *pts, FillRule rule = ODD_EVEN_RULE );
	virtual void Rect( double x, double y, double width, double height );
	virtual void Circle( double x, double y, double radius );
	virtual void Arc( double x, double y, double width, double height, double angle1, double angle2 );
	virtual void Sector( double x, double y, double radius, double angle1, double angle2 );
	virtual void Image( const wxImage &img, const wxPLRealRect &rect );

	virtual void MoveTo( double x, double y );
	virtual void LineTo( double x, double y );
	virtual void CloseSubPath();
	virtual void Path( FillRule rule = WINDING_RULE );
	
	virtual void Text( const wxString &text, double x, double y,  double angle=0 );
	virtual void Measure( const wxString &text, double *width, double *height );

	virtual void TextSize( double points );
	virtual bool TextFace( const wxString &face, bool bold=false, bool italic=false );
	virtual void TextAdjust( double relpt );
	virtual double TextSize() const;
	virtual void TextColour( const wxColour &col );

private:		
	int GetDrawingStyle();
};

/*
class wxPLDCOutputDevice : public wxPLOutputDevice
{
	wxDC *m_dc, *m_aadc, *m_curdc;
	wxPen m_pen;
	wxBrush m_brush;
	wxFont m_font0;
	double m_fontSize;
	bool m_fontBold;
	double m_scale;
public:
	wxPLDCOutputDevice( wxDC *dc, wxDC *aadc = 0, double scale=1.0 );
	wxDC *GetDC();
	
	virtual void SetAntiAliasing( bool b );

	virtual void Clip( double x, double y, double width, double height );
	virtual void Unclip();
	virtual void Brush( const wxColour &c, Style sty );
	virtual void Pen( const wxColour &c, double size, 
		Style line = SOLID, Style join = MITER, Style cap = BUTT );
	virtual void Line( double x1, double y1, double x2, double y2 );
	virtual void Lines( size_t n, const wxRealPoint *pts );
	virtual void Polygon( size_t n, const wxRealPoint *pts, Style sty );
	virtual void Rect( double x, double y, double width, double height );
	virtual void Circle( double x, double y, double radius );
	virtual void Font( double relpt = 0, bool bold = false );
	virtual void Font( double *rel, bool *bld ) const;
	virtual void Text( const wxString &text, double x, double y, double angle=0 );
	virtual void Measure( const wxString &text, double *width, double *height );
};
*/

class wxPLGraphicsOutputDevice : public wxPLOutputDevice
{
	wxGraphicsContext *m_gc;
	double m_textSize;
	double m_textSizeAdjust;
	wxColour m_textColour;

	int m_fontIndex;
	double m_scale;
	bool m_pen, m_brush;
	wxGraphicsPath m_path;

public:
	wxPLGraphicsOutputDevice( wxGraphicsContext *gc );

	void SetScale( double sf ) { m_scale = sf; }
	
	virtual void SetAntiAliasing( bool b );
	virtual bool GetAntiAliasing() const;

	virtual void Clip( double x, double y, double width, double height );
	virtual void Unclip();
	virtual void Brush( const wxColour &c, Style sty );
	virtual void Pen( const wxColour &c, double size, 
		Style line = SOLID, Style join = MITER, Style cap = BUTT );
	virtual void Line( double x1, double y1, double x2, double y2 );
	virtual void Lines( size_t n, const wxRealPoint *pts );
	virtual void Polygon( size_t n, const wxRealPoint *pts, FillRule rule = ODD_EVEN_RULE );
	virtual void Rect( double x, double y, double width, double height );
	virtual void Circle( double x, double y, double radius );
	virtual void Sector( double x, double y, double radius, double angle1, double angle2 );
	virtual void Arc( double x, double y, double width, double height, double angle1, double angle2 );
	virtual void Image( const wxImage &img, const wxPLRealRect &rect );

	virtual void MoveTo( double x, double y );
	virtual void LineTo( double x, double y );
	virtual void CloseSubPath();
	virtual void Path( FillRule rule = WINDING_RULE );

	virtual void Text( const wxString &text, double x, double y, double angle=0 );
	virtual void Measure( const wxString &text, double *width, double *height );

	virtual void TextSize( double points );
	virtual bool TextFace( const wxString &face, bool bold=false, bool italic=false );
	virtual void TextAdjust( double relpt );
	virtual double TextSize() const;
	virtual void TextColour( const wxColour &col );

};

class wxPLFormattedOutputHelper 
{
	wxPLOutputDevice *m_dc;
	wxPLRealRect m_rect;
public:
	wxPLFormattedOutputHelper( const wxPLRealRect &rct, wxPLOutputDevice *dc = 0 );

	void SetOutputDevice( wxPLOutputDevice *dc );
	void SetOrigin( double xx, double yy );
	void SetSize( double xx, double yy );
	void SetRect( const wxPLRealRect &rct );

	// cursor position-based rendering/styling
	void Style( const wxString &face, int size, const wxColour &c, 
		bool bold, bool ital, int align, double line_width, wxPLOutputDevice::Style line_style, bool wrap );
	void Style( wxString *face, int *size, wxColour *c, 
		bool *b, bool *it, int *al, double *line_width, wxPLOutputDevice::Style *line_style, bool *wrap );
	void TableStyle( int hdrSize, const wxString &hdrFace, int hdrAlign, bool hdrBold, const wxColour &hdrColor,
		bool hdrLine, int cellAlign, bool gridLines, const std::vector<double> &rowSizes, const std::vector<double> &colSizes,
		bool tabBorder );
	void TableStyle( int *hdrSize, wxString *hdrFace, int *hdrAlign, bool *hdrBold, wxColour *hdrColor,
		bool *hdrLine, int *cellAlign, bool *gridLines, std::vector<double> *rowSizes, std::vector<double> *colSizes,
		bool *tabBorder );

	void MoveTo( double x, double y );
	void LineTo( double x, double y );
	void GetCursorPos( double *x, double *y );
	void Measure( const wxString &s, double *w, double *h );

	void RenderText( const wxString &s );
	void RenderImage( const wxImage &img, double width, double height );
	void RenderTable( const wxMatrix<wxString> &tab );
	void RenderBarGraph( const std::vector<double> &values, const wxArrayString & xlabels, const wxString &xlabel,
		const wxString &ylabel, const wxString &title, bool show_values, double xsize, double ysize,
		int decimals, const wxColour &color, bool show_yaxis_ticks = false, const wxString &ticks_format = wxEmptyString);
private:
	double m_curXPos, m_curYPos, m_curLineHeight;

	void Font( const wxString &face, int sizepts, bool bold, bool italic );

	// text properties
	bool m_wrapText;
	wxString m_curFace;
	int m_curSize, m_curAlign;
	wxColour m_curColour;
	bool m_curBold, m_curItalic;
	double m_curLineWidth;
	wxPLOutputDevice::Style m_curLineStyle;

	// table properties
	int m_headerSize;
	wxString m_headerFace;
	int m_headerAlign;
	bool m_headerBold;
	wxColour m_headerColour;
	int m_cellAlign;
	bool m_gridLines;
	bool m_headerLine;
	std::vector<double> m_rowSizes;	
	std::vector<double> m_colSizes;
	bool m_tableBorder;
};


#endif
