#ifndef __DVTimeSeriesCtrl_h
#define __DVTimeSeriesCtrl_h

/*
 * wxDVTimeSeriesCtrl.h
 *
 * This is a wxPanel that contains DView time series functionality.
 * It contains the plot window (WPPlotSurface2D) as well as the scrollbars,
 * axes, and ability to turn multiple plots on or off.
 *
 * This control will display a line plot on a time series axis.
 */

#include <wx/panel.h>
#include <wx/dialog.h>
#include "wex/numeric.h"
#include "wex/plot/plplotctrl.h"
#include "wex/dview/dvplothelper.h"

class wxDVTimeSeriesDataSet;
class wxPLTimeAxis;
class wxPLLinePlot;
class wxDVSelectionListCtrl;
class wxGridSizer;
class wxCheckBox;
class wxDVTimeSeriesPlot;
class wxScrollBar;

enum wxDVStatType { wxDV_AVERAGE = 0, wxDV_SUM };
enum wxDVTimeSeriesType { wxDV_RAW = 0, wxDV_HOURLY, wxDV_DAILY, wxDV_MONTHLY };
enum wxDVTimeSeriesStyle { wxDV_NORMAL, wxDV_STEPPED };

class wxDVTimeSeriesSettingsDialog : public wxDialog
{
public:
	wxDVTimeSeriesSettingsDialog( wxWindow *parent, const wxString &title, bool isBottomGraphVisible = true );
	~wxDVTimeSeriesSettingsDialog();

	void SetTopYBounds( double y1min, double y1max );
	void SetBottomYBounds( double y2min, double y2max );
	void GetTopYBounds( double *y1min, double *y1max );
	void GetBottomYBounds( double *y2min, double *y2max );

	void SetLineStyle( int id );
	int GetLineStyle();
	void SetSync( bool b );
	bool GetSync();
	void SetStatType(wxDVStatType StatType);
	wxDVStatType GetStatType();
	void SetAutoscale( bool b );
	bool GetAutoscale();
	void SetBottomAutoscale( bool b );
	bool GetBottomAutoscale();

protected:
	void OnClickTopHandler(wxCommandEvent& event);
	void OnClickBottomHandler(wxCommandEvent& event);
	void OnClickStatHandler(wxCommandEvent& event);

private:
	wxCheckBox *mSyncCheck;
	wxCheckBox *mStatTypeCheck;
	wxCheckBox *mTopAutoscaleCheck;
	wxCheckBox *mBottomTopAutoscaleCheck;
	wxNumericCtrl *mTopYMaxCtrl;
	wxNumericCtrl *mTopYMinCtrl;
	wxNumericCtrl *mBottomYMaxCtrl;
	wxNumericCtrl *mBottomYMinCtrl;
	wxChoice *mLineStyleCombo;

	DECLARE_EVENT_TABLE();
	
};

class wxDVTimeSeriesCtrl : public wxPanel
{
public:
	wxDVTimeSeriesCtrl(wxWindow *parent, wxWindowID id, wxDVTimeSeriesType seriesType, wxDVStatType wxDVStatType);
	virtual ~wxDVTimeSeriesCtrl();

	//When a data set is added, wxDVTimeSeriesCtrl creates a plottable with a pointer to that data.  Does not take ownership.
	void AddDataSet(wxDVTimeSeriesDataSet *d, const wxString& group, bool refresh_ui);
	bool RemoveDataSet(wxDVTimeSeriesDataSet *d); //Releases ownership, does not delete. //true if found & removed.
	void RemoveAllDataSets(); //Clears all data sets from graphs and memory.

	//Data Selection:
	wxDVSelectionListCtrl* GetDataSelectionList();
	void SetTopSelectedNames(const wxString& names);
	void SetBottomSelectedNames(const wxString& names);
	void SetSelectedNamesForColIndex(const wxString& names, int index);
	void SelectDataSetAtIndex(int index);

	//View Setters/Getters
	void SetViewMin(double min);
	double GetViewMin();
	void SetViewMax(double max);
	double GetViewMax();
	void SetViewRange(double min, double max);

	void GetVisibleDataMinAndMax(double* min, double* max, const std::vector<int>& selectedChannelIndices);
	double GetMinPossibleTimeForVisibleChannels();
	double GetMaxPossibleTimeForVisibleChannels();

	void KeepNewBoundsWithinLimits(double* newMin, double* newMax);
	void KeepNewBoundsWithinLowerLimit(double* newMin, double* newMax);
	void MakeXBoundsNice(double* xMin, double* xMax);

	bool GetSyncWithHeatMap();
	void SetSyncWithHeatMap(bool b);
	wxDVTimeSeriesType GetwxDVTimeSeriesType();
	wxDVStatType GetStatType();
	void SetStatType(wxDVStatType StatType);

	/*Graph Specific Methods*/
	bool CanZoomIn(void);
	bool CanZoomOut(void);
	void ZoomFactorAndUpdate(double factor, double shiftPercent = 0.0); //A factor of 2 would zoom in twice as far as current level.
	void ZoomToFit();
	void PanByPercent(double p); //Negative goes left.
	void UpdateScrollbarPosition(void);
	void AutoscaleYAxis(bool forceUpdate = false);
	
	void Invalidate();

protected:
	void OnZoomIn(wxCommandEvent& e);
	void OnZoomOut(wxCommandEvent& e);
	void OnZoomFit(wxCommandEvent& e);
	void OnSettings( wxCommandEvent &e );

	void OnDataChannelSelection(wxCommandEvent& e);
	void OnMouseWheel(wxMouseEvent& e);
	void OnHighlight(wxCommandEvent& e);
	void OnGraphScroll(wxScrollEvent& e);
	void OnGraphScrollLineUp(wxScrollEvent& e);
	void OnGraphScrollLineDown(wxScrollEvent& e);
	void OnGraphScrollPageUp(wxScrollEvent& e);
	void OnGraphScrollPageDown(wxScrollEvent& e);
	/*
	void OnPlotDrag(wxCommandEvent& e);
	void OnPlotDragStart(wxCommandEvent& e);
	void OnPlotDragEnd(wxCommandEvent& e);
	*/
	
	
	void AutoscaleYAxis(wxPLAxis* axisToScale, 
		const std::vector<int>& selectedChannelIndices, bool forceUpdate = false);
	
private:
	std::vector<wxDVTimeSeriesPlot*> m_plots;

	enum GraphAxisPosition
	{
		TOP_LEFT_AXIS = 0, 
		TOP_RIGHT_AXIS,
		BOTTOM_LEFT_AXIS, 
		BOTTOM_RIGHT_AXIS,
		GRAPH_AXIS_POSITION_COUNT
	};

	//This array contains the visible graphs associated with each axis position on each graph.
	std::vector<std::vector<int>*> m_selectedChannelIndices; 

	wxPLPlotCtrl *m_plotSurface;
	wxPLTimeAxis *m_xAxis;
	wxScrollBar *m_graphScrollBar;
	wxDVSelectionListCtrl *m_dataSelector;

	bool m_topAutoScale, m_bottomAutoScale, m_syncToHeatMap;
	int m_lineStyle; // line, stepped, points
	wxDVTimeSeriesType m_seriesType;
	wxDVStatType m_statType;

	void AddGraphAfterChannelSelection(wxPLPlotCtrl::PlotPos pPos, int index);
	void RemoveGraphAfterChannelSelection(wxPLPlotCtrl::PlotPos pPos, int index);
	void SetYAxisLabelText();
	void ClearAllChannelSelections(wxPLPlotCtrl::PlotPos pPos);
	void RefreshDisabledCheckBoxes();
	void RefreshDisabledCheckBoxes(wxPLPlotCtrl::PlotPos pPos);

	DECLARE_EVENT_TABLE()
};

#endif