/*
* wxDVVariableStatistics.cpp
*
* This class Is a wxPanel that contains a table of statistics for the associated dataset
*/

#include <wx/scrolbar.h>
#include <wx/gbsizer.h>
#include <wx/tokenzr.h>
#include <wx/statline.h>
#include <wx/gdicmn.h>
#include <math.h>

#include "wex/plot/pllineplot.h"

#include "wex/icons/zoom_in.cpng"
#include "wex/icons/zoom_out.cpng"
#include "wex/icons/zoom_fit.cpng"
#include "wex/icons/preferences.cpng"

#include "wex/dview/dvstatisticstablectrl.h"
#include "wex/plot/plplotctrl.h"


//Tree Model Node

dvStatisticsTreeModelNode::dvStatisticsTreeModelNode(dvStatisticsTreeModelNode* parent, wxString nodeName)
{
	m_parent = parent;
	m_nodeName = nodeName;
	m_container = true;
}

dvStatisticsTreeModelNode::dvStatisticsTreeModelNode(dvStatisticsTreeModelNode* parent, wxString nodeName,
	double avg, double min, double max, double sum, double stdev, double avgdailymin, double avgdailymax)
{
	m_parent = parent;

	m_avg = avg;
	m_min = min;
	m_max = max;
	m_sum = sum;
	m_stdev = stdev;
	m_avgdailymin = avgdailymin;
	m_avgdailymax = avgdailymax;
	m_nodeName = nodeName;

	m_container = false;
}

dvStatisticsTreeModelNode::~dvStatisticsTreeModelNode()
{
	RemoveAllChildren();
}

bool dvStatisticsTreeModelNode::IsContainer() const 
{ 
	return m_container; 
}

dvStatisticsTreeModelNode* dvStatisticsTreeModelNode::GetParent() 
{ 
	return m_parent; 
}

std::vector<dvStatisticsTreeModelNode*> dvStatisticsTreeModelNode::GetChildren()
{ 
	return m_children; 
}

dvStatisticsTreeModelNode* dvStatisticsTreeModelNode::GetNthChild(unsigned int n) 
{ 
	if (n >= m_children.size()) { return NULL; }
	return m_children[n]; 
}

void dvStatisticsTreeModelNode::Append(dvStatisticsTreeModelNode* child) 
{ 
	m_children.push_back(child);
}

unsigned int dvStatisticsTreeModelNode::GetChildCount() const 
{ 
	return m_children.size();
}

void dvStatisticsTreeModelNode::RemoveAllChildren()
{
	m_children.clear();
}


//Tree Model

dvStatisticsTreeModel::dvStatisticsTreeModel()
{
	m_root = new dvStatisticsTreeModelNode(NULL, "All");
}

int dvStatisticsTreeModel::Compare(const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int column, bool ascending) const
{
	wxASSERT(item1.IsOk() && item2.IsOk());
	// should never happen

	if (IsContainer(item1) && IsContainer(item2))
	{
		wxVariant value1, value2;
		GetValue(value1, item1, 0);
		GetValue(value2, item2, 0);

		wxString str1 = value1.GetString();
		wxString str2 = value2.GetString();
		int res = str1.Cmp(str2);
		if (res) return res;

		// items must be different
		wxUIntPtr litem1 = (wxUIntPtr)item1.GetID();
		wxUIntPtr litem2 = (wxUIntPtr)item2.GetID();

		return litem1 - litem2;
	}

	return wxDataViewModel::Compare(item1, item2, column, ascending);
}

wxString dvStatisticsTreeModel::GetColumnType(unsigned int col) const
{
	if (col == 0)
	{
		return wxT("string");
	}
	else
	{
		return wxT("double");
	}
}

void dvStatisticsTreeModel::GetValue(wxVariant &variant, const wxDataViewItem &item, unsigned int col) const
{
	wxASSERT(item.IsOk());

	dvStatisticsTreeModelNode *node = (dvStatisticsTreeModelNode*)item.GetID();
	switch (col)
	{
	case 0:
		variant = node->m_nodeName;
		break;
	case 1:
		variant = node->m_avg;
		break;
	case 2:
		variant = node->m_min;
		break;
	case 3:
		variant = node->m_max;
		break;
	case 4:
		variant = node->m_sum;
		break;
	case 5:
		variant = node->m_stdev;
		break;
	case 6:
		variant = node->m_avgdailymin;
		break;
	case 7:
		variant = node->m_avgdailymax;
		break;
	default:
		break;
	}
}

bool dvStatisticsTreeModel::SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col)
{
	//wxASSERT(item.IsOk());

	//dvStatisticsTreeModelNode *node = (dvStatisticsTreeModelNode*)item.GetID();
	//switch (col)
	//{
	//case 0:
	//	node->m_title = variant.GetString();
	//	return true;
	//case 1:
	//	node->m_artist = variant.GetString();
	//	return true;
	//case 2:
	//	node->m_year = variant.GetLong();
	//	return true;
	//case 3:
	//	node->m_quality = variant.GetString();
	//	return true;
	//default:
	//	break;
	//}

	return false;
}

wxDataViewItem dvStatisticsTreeModel::GetParent(const wxDataViewItem &item) const
{
	// the invisible root node has no parent
	if (!item.IsOk())
		return wxDataViewItem(0);

	dvStatisticsTreeModelNode *node = (dvStatisticsTreeModelNode*)item.GetID();

	// "MyMusic" also has no parent
	if (node == m_root)
		return wxDataViewItem(0);

	return wxDataViewItem((void*)node->GetParent());
}

bool dvStatisticsTreeModel::IsContainer(const wxDataViewItem &item) const
{
	// the invisble root node can have children
	if (!item.IsOk())
		return true;

	dvStatisticsTreeModelNode *node = (dvStatisticsTreeModelNode*)item.GetID();
	return node->IsContainer();
}

unsigned int dvStatisticsTreeModel::GetChildren(const wxDataViewItem &parent, wxDataViewItemArray &array) const
{
	dvStatisticsTreeModelNode *node = (dvStatisticsTreeModelNode*)parent.GetID();

	if (!node)
	{
		array.Add(wxDataViewItem((void*)m_root));
		return 1;
	}

	if (node->GetChildCount() == 0)
	{
		return 0;
	}

	unsigned int count = node->GetChildren().size();
	for (unsigned int pos = 0; pos < count; pos++)
	{
		dvStatisticsTreeModelNode *child = node->GetChildren()[pos];
		array.Add(wxDataViewItem((void*)child));
	}

	return count;
}

void dvStatisticsTreeModel::Refresh(std::vector<wxDVVariableStatistics*> stats)
{
	wxDVStatisticsDataSet *ds;
	dvStatisticsTreeModelNode *groupNode;
	dvStatisticsTreeModelNode *variableNode;
	dvStatisticsTreeModelNode *monthNode;
	StatisticsPoint p;
	wxString groupName = "";

	//Clear existing nodes
	if (m_root == NULL)
	{
		m_root = new dvStatisticsTreeModelNode(NULL, "All");
	}
	else
	{
		m_root->RemoveAllChildren();
	}

	//Repopulate nodes, organizing them by group
	for (int i = 0; i < stats.size(); i++)
	{
		groupName = "";
		for (int j = 0; j < m_root->GetChildCount(); j++)
		{
			if (m_root->GetNthChild(j)->m_nodeName == stats[i]->GetGroupName())
			{
				groupNode = m_root->GetNthChild(j);
				groupName = groupNode->m_nodeName;
				break;
			}
		}

		if (groupName == "") 
		{ 
			groupName = stats[i]->GetGroupName();
			groupNode = new dvStatisticsTreeModelNode(m_root, groupName); 
			m_root->Append(groupNode);
		}

		ds = stats[i]->GetDataSet();

		variableNode = new dvStatisticsTreeModelNode(groupNode, ds->GetSeriesTitle() + " (" + ds->GetUnits() + ")");

		for (int j = 0; j < ds->Length(); j++)
		{
			p = ds->At(j);
			monthNode = new dvStatisticsTreeModelNode(variableNode, p.name, p.Mean, p.Min, p.Max, p.Sum, p.StDev, p.AvgDailyMin, p.AvgDailyMax);
			variableNode->Append(monthNode);
		}

		groupNode->Append(variableNode);
	}
}


//wxDVVariableStatistics

wxDVVariableStatistics::wxDVVariableStatistics(wxDVStatisticsDataSet *ds, wxString GroupName, bool OwnsDataset)
: m_data(ds)
{
	m_ownsDataset = OwnsDataset;
	m_groupName = GroupName;
}

wxDVVariableStatistics::~wxDVVariableStatistics()
{
	if (m_ownsDataset)
	{
		delete m_data;
	}
}
//
//wxString wxDVVariableStatistics::GetXDataLabel() const
//{
//	return _("Hours since 00:00 Jan 1");
//}
//
//wxString wxDVVariableStatistics::GetYDataLabel() const
//{
//	wxString label = m_data->GetSeriesTitle();
//	if (!m_data->GetUnits().IsEmpty())
//		label += " (" + m_data->GetUnits() + ")";
//	return label;
//}
//
//wxRealPoint wxDVVariableStatistics::At(size_t i) const
//{
//	return (i < m_data->Length())
//		? wxRealPoint(m_data->At(i).x, m_data->At(i).Mean)
//		: wxRealPoint(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());
//}

StatisticsPoint wxDVVariableStatistics::At(size_t i, double m_offset, double m_timestep) const
{
	StatisticsPoint p = StatisticsPoint();

	if ((i < m_data->Length()) && (i >= 0))
	{
		p.x = m_data->At(i).x;
		p.Sum = m_data->At(i).Sum;
		p.Min = m_data->At(i).Min;
		p.Max = m_data->At(i).Max;
		p.Mean = m_data->At(i).Mean;
		p.StDev = m_data->At(i).StDev;
		p.AvgDailyMin = m_data->At(i).AvgDailyMin;
		p.AvgDailyMax = m_data->At(i).AvgDailyMax;
	}
	else
	{
		p.x = m_offset + (i * m_timestep);
		p.Sum = 0.0;
		p.Min = 0.0;
		p.Max = 0.0;
		p.Mean = 0.0;
		p.StDev = 0.0;
		p.AvgDailyMin = 0.0;
		p.AvgDailyMax = 0.0;
	}

	return p;
}
//
//size_t wxDVVariableStatistics::Len() const
//{
//	return m_data->Length();
//}
//
//void wxDVVariableStatistics::Draw(wxDC &dc, const wxPLDeviceMapping &map)
//{
//	if (!m_data || m_data->Length() < 2) return;
//
//	size_t len;
//	std::vector< wxPoint > points;
//	wxRealPoint rpt;
//	wxRealPoint rpt2;
//	double tempY;
//	wxRealPoint wmin = map.GetWorldMinimum();
//	wxRealPoint wmax = map.GetWorldMaximum();
//
//	dc.SetPen(wxPen(wxColour("black") , 2, wxPENSTYLE_SOLID));
//
//	//TODO:  UPDATE BELOW CODE TO DO statistics table INSTEAD OF LINE GRAPH
//	//len = m_data->Length();
//	//if (m_data->At(0).x < wmin.x) { len++; }
//	//if (m_data->At(m_data->Length() - 1).x > wmax.x) { len++; }
//
//	//points.reserve(len);
//
//	//for (size_t i = 0; i < m_data->Length(); i++)
//	//{
//	//	rpt = m_data->At(i);
//	//	if (rpt.x < wmin.x || rpt.x > wmax.x) continue;
//	//	points.push_back(map.ToDevice(m_data->At(i)));
//	//}
//
//	//if (points.size() == 0) return;
//
//	//dc.DrawLines(points.size(), &points[0]);
//}
//
//void wxDVVariableStatistics::DrawInLegend(wxDC &dc, const wxRect &rct)
//{
//	// nothing to do here
//}

double wxDVVariableStatistics::GetPeriodLowerBoundary(double hourNumber)
{
	hourNumber = fmod(hourNumber, 8760);

	if (hourNumber < 744.0) { hourNumber = 0.0; }
	else if (hourNumber < 1416.0) { hourNumber = 744.0; }
	else if (hourNumber < 2160.0) { hourNumber = 1416.0; }
	else if (hourNumber < 2880.0) { hourNumber = 2160.0; }
	else if (hourNumber < 3624.0) { hourNumber = 2880.0; }
	else if (hourNumber < 4344.0) { hourNumber = 3624.0; }
	else if (hourNumber < 5088.0) { hourNumber = 4344.0; }
	else if (hourNumber < 5832.0) { hourNumber = 5088.0; }
	else if (hourNumber < 6552.0) { hourNumber = 5832.0; }
	else if (hourNumber < 7296.0) { hourNumber = 6552.0; }
	else if (hourNumber < 8016.0) { hourNumber = 7296.0; }
	else if (hourNumber < 8760.0) { hourNumber = 8016.0; }

	return hourNumber;
}

double wxDVVariableStatistics::GetPeriodUpperBoundary(double hourNumber)
{
	hourNumber = fmod(hourNumber, 8760);

	if (hourNumber < 744.0) { hourNumber = 744.0; }
	else if (hourNumber < 1416.0) { hourNumber = 1416.0; }
	else if (hourNumber < 2160.0) { hourNumber = 2160.0; }
	else if (hourNumber < 2880.0) { hourNumber = 2880.0; }
	else if (hourNumber < 3624.0) { hourNumber = 3624.0; }
	else if (hourNumber < 4344.0) { hourNumber = 4344.0; }
	else if (hourNumber < 5088.0) { hourNumber = 5088.0; }
	else if (hourNumber < 5832.0) { hourNumber = 5832.0; }
	else if (hourNumber < 6552.0) { hourNumber = 6552.0; }
	else if (hourNumber < 7296.0) { hourNumber = 7296.0; }
	else if (hourNumber < 8016.0) { hourNumber = 8016.0; }
	else if (hourNumber < 8760.0) { hourNumber = 8760.0; }

	return hourNumber;
}

wxString wxDVVariableStatistics::GetGroupName()
{
	return m_groupName;
}

std::vector<wxString> wxDVVariableStatistics::GetExportableDatasetHeaders(wxUniChar sep, StatisticsType type) const
{
	std::vector<wxString> tt;
	wxString xLabel = "Month";	//GetXDataLabel();
	wxString yLabel = m_groupName;	//GetYDataLabel();

	if (xLabel.size() == 0) { xLabel = "Month"; }

	//Remove sep chars that we don't want
	while (xLabel.Find(sep) != wxNOT_FOUND)
	{
		xLabel = xLabel.BeforeFirst(sep) + xLabel.AfterFirst(sep);
	}

	while (yLabel.Find(sep) != wxNOT_FOUND)
	{
		yLabel = yLabel.BeforeFirst(sep) + yLabel.AfterFirst(sep);
	}

	tt.push_back(xLabel);
	if (type == MEAN) { tt.push_back("Mean " + yLabel); }
	if (type == MIN) { tt.push_back("Min. " + yLabel); }
	if (type == MAX) { tt.push_back("Max. " + yLabel); }
	if (type == SUMMATION) { tt.push_back("Sum " + yLabel); }
	if (type == STDEV) { tt.push_back("St. Dev. " + yLabel); }
	if (type == AVGDAILYMIN) { tt.push_back("Avg. Daily Min. " + yLabel); }
	if (type == AVGDAILYMAX) { tt.push_back("Avg. Daily Max. " + yLabel); }

	return tt;
}

std::vector<wxRealPoint> wxDVVariableStatistics::GetExportableDataset(StatisticsType type) const
{
	std::vector<wxRealPoint> data;
	wxRealPoint pt;
	StatisticsPoint sp;

	for (size_t i = 0; i < m_data->Length(); i++)
	{
		sp = At(i, m_data->GetOffset(), m_data->GetTimeStep());

		if (type == MEAN) { pt = wxRealPoint(sp.x, sp.Mean); }
		if (type == MIN) { pt = wxRealPoint(sp.x, sp.Min); }
		if (type == MAX) { pt = wxRealPoint(sp.x, sp.Max); }
		if (type == SUMMATION) { pt = wxRealPoint(sp.x, sp.Sum); }
		if (type == STDEV) { pt = wxRealPoint(sp.x, sp.StDev); }
		if (type == AVGDAILYMIN) { pt = wxRealPoint(sp.x, sp.AvgDailyMin); }
		if (type == AVGDAILYMAX) { pt = wxRealPoint(sp.x, sp.AvgDailyMax); }

		data.push_back(pt);
	}

	return data;
}

enum{
	ID_PLOT_SURFACE = wxID_HIGHEST + 1
};


//wxDVStatisticsTableCtrl

wxDVStatisticsTableCtrl::wxDVStatisticsTableCtrl(wxWindow *parent, wxWindowID id)
: wxPanel(parent, id)
{
	m_ctrl = new wxDataViewCtrl(this, ID_STATISTICS_CTRL, wxDefaultPosition, wxSize(1040, 720), wxDV_ROW_LINES);
	m_StatisticsModel = new dvStatisticsTreeModel();

	//m_plotSurface = new wxPLPlotCtrl(this, ID_PLOT_SURFACE);
	//m_plotSurface->SetAllowHighlighting(false);
	//m_plotSurface->ShowTitle(false);
	//m_plotSurface->ShowLegend(false);

	wxBoxSizer *table_sizer = new wxBoxSizer(wxHORIZONTAL);
	table_sizer->Add(m_ctrl, 1, wxEXPAND | wxALL, 4);

	wxBoxSizer *top_sizer = new wxBoxSizer(wxVERTICAL);
	top_sizer->Add(table_sizer, 1, wxALL | wxEXPAND, 0);
	SetSizer(top_sizer);
}

wxDVStatisticsTableCtrl::~wxDVStatisticsTableCtrl(void)
{
	RemoveAllDataSets();
}

void wxDVStatisticsTableCtrl::Invalidate()
{
	//m_plotSurface->Invalidate();
	//m_plotSurface->Refresh();
}

void wxDVStatisticsTableCtrl::RebuildDataViewCtrl()
{
	wxDataViewTextRenderer *tr;

	m_StatisticsModel->Refresh(m_variableStatistics);
	m_ctrl->ClearColumns();
	m_ctrl->AssociateModel(m_StatisticsModel.get());

	tr = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxALIGN_LEFT);
	wxDataViewColumn *column0 = new wxDataViewColumn("", tr, 0, 200, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
	m_ctrl->AppendColumn(column0);

	tr = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxALIGN_RIGHT);
	wxDataViewColumn *column1 = new wxDataViewColumn("Mean", tr, 1, 120, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE);
	m_ctrl->AppendColumn(column1);

	tr = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxALIGN_RIGHT);
	wxDataViewColumn *column2 = new wxDataViewColumn("Min", tr, 2, 120, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE);
	m_ctrl->AppendColumn(column2);

	tr = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxALIGN_RIGHT);
	wxDataViewColumn *column3 = new wxDataViewColumn("Max", tr, 3, 120, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE);
	m_ctrl->AppendColumn(column3);

	tr = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxALIGN_RIGHT);
	wxDataViewColumn *column4 = new wxDataViewColumn("Sum", tr, 4, 120, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE);
	m_ctrl->AppendColumn(column4);

	tr = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxALIGN_RIGHT);
	wxDataViewColumn *column5 = new wxDataViewColumn("Std Dev", tr, 5, 120, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE);
	m_ctrl->AppendColumn(column5);

	tr = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxALIGN_RIGHT);
	wxDataViewColumn *column6 = new wxDataViewColumn("Avg Daily Min", tr, 6, 120, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE);
	m_ctrl->AppendColumn(column6);

	tr = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxALIGN_RIGHT);
	wxDataViewColumn *column7 = new wxDataViewColumn("Avg Daily Max", tr, 7, 120, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE);
	m_ctrl->AppendColumn(column7);
}

void wxDVStatisticsTableCtrl::OnCollapse(wxCommandEvent& WXUNUSED(event))
{
	wxDataViewItem item = m_ctrl->GetSelection();
	if (item.IsOk())
		m_ctrl->Collapse(item);
}

void wxDVStatisticsTableCtrl::OnExpand(wxCommandEvent& WXUNUSED(event))
{
	wxDataViewItem item = m_ctrl->GetSelection();
	if (item.IsOk())
		m_ctrl->Expand(item);
}

void wxDVStatisticsTableCtrl::AddDataSet(wxDVTimeSeriesDataSet *d, const wxString& group)
{
	wxDVStatisticsDataSet *s;
	wxDVVariableStatistics *p;

	s = new wxDVStatisticsDataSet(d);
	p = new wxDVVariableStatistics(s, group, true);

	m_variableStatistics.push_back(p); //Add to data sets list.
}

bool wxDVStatisticsTableCtrl::RemoveDataSet(wxDVTimeSeriesDataSet *d)
{
	//wxDVVariableStatistics *plotToRemove = NULL;
	wxDVStatisticsDataSet *ds;
	int removedIndex = 0;

	//Find the plottable:
	for (size_t i = 0; i < m_variableStatistics.size(); i++)
	{
		ds = m_variableStatistics[i]->GetDataSet();

		if (ds->IsSourceDataset(d))
		{
			removedIndex = i;
			//plotToRemove = m_variableStatistics[i];
			break;
		}
	}

	//if (!plotToRemove)
	//	return false;

	//for (int i = 0; i<wxPLPlotCtrl::NPLOTPOS; i++)
	//	m_plotSurface->RemovePlot(plotToRemove);

	m_variableStatistics.erase(m_variableStatistics.begin() + removedIndex); //This is more efficient than remove when we already know the index.

	RebuildDataViewCtrl();

	return true;
}

void wxDVStatisticsTableCtrl::RemoveAllDataSets()
{
	//Remove all data sets. Deleting a data set also deletes its plottable.
	for (size_t i = 0; i < m_variableStatistics.size(); i++)
		delete m_variableStatistics[i];

	m_variableStatistics.clear();

	RebuildDataViewCtrl();
}
