#include <wx/wx.h>
#include <wx/aui/aui.h>

#include "wex/plot/plhistplot.h"

#include "wex/dview/dvplotctrl.h"
#include "wex/dview/dvselectionlist.h"
#include "wex/dview/dvcolourmap.h"

#include "wex/icons/time.cpng"
#include "wex/icons/dmap.cpng"
#include "wex/icons/calendar.cpng"
#include "wex/icons/barchart.cpng"
#include "wex/icons/curve.cpng"
#include "wex/icons/scatter.cpng"

#include "wex/metro.h"


enum { ID_NOTEBOOK = wxID_HIGHEST + 141 };

enum { TAB_TIME_SERIES = 0, TAB_DMAP, TAB_PROFILE, TAB_PNCDF};

/*
class wxDViewNotebook : public wxAuiNotebook
{
public:
	wxDViewNotebook( wxWindow *parent, int id )
		: wxAuiNotebook( parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE )
	{
		m_mgr.GetArtProvider()->SetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE,0);
	}
};
*/

BEGIN_EVENT_TABLE(wxDVPlotCtrl, wxPanel)
	EVT_NOTEBOOK_PAGE_CHANGING(ID_NOTEBOOK, wxDVPlotCtrl::OnPageChanging)
END_EVENT_TABLE()

/* Constructors and Destructors */
wxDVPlotCtrl::wxDVPlotCtrl(wxWindow* parent, wxWindowID id, 
	const wxPoint& pos, const wxSize& size)
	: wxPanel(parent, id, pos, size, wxTAB_TRAVERSAL)
{
	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(topSizer);

	m_plotNotebook = new wxMetroNotebook(this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxMT_LIGHTTHEME);
	topSizer->Add(m_plotNotebook, 1, wxEXPAND, 0);

	m_timeSeries = new wxDVTimeSeriesCtrl(m_plotNotebook, wxID_ANY, wxDV_RAW, wxDV_AVERAGE);
	m_hourlyTimeSeries = new wxDVTimeSeriesCtrl(m_plotNotebook, wxID_ANY, wxDV_HOURLY, wxDV_AVERAGE);
	m_dailyTimeSeries = new wxDVTimeSeriesCtrl(m_plotNotebook, wxID_ANY, wxDV_DAILY, wxDV_AVERAGE);
	m_monthlyTimeSeries = new wxDVTimeSeriesCtrl(m_plotNotebook, wxID_ANY, wxDV_MONTHLY, wxDV_AVERAGE);
	m_dMap = new wxDVDMapCtrl(m_plotNotebook, wxID_ANY);
	m_profilePlots = new wxDVProfileCtrl(m_plotNotebook, wxID_ANY);
	m_statisticsTable = new wxDVStatisticsTableCtrl(m_plotNotebook, wxID_ANY);
	m_pnCdf = new wxDVPnCdfCtrl(m_plotNotebook, wxID_ANY);
	m_durationCurve = new wxDVDCCtrl(m_plotNotebook, wxID_ANY);
	m_scatterPlot = new wxDVScatterPlotCtrl(m_plotNotebook, wxID_ANY);

//	m_plotNotebook->SetTextOnRight( true );

	DisplayTabs();
}

wxDVPlotCtrl::~wxDVPlotCtrl()
{
	for (int i=0; i<m_dataSets.size(); i++)
		delete m_dataSets[i];
}

double wxDVPlotCtrl::GetMinTimeStep()
{
	double MinTimeStep = 1000000000.0;	//Rediculously high time step - no real data file would ever use a time step this big.
	double TimeStep;

	for (int i = 0; i < m_dataSets.size(); i++)
	{
		TimeStep = m_dataSets[i]->GetTimeStep();
		if (TimeStep < MinTimeStep) { MinTimeStep = TimeStep; }
	}

	return MinTimeStep;
}

void wxDVPlotCtrl::DisplayTabs()
{
	//TODO:  Is there a better way to do this (hide and show tabs)?  (See also RemoveAllDataSets() method) This might also fix the small blank square that sometimes appears over the tab strip.
	double MinTimeStep = GetMinTimeStep();

	if (MinTimeStep >= 1.0 && m_plotNotebook->GetPageCount() == 10) { m_plotNotebook->DeletePage(1); }	//hourly is the second tab - delete it if it exists (pagecount = 10)
	if (MinTimeStep >= 24.0 && m_plotNotebook->GetPageCount() == 9) { m_plotNotebook->DeletePage(1); }	//daily is the second tab (hourly has been deleted) - delete it if it exists (pagecount = 9)
	if (MinTimeStep >= 672.0 && m_plotNotebook->GetPageCount() == 8) { m_plotNotebook->DeletePage(1); }	//monthly is the second tab (hourly & daily have been deleted) - delete it if it exists (pagecount = 8)

	int PageCount = m_plotNotebook->GetPageCount();	//Get count of remaining pages

	//We have to remove all remaining pages and then add back the ones we want since wxMetroNotebook does not have a way to insert a page at a specified index
	//However, since the time series tab is always first and always displayed we don't remove it since there is a bug in the wxMetroNotebook that errors on removing the last item from the pagelist.
	for (int i = PageCount - 1; i > 0; i--)
	{
		m_plotNotebook->RemovePage(i);
	}

	if (m_plotNotebook->GetPageCount() == 0) { m_plotNotebook->AddPage(m_timeSeries, _("Time Series"), /*wxBITMAP_PNG_FROM_DATA( time ), */true); } //Only add this if it doesn't already exist
	if (MinTimeStep < 1.0) { m_plotNotebook->AddPage(m_hourlyTimeSeries, _("Hourly"), /*wxBITMAP_PNG_FROM_DATA( time ), */false); }
	if (MinTimeStep < 24.0) { m_plotNotebook->AddPage(m_dailyTimeSeries, _("Daily"), /*wxBITMAP_PNG_FROM_DATA( time ), */false); }
	if (MinTimeStep < 672.0) { m_plotNotebook->AddPage(m_monthlyTimeSeries, _("Monthly"), /*wxBITMAP_PNG_FROM_DATA( time ), */false); }
	m_plotNotebook->AddPage(m_profilePlots, _("Profiles"), /*wxBITMAP_PNG_FROM_DATA( calendar ), */false);
	m_plotNotebook->AddPage(m_statisticsTable, _("Statistics Table"), /*wxBITMAP_PNG_FROM_DATA( dmap ), */false);
	m_plotNotebook->AddPage(m_dMap, _("Heat Map"), /*wxBITMAP_PNG_FROM_DATA( dmap ), */false);
	m_plotNotebook->AddPage(m_scatterPlot, _("Scatter"), /*wxBITMAP_PNG_FROM_DATA( scatter ), */false);
	m_plotNotebook->AddPage(m_pnCdf, _("PDF / CDF"), /*wxBITMAP_PNG_FROM_DATA( barchart ), */false);
	m_plotNotebook->AddPage(m_durationCurve, _("Duration Curve"), /*wxBITMAP_PNG_FROM_DATA( curve ), */false);

	m_plotNotebook->Refresh();
}

//This function is used to add a data set to Dview.  
//It adds that data set to all of the tabs.
//The group is optional, it is the name of the group that the data set belongs to if grouping is desired.
void wxDVPlotCtrl::AddDataSet(wxDVTimeSeriesDataSet *d, const wxString& group, bool update_ui)
{
	//Take ownership of the data Set.  We will delete it on destruction.
	m_dataSets.push_back(d);

	m_timeSeries->AddDataSet(d, group, update_ui);
	m_hourlyTimeSeries->AddDataSet(d, group, update_ui);
	m_dailyTimeSeries->AddDataSet(d, group, update_ui);
	m_monthlyTimeSeries->AddDataSet(d, group, update_ui);
	m_dMap->AddDataSet(d, group, update_ui);
	m_profilePlots->AddDataSet(d, group, update_ui);
	m_statisticsTable->AddDataSet(d, group);
	m_pnCdf->AddDataSet(d, group, update_ui);
	m_durationCurve->AddDataSet(d, group, update_ui);
	m_scatterPlot->AddDataSet(d, group, update_ui);
}

void wxDVPlotCtrl::RemoveDataSet(wxDVTimeSeriesDataSet *d)
{
	m_timeSeries->RemoveDataSet(d);
	m_hourlyTimeSeries->RemoveDataSet(d);
	m_dailyTimeSeries->RemoveDataSet(d);
	m_monthlyTimeSeries->RemoveDataSet(d);
	m_dMap->RemoveDataSet(d);
	m_profilePlots->RemoveDataSet(d);
	m_statisticsTable->RemoveDataSet(d);
	m_pnCdf->RemoveDataSet(d);
	m_durationCurve->RemoveDataSet(d);
	m_scatterPlot->RemoveDataSet(d);

	m_dataSets.erase( std::find( m_dataSets.begin(), m_dataSets.end(), d) );
}

void wxDVPlotCtrl::RemoveAllDataSets()
{
	m_timeSeries->RemoveAllDataSets();
	m_hourlyTimeSeries->RemoveAllDataSets();
	m_dailyTimeSeries->RemoveAllDataSets();
	m_monthlyTimeSeries->RemoveAllDataSets();
	m_dMap->RemoveAllDataSets();
	m_profilePlots->RemoveAllDataSets();
	m_statisticsTable->RemoveAllDataSets();
	m_pnCdf->RemoveAllDataSets();
	m_durationCurve->RemoveAllDataSets();
	m_scatterPlot->RemoveAllDataSets();

	for (int i=0; i<m_dataSets.size(); i++)
	{
		delete m_dataSets[i];
	}

	m_dataSets.clear();

	//Remove hourly, daily, and monthly tab pages if they exist
	switch (m_plotNotebook->GetPageCount())
	{
		case 10:
			m_plotNotebook->RemovePage(3);
			m_plotNotebook->RemovePage(2);
			m_plotNotebook->RemovePage(1);
			break;
		case 9:
			m_plotNotebook->RemovePage(2);
			m_plotNotebook->RemovePage(1);
			break;
		case 8:
			m_plotNotebook->RemovePage(1);
			break;
	}
	m_plotNotebook->Refresh();

	//delete and re-add these controls (fixes bug with tabs not behaving correctly after we remove a tab that doesn't get used with the next loaded file)
	//TODO:  Is there a better way to do this (hide and show tabs)?  (See also DisplayTabs() method) This might also fix the small blank square that sometimes appears over the tab strip.
	delete m_hourlyTimeSeries;
	delete m_dailyTimeSeries;
	delete m_monthlyTimeSeries;

	m_hourlyTimeSeries = new wxDVTimeSeriesCtrl(m_plotNotebook, wxID_ANY, wxDV_HOURLY, wxDV_AVERAGE);
	m_dailyTimeSeries = new wxDVTimeSeriesCtrl(m_plotNotebook, wxID_ANY, wxDV_DAILY, wxDV_AVERAGE);
	m_monthlyTimeSeries = new wxDVTimeSeriesCtrl(m_plotNotebook, wxID_ANY, wxDV_MONTHLY, wxDV_AVERAGE);

	DisplayTabs();
}

wxDVStatisticsTableCtrl* wxDVPlotCtrl::GetStatisticsTable()
{
	return m_statisticsTable;
}

wxDVPlotCtrlSettings wxDVPlotCtrl::GetPerspective()
{
	wxDVPlotCtrlSettings settings;

	settings.SetProperty(wxT("tabIndex"), m_plotNotebook->GetSelection());

	//***TimeSeries Properties***
	settings.SetProperty(wxT("tsAxisMin"), m_timeSeries->GetViewMin());
	settings.SetProperty(wxT("tsAxisMax"), m_timeSeries->GetViewMax());
	
	settings.SetProperty(wxT("tsTopSelectedNames"), m_timeSeries->GetDataSelectionList()->GetSelectedNamesInCol(0));
	settings.SetProperty(wxT("tsBottomSelectedNames"), m_timeSeries->GetDataSelectionList()->GetSelectedNamesInCol(1));

	//***HourlyTimeSeries Properties***
	settings.SetProperty(wxT("tsAxisMin"), m_hourlyTimeSeries->GetViewMin());
	settings.SetProperty(wxT("tsAxisMax"), m_hourlyTimeSeries->GetViewMax());

	settings.SetProperty(wxT("tsTopSelectedNames"), m_hourlyTimeSeries->GetDataSelectionList()->GetSelectedNamesInCol(0));
	settings.SetProperty(wxT("tsBottomSelectedNames"), m_hourlyTimeSeries->GetDataSelectionList()->GetSelectedNamesInCol(1));

	//***DailyTimeSeries Properties***
	settings.SetProperty(wxT("tsAxisMin"), m_dailyTimeSeries->GetViewMin());
	settings.SetProperty(wxT("tsAxisMax"), m_dailyTimeSeries->GetViewMax());
	
	settings.SetProperty(wxT("tsTopSelectedNames"), m_dailyTimeSeries->GetDataSelectionList()->GetSelectedNamesInCol(0));
	settings.SetProperty(wxT("tsBottomSelectedNames"), m_dailyTimeSeries->GetDataSelectionList()->GetSelectedNamesInCol(1));

	//***MonthlyTimeSeries Properties***
	settings.SetProperty(wxT("tsAxisMin"), m_monthlyTimeSeries->GetViewMin());
	settings.SetProperty(wxT("tsAxisMax"), m_monthlyTimeSeries->GetViewMax());
	
	settings.SetProperty(wxT("tsTopSelectedNames"), m_monthlyTimeSeries->GetDataSelectionList()->GetSelectedNamesInCol(0));
	settings.SetProperty(wxT("tsBottomSelectedNames"), m_monthlyTimeSeries->GetDataSelectionList()->GetSelectedNamesInCol(1));

	//***DMap Tap Properties***
	settings.SetProperty(wxT("dmapTopCurrentName"), m_dMap->GetCurrentDataName(wxPLPlotCtrl::PLOT_TOP));
	settings.SetProperty(wxT("dmapBottomCurrentName"), m_dMap->GetCurrentDataName(wxPLPlotCtrl::PLOT_BOTTOM));
	settings.SetProperty(wxT("dmapZMin"), m_dMap->GetZMin());
	settings.SetProperty(wxT("dmapZMax"), m_dMap->GetZMax());
	settings.SetProperty(wxT("dmapXMin"), m_dMap->GetXMin());
	settings.SetProperty(wxT("dmapXMax"), m_dMap->GetXMax());
	settings.SetProperty(wxT("dmapYMin"), m_dMap->GetYMin());
	settings.SetProperty(wxT("dmapYMax"), m_dMap->GetYMax());
	settings.SetProperty(wxT("dmapColourMap"), m_dMap->GetCurrentColourMap()->GetName());
	//Add link to ts

	//***Monthly Profile Properties***
	settings.SetProperty(wxT("profileJanSelected"), m_profilePlots->IsMonthIndexSelected(0));
	settings.SetProperty(wxT("profileFebSelected"), m_profilePlots->IsMonthIndexSelected(1));
	settings.SetProperty(wxT("profileMarSelected"), m_profilePlots->IsMonthIndexSelected(2));
	settings.SetProperty(wxT("profileAprSelected"), m_profilePlots->IsMonthIndexSelected(3));
	settings.SetProperty(wxT("profileMaySelected"), m_profilePlots->IsMonthIndexSelected(4));
	settings.SetProperty(wxT("profileJunSelected"), m_profilePlots->IsMonthIndexSelected(5));
	settings.SetProperty(wxT("profileJulSelected"), m_profilePlots->IsMonthIndexSelected(6));
	settings.SetProperty(wxT("profileAugSelected"), m_profilePlots->IsMonthIndexSelected(7));
	settings.SetProperty(wxT("profileSepSelected"), m_profilePlots->IsMonthIndexSelected(8));
	settings.SetProperty(wxT("profileOctSelected"), m_profilePlots->IsMonthIndexSelected(9));
	settings.SetProperty(wxT("profileNovSelected"), m_profilePlots->IsMonthIndexSelected(10));
	settings.SetProperty(wxT("profileDecSelected"), m_profilePlots->IsMonthIndexSelected(11));
	settings.SetProperty(wxT("profileAnnualSelected"), m_profilePlots->IsMonthIndexSelected(12));

	settings.SetProperty(wxT("profileSelectedNames"), m_profilePlots->GetDataSelectionList()->GetSelectedNamesInCol(0));

	//***Statistics Table Properties:  None

	//***PDF CDF Tab Properties***
	settings.SetProperty(wxT("pnCdfCurrentName"), m_pnCdf->GetCurrentDataName());
	settings.SetProperty(wxT("pnCdfNormalize"), int(m_pnCdf->GetNormalizeType()));
	settings.SetProperty(wxT("pnCdfBinSelectionIndex"), m_pnCdf->GetBinSelectionIndex());
	settings.SetProperty(wxT("pnCdfBins"), m_pnCdf->GetNumberOfBins());
	settings.SetProperty(wxT("pnCdfYMax"), m_pnCdf->GetYMax());


	//*** DURATION CURVE PROPERTIES*** 
	settings.SetProperty(wxT("dcSelectedNames"), m_durationCurve->GetDataSelectionList()->GetSelectedNamesInCol(0));


	//*** SCATTER PLOT PROPERTIES ***
	settings.SetProperty(wxT("scatterXDataName"), m_scatterPlot->GetScatterSelectionList()->GetSelectedNamesInCol(0));
	settings.SetProperty(wxT("scatterYDataNames"), m_scatterPlot->GetScatterSelectionList()->GetSelectedNamesInCol(1));


	return settings;
}

void wxDVPlotCtrl::SetPerspective(wxDVPlotCtrlSettings& settings)
{
	long i;
	settings.GetProperty(wxT("tabIndex")).ToLong(&i);
	m_plotNotebook->SetSelection(i);

	//***TimeSeries Properties***
	m_timeSeries->SetTopSelectedNames(settings.GetProperty(wxT("tsTopSelectedNames")));
	m_timeSeries->SetBottomSelectedNames(settings.GetProperty(wxT("tsBottomSelectedNames")));

	//***HourlyTimeSeries Properties***
	m_hourlyTimeSeries->SetTopSelectedNames(settings.GetProperty(wxT("tsTopSelectedNames")));
	m_hourlyTimeSeries->SetBottomSelectedNames(settings.GetProperty(wxT("tsBottomSelectedNames")));

	//***DailyTimeSeries Properties***
	m_dailyTimeSeries->SetTopSelectedNames(settings.GetProperty(wxT("tsTopSelectedNames")));
	m_dailyTimeSeries->SetBottomSelectedNames(settings.GetProperty(wxT("tsBottomSelectedNames")));

	//***MonthlyTimeSeries Properties***
	m_monthlyTimeSeries->SetTopSelectedNames(settings.GetProperty(wxT("tsTopSelectedNames")));
	m_monthlyTimeSeries->SetBottomSelectedNames(settings.GetProperty(wxT("tsBottomSelectedNames")));

	//Set min/max after setting plots to make sure there is an axis to set.
	double min, max;
	settings.GetProperty(wxT("tsAxisMin")).ToDouble(&min);
	settings.GetProperty(wxT("tsAxisMax")).ToDouble(&max);
	m_timeSeries->SetViewMin(min);
	m_timeSeries->SetViewMax(max);
	m_hourlyTimeSeries->SetViewMin(min);
	m_hourlyTimeSeries->SetViewMax(max);
	m_dailyTimeSeries->SetViewMin(min);
	m_dailyTimeSeries->SetViewMax(max);
	m_monthlyTimeSeries->SetViewMin(min);
	m_monthlyTimeSeries->SetViewMax(max);
	

	//***DMap Tab Properties***
	m_dMap->SetCurrentDataName(settings.GetProperty(wxT("dmapTopCurrentName")), wxPLPlotCtrl::PLOT_TOP);
	m_dMap->SetCurrentDataName(settings.GetProperty(wxT("dmapBottomCurrentName")), wxPLPlotCtrl::PLOT_BOTTOM);
	m_dMap->SetColourMapName(settings.GetProperty(wxT("dmapColourMap"))); //Do this before setting z min/max.

	settings.GetProperty(wxT("dmapZMin")).ToDouble(&min);
	settings.GetProperty(wxT("dmapZMax")).ToDouble(&max);
	m_dMap->SetZMin(min);
	m_dMap->SetZMax(max);
	settings.GetProperty(wxT("dmapXMin")).ToDouble(&min);
	settings.GetProperty(wxT("dmapXMax")).ToDouble(&max);
	m_dMap->SetXMin(min);
	m_dMap->SetXMax(max);
	settings.GetProperty(wxT("dmapYMin")).ToDouble(&min);
	settings.GetProperty(wxT("dmapYMax")).ToDouble(&max);
	m_dMap->SetYMin(min);
	m_dMap->SetYMax(max);


	//***Monthly Profile Properties***
	m_profilePlots->SetMonthIndexSelected(0, settings.GetProperty(wxT("profileJanSelected")) == wxT("1"));
	m_profilePlots->SetMonthIndexSelected(1, settings.GetProperty(wxT("profileFebSelected")) == wxT("1"));
	m_profilePlots->SetMonthIndexSelected(2, settings.GetProperty(wxT("profileMarSelected")) == wxT("1"));
	m_profilePlots->SetMonthIndexSelected(3, settings.GetProperty(wxT("profileAprSelected")) == wxT("1"));
	m_profilePlots->SetMonthIndexSelected(4, settings.GetProperty(wxT("profileMaySelected")) == wxT("1"));
	m_profilePlots->SetMonthIndexSelected(5, settings.GetProperty(wxT("profileJunSelected")) == wxT("1"));
	m_profilePlots->SetMonthIndexSelected(6, settings.GetProperty(wxT("profileJulSelected")) == wxT("1"));
	m_profilePlots->SetMonthIndexSelected(7, settings.GetProperty(wxT("profileAugSelected")) == wxT("1"));
	m_profilePlots->SetMonthIndexSelected(8, settings.GetProperty(wxT("profileSepSelected")) == wxT("1"));
	m_profilePlots->SetMonthIndexSelected(9, settings.GetProperty(wxT("profileOctSelected")) == wxT("1"));
	m_profilePlots->SetMonthIndexSelected(10, settings.GetProperty(wxT("profileNovSelected")) == wxT("1"));
	m_profilePlots->SetMonthIndexSelected(11, settings.GetProperty(wxT("profileDecSelected")) == wxT("1"));
	m_profilePlots->SetMonthIndexSelected(12, settings.GetProperty(wxT("profileAnnualSelected")) == wxT("1"));

	m_profilePlots->SetSelectedNames(settings.GetProperty(wxT("profileSelectedNames")));

	//***Statistics Table Properties:  None

	//***PDF CDF Tab Properties***
	long normalize;
	settings.GetProperty(wxT("pnCdfNormalize")).ToLong(&normalize);
	m_pnCdf->SetNormalizeType( wxPLHistogramPlot::NormalizeType(normalize));
	long binIndex;
	settings.GetProperty(wxT("pnCdfBinSelectionIndex")).ToLong(&binIndex);
	m_pnCdf->SetBinSelectionIndex(binIndex);
	long bins;
	settings.GetProperty(wxT("pnCdfBins")).ToLong(&bins);
	m_pnCdf->SetNumberOfBins(bins);
	m_pnCdf->SetCurrentDataName(settings.GetProperty(wxT("pnCdfCurrentName")), true);
	double yMax;
	settings.GetProperty(wxT("pnCdfYMax")).ToDouble(&yMax);
	m_pnCdf->SetYMax(yMax);


	//*** DURATION CURVE PROPERTIES ***
	m_durationCurve->SetSelectedNames(settings.GetProperty(wxT("dcSelectedNames")), true);
	
	
	//*** SCATTER PLOT PROPERTIES ***
	m_scatterPlot->SetXSelectedName(settings.GetProperty(wxT("scatterXDataName")));
	m_scatterPlot->SetYSelectedNames(settings.GetProperty(wxT("scatterYDataNames")));
		

	Refresh();
	Update();
}

void wxDVPlotCtrl::SelectTabIndex(int index)
{
	if (index >= 0 && (unsigned int)index < m_plotNotebook->GetPageCount())
		m_plotNotebook->SetSelection(index);
}

void wxDVPlotCtrl::SelectDataIndex(int index, bool allTabs)
{
	//Only select this data on the slower tabs (which require sorting) if allTabs is true.

	if (index < 0 || index >= m_dataSets.size())
		return;

	m_timeSeries->SelectDataSetAtIndex(index);
	m_hourlyTimeSeries->SelectDataSetAtIndex(index);
	m_dailyTimeSeries->SelectDataSetAtIndex(index);
	m_monthlyTimeSeries->SelectDataSetAtIndex(index);
	m_dMap->SelectDataSetAtIndex(index, wxPLPlotCtrl::PLOT_TOP);	//If this tab has two datasets selected we return the one associated with the top plot
	m_profilePlots->SelectDataSetAtIndex(index);
	if (allTabs)
	{
		m_pnCdf->SelectDataSetAtIndex(index);
		m_durationCurve->SelectDataSetAtIndex(index);
	}

	//NOTE:  Statistics Table does not allow selection of an individual dataset
	//and data set selection does not make sense for scatter plot.
}

void wxDVPlotCtrl::SelectDataIndexOnTab(int index, int tab)
{
	if (index < 0 || index >= m_dataSets.size()) return;

	switch(tab)
	{
	case TAB_TS:
		m_timeSeries->SelectDataSetAtIndex(index);
		break;
	case TAB_HTS:
		m_hourlyTimeSeries->SelectDataSetAtIndex(index);
		break;
	case TAB_DTS:
		m_dailyTimeSeries->SelectDataSetAtIndex(index);
		break;
	case TAB_MTS:
		m_monthlyTimeSeries->SelectDataSetAtIndex(index);
		break;
	case TAB_DMAP:
		m_dMap->SelectDataSetAtIndex(index, wxPLPlotCtrl::PLOT_TOP);	//If this tab has two datasets selected we return the one associated with the top plot
		break;
	case TAB_PROFILE:
		m_profilePlots->SelectDataSetAtIndex(index);
		break;
	case TAB_PDF:
		m_pnCdf->SelectDataSetAtIndex(index);
		break;
	case TAB_DC:
		m_durationCurve->SelectDataSetAtIndex(index);
		break;
	case TAB_SCATTER:
		// There is not a good way to handle this right now.
		break;
	}

	//NOTE:  Statistics Table does not allow selection of an individual dataset
}

void wxDVPlotCtrl::OnPageChanging(wxNotebookEvent& e)
{
//	wxMessageBox("page changing");
	//if (!mSynchAxesCheckBox->IsChecked()) return; //Only using it for this right now.
	
	if ( !m_timeSeries || !m_dMap ) return;

	if (e.GetOldSelection() == TAB_DMAP)
	{
		if (m_dMap->GetSyncWithTimeSeries())
		{
			m_timeSeries->SetViewMin(m_dMap->GetXMin());
			m_timeSeries->SetViewMax(m_dMap->GetXMax());
			m_timeSeries->SetSyncWithHeatMap(true);
		}
		else
			m_timeSeries->SetSyncWithHeatMap(false);
	}
	else if (e.GetOldSelection() == TAB_TIME_SERIES)
	{
		if (m_timeSeries->GetSyncWithHeatMap())
		{
			double newXMin = m_timeSeries->GetViewMin();
			double newXMax = m_timeSeries->GetViewMax();
			m_dMap->KeepXBoundsWithinLimits(&newXMin, &newXMax);
			m_dMap->SetXMin(newXMin);
			m_dMap->SetXMax(newXMax);
			m_dMap->SetSyncWithTimeSeries(true);
		}
		else
			m_dMap->SetSyncWithTimeSeries(false);
	}
}

void wxDVPlotCtrl::SelectDataOnBlankTabs()
{
	if (m_dataSets.size() == 0) return;

	if ( m_timeSeries != 0
		&& m_timeSeries->GetDataSelectionList()->GetSelectedNamesInCol(0).size() == 0 
		&& m_timeSeries->GetDataSelectionList()->GetSelectedNamesInCol(1).size() == 0 )
		m_timeSeries->SelectDataSetAtIndex(0);

	if (m_hourlyTimeSeries != 0
		&& m_hourlyTimeSeries->GetDataSelectionList()->GetSelectedNamesInCol(0).size() == 0
		&& m_hourlyTimeSeries->GetDataSelectionList()->GetSelectedNamesInCol(1).size() == 0)
		m_hourlyTimeSeries->SelectDataSetAtIndex(0);

	if ( m_dailyTimeSeries != 0
		&& m_dailyTimeSeries->GetDataSelectionList()->GetSelectedNamesInCol(0).size() == 0 
		&& m_dailyTimeSeries->GetDataSelectionList()->GetSelectedNamesInCol(1).size() == 0 )
		m_dailyTimeSeries->SelectDataSetAtIndex(0);

	if ( m_monthlyTimeSeries != 0
		&& m_monthlyTimeSeries->GetDataSelectionList()->GetSelectedNamesInCol(0).size() == 0 
		&& m_monthlyTimeSeries->GetDataSelectionList()->GetSelectedNamesInCol(1).size() == 0 )
		m_monthlyTimeSeries->SelectDataSetAtIndex(0);

	if (m_dMap->GetCurrentDataName(wxPLPlotCtrl::PLOT_TOP).size() == 0)
		m_dMap->SelectDataSetAtIndex(0, wxPLPlotCtrl::PLOT_TOP);

	if (m_profilePlots->GetDataSelectionList()->GetSelectedNamesInCol(0).size() == 0)
		m_profilePlots->SelectDataSetAtIndex(0);

	if ( m_dataSets[0]->Length() <= 8760)
	{
		if (m_pnCdf->GetCurrentDataName().size() == 0)
			m_pnCdf->SelectDataSetAtIndex(0);

		if (m_durationCurve->GetDataSelectionList()->GetSelectedNamesInCol(0).size() == 0)
			m_durationCurve->SelectDataSetAtIndex(0);
	}

	wxDVSelectionListCtrl *SelList = m_scatterPlot->GetScatterSelectionList();
	if (SelList->GetSelectedNamesInCol(0).size() == 0) { m_scatterPlot->SelectXDataAtIndex(0); }
	if (SelList->GetSelectedNamesInCol(1).size() == 0) { m_scatterPlot->SelectYDataAtIndex(SelList->GetUnsortedRowIndex(1)); }

	//NOTE:  Statistics Table does not allow selection of an individual dataset
}

