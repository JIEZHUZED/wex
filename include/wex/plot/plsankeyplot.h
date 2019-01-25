#ifndef __pl_sankeyplot_h
#define __pl_sankeyplot_h

#include "wex/plot/pltext.h"
#include "wex/plot/plplot.h"

class wxPLSankeyPlot : public wxPLPlottable
{
public:
	wxPLSankeyPlot();
	void SetupExample();
	void Add( double value, const wxString &label, bool baseline=false );

	void SetFormat( int deci, bool thousep );
	
	virtual wxRealPoint At( size_t i ) const;
	virtual size_t Len() const;
	virtual void Draw( wxPLOutputDevice &dc, const wxPLDeviceMapping &map );
	virtual void DrawInLegend( wxPLOutputDevice &dc, const wxPLRealRect &rct);
	virtual wxPLAxis *SuggestXAxis();
	virtual wxPLAxis *SuggestYAxis();

private:
	int m_decimals;
	bool m_thousep;

	struct sankey_item {
		sankey_item(double v, const wxString &l, bool b) :value(v), label(l), baseline(b) { }
		double value;
		wxString label;
		bool baseline;
	};
	std::vector<sankey_item> m_list;
};

#endif
