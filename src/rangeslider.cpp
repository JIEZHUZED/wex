#include <memory>

#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/menu.h>
#include <wx/tokenzr.h>

#include <wex/metro.h>
#include <wex/utils.h>
#include <wex/mtrand.h>

#include <wex/rangeslider.h>

#define TITLE_FONT_SIZEUP 3
#define POINTER_HEIGHT_PIXELS 7

static const int viewable_range[] = { 1, 3, 6, 12, 24, 60, 120 };
static const int n_viewable_range = sizeof(viewable_range)/sizeof(int);

enum { _idFirst = wxID_HIGHEST + 194,
	ID_VIEW_RANGE, ID_VIEW_RANGE_LAST = ID_VIEW_RANGE+10,
	ID_SCROLL_LEFT, ID_SCROLL_RIGHT, ID_UPDATE_TIMER };

BEGIN_EVENT_TABLE( wxDateRangeSlider, wxWindow )
	EVT_CHAR( wxDateRangeSlider::OnChar )
	EVT_MENU_RANGE( ID_VIEW_RANGE, ID_VIEW_RANGE_LAST, wxDateRangeSlider::OnCommand )
	EVT_MENU( ID_SCROLL_LEFT, wxDateRangeSlider::OnCommand )
	EVT_MENU( ID_SCROLL_RIGHT, wxDateRangeSlider::OnCommand )
	EVT_BUTTON( ID_SCROLL_LEFT, wxDateRangeSlider::OnCommand )
	EVT_BUTTON( ID_SCROLL_RIGHT, wxDateRangeSlider::OnCommand )
	EVT_LEFT_DOWN( wxDateRangeSlider::OnLeftDown )
	EVT_LEFT_UP( wxDateRangeSlider::OnLeftUp )
	EVT_LEFT_DCLICK( wxDateRangeSlider::OnDoubleClick )
	EVT_MOTION( wxDateRangeSlider::OnMotion )
	EVT_RIGHT_DOWN( wxDateRangeSlider::OnRightDown )
	EVT_PAINT( wxDateRangeSlider::OnPaint )
	EVT_ERASE_BACKGROUND( wxDateRangeSlider::OnErase )
	EVT_SIZE( wxDateRangeSlider::OnSize )
	EVT_TIMER( ID_UPDATE_TIMER, wxDateRangeSlider::OnTimer )
END_EVENT_TABLE()

wxDateRangeSlider::wxDateRangeSlider( wxWindow *parent, int id, const wxString &label )
	: wxWindow( parent, id, wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN ),
	m_border( 5, 5 ),
	m_liveUpdate( true ),
	m_cursorBorder( 4, 4 ),
	m_movingCursor( 0 ),
	m_label(label),
	m_anchor(0),
	m_dragging(false),
	m_sendEventPending(false),
	m_updateTimer( this, ID_UPDATE_TIMER )
{
	SetBackgroundStyle( wxBG_STYLE_CUSTOM );
	
	m_start = wxDateTime( 1, wxDateTime::Dec, 2016, 0, 0, 0 );
	m_viewableMonths = 3; // six month viewable range
	
	m_leftCursor.DT =  wxDateTime( 17, wxDateTime::Jan, 2017, 12, 0 );
	m_rightCursor.DT = wxDateTime( 5, wxDateTime::Feb, 2017, 12, 0 );


	wxMTRand rng;
	wxDateTime dt = m_leftCursor.DT - wxDateSpan(0,0, 2, 3);
	while( dt < m_rightCursor.DT + wxDateSpan(0,0,5,1) )
	{
		AddDataPoint( dt, rng() );
		dt += wxTimeSpan( 3 );
	}


	m_btnLeft = new wxMetroButton( this, ID_SCROLL_LEFT, wxEmptyString, wxNullBitmap, 
		wxDefaultPosition, wxDefaultSize, wxMB_LEFTARROW );

	m_btnRight = new wxMetroButton( this, ID_SCROLL_RIGHT, wxEmptyString, wxNullBitmap, 
		wxDefaultPosition, wxDefaultSize, wxMB_RIGHTARROW );

	LayoutGeometry();
}

wxDateRangeSlider::~wxDateRangeSlider()
{
	/* nothing to do */
}


void wxDateRangeSlider::SetRange( const wxDateTime &start, int span )
{

	m_start = start;
	m_start.SetDay( 1 );
	m_start.SetHour( 0 );

	m_viewableMonths = span;

	Invalidate();
}

int wxDateRangeSlider::GetLabelSectionHeight( wxDC &dc ) const
{
	int y = 0;
	wxFont font( wxMetroTheme::Font( wxMT_LIGHT ) );
	if ( m_label.Len() > 0 )
	{
		wxFont lblfont( font );
		lblfont.SetPointSize( font.GetPointSize() + TITLE_FONT_SIZEUP );
		dc.SetFont( lblfont );

		y += dc.GetCharHeight();
		
		if ( m_label.Find( '\n' ) != wxNOT_FOUND )
			y += dc.GetCharHeight();
	}
	return y;
}

int wxDateRangeSlider::GetCursorSectionHeight( wxDC &dc ) const
{
	wxFont font( wxMetroTheme::Font( wxMT_LIGHT ) );
	dc.SetFont( font );
	return dc.GetCharHeight() + m_cursorBorder.y*2;
}

int wxDateRangeSlider::GetTimeViewSectionHeight( wxDC & ) const
{	
	wxSize lrbtnsz( m_btnLeft->GetBestSize() );
	int miny = (int)(30*wxGetScreenHDScale());
	if ( lrbtnsz.y > miny )
		miny = lrbtnsz.y;

	return miny;
}


wxSize wxDateRangeSlider::DoGetBestSize() const
{
	wxClientDC dc( const_cast<wxDateRangeSlider*>(this) );

	int y = m_border.y;

	y += GetLabelSectionHeight( dc );
	y += GetCursorSectionHeight( dc );
	y += 2*GetTimeViewSectionHeight( dc );

	y += m_border.y;

	return wxSize( (int)(1000*wxGetScreenHDScale()), y );
}


void wxDateRangeSlider::LayoutGeometry()
{
	wxClientDC dc( this );
	wxSize client( GetClientSize() );

	m_labelRect = wxRect( m_border.x, 
		m_border.y, 
		client.x - 2 * m_border.x, 
		GetLabelSectionHeight( dc ) );

	m_cursorRect = wxRect( m_border.x,
		m_labelRect.y + m_labelRect.height,
		m_labelRect.width,
		GetCursorSectionHeight( dc ) );

	m_cursorSize = wxSize( m_cursorRect.height, m_cursorRect.height );

	wxSize szl = m_btnLeft->GetBestSize();
	wxSize szr = m_btnRight->GetBestSize();
	int dmh = GetTimeViewSectionHeight( dc );

	m_dayViewRect = wxRect( m_border.x + szl.x,
		m_cursorRect.y + m_cursorRect.height,
		m_labelRect.width - szl.x - szr.x,
		dmh );

	m_monthViewRect = wxRect( m_border.x,
		m_dayViewRect.y + m_dayViewRect.height,
		m_cursorRect.width,
		dmh );

	m_btnLeft->SetSize( m_border.x, m_dayViewRect.y, szl.x, m_dayViewRect.height );
	m_btnRight->SetSize( client.x-m_border.x-szr.x, m_dayViewRect.y, szr.x, m_dayViewRect.height );


	
}

void wxDateRangeSlider::Invalidate()
{
	// recalculate geometry
	LayoutGeometry();

	// repaint
	Refresh();
}

void wxDateRangeSlider::OnErase( wxEraseEvent & )
{
	// don't do anything - just override default built-in behavior
}

void wxDateRangeSlider::OnPaint( wxPaintEvent & )
{
	wxAutoBufferedPaintDC _dc(this);
	wxGCDC dc( _dc );

	dc.SetBackground( *wxWHITE_BRUSH );
	dc.Clear();
	

	wxSize client( GetClientSize() );

	//dc.SetBrush( *wxTRANSPARENT_BRUSH );
	//dc.SetPen( *wxRED_PEN );
	//dc.DrawRectangle( m_labelRect );
	
	//dc.SetPen( *wxTRANSPARENT_PEN );
	//dc.SetBrush( *wxYELLOW_BRUSH );
	//dc.DrawRectangle( m_cursorRect );
	
	//dc.SetPen( *wxRED_PEN );
	//dc.SetBrush( *wxTRANSPARENT_BRUSH );
	//dc.DrawRectangle( m_dayViewRect );
	
	//dc.SetPen( *wxTRANSPARENT_PEN );
	//dc.SetBrush( *wxYELLOW_BRUSH );
	//dc.DrawRectangle( m_monthViewRect );

	// draw label and sublabel if it exists 
	wxArrayString tp( wxStringTokenize( m_label, "\n" ) );
	if ( tp.size() > 0 )
	{		
		wxFont font( wxMetroTheme::Font( wxMT_LIGHT ) );
		font.SetPointSize( font.GetPointSize() + TITLE_FONT_SIZEUP );
		dc.SetTextForeground( wxMetroTheme::Colour( wxMT_FOREGROUND ) );
		dc.SetFont( font );
		int y = m_labelRect.y;
		dc.DrawText( tp[0], m_labelRect.x, y );
		if ( tp.size() > 1 )
		{
			y += dc.GetCharHeight()+1;
			font.SetPointSize( font.GetPointSize() - TITLE_FONT_SIZEUP );
			dc.SetFont( font );
			dc.SetTextForeground( *wxLIGHT_GREY );
			dc.DrawText( tp[1], m_labelRect.x, y );
		}
	}
	
	
	wxFont font( *wxNORMAL_FONT );
	font.SetPointSize( font.GetPointSize()-1 );
	dc.SetFont( font );
	dc.SetTextForeground( *wxBLACK );
	dc.SetPen( *wxLIGHT_GREY_PEN );

	int txlast = 0;
	m_dayPos.clear();
	int ndays = DaysVisible();
	for ( int i=0; i <= ndays; i++ )
	{
		int x = DayOffsetToScreen( i );
		wxDateTime dtcur = m_start + wxTimeSpan(i*24+12);		
		m_dayPos.push_back( { dtcur, x } );
	}

	for( size_t i=0;i<m_dayPos.size();i++ )
	{
		int x = m_dayPos[i].x;
		wxDateTime dtcur = m_dayPos[i].DT;

		dc.DrawLine( x, 
			m_dayViewRect.y, 
			x, 
			m_dayViewRect.y 
				+ (dtcur.GetDay() == 1 ? m_dayViewRect.height*2 : m_dayViewRect.height ) );
		
		int next_month_x = 0;
		for( size_t j=i+1;j<m_dayPos.size();j++ )
		{
			if ( m_dayPos[j].DT.GetDay() == 1 )
			{
				next_month_x = m_dayPos[j].x;
				break;
			}
		}

		if ( x > txlast+2 )
		{
			wxString dstr( wxString::Format("%d",(int)dtcur.GetDay()) );
			wxSize dsz( dc.GetTextExtent( dstr ) );

			if ( x + dsz.x + 1 < next_month_x )
			{
				dc.DrawText( dstr,  
					x+1, m_monthViewRect.y+3 );
				txlast = x + dsz.x;
			}
		}

		
		if (dtcur.GetDay() == 1 )
		{
			wxString s(dtcur.Format( "%b %Y"));
			if ( x + dc.GetTextExtent(s).x < client.x )
				dc.DrawText( s, x+2, m_monthViewRect.y + m_monthViewRect.height - dc.GetCharHeight() );
		}

	}

	m_highlightRect = wxRect(
		DateTimeToScreen( m_leftCursor.DT ),
		m_dayViewRect.y,
		DateTimeToScreen( m_rightCursor.DT ) - DateTimeToScreen( m_leftCursor.DT ),
		m_dayViewRect.height );
	dc.SetBrush( wxBrush( wxColour(120,120,120,90) ) );
	dc.SetPen( *wxTRANSPARENT_PEN );
	dc.DrawRectangle( m_highlightRect );
	
	dc.SetBrush( wxBrush( wxColour(80,100,130,180) ) );
	dc.SetPen( *wxTRANSPARENT_PEN );
	DrawDataPlot( dc );


	dc.SetFont( wxMetroTheme::Font( wxMT_NORMAL ) );
	dc.SetTextForeground( *wxWHITE );
	dc.SetPen( *wxTRANSPARENT_PEN );
	dc.SetBrush( *wxBLACK_BRUSH );

	DrawCursor( dc, &m_leftCursor );
	DrawCursor( dc, &m_rightCursor );


}

void wxDateRangeSlider::DrawDataPlot( wxDC &dc )
{
	if ( m_data.size() < 2 ) return;
	wxDateTime data_start = m_data.front().dt;
	wxDateTime data_end = m_data.back().dt;
	wxDateTime view_start = GetRangeStart();
	wxDateTime view_end = GetRangeEnd();
	
	if ( data_end <= view_start
		|| data_start >= view_end )
		return;

	if ( data_start < view_start )
		data_start = view_start;

	if ( data_end > view_end )
		data_end = view_end;

	double dmin = 1e99;
	double dmax = -1e99;
	
	for( size_t i=0;i<m_data.size();i++ )
	{
		if ( m_data[i].val < dmin ) dmin = m_data[i].val;
		if ( m_data[i].val > dmax ) dmax = m_data[i].val;
	}

	if ( dmax == dmin ) return;

	double scale = m_dayViewRect.height / (dmax-dmin);


	double hours_visible = DaysVisible()*24;

	int xzero = m_dayViewRect.y + m_dayViewRect.height - (int)(0.0-dmin)*scale;
	if ( 0.0 < dmin ) xzero = m_dayViewRect.y+m_dayViewRect.height;
	else if ( 0.0 > dmax ) xzero = m_dayViewRect.y;

	std::vector<wxPoint> pts;
	
	for( size_t i=0;i<m_data.size();i++ )
	{

		wxTimeSpan delta = m_data[i].dt - view_start;
		double hour_delta = delta.GetHours();
		wxPoint p(
				m_dayViewRect.x + (int)(hour_delta * m_dayViewRect.width / hours_visible),
				m_dayViewRect.y + m_dayViewRect.height - (int)((m_data[i].val - dmin)*scale )
			);

		if ( i==0 )
			pts.push_back( wxPoint( p.x, xzero ) );

		if ( pts.size() > 1 && pts.back().x == p.x )
			continue;
		
		if ( m_data[i].dt < data_start
			|| m_data[i].dt > data_end )
			continue;

		pts.push_back( p );
	}

	if ( pts.size() > 0 )
		pts.push_back( wxPoint( pts.back().x, xzero ) );

	dc.DrawPolygon( pts.size(), &pts[0] );
}

void wxDateRangeSlider::DrawCursor( wxDC &dc, cursor *cc )
{
	wxDateTime pos = cc->DT;
	if ( pos < m_start || pos > GetRangeEnd()+wxTimeSpan(12) )
	{
		cc->R = wxRect(0,0,0,0);
		return;
	}

	wxSize client( GetClientSize() );

	int x = DateTimeToScreen( pos );

	int pointer_height = (int)(POINTER_HEIGHT_PIXELS*wxGetScreenHDScale());
	int yb = m_cursorRect.y + m_cursorRect.height;	
	int xoff = (cc==&m_rightCursor) ? m_cursorSize.x : -m_cursorSize.x;
	int xoff2 = (cc==&m_rightCursor) ? pointer_height : -pointer_height;

	wxPoint pts[5];
	pts[0] = wxPoint(x, m_cursorRect.y);
	pts[1] = wxPoint(x+xoff, m_cursorRect.y);
	pts[2] = wxPoint(x+xoff, yb );
	pts[3] = wxPoint(x+xoff2, yb );
	pts[4] = wxPoint(x, yb+pointer_height);
	dc.DrawPolygon( 5, pts );

	
	cc->R.x = x + ((cc==&m_rightCursor) ? 0 : -m_cursorSize.x);
	cc->R.y = m_cursorRect.y;
	cc->R.width = m_cursorSize.x;
	cc->R.height = m_cursorRect.height;

	wxString sday( wxString::Format( "%d", (int)cc->DT.GetDay() ) );
	wxSize ssz( dc.GetTextExtent( sday ) );
	dc.DrawText( sday, 
		cc->R.x + cc->R.width/2 - ssz.x/2, 
		m_cursorRect.y + m_cursorSize.y/2 - ssz.y/2 );

}

wxDateTime wxDateRangeSlider::GetRangeEnd()
{
	return m_start + wxDateSpan( 0, m_viewableMonths, 0, 0 );
}

void wxDateRangeSlider::SetSelection( const wxDateTime &min, const wxDateTime &max )
{
	if ( min.GetDateOnly() < max.GetDateOnly() )
	{
		m_leftCursor.DT = min;
		m_leftCursor.DT.ResetTime();
		m_leftCursor.DT.SetHour( 12 );

		m_rightCursor.DT = max;
		m_rightCursor.DT.ResetTime();
		m_rightCursor.DT.SetHour( 12 );

		Refresh();		
	}
}

void wxDateRangeSlider::GetSelection( wxDateTime &min, wxDateTime &max )
{
	min = m_leftCursor.DT;
	min.ResetTime();

	max = m_rightCursor.DT;
	max.ResetTime();
}

void wxDateRangeSlider::ClearData()
{
	m_data.clear();
}

void wxDateRangeSlider::AddDataPoint( const wxDateTime &dt, double val )
{
	if ( m_data.size() > 0 
		&& m_data.back().dt >= dt )
		return;

	m_data.push_back( { dt, std::isnan(val) ? 0.0 : val } );
}


int wxDateRangeSlider::DaysVisible()
{
	wxTimeSpan span = GetRangeEnd() - m_start;
	return span.GetDays();
}

int wxDateRangeSlider::DateTimeToScreen( const wxDateTime &dt )
{
	wxTimeSpan span = dt-m_start;
	return DayOffsetToScreen( span.GetDays() );
}

int wxDateRangeSlider::DayOffsetToScreen( int day )
{
	return m_dayViewRect.x + (int)((double)(day * m_dayViewRect.width) / (double)DaysVisible());
}

wxDateTime wxDateRangeSlider::ScreenToNearestDateTime( int x )
{
	size_t idx = 0;
	double dist = 1e99;
	for( size_t i=0;i<m_dayPos.size();i++ )
	{
		double d = fabs( (double)(x-m_dayPos[i].x) );
		if ( d < dist )
		{
			dist = d;
			idx = i;
		}
	}

	return m_dayPos[idx].DT;
}

void wxDateRangeSlider::OnSize( wxSizeEvent & )
{
	Invalidate();
}

void wxDateRangeSlider::OnChar( wxKeyEvent &evt )
{
	switch( evt.GetKeyCode() )
	{
	case WXK_LEFT:
		m_start -= wxDateSpan( 0, 1, 0, 0 );
		Invalidate();
		break;

	case WXK_RIGHT:
		m_start += wxDateSpan( 0, 1, 0, 0 );
		Invalidate();
		break;
	}
}

void wxDateRangeSlider::OnLeftDown( wxMouseEvent &evt )
{
	wxPoint p( evt.GetPosition() );
	SetFocus(); // take keyboard focus.
	
	m_movingCursor = InCursor( p );

	if ( m_monthViewRect.Contains(p) )
	{
		wxDateTime dt( ScreenToNearestDateTime( p.x ) );
		dt.SetDay( 1 );

		m_leftCursor.DT = dt;		
		m_rightCursor.DT = dt + wxDateSpan(0,1,0,0);
		Refresh();
		if ( !m_sendEventPending ) SendChangeEvent();
	}

	if ( m_highlightRect.Contains(p) )
	{
		CaptureMouse();
		m_dragging = true;
		m_anchor = p.x;
		m_dragLeftStart = m_leftCursor.DT;
		m_dragRightStart = m_rightCursor.DT;
	}
}

wxDateRangeSlider::cursor *wxDateRangeSlider::InCursor( const wxPoint &p )
{
	if ( m_leftCursor.R.Contains(p) ) return &m_leftCursor;
	if ( m_rightCursor.R.Contains(p) ) return &m_rightCursor;

	return 0;
}

void wxDateRangeSlider::OnMotion( wxMouseEvent &evt )
{
	bool send_event = false;

	if ( m_movingCursor )
	{

		m_movingCursor->DT = ScreenToNearestDateTime( evt.GetX() );

		if ( m_movingCursor == &m_rightCursor
			&& m_rightCursor.DT <= m_leftCursor.DT )
			m_leftCursor.DT = m_rightCursor.DT - wxTimeSpan(24);

		if ( m_movingCursor == &m_leftCursor
			&& m_leftCursor.DT >= m_rightCursor.DT )
			m_rightCursor.DT = m_leftCursor.DT + wxTimeSpan(24);
		
		Refresh();
		send_event = true;
	}
	else if ( m_dragging )
	{
		int delta_pix = evt.GetX() - m_anchor;
		int delta_day = (int)( ( (double)delta_pix * DaysVisible()) / (double)m_dayViewRect.width );

		m_leftCursor.DT = m_dragLeftStart + wxTimeSpan( 24*delta_day );
		m_rightCursor.DT = m_dragRightStart + wxTimeSpan( 24*delta_day );
		Refresh();
		send_event = true;
	}

	if ( m_liveUpdate && send_event )
	{
		m_sendEventPending = true;
		m_updateTimer.Stop();
		m_updateTimer.StartOnce( 5 );
	}
}

void wxDateRangeSlider::OnLeftUp( wxMouseEvent & )
{
	if ( m_movingCursor )
	{
		m_movingCursor = 0;
		Invalidate();
		if ( !m_sendEventPending ) SendChangeEvent();
	}

	if ( m_dragging )
	{
		ReleaseMouse();
		m_dragging = false;
		if ( !m_sendEventPending ) SendChangeEvent();
	}
}

void wxDateRangeSlider::OnDoubleClick( wxMouseEvent & )
{
	if ( m_data.size() < 2 ) return;

	wxDateTime dt1( m_data.front().dt );
	wxDateTime dt2( m_data.back().dt );


	if ( dt1.IsValid() && dt2.IsValid() && dt1 < dt2 )
	{
		SetRange( dt1, 1+(dt2-dt1).GetDays()/30 );
		SetSelection( dt1, dt2 );
	}
}

void wxDateRangeSlider::OnTimer( wxTimerEvent & )
{
	SendChangeEvent();
}


void wxDateRangeSlider::OnCommand( wxCommandEvent &evt )
{
	if ( evt.GetId() >= ID_VIEW_RANGE && evt.GetId() < ID_VIEW_RANGE_LAST )
	{
		m_viewableMonths = viewable_range[evt.GetId() - ID_VIEW_RANGE];
		Invalidate();
	}
	else if ( evt.GetId() == ID_SCROLL_LEFT )
	{
		m_start -= wxDateSpan( 0, 1, 0, 0 );
		Invalidate();
	}
	else if ( evt.GetId() == ID_SCROLL_RIGHT )
	{
		m_start += wxDateSpan( 0, 1, 0, 0 );
		Invalidate();
	}

}

void wxDateRangeSlider::OnRightDown( wxMouseEvent & )
{
	wxMenu menu;
	for( int i=0;i<n_viewable_range;i++ )
		menu.Append( ID_VIEW_RANGE+i, wxString::Format("%d months",viewable_range[i]) );

	menu.AppendSeparator();
	menu.Append( ID_SCROLL_LEFT, "Previous" );
	menu.Append( ID_SCROLL_RIGHT, "Next" );

	PopupMenu( &menu );
}

void wxDateRangeSlider::SendChangeEvent()
{
	wxDateTime min, max;
	GetSelection( min, max );
	
	if ( min.IsValid() && max.IsValid() &&
		(	!m_lastEventDateStart.IsValid()
			|| !m_lastEventDateEnd.IsValid()
			|| min != m_lastEventDateStart
			|| max != m_lastEventDateEnd) )
	{	
		wxCommandEvent evt( wxEVT_SLIDER, GetId() );
		ProcessEvent( evt );
		
		m_lastEventDateStart = min;
		m_lastEventDateEnd = max;
	}

	m_sendEventPending = false;
}
