#include "wex/plot/plplot.h"
#include "wex/plot/plsankeyplot.h"
#include "wex/numeric.h"

wxPLSankeyPlot::wxPLSankeyPlot()
{
	m_decimals = 2;
	m_thousep = true;
}

void wxPLSankeyPlot::Add( double value, const wxString &label, bool baseline )
{
	m_list.push_back( sankey_item(value,label,baseline) );
}

void wxPLSankeyPlot::SetFormat( int deci, bool thousep )
{
	m_decimals = deci;
	m_thousep = thousep;
}

wxRealPoint wxPLSankeyPlot::At( size_t i ) const
{
	if ( i < m_list.size() ) return wxRealPoint( i, m_list[i].value );
	else return wxRealPoint( std::numeric_limits<double>::quiet_NaN(),
		std::numeric_limits<double>::quiet_NaN() );

}

size_t wxPLSankeyPlot::Len() const
{
	return m_list.size();
}

void wxPLSankeyPlot::Draw( wxPLOutputDevice &dv, const wxPLDeviceMapping &map )
{	
	wxRealPoint geompos, geomsz;	
	map.GetDeviceExtents( &geompos, &geomsz );
	double x = geompos.x, 
		y = geompos.y, 
		width = geomsz.x, 
		height = geomsz.y;

	double textheight = 0.0;
	dv.TextAdjust( -2 );
	dv.Measure( "hy", 0, &textheight );

	double space = textheight*0.5;

	if ( m_list.size() == 0 )
	{
		dv.Text( "No Sankey diagram specified.", x, y+space );
		return;
	}

	// find out longest text string
	double textwidthmax = 0;
	for( size_t i=0;i<m_list.size();i++ )
	{
		double textwidth = 0.0;
		dv.Measure( m_list[i].label, &textwidth, 0 );
		if ( textwidth > textwidthmax ) textwidthmax = textwidth;
	}
	
	double cur_x = x + width - textwidthmax - space;
	double cur_y = y + space;

	double cur_baseline = 0;
	double cur_baseline_size = 0;
	double sec_top = 0;
	const double R1 = 4*space;
	 
	dv.Pen( *wxBLACK, 1.0 );
	dv.NoBrush();
	for( size_t i=0;i<m_list.size();i++ )
	{
		sankey_item &li = m_list[i];

		if ( li.baseline )
		{
			cur_baseline = li.value;

			double sec_height = 2*textheight + space;

			cur_baseline_size = cur_x - (x+space);

			
			if ( i > 0 ) // close up previous section and move down
			{
				dv.Line( x+space, cur_y, x+space+cur_baseline_size, cur_y ); // section top line
				dv.Line( x+space, cur_y, x+space, sec_top ); // section left line
				cur_y += space; // spacing between sections
			}
			
			sec_top = cur_y;
			dv.Line( x+space, cur_y, x+space+cur_baseline_size, cur_y ); // section top line
			dv.Line( cur_x, cur_y, cur_x, cur_y+sec_height ); // right vertical line
						
			dv.TextAdjust(-1);
			dv.Text( li.label, x+2*space, cur_y+space );
			dv.Text( wxNumericFormat( li.value, wxNUMERIC_REAL, m_decimals, m_thousep, wxEmptyString, wxEmptyString ),
				x + 2*space, cur_y+space+textheight );

			cur_y += sec_height;
		}
		else
		{
			wxRealPoint tp;
			double LW = fabs(li.value/100.0*cur_baseline_size);
			double R2 = R1+LW;

			if ( li.value <= 0.0 )
			{
				dv.Arc( cur_x+R1, cur_y, R1, 180, 270 );
				dv.Arc( cur_x+R1, cur_y, R2, 180, 270 );

				double sec_height = R2+space;
				dv.Line( cur_x-LW, cur_y, cur_x-LW, cur_y+sec_height ); // right vertical line
				
				tp = wxRealPoint( cur_x+R1+space, cur_y+0.5*(R1+R2) );

				cur_x -= LW;
				cur_y += sec_height;
			}
			else
			{
				
				double sec_height = R2+3.0*space;

				dv.Arc( cur_x+R2, cur_y+sec_height, R2, 90, 180 );
				dv.Arc( cur_x+R2, cur_y+sec_height, R1, 90, 180 );
				
				dv.Line( cur_x, cur_y, cur_x, cur_y+sec_height ); // right vertical line
				dv.Line( cur_x+LW, cur_y+sec_height, cur_x+LW, cur_y+sec_height+space ); // right vertical line

				tp = wxRealPoint( cur_x+R2+space, cur_y+sec_height-0.5*(R1+R2) );
				cur_y += sec_height+space;
				cur_x += LW;
			}

			double arrowsize = LW*0.5 + 2;
			if ( LW < 6 ) LW = 6;

			if ( fabs(li.value) > 1e-9 )
			{
				dv.Brush(*wxWHITE);
				wxRealPoint arrow[3];
				arrow[0] = li.value < 0 ? tp : wxRealPoint(tp.x-space-space, tp.y);
				arrow[1] = wxRealPoint( tp.x-space, tp.y-arrowsize );
				arrow[2] = wxRealPoint( tp.x-space, tp.y+arrowsize );
				dv.Polygon( 3, arrow );
			}
			else
			{
				dv.Brush(*wxBLACK);
				dv.Circle( tp.x-space, tp.y, 2 );
			}
			dv.NoBrush();

			//dv.Line( tp.x-space/2, tp.y, tp.y-LW*
			//dv.Line( tp.x, tp.y, tp.x+5, tp.y );
			
			dv.TextAdjust(-2);
			dv.Text( li.label, 
				tp.x+space/2, tp.y-textheight);
			dv.Text( wxNumericFormat(li.value, wxNUMERIC_REAL, 1, false, (li.value<0)?"":"+" , " %"), 
				tp.x+space/2, tp.y);

		}


	}

	// close up with bottom point
	cur_y += space;
	double cur_width = cur_x - (x+space);
	dv.Line( x+space, cur_y, x+space+cur_width/2, cur_y+space*2 );
	dv.Line( x+space+cur_width/2, cur_y+space*2, cur_x, cur_y );
	dv.Line( x+space, cur_y, x+space, sec_top ); // left vertical line
	dv.Line( x+space+cur_width, cur_y, x+space+cur_width, cur_y-space );

}

void wxPLSankeyPlot::SetupExample()
{

	Add( 52595, "Nominal POA", true );
	
	Add( -1.5, "Shading" );
	Add( -4.9, "Soiling" );
	
	Add( 8142, "DC kWh @ STC", true );
	Add( -13.64, "Module modeled loss" );
	Add( -2.2, "Mismatch" );
	Add( -0.5, "Connections" );
	Add( -1, "Nameplate" );
	
	Add( 7135, "Net DC output", true );
	Add( -1.8, "Clipping" );
	Add( -2.9, "Inverter efficiency" );
	Add( 0.0, "No loss" );
	Add( -1.7, "Wiring" );
	Add( 4.2, "Performance adjustment" );
	
	Add( 6777, "Energy to grid", true );
}

void wxPLSankeyPlot::DrawInLegend( wxPLOutputDevice &dc, const wxPLRealRect &rct)
{
	// nothing to do - don't want it to show up in legend
}

wxPLAxis *wxPLSankeyPlot::SuggestXAxis()
{
	return new wxPLLinearAxis( 0, 1 );
}

wxPLAxis *wxPLSankeyPlot::SuggestYAxis()
{
	return new wxPLLinearAxis( 0, 1 );
}


