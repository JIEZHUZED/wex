#ifndef __rangeslider_h
#define __rangeslider_h

#include <vector>
#include <chrono>

#include <wx/timer.h>
#include <wx/window.h>

class wxMetroButton;

#define EVT_DATERANGESLIDER( id, handler ) EVT_SLIDER( id, handler )

class wxDateRangeSlider : public wxWindow
{
public:

	wxDateRangeSlider( wxWindow *parent, int id, const wxString &label=wxEmptyString );
	virtual ~wxDateRangeSlider();
		
	void SetRange( const wxDateTime &start, int span_months );
	wxDateTime GetRangeStart() { return m_start; }
	wxDateTime GetRangeEnd();
	int DaysVisible();

	void SetSelection( const wxDateTime &min, const wxDateTime &max );
	void GetSelection( wxDateTime &min, wxDateTime &max );

	void SetLiveUpdate( bool b );

	void ClearData();
	void AddDataPoint( const wxDateTime &dt, double value );


protected:
	void Invalidate();

	void OnChar( wxKeyEvent & );
	void OnCommand( wxCommandEvent & );
	void OnErase( wxEraseEvent & );
	void OnPaint( wxPaintEvent & );
	void OnSize( wxSizeEvent & );
	void OnLeftDown( wxMouseEvent & );
	void OnRightDown( wxMouseEvent & );
	void OnMotion( wxMouseEvent & );
	void OnLeftUp( wxMouseEvent & );
	void OnDoubleClick( wxMouseEvent & );
	void OnTimer( wxTimerEvent & );

	int DateTimeToScreen( const wxDateTime &dt );
	int DayOffsetToScreen( int day );
	wxDateTime ScreenToNearestDateTime( int x );

private:
	bool m_liveUpdate;
	wxTimer m_updateTimer;
	bool m_sendEventPending;

	wxSize m_border, m_cursorBorder;
	wxSize m_cursorSize;

	int m_anchor;
	bool m_dragging;
	wxDateTime m_dragLeftStart, m_dragRightStart;

	wxString m_label;

	wxRect m_labelRect;
	wxRect m_cursorRect;
	wxRect m_dayViewRect;
	wxRect m_monthViewRect;
	wxRect m_highlightRect;


	virtual wxSize DoGetBestSize() const;
	int GetLabelSectionHeight( wxDC &dc ) const;
	int GetCursorSectionHeight( wxDC &dc ) const;
	int GetTimeViewSectionHeight( wxDC &dc ) const;
	void LayoutGeometry();
	
	struct cursor
	{
		wxDateTime DT;
		wxRect R;
	};

	struct dayp
	{
		wxDateTime DT;
		int x;
	};

	std::vector<dayp> m_dayPos;

	void DrawCursor( wxDC &, cursor * );
	void DrawDataPlot( wxDC & );
	
	wxMetroButton *m_btnLeft, *m_btnRight;

	cursor *InCursor( const wxPoint &p );
	
	cursor m_leftCursor, m_rightCursor;

	wxDateTime m_start;
	int m_viewableMonths;
	cursor *m_movingCursor;

	struct dtval
	{
		wxDateTime dt;
		double val;
	};

	std::vector<dtval> m_data;

	void SendChangeEvent();
	wxDateTime m_lastEventDateStart, m_lastEventDateEnd;


	DECLARE_EVENT_TABLE();
};


#endif
