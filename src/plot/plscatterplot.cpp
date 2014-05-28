#include <algorithm>

#include <wx/dc.h>
#include "wex/plot/plscatterplot.h"

wxPLScatterPlot::wxPLScatterPlot()
{
	m_colour = *wxBLUE;
	m_size = 1;
	m_scale = false;
	m_antiAliasing = false;
	m_drawLineOfPerfectAgreement = false;
}

wxPLScatterPlot::wxPLScatterPlot( const std::vector<wxRealPoint> &data,
	const wxString &label,
	const wxColour &col,
	int size,
	bool scale )
	: wxPLPlottable( label )
{
	m_data = data;
	m_colour = col;
	m_size = size;
	m_scale = scale;
	m_antiAliasing = false;
	m_drawLineOfPerfectAgreement = false;
}


wxPLScatterPlot::~wxPLScatterPlot()
{
	// nothing to do currently
}
	
wxRealPoint wxPLScatterPlot::At( size_t i ) const
{
	return m_data[i];
}

size_t wxPLScatterPlot::Len() const
{
	return m_data.size();
}

void wxPLScatterPlot::Draw( wxDC &dc, const wxPLDeviceMapping &map )
{
	dc.SetPen( wxPen( m_colour, m_size ) );
	dc.SetBrush( wxBrush( m_colour ) );

	wxRealPoint min = map.GetWorldMinimum();
	wxRealPoint max = map.GetWorldMaximum();

	for ( size_t i=0; i<Len(); i++ )
	{
		const wxRealPoint p = At(i);
		if ( p.x >= min.x && p.x <= max.x
			&& p.y >= min.y && p.y <= max.y )
			if ( m_size < 2 )
				dc.DrawPoint( map.ToDevice( p ) );			
			else
				dc.DrawCircle( map.ToDevice(p), m_size );
	}

	if (m_drawLineOfPerfectAgreement && !m_isLineOfPerfectAgreementDrawn)
	{
		m_isLineOfPerfectAgreementDrawn = true;
		dc.SetPen(wxPen(*wxBLACK, m_size));
		dc.SetBrush(wxBrush(*wxBLACK));
		wxRealPoint pstart;
		wxRealPoint pend;

		if (min.x <= min.y) 
		{ 
			pstart = wxRealPoint(min.y, min.y);
		}
		else
		{
			pstart = wxRealPoint(min.x, min.x);
		}

		if (max.x <= max.y)
		{
			pend = wxRealPoint(max.x, max.x);
		}
		else
		{
			pend = wxRealPoint(max.y, max.y);
		}

		dc.DrawLine(map.ToDevice(pstart), map.ToDevice(pend));
	}
}

void wxPLScatterPlot::DrawInLegend( wxDC &dc, const wxRect &rct)
{
	dc.SetPen( wxPen( m_colour, 1 ) );
	dc.SetBrush( wxBrush( m_colour ) );
	wxCoord rad = std::min( rct.width, rct.height );
	rad = rad/2 - 2;
	if ( rad < 2 ) rad = 2;
	dc.DrawCircle( rct.x+rct.width/2, rct.y+rct.height/2, rad );
}

void wxPLScatterPlot::SetLineOfPerfectAgreementFlag(bool flagValue)
{
	m_drawLineOfPerfectAgreement = flagValue;
	m_isLineOfPerfectAgreementDrawn = false;
}
