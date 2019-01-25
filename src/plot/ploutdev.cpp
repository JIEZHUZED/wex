#include <vector>
#include <math.h>
#include <wex/pdf/pdfdoc.h>
#include <wex/pdf/pdffont.h>
#include <wex/plot/ploutdev.h>
#include <wex/plot/pltext.h>
#include <wex/plot/plaxis.h>
#include <wex/numeric.h>

#define LK_USE_WXWIDGETS 1
#include <lk/stdlib.h>

wxPLOutputDevice::wxPLOutputDevice()
{
	// initialize freetype font system

}



double wxPLOutputDevice::WrappedText( const wxString &str, double x, double y, 
	double xlimit, wxArrayString *lines )
{
	int line = 0;
	double line_height = 0;
	wxString remaining = str;

	while ( !remaining.IsEmpty() )
	{
		wxString line_text = remaining;
		double line_width;
		Measure(line_text, &line_width, &line_height);
		while(line_width > 5.0 && x+line_width >= xlimit-3 && line_text.Len() > 0)
		{
			int pos = line_text.Find(' ', true);
			if (pos < 0)
				line_text.Truncate( line_text.Len()-1 );
			else
				line_text.Truncate(pos);

			Measure(line_text, &line_width, &line_height);
		}

		if (line_text.IsEmpty() || line_width < 5)
			break;

		if (lines) lines->Add( line_text );
		
		Text(line_text, x, y);

		line++;
		y += line_height;

		remaining = remaining.Mid(line_text.Len());
		remaining.Trim(false).Trim();
	}

	return y;
}


wxPLFormattedOutputHelper::wxPLFormattedOutputHelper( const wxPLRealRect &rct, wxPLOutputDevice *dc )
{
	m_rect = rct;
	// setup rendering state variables
	m_curXPos = 0;
	m_curYPos = 0;
	m_dc = dc;
	
	// initialize styles
	Style( wxFT_FONT_DEFAULT_FAMILY, 12, *wxBLACK, false, false, 
		wxLEFT, 1.0, wxPLOutputDevice::SOLID, false );
	TableStyle( 12, wxFT_FONT_DEFAULT_FAMILY, wxCENTER, true, *wxBLACK, true, 
		wxLEFT, false, std::vector<double>(), std::vector<double>(), true );
}

void wxPLFormattedOutputHelper::SetOutputDevice( wxPLOutputDevice *dc ) { m_dc = dc; }
void wxPLFormattedOutputHelper::SetOrigin( double xx, double yy ) { m_curXPos = m_rect.x = xx; m_curYPos = m_rect.y = yy; }
void wxPLFormattedOutputHelper::SetSize( double xx, double yy ) { m_rect.width = xx; m_rect.height = yy; }
void wxPLFormattedOutputHelper::SetRect( const wxPLRealRect &rct ) { m_rect = rct; m_curXPos = rct.x; m_curYPos = rct.y; }

void wxPLFormattedOutputHelper::Font( const wxString &face, int sizepts, bool bold, bool italic )
{
	if ( !m_dc ) return;
	m_dc->TextFace( face, bold, italic );
	m_dc->TextSize( sizepts );
}

void wxPLFormattedOutputHelper::Style( const wxString &face, int size, const wxColour &c, bool bold, 
	bool ital, int align, double line_width, wxPLOutputDevice::Style line_style, bool wrap)
{
	if (!m_dc) return;

	m_curFace = face;
	m_curSize = size;
	m_curColour = c;
	m_curBold = bold;
	m_curItalic = ital;
	m_curAlign = align;

	m_curLineWidth = line_width;
	m_curLineStyle = line_style;
	m_wrapText = wrap;

	m_dc->Pen( m_curColour, m_curLineWidth, m_curLineStyle );
	m_dc->TextColour( m_curColour );
	Font( face, size, bold, ital );
	
	m_curLineHeight = 0.2f;
	m_dc->Measure("hy", 0, &m_curLineHeight);
}

void wxPLFormattedOutputHelper::Style( wxString *face, int *size, wxColour *c, bool *b, 
	bool *it, int *al, double *line_width, wxPLOutputDevice::Style *line_style, bool *wrap )
{
	*face = m_curFace;
	*size = m_curSize;
	*c = m_curColour;
	*b = m_curBold;
	*it = m_curItalic;
	*al = m_curAlign;	
	*line_width = m_curLineWidth;
	*line_style = m_curLineStyle;
	*wrap = m_wrapText;
}

void wxPLFormattedOutputHelper::TableStyle( int hdrSize, const wxString &hdrFace, int hdrAlign, bool hdrBold, const wxColour &hdrColor,
	bool hdrLine, int cellAlign, bool gridLines, const std::vector<double> &rowSizes, const std::vector<double> &colSizes,
	bool tabBorder )
{
	m_headerSize = hdrSize;
	m_headerFace = hdrFace;
	m_headerAlign = hdrAlign;
	m_headerBold = hdrBold;
	m_headerColour = hdrColor;
	m_headerLine = hdrLine;
	m_cellAlign = cellAlign;
	m_gridLines = gridLines;
	m_rowSizes = rowSizes;
	m_colSizes = colSizes;
	m_tableBorder = tabBorder;
}

void wxPLFormattedOutputHelper::TableStyle( int *hdrSize, wxString *hdrFace, int *hdrAlign, bool *hdrBold, wxColour *hdrColor,
	bool *hdrLine, int *cellAlign, bool *gridLines, std::vector<double> *rowSizes, std::vector<double> *colSizes,
	bool *tabBorder)
{
	*hdrSize = m_headerSize;
	*hdrFace = m_headerFace;
	*hdrAlign = m_headerAlign; 
	*hdrBold = m_headerBold;
	*hdrColor = m_headerColour;
	*hdrLine = m_headerLine;
	*cellAlign = m_cellAlign;
	*gridLines = m_gridLines;
	*rowSizes = m_rowSizes;
	*colSizes = m_colSizes;
	*tabBorder = m_tableBorder;
}

void wxPLFormattedOutputHelper::MoveTo( double x, double y )
{
	m_curXPos = x + m_rect.x;
	m_curYPos = y + m_rect.y;
}

void wxPLFormattedOutputHelper::LineTo( double x, double y )
{
	if (!m_dc) return;
	m_dc->Pen( m_curColour, m_curLineWidth, m_curLineStyle );
	m_dc->Line( m_curXPos, m_curYPos, x+m_rect.x, y+m_rect.y );
	m_curXPos = x+m_rect.x;
	m_curYPos = y+m_rect.y;
}

void wxPLFormattedOutputHelper::GetCursorPos( double *x, double *y )
{
	*x = m_curXPos-m_rect.x;
	*y = m_curYPos-m_rect.y;
}

void wxPLFormattedOutputHelper::Measure( const wxString &s, double *w, double *h )
{
	if (!m_dc)
	{
		*w = *h = 0;
		return;
	}
	m_dc->Measure( s, w, h );
}

void wxPLFormattedOutputHelper::RenderText( const wxString &str )
{
	if (!m_dc) return;

	
	char cur_delim[2] = {0,0};
	wxString::size_type m_pos = 0;
	wxString token;
	
	while (m_pos < str.length())
	{
		std::string::size_type pos = str.find_first_of('\n', m_pos);
		if (pos == std::string::npos)
		{
			cur_delim[0] = 0;
			token.assign(str, m_pos, wxString::npos);
			m_pos = str.length();
		}
		else
		{
			cur_delim[0] = str[pos];
			std::string::size_type len = pos - m_pos;			
			token.assign(str, m_pos, len);
			m_pos = pos + 1;
		}
				
		// write text
		
		// advance to next line if alignment is changed and the current X
		// position is not at the left of the object
		if (m_curAlign != wxLEFT && m_curXPos != m_rect.x )
		{
			m_curXPos = m_rect.x;
			m_curYPos += m_curLineHeight;
		}

		if (!token.IsEmpty())
		{	
			if ( m_wrapText )
			{
				m_curYPos += m_dc->WrappedText( token, m_curXPos, m_curYPos, m_rect.x+m_rect.width );
			}
			else
			{
				double width;
				m_dc->Measure( token, &width, 0 );
				if (m_curAlign == wxLEFT) 
					m_dc->Text( token, m_curXPos, m_curYPos );
				else
				{
					if ( m_curAlign == wxCENTER ) m_dc->Text( token, m_rect.x + m_rect.width/2 - width/2, m_curYPos ); // center
					else m_dc->Text( token, m_curXPos + m_rect.width - width, m_curYPos ); // right
				}
		
				m_curXPos += width;
			}
		}

		// move to next line if needed
		if ( cur_delim[0] != 0 )
		{
			m_curXPos = m_rect.x;
			m_curYPos += m_curLineHeight;
		}
	}
}

void wxPLFormattedOutputHelper::RenderImage( const wxImage &img, double width, double height )
{
	if ( !m_dc ) return;
	m_dc->Image( img, wxPLRealRect(m_curXPos, m_curYPos, width, height) );
	m_curYPos += height;
}

struct cellgeom
{
	double width, height;
};

void wxPLFormattedOutputHelper::RenderTable( const wxMatrix<wxString> &tab )
{
	if (!m_dc) return;
		
	double tab_x = m_curXPos;
	double tab_y = m_curYPos;
	double tab_width = 0.0f;
	double tab_height = 0.0f;

	wxMatrix<cellgeom> cellsize( tab.Rows(), tab.Cols(), cellgeom() );
	for (int r=0;r<(int)tab.Rows();r++)
	{
		if (r == 0) Font( m_headerFace, m_headerSize, m_headerBold, false );
		else if (r == 1) Font( m_curFace, m_curSize, m_curBold, m_curItalic );

		for (int c=0;c<(int)tab.Cols();c++)
		{
			double width, height;
			m_dc->Measure( tab(r,c), &width, &height );
			cellsize(r,c).width = width;
			cellsize(r,c).height = height;
		}
	}

#define CELLSPACE_POINTS 6
	
	std::vector<double> col_widths( tab.Cols() );
	for (int c=0;c<(int)tab.Cols();c++)
	{
		col_widths[c] = 0;
		for (int r=0;r<(int)tab.Rows();r++)
			if (cellsize(r,c).width > col_widths[c])
				col_widths[c] = cellsize(r,c).width;

		col_widths[c] += CELLSPACE_POINTS;

		if ( c < (int)m_colSizes.size()
			&& m_colSizes[c] > 0)
			col_widths[c] = m_colSizes[c];

		tab_width += col_widths[c];
	}

	std::vector<double> row_heights( tab.Rows() );
	for (int r=0;r<(int)tab.Rows();r++)
	{
		row_heights[r] = 0;
		for (int c=0;c<(int)tab.Cols();c++)
			if (cellsize(r,c).height > row_heights[r])
				row_heights[r] = cellsize(r,c).height;

		row_heights[r] += CELLSPACE_POINTS;

		if ( r < (int)m_rowSizes.size()
			&& m_rowSizes[r] > 0)
			row_heights[r] = m_rowSizes[r];

		tab_height += row_heights[r];
	}

	if (m_curAlign == wxCENTER)
		tab_x = m_rect.x + m_rect.width/2 - tab_width/2;
	else if (m_curAlign == wxRIGHT)
		tab_x = m_rect.x + m_rect.width - tab_width;

	m_dc->NoPen(); // clear line style to default.
	
	for (int r=0;r<(int)tab.Rows();r++)
	{
		if ( r == 0 )
		{ // configure header text properties
			m_dc->TextColour( m_headerColour );
			Font( m_headerFace, m_headerSize, m_headerBold, false );
		}
		else if ( r == 1 )
		{ // return to normal text style
			m_dc->TextColour( m_curColour );
			Font( m_curFace, m_curSize, m_curBold, m_curItalic );
		}

		double xpos = tab_x;
		for (int c=0;c<(int)tab.Cols();c++)
		{
			int align = (r==0) ? m_headerAlign : m_cellAlign;
			switch( align )
			{
			case wxLEFT:
				m_dc->Text( tab(r,c), xpos + CELLSPACE_POINTS/2, 
					m_curYPos + CELLSPACE_POINTS/2 );
				break;
			case wxCENTER:
				m_dc->Text( tab(r,c), xpos + col_widths[c]/2 - cellsize(r,c).width/2,
					m_curYPos + CELLSPACE_POINTS/2 );
				break;
			case wxRIGHT:
				m_dc->Text( tab(r,c), xpos + col_widths[c] - cellsize(r,c).width - CELLSPACE_POINTS/2,
					m_curYPos + CELLSPACE_POINTS/2 );
				break;
			}
			xpos += col_widths[c];
			if (xpos > m_rect.x + m_rect.width) break;
		}

		m_curYPos += row_heights[r];
		if (m_curYPos > m_rect.y + m_rect.height) break;
	}
	
	if ( m_gridLines )
	{
		m_dc->Pen( *wxLIGHT_GREY, 0.3 );
		double xpos = tab_x;
		for (int c=0;c<(int)tab.Cols();c++)
		{
			if (c > 0) m_dc->Line( xpos, tab_y, xpos, tab_y + tab_height*0.99 );
			xpos += col_widths[c];
		}
	
		double ypos = tab_y;
		for (int r=0;r<(int)tab.Rows();r++)
		{
			if (r > 0) m_dc->Line( tab_x, ypos, tab_x+tab_width, ypos );
			ypos += row_heights[r];
		}
	}
	
	m_dc->Pen( *wxBLACK, 0.5 );
	if (m_headerLine && row_heights.size() > 0)
		m_dc->Line( tab_x, tab_y+row_heights[0], 
			tab_x+tab_width, tab_y+row_heights[0] );
	
	if (m_tableBorder)
	{
		m_dc->NoBrush();
		m_dc->Rect( tab_x, tab_y, tab_width, tab_height );
	}

	m_dc->Pen( m_curColour ); // restore original color, return to normal text style
	Font( m_curFace, m_curSize, m_curBold, m_curItalic );
}

struct fRect
{
	fRect() { x=y=width=height=0; }
	fRect( double _x, double _y, double _w, double _h ) : x(_x), y(_y), width(_w), height(_h) { }
	double x, y, width, height;
};

void wxPLFormattedOutputHelper::RenderBarGraph( const std::vector<double> &values, const wxArrayString &xlabels, const wxString &xlabel,
	const wxString &ylabel, const wxString &title, bool show_values, double xsize, double ysize, int decimals,
	const wxColour &color, bool show_yaxis_ticks, const wxString &ticks_format )
{
	if (!m_dc) return;

	double gr_x = m_curXPos;
	double gr_y = m_curYPos;
	double gr_width = xsize;
	double gr_height = ysize;
	
	if (m_curAlign == wxCENTER)
		gr_x = m_rect.x + m_rect.width/2 - gr_width/2;
	else if (m_curAlign == wxRIGHT)
		gr_x = m_rect.x + m_rect.width - gr_width;

	double ymin = 0;
	double ymax = 0;

	for (int i=0;i<(int)values.size();i++)
	{
		if (values[i] < ymin) ymin = values[i];
		if (values[i] > ymax) ymax = values[i];
	}

	if (ymin == 0 && ymax == 0) ymax = 1;

	ymin *= 1.1;
	ymax *= 1.1;
	
	wxPLLinearAxis yaxis( ymin, ymax );
	double physmax = gr_height; // in points already

	std::vector<wxPLAxis::TickData> ticks;
	if (show_yaxis_ticks)
		yaxis.GetAxisTicks( -1, physmax, ticks );
	else
		yaxis.GetAxisTicks(0, physmax, ticks);

	// save the current style data
	wxString saveFace;
	int saveSize, saveAlign;
	wxColour saveColour;
	bool saveBold, saveItalic, saveWrap;
	double saveWidth;
	wxPLOutputDevice::Style saveStyle;
	Style(&saveFace, &saveSize, &saveColour, &saveBold, &saveItalic, &saveAlign, &saveWidth, &saveStyle, &saveWrap );

	m_dc->NoPen(); // clear line style
	
	Font( wxFT_FONT_DEFAULT_FAMILY, 12, true, false );
	double yl_width = 0.0f, yl_height = 0.0f;
	if ( !ylabel.IsEmpty() )
		m_dc->Measure(ylabel, &yl_width, &yl_height);

	
	double xl_width = 0.0f, xl_height = 0.0f;
	if ( !xlabel.IsEmpty() )
		m_dc->Measure(xlabel, &xl_width, &xl_height);


	double ti_width = 0.0f, ti_height = 0.0f;
	if ( !title.IsEmpty() )
		m_dc->Measure(title, &ti_width, &ti_height);

	// draw labels & title
	m_dc->TextColour( *wxBLACK );
	if ( !title.IsEmpty() )
		m_dc->Text( title, gr_x+gr_width/2-ti_width/2, gr_y, 0 );

	Font( wxFT_FONT_DEFAULT_FAMILY, 12, false, false ); // turn off bold for axis labels
	if ( !ylabel.IsEmpty() )
		m_dc->Text( ylabel, gr_x, gr_y+gr_height/2+yl_width/2, 90 );

	if ( !xlabel.IsEmpty() )
		m_dc->Text( xlabel, gr_x+gr_width/2-xl_width/2, gr_y+gr_height-xl_height, 0 );

	// calculate max y tick width
	Font( wxFT_FONT_DEFAULT_FAMILY, 10, false, false ); // font for tick labels
	 
	double max_ytick_width = 0;
	std::vector<double> ytick_widths;
	wxArrayString ytick_labels;
	double ytick_height = 7; // 7 pt width
	m_dc->Measure("yh", 0, &ytick_height);
	for (size_t i=0;i<ticks.size();i++)
	{
		if ( ticks[i].size != wxPLAxis::TickData::LARGE )
			continue;

		wxString label;
		if (ticks_format != wxEmptyString)
			label = lk::format((const char *)ticks_format.c_str(), ticks[i].world);
		else if (decimals <= 0 && fabs(ticks[i].world)>999)
			label = wxNumericFormat( ticks[i].world, wxNUMERIC_REAL, wxNUMERIC_GENERIC, true, wxEmptyString, wxEmptyString );
		else if (decimals < 6)
			label = wxNumericFormat( ticks[i].world, wxNUMERIC_REAL, decimals, true, wxEmptyString, wxEmptyString );		
		else
			label = wxString::Format("%lg", ticks[i].world);

		double tw = 4; // pt
		m_dc->Measure(label, &tw, 0);
		if (tw > max_ytick_width) max_ytick_width = tw;

		ytick_labels.Add( label );
		ytick_widths.push_back( tw );
	}

	double max_xtick_width = 0;
	std::vector<double> xtick_widths;
	wxArrayString xtick_labels;
	for (size_t i=0;i<values.size();i++)
	{		
		wxString label;
		if ( i < xlabels.size() ) label = xlabels[i];
		double tw = 4; // pt
		m_dc->Measure( label, &tw, 0 );
		if (tw > max_xtick_width) max_xtick_width = tw;

		xtick_labels.Add( label );
		xtick_widths.push_back( tw );
	}

	double max_yvaltext_width = 0;
	double max_yvaltext_height = 0;
	std::vector<double> yvaltext_widths;
	wxArrayString yvaltext_labels;
	if (show_values)
	{
		for (int i=0;i<(int)values.size();i++)
		{
			wxString label;
			if (decimals <= 0 && fabs(values[i])>999)
				label = wxNumericFormat( values[i], wxNUMERIC_REAL, wxNUMERIC_GENERIC, true, wxEmptyString, wxEmptyString );
			else if (decimals < 6)
				label = wxNumericFormat( values[i], wxNUMERIC_REAL, decimals, true, wxEmptyString, wxEmptyString );		
			else
				label = wxString::Format("%lg", values[i] );

			double tw = 4;
			double th = 1;
			m_dc->Measure( label, &tw, &th );
			if (tw > max_yvaltext_width) max_yvaltext_width = tw;
			if (th > max_yvaltext_height) max_yvaltext_height = th;

			yvaltext_widths.push_back( tw );
			yvaltext_labels.Add( label );
		}
	}

	double max_xtick_height = ytick_height;

	fRect gbox;
	gbox.x = 7 + (ylabel.IsEmpty() ? 0 : yl_height) + max_ytick_width + 5;
	gbox.y = (title.IsEmpty() ? 0 : yl_height );
	gbox.width = gr_width-gbox.x;
		
	double xspacing = gbox.width * 0.95f / ( (double)values.size() );
	double bar_width = xspacing*0.85f;

	bool vertical_xtick_text = false;
	double height_reduction = 0;
	if (ymin==0) height_reduction = max_xtick_height;
	if ( max_xtick_width > xspacing )
	{
		height_reduction = max_xtick_width;
		vertical_xtick_text = true;
	}

	gbox.height = gr_height - (xlabel.IsEmpty() ? 0 : xl_height ) - (title.IsEmpty() ? 0 : ti_height) - height_reduction;

	bool vertical_yvaltext_text = false;
	if ( show_values && max_yvaltext_width > xspacing )
	{
		gbox.y += max_yvaltext_width;
		gbox.height -= max_yvaltext_width; // try to keep yval labels from overlapping graph title
		vertical_yvaltext_text = true;
	}
	else if ( show_values && max_yvaltext_height > 0 )
	{
		gbox.y += max_yvaltext_height/2;
		gbox.height -= max_yvaltext_height/2;
	}

	m_dc->Pen( *wxBLACK );

	double sfy = (gbox.height)/(ymax-ymin); // y scale: world_to_device
	double ory = ymax*sfy;

#define TO_DEVICE(y) (-1.0f*(y)*sfy + ory)
#define LINEOUT( x1, y1, x2, y2 ) m_dc->Line( gr_x+gbox.x+(x1), gr_y+gbox.y+(y1), gr_x+gbox.x+(x2), gr_y+gbox.y+(y2) )
#define FILLOUT( x1, y1, ww, hh ) m_dc->Rect( gr_x+gbox.x+(x1), gr_y+gbox.y+(y1), ww, hh )
#define TEXTOUT( xx, yy, ss, aa ) m_dc->Text( (ss), gr_x+gbox.x+(xx), gr_y+gbox.y+(yy), aa )

	for ( size_t i=0;i<ticks.size(); i++)
	{
		if ( ticks[i].size == wxPLAxis::TickData::LARGE )
		{
			double y = TO_DEVICE(ticks[i].world);
			LINEOUT( 0, y, 6, y );
			TEXTOUT( -ytick_widths[i]-2, y-ytick_height/2, ytick_labels[i], 0 );
		}
	}

	for ( size_t i=0;i<ticks.size(); i++)
	{
		if ( ticks[i].size == wxPLAxis::TickData::SMALL )
		{
			double y = TO_DEVICE(ticks[i].world);
			LINEOUT( 0, y, 3, y );
		}
	}

	m_dc->NoPen();
	m_dc->Brush( color );
	for ( size_t i=0;i<values.size();i++ )
	{
		double x = xspacing*0.8f + xspacing*i;
		double y0 = TO_DEVICE( values[i] );
		double y1 = TO_DEVICE( 0 );		
		FILLOUT( x-bar_width/2, y1 < y0 ? y1 : y0, bar_width, (double)fabs(y1-y0) );		
	}

	m_dc->Pen( *wxBLACK );
	m_dc->TextColour( *wxBLACK );
	double y0 = TO_DEVICE( 0 );		
	for ( size_t i=0;i<values.size();i++ )
	{
		double x = xspacing*0.8f + xspacing*i;
		LINEOUT( x, y0, x, y0-6 );		
		if ( vertical_xtick_text ) TEXTOUT( x-max_xtick_height/2, y0+xtick_widths[i] + 2, xtick_labels[i], 90);
		else TEXTOUT( x-xtick_widths[i]/2, y0 + 2, xtick_labels[i], 0 );

		if (show_values)
		{
			double yy = TO_DEVICE( values[i] );
			if (vertical_yvaltext_text)
				TEXTOUT( x-max_xtick_height/2, values[i] > 0 ? yy - 2 : yy+yvaltext_widths[i] + 2, yvaltext_labels[i], 90);
			else
				TEXTOUT( x-yvaltext_widths[i]/2, values[i] > 0 ? yy - max_xtick_height - 2 : yy + 2 , yvaltext_labels[i], 0 );
		}		
	}

	LINEOUT( 0, 0, 0, gbox.height );
	LINEOUT( 0, y0, gbox.width, y0 );
	

#undef TO_DEVICE
#undef LINEOUT
#undef TEXTOUT

	m_curXPos += gr_width;
	m_curYPos += gr_height;	

	// restore style information
	Style(saveFace, saveSize, saveColour, saveBold, saveItalic, saveAlign, saveWidth, saveStyle, saveWrap );
}


wxPLPdfOutputDevice::wxPLPdfOutputDevice( wxPdfDocument &doc ) 
	: wxPLOutputDevice(), m_pdf(doc)	
{
	m_textSize = 12.0;
	m_textSizeAdjust = 0.0;
	m_pen = m_brush = true;
	m_pdf.SetTextColour( *wxBLACK );
}

void wxPLPdfOutputDevice::SetAntiAliasing( bool )
{
	// nothing to do here for PDFs...
}

void wxPLPdfOutputDevice::Clip( double x, double y, double width, double height ) {
	m_pdf.ClippingRect( x, y, width, height );
}

void wxPLPdfOutputDevice::Unclip() {
	m_pdf.UnsetClipping();
}

void wxPLPdfOutputDevice::Pen( const wxColour &c, double size, 
	Style line, Style join, Style cap )
{
	if ( line == NONE ) {
		m_pen = false;
		return;
	}

	m_pen = true;

	wxPdfArrayDouble dash;
	wxPdfLineStyle style;
		
	style.SetColour( c );
		
	style.SetWidth( size );

	double dsize = size;
	if ( dsize < 1.5 ) dsize = 1.5;
		
	switch( line )
	{
	case DOT:
		dash.Add( dsize );
		dash.Add( dsize );
		break;
	case DASH:
		dash.Add( 2.0*dsize );
		dash.Add( dsize );
		break;
	case DOTDASH:
		dash.Add( dsize );
		dash.Add( dsize );
		dash.Add( 2.0*dsize );
		dash.Add( dsize );
		break;
	}
	style.SetDash( dash );

	switch( join )
	{
	case ROUND: style.SetLineJoin( wxPDF_LINEJOIN_ROUND ); break;
	case BEVEL: style.SetLineJoin( wxPDF_LINEJOIN_BEVEL ); break;
	default: style.SetLineJoin( wxPDF_LINEJOIN_MITER ); break;
	}
		
	switch( cap )
	{
	case ROUND: style.SetLineCap( wxPDF_LINECAP_ROUND ); break;
	default: style.SetLineCap( wxPDF_LINECAP_BUTT ); break;
	}

	m_pdf.SetLineStyle( style );
}

void wxPLPdfOutputDevice::Brush( const wxColour &c, Style sty ) {
	if ( sty == NONE )
	{
		m_brush = false;
		return;
	}

	// currently, hatch and other patterns not supported
	m_pdf.SetFillColour( c );
	m_brush = true;
}

void wxPLPdfOutputDevice::Line( double x1, double y1, double x2, double y2 )	{
	m_pdf.Line( x1, y1, x2, y2 );
}

void wxPLPdfOutputDevice::Lines( size_t n, const wxRealPoint *pts ) {
	for (size_t i = 0; i < n; ++i)
	{			
		if (i == 0) m_pdf.MoveTo( pts[i].x, pts[i].y );
		else m_pdf.LineTo( pts[i].x, pts[i].y );
	}
	m_pdf.EndPath(wxPDF_STYLE_DRAW);
}

int wxPLPdfOutputDevice::GetDrawingStyle()
{
	int style = wxPDF_STYLE_NOOP;
	if ( m_brush && m_pen ) style = wxPDF_STYLE_FILLDRAW;
	else if (m_pen) style = wxPDF_STYLE_DRAW;
	else if (m_brush) style = wxPDF_STYLE_FILL;
	return style;
}

void wxPLPdfOutputDevice::Polygon( size_t n, const wxRealPoint *pts, FillRule rule ) {		
	if ( n == 0 ) return;
	int saveFillingRule = m_pdf.GetFillingRule();
	m_pdf.SetFillingRule( rule==ODD_EVEN_RULE ? wxODDEVEN_RULE : wxWINDING_RULE );
	wxPdfArrayDouble xp(n, 0.0), yp(n, 0.0);
	for( size_t i=0;i<n;i++ )
	{
		xp[i] = pts[i].x;
		yp[i] = pts[i].y;
	}
	m_pdf.Polygon(xp, yp, GetDrawingStyle() );
	m_pdf.SetFillingRule(saveFillingRule);
}
void wxPLPdfOutputDevice::Rect( double x, double y, double width, double height ) {
	m_pdf.Rect( x, y, width, height, GetDrawingStyle() );
}

void wxPLPdfOutputDevice::Circle( double x, double y, double radius ) {
	m_pdf.Circle( x, y, radius, 0.0, 360.0, GetDrawingStyle() );
}

void wxPLPdfOutputDevice::Sector( double x, double y, double radius, double angle1, double angle2 ) {
	m_pdf.Sector( x, y, radius, angle1, angle2, GetDrawingStyle(), true, 90.0 );
}

void wxPLPdfOutputDevice::Arc( double x, double y, double width, double height, double angle1, double angle2 ) {
	m_pdf.Ellipse( x+width/2, y+height/2, width/2, height/2, 0, angle1, angle2, GetDrawingStyle() );
}

void wxPLPdfOutputDevice::Image( const wxImage &img, const wxPLRealRect &rct )
{
	static int m_imageIndex = 0;
	m_pdf.Image( wxString::Format("img_%d", ++m_imageIndex), img, rct.y, rct.x, rct.width, rct.height );
}

void wxPLPdfOutputDevice::MoveTo( double x, double y )
{
	m_shape.MoveTo( x, y );
}

void wxPLPdfOutputDevice::LineTo( double x, double y )
{
	m_shape.LineTo( x, y );
}

void wxPLPdfOutputDevice::CloseSubPath()
{
	m_shape.ClosePath();
}

void wxPLPdfOutputDevice::Path( FillRule rule )
{
	int saveFillingRule = m_pdf.GetFillingRule();
	m_pdf.SetFillingRule( rule==ODD_EVEN_RULE ? wxODDEVEN_RULE : wxWINDING_RULE );
	m_pdf.Shape( m_shape, GetDrawingStyle() );
	m_shape = wxPdfShape(); // clear the path for next one
	m_pdf.SetFillingRule( saveFillingRule );
}

	
void wxPLPdfOutputDevice::TextSize( double points ) {
	m_textSize = points;
}

void wxPLPdfOutputDevice::TextAdjust( double relative )
{
	m_textSizeAdjust = relative;
}

double wxPLPdfOutputDevice::TextSize( ) const {
	return m_textSize;
}

#include <wex/pdf/pdffontmanager.h>

bool wxPLPdfOutputDevice::TextFace( const wxString &face, bool bold, bool italic )
{
	wxPdfFontManager *fmng = wxPdfFontManager::GetFontManager();
	
	int style = wxPDF_FONTSTYLE_REGULAR;
	if ( bold ) style |= wxPDF_FONTSTYLE_BOLD;
	if ( italic ) style |= wxPDF_FONTSTYLE_ITALIC;

	wxPdfFont font( fmng->GetFont( face, style ) );
	if ( !font.IsValid() )
	{
		int ifont = wxFreeTypeFindClosestFont( face, bold, italic );

		if ( ifont >= 0 )
		{
			wxString file( wxFreeTypeFontDataFile( ifont ) );
			if ( wxFileExists( file ) )
			{
				font = fmng->RegisterFont(file);
			}
			else
			{
				wxString tempfile = wxFileName::GetTempDir() + "/font.otf";
				wxFreeTypeWriteBuiltinFontData( ifont, tempfile );

				font = fmng->RegisterFont( tempfile );
				wxRemoveFile( tempfile );
			}
		}

	}

	if ( font.IsValid() )
	{
		m_pdf.SetFont( font );
		return true;
	}
	else
	{
		m_pdf.SetFont( "Times New Roman", wxPDF_FONTSTYLE_REGULAR, 11 );
		return false;
	}
}

void wxPLPdfOutputDevice::TextColour( const wxColour &c ) {	
	m_pdf.SetTextColour( c );	
}

void wxPLPdfOutputDevice::Text( const wxString &text, double x, double y,  double angle ) {		
	double points = m_textSize+m_textSizeAdjust;
	m_pdf.SetFontSize( points );
	double asc = (double)abs(m_pdf.GetFontDescription().GetAscent());
	double des = (double)abs(m_pdf.GetFontDescription().GetDescent());
	double em = asc+des;
	double ascfrac = asc/em;
	double ybase = points*ascfrac;

	if ( fabs(angle) < 0.5)
	{
		m_pdf.Text( x, y + ybase, text );
	}
	else
	{
		double xx = x + ybase*sin( angle*M_PI/180 );
		double yy = y + ybase*cos( angle*M_PI/180 );
		m_pdf.RotatedText( xx, yy, text, angle );
	}
}

void wxPLPdfOutputDevice::Measure( const wxString &text, double *width, double *height ) {
	m_pdf.SetFontSize( m_textSize+m_textSizeAdjust );
	if (width) *width = m_pdf.GetStringWidth(text);
	if (height) *height = m_pdf.GetFontSize();
}

#define CAST(x) ((int)wxRound(m_scale*(x)))

static void TranslateBrush( wxBrush *b, const wxColour &c, wxPLOutputDevice::Style sty )
{
	b->SetColour( c );
	switch( sty )
	{
	case wxPLOutputDevice::NONE: *b = *wxTRANSPARENT_BRUSH; break;
	case wxPLOutputDevice::HATCH: b->SetStyle(wxBRUSHSTYLE_CROSSDIAG_HATCH); break;
	default: b->SetStyle( wxBRUSHSTYLE_SOLID ); break;
	}
}

static void TranslatePen( wxPen *p, const wxColour &c, double size, 
	wxPLOutputDevice::Style line, wxPLOutputDevice::Style join, wxPLOutputDevice::Style cap )
{	
	p->SetColour( c );
	p->SetWidth( size < 1.0 ? 1 : ((int)size) );

	switch( join )
	{
	case wxPLOutputDevice::MITER: p->SetJoin( wxJOIN_MITER ); break;
	case wxPLOutputDevice::BEVEL: p->SetJoin( wxJOIN_BEVEL ); break;
	default: p->SetJoin( wxJOIN_ROUND ); break;
	}
	switch( cap )
	{
	case wxPLOutputDevice::ROUND: p->SetCap( wxCAP_ROUND ); break;
	case wxPLOutputDevice::MITER: p->SetCap( wxCAP_PROJECTING ); break;
	default: p->SetCap( wxCAP_BUTT );
	}
	switch( line )
	{
	case wxPLOutputDevice::NONE: p->SetStyle( wxPENSTYLE_TRANSPARENT ); break;
	case wxPLOutputDevice::DOT: p->SetStyle( wxPENSTYLE_DOT ); break;
	case wxPLOutputDevice::DASH: p->SetStyle( wxPENSTYLE_SHORT_DASH ); break;
	case wxPLOutputDevice::DOTDASH: p->SetStyle( wxPENSTYLE_DOT_DASH ); break;
	default: p->SetStyle( wxPENSTYLE_SOLID ); break;
	}
}


/*
wxPLDCOutputDevice::wxPLDCOutputDevice( wxDC *dc, wxDC *aadc, double scale ) 
	: wxPLOutputDevice(), m_dc(dc), m_aadc(aadc), m_curdc(dc),
		m_pen( *wxBLACK_PEN ), m_brush( *wxBLACK_BRUSH ), 
		m_font0( m_dc->GetFont() ), m_scale( scale )
{
	m_fontSize = 0;
	m_fontBold = ( m_font0.GetWeight() == wxFONTWEIGHT_BOLD );
}


void wxPLDCOutputDevice::SetAntiAliasing( bool on )
{
	if ( 0 != m_aadc )
	{
		if ( on )
		{
			m_aadc->SetPen( m_curdc->GetPen() );
			m_aadc->SetBrush( m_curdc->GetBrush() );
			m_aadc->SetFont( m_curdc->GetFont() );
			m_curdc = m_aadc;
		}
		else
		{
			m_dc->SetPen( m_curdc->GetPen() );
			m_dc->SetBrush( m_curdc->GetBrush() );
			m_dc->SetFont( m_curdc->GetFont() );
			m_curdc = m_dc;
		}
	}
}

wxDC *wxPLDCOutputDevice::GetDC() { return m_curdc; }
	
void wxPLDCOutputDevice::Clip( double x, double y, double width, double height ) { 
	m_curdc->SetClippingRegion( CAST(x), CAST(y), CAST(width), CAST(height) );
}

void wxPLDCOutputDevice::Unclip() {
	m_curdc->DestroyClippingRegion();
}

void wxPLDCOutputDevice::Brush( const wxColour &c, Style sty ) { 

	TranslateBrush( &m_brush, c, sty );
	m_curdc->SetBrush( m_brush );
}

void wxPLDCOutputDevice::Pen( const wxColour &c, double size, 
	Style line, Style join, Style cap ) {

	TranslatePen( &m_pen, c, m_scale*size, line, join, cap );
	m_curdc->SetPen( m_pen );
}

void wxPLDCOutputDevice::Line( double x1, double y1, double x2, double y2 ) {
	m_curdc->DrawLine( CAST(x1), CAST(y1), CAST(x2), CAST(y2) );
}

void wxPLDCOutputDevice::Lines( size_t n, const wxRealPoint *pts ) {
	if ( n == 0 ) return;
	std::vector<wxPoint> ipt( n, wxPoint(0,0) );
	for( size_t i=0;i<n;i++ )
		ipt[i] = wxPoint( CAST(pts[i].x), CAST(pts[i].y) );

	m_curdc->DrawLines( n, &ipt[0] );
}

void wxPLDCOutputDevice::Polygon( size_t n, const wxRealPoint *pts, Style sty ) {
	if ( n == 0 ) return;
	std::vector<wxPoint> ipt( n, wxPoint(0,0) );
	for( size_t i=0;i<n;i++ )
		ipt[i] = wxPoint( CAST(pts[i].x), CAST(pts[i].y) );

	m_curdc->DrawPolygon( n, &ipt[0], 0, 0, sty==ODDEVEN ?  wxODDEVEN_RULE : wxWINDING_RULE );
}
void wxPLDCOutputDevice::Rect( double x, double y, double width, double height ) {
	m_curdc->DrawRectangle( CAST(x), CAST(y), CAST(width), CAST(height) );
}
void wxPLDCOutputDevice::Circle( double x, double y, double radius ) {
	int irad = CAST(radius);
	if ( irad < 1 ) m_curdc->DrawPoint( CAST(x), CAST(y) );
	else m_curdc->DrawCircle( CAST(x), CAST(y), irad );
}
	
void wxPLDCOutputDevice::Font( double relpt, bool bold ) {
	wxFont font( m_font0 );
	if ( relpt != 0 ) font.SetPointSize( font.GetPointSize() + CAST(relpt) );
	font.SetWeight( bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL );
	m_curdc->SetFont( font );
	m_fontSize = relpt;
	m_fontBold = bold;
}
	
void wxPLDCOutputDevice::Font( double *rel, bool *bld ) const {
	if ( rel ) *rel = m_fontSize;
	if ( bld ) *bld = m_fontBold;
}

void wxPLDCOutputDevice::Text( const wxString &text, double x, double y, double angle ) {
	if ( angle != 0 ) m_curdc->DrawRotatedText( text, CAST(x), CAST(y), angle );
	else m_curdc->DrawText( text, CAST(x), CAST(y) );
}
void wxPLDCOutputDevice::Measure( const wxString &text, double *width, double *height ) {
	wxSize sz( m_curdc->GetTextExtent( text ) );
	if ( width )  *width = (double)sz.x/m_scale;
	if ( height ) *height = (double)sz.y/m_scale;
}
*/



#define SCALE(x) (m_scale*(x))



wxPLGraphicsOutputDevice::wxPLGraphicsOutputDevice( wxGraphicsContext *gc )
	: wxPLOutputDevice(), m_gc(gc), m_scale( 1.0 )
{
	m_textSize = 12.0;
	m_textSizeAdjust = 0;
	m_textColour = *wxBLACK;
	m_fontIndex = 0;

	m_pen = m_brush = true;
	Pen( *wxBLACK, 1, SOLID );
	Brush( *wxBLACK, SOLID );
	m_path = gc->CreatePath();
}
	
void wxPLGraphicsOutputDevice::SetAntiAliasing( bool b )
{	
	m_gc->SetAntialiasMode( b ? wxANTIALIAS_DEFAULT : wxANTIALIAS_NONE );
}

bool wxPLGraphicsOutputDevice::GetAntiAliasing() const
{
	return m_gc->GetAntialiasMode() != wxANTIALIAS_NONE;
}

void wxPLGraphicsOutputDevice::Clip( double x, double y, double width, double height )
{
	m_gc->Clip( SCALE(x), SCALE(y), SCALE(width), SCALE(height) );
}

void wxPLGraphicsOutputDevice::Unclip()
{
	m_gc->ResetClip();
}

void wxPLGraphicsOutputDevice::Brush( const wxColour &c, Style sty )
{
	wxBrush brush;
	TranslateBrush( &brush, c, sty );
	m_gc->SetBrush( brush );
	m_brush = (sty!=NONE);
}

void wxPLGraphicsOutputDevice::Pen( const wxColour &c, double size, 
	Style line, Style join, Style cap )
{
	wxPen pen;
	TranslatePen( &pen, c, m_scale*size, line, join, cap );

	// smallest line in pixels supported by Direct2D graphics backend
	// ref: http://trac.wxwidgets.org/changeset/77694/svn-wx
	if ( line!=NONE && pen.GetWidth() < 1 ) 
		pen.SetWidth( 1 );

	m_gc->SetPen( pen );
	m_pen = (line!=NONE);
}

void wxPLGraphicsOutputDevice::Line( double x1, double y1, double x2, double y2 )
{
    m_gc->StrokeLine( SCALE(x1),SCALE(y1),SCALE(x2),SCALE(y2) );
}

void wxPLGraphicsOutputDevice::Lines( size_t n, const wxRealPoint *pts )
{
    wxPoint2DDouble* pointsD = new wxPoint2DDouble[n];
    for( size_t i = 0; i < n; ++i)
    {
        pointsD[i].m_x = SCALE(pts[i].x);
        pointsD[i].m_y = SCALE(pts[i].y);
    }

    m_gc->StrokeLines( n , pointsD );
    delete[] pointsD;
}

void wxPLGraphicsOutputDevice::Polygon( size_t n, const wxRealPoint *pts, FillRule sty )
{
    bool closeIt = false;
    if (pts[n-1] != pts[0])
        closeIt = true;
	
    wxPoint2DDouble* pointsD = new wxPoint2DDouble[n+(closeIt?1:0)];
    for( size_t i = 0; i < n; ++i)
    {
        pointsD[i].m_x = SCALE(pts[i].x);
        pointsD[i].m_y = SCALE(pts[i].y);
    }
    if ( closeIt )
        pointsD[n] = pointsD[0];
	
	m_gc->DrawLines( n+(closeIt?1:0), pointsD, sty==ODD_EVEN_RULE ?  wxODDEVEN_RULE : wxWINDING_RULE );
	delete[] pointsD;
}

void wxPLGraphicsOutputDevice::Rect( double x, double y, double width, double height )
{
	m_gc->DrawRectangle( SCALE(x), SCALE(y), SCALE(width), SCALE(height) );
}

void wxPLGraphicsOutputDevice::Circle( double x, double y, double radius )
{
	wxGraphicsPath path = m_gc->CreatePath();
	path.AddCircle( SCALE(x), SCALE(y), SCALE(radius) );
	m_gc->DrawPath( path );
}

void wxPLGraphicsOutputDevice::Image( const wxImage &img, const wxPLRealRect &rct )
{
	double img_width_inches = rct.width < 1 ? img.GetWidth() / 72.0f : rct.width;
	double img_height_inches = rct.height < 1 ? img.GetHeight() / 72.0f : rct.height;
				
	double dpix, dpiy;
	m_gc->GetDPI( &dpix, &dpiy );
	double ppi = 0.5*(dpix+dpiy);
	
	int pix_width = (int)( m_scale * img_width_inches * ppi );
	int pix_height = (int)( m_scale * img_height_inches * ppi );

	m_gc->DrawBitmap( img.Scale( pix_width, pix_height, ::wxIMAGE_QUALITY_NORMAL ),
		SCALE(rct.x), SCALE(rct.y), pix_width, pix_height );
}

void wxPLGraphicsOutputDevice::Sector( double x, double y, double radius, double angle1, double angle2 )
{
	double sa = (angle1-90.0);
	double ea = (angle2-90.0);

	wxGraphicsPath path = m_gc->CreatePath();
	path.AddArc( SCALE(x), SCALE(y), SCALE(radius), wxDegToRad(sa), wxDegToRad(ea), true );
	path.AddLineToPoint( SCALE(x), SCALE(y) );
	m_gc->DrawPath( path );
}

void wxPLGraphicsOutputDevice::Arc( double x, double y, double width, double height, double angle1, double angle2 )
{
	if ( angle1 == angle2 ) angle2 += 360;
	
	double dx = x + width / 2.0;
	double dy = y + height / 2.0;
	double factor = ((wxDouble) width) / height;
	m_gc->PushState();
	m_gc->Translate(SCALE(dx), SCALE(dy));
	m_gc->Scale(factor, 1.0);
	
	wxGraphicsPath path = m_gc->CreatePath();
	
	if ( m_brush )
    {
        path.MoveToPoint( 0, 0 );
        path.AddArc( 0, 0, SCALE(height/2.0), wxDegToRad(-angle1), wxDegToRad(-angle2), false );
        path.AddLineToPoint( 0, 0 );
        m_gc->FillPath( path );

        path = m_gc->CreatePath();
        path.AddArc( 0, 0, SCALE(height/2.0), wxDegToRad(-angle1), wxDegToRad(-angle2), false );
        m_gc->StrokePath( path );
    }
    else
    {
        path.AddArc( 0, 0, SCALE(height/2.0), wxDegToRad(-angle1), wxDegToRad(-angle2), false );
        m_gc->DrawPath( path );
    }

	m_gc->PopState(); 
}

void wxPLGraphicsOutputDevice::MoveTo( double x, double y )
{
	m_path.MoveToPoint( SCALE(x), SCALE(y) );
}

void wxPLGraphicsOutputDevice::LineTo( double x, double y )
{
	m_path.AddLineToPoint( SCALE(x), SCALE(y) );
}

void wxPLGraphicsOutputDevice::CloseSubPath()
{
	m_path.CloseSubpath();
}

void wxPLGraphicsOutputDevice::Path( FillRule rule )
{
	if ( m_brush && m_pen ) m_gc->DrawPath( m_path, rule == WINDING_RULE ? wxWINDING_RULE : wxODDEVEN_RULE );
	else if ( m_pen ) m_gc->StrokePath( m_path );
	else if ( m_brush ) m_gc->FillPath( m_path, rule == WINDING_RULE ? wxWINDING_RULE : wxODDEVEN_RULE );

	m_path = m_gc->CreatePath();
}

void wxPLGraphicsOutputDevice::TextColour( const wxColour &c )
{
	m_textColour = c;
}
void wxPLGraphicsOutputDevice::TextSize( double points )
{
	m_textSize = points;
}

bool wxPLGraphicsOutputDevice::TextFace( const wxString &face, bool bold, bool italic )
{
	m_fontIndex = wxFreeTypeFindClosestFont( face, bold, italic );
	return ( m_fontIndex > 0 );
}

void wxPLGraphicsOutputDevice::TextAdjust( double relpt )
{
	m_textSizeAdjust = relpt;
}

double wxPLGraphicsOutputDevice::TextSize() const
{
	return m_textSize;
}


#define FREETYPE_TEXT 1

#include <wex/utils.h>

void wxPLGraphicsOutputDevice::Text( const wxString &text, double x, double y, double angle )
{
#ifdef FREETYPE_TEXT
	wxPoint pos( (int)SCALE(x), (int)SCALE(y) );
	unsigned int dpi = wxGetDrawingDPI();
	wxFreeTypeDraw( *m_gc, pos, m_fontIndex, 
		m_textSize+m_textSizeAdjust, dpi, text, m_textColour, angle );
#else
	if ( angle == 0.0 ) m_gc->DrawText( text, SCALE(x), SCALE(y) );
	else m_gc->DrawText( text, SCALE(x), SCALE(y), angle * 3.1415926/180.0 );
#endif
}


void wxPLGraphicsOutputDevice::Measure( const wxString &text, double *width, double *height )
{
#ifdef FREETYPE_TEXT
	unsigned int dpi = wxGetDrawingDPI();
	wxRealPoint sz = wxFreeTypeMeasure( m_fontIndex, 
		m_textSize+m_textSizeAdjust, dpi, text );

	if ( width ) *width = (sz.x+0.5)/m_scale;
	if ( height ) *height = (sz.y+0.5)/m_scale;
#else
    wxDouble h , d , e , w;
    m_gc->GetTextExtent( text, &w, &h, &d, &e );
    if ( height )
        *height = (wxCoord)(h+0.5)/m_scale;
    
	//if ( descent )
    //    *descent = (wxCoord)(d+0.5);
    //if ( externalLeading )
    //    *externalLeading = (wxCoord)(e+0.5);

    if ( width )
        *width = (wxCoord)(w+0.5)/m_scale;
#endif
}
