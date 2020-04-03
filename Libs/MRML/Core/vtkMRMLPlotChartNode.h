/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

#ifndef __vtkMRMLPlotChartNode_h
#define __vtkMRMLPlotChartNode_h

#include "vtkMRMLNode.h"

class vtkCollection;
class vtkDataObject;
class vtkMRMLPlotSeriesNode;
class vtkStringArray;

#include <string>

/// \brief MRML node for referencing a collection of data to plot.
class VTK_MRML_EXPORT vtkMRMLPlotChartNode : public vtkMRMLNode
{
 public:
  //----------------------------------------------------------------
  /// Standard methods for MRML nodes
  //----------------------------------------------------------------

  static vtkMRMLPlotChartNode *New();
  vtkTypeMacro(vtkMRMLPlotChartNode,vtkMRMLNode);

  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes.
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLPlotChartNode);

  ///
  /// Get node XML tag name (like Volume, Model).
  const char* GetNodeTagName() override {return "PlotChart";};

  ///
  /// Method to propagate events generated in mrml.
  void ProcessMRMLEvents(vtkObject *caller,
                                 unsigned long event,
                                 void *callData) override;

  /// PlotModifiedEvent is fired when:
  ///  - a new plot node is observed
  ///  - a plot node is not longer observed
  ///  - an associated plot node is modified
  /// Note that when SetAndObserve(Nth)NodeID() is called with an ID that
  /// has not yet any associated plot node in the scene, then
  /// plotModifiedEvent is not fired until found for the first time in
  /// the scene, e.g. Get(Nth)PlotSeriesNode(), UpdateScene()...
  enum
    {
    PlotModifiedEvent = 17000,
    };

  /// Properties used by SetPropertyToAllPlotSeriesNodes() and GetPropertyFromAllPlotSeriesNodes() methods.
  enum PlotSeriesNodeProperty
  {
    PlotType,
    PlotXColumnName,
    PlotYColumnName,
    PlotMarkerStyle
  };

  //----------------------------------------------------------------
  /// Access methods
  //----------------------------------------------------------------

  ///
  /// Convenience method that returns the ID of the first plot data node in the chart.
  /// \sa GetNthPlotSeriesNodeID(int), GetPlotSeriesNode()
  const char *GetPlotSeriesNodeID();

  ///
  /// Convenience method that returns the first plot data node.
  /// \sa GetNthPlotSeriesNode(int), GetPlotSeriesNodeID()
  vtkMRMLPlotSeriesNode* GetPlotSeriesNode();

  ///
  /// Return the ID of n-th plot data node ID. Or 0 if no such node exist.
  const char *GetNthPlotSeriesNodeID(int n);

  ///
  /// Get associated plot data node. Can be 0 in temporary states; e.g. if
  /// the plot node has no scene, or if the associated plot is not
  /// yet into the scene.
  vtkMRMLPlotSeriesNode* GetNthPlotSeriesNode(int n);

  ///
  /// Return the index of the Nth plot node ID.
  /// If not found, it returns -1.
  int GetPlotSeriesNodeIndexFromID(const char* plotSeriesNodeID);

  ///
  /// Get IDs of all associated plot data nodes.
  virtual int GetPlotSeriesNodeIDs(std::vector<std::string> &plotSeriesNodeIDs);

  ///
  /// Get names of all associated plot data nodes.
  virtual int GetPlotSeriesNodeNames(std::vector<std::string> &plotSeriesNodeNames);

  ///
  /// Return the number of plot node IDs (and plot nodes as they always
  /// have the same size).
  int GetNumberOfPlotSeriesNodes();

  ///
  /// Adds a plot data node to the chart.
  /// \sa SetAndObserverNthPlotSeriesNodeID(int, const char*)
  void AddAndObservePlotSeriesNodeID(const char *plotSeriesNodeID);

  ///
  /// Convenience method that sets the first plot data node in the chart.
  /// \sa SetAndObserverNthPlotSeriesNodeID(int, const char*)
  void SetAndObservePlotSeriesNodeID(const char *plotSeriesNodeID);

  ///
  /// Set and observe the Nth plot data node ID in the list.
  /// If n is larger than the number of plot nodes, the plot node ID
  /// is added at the end of the list. If PlotSeriesNodeID is 0, the node ID is
  /// removed from the list.
  /// \sa SetAndObservePlotSeriesNodeID(const char*),
  /// AddAndObservePlotSeriesNodeID(const char *), RemoveNthPlotSeriesNodeID(int)
  void SetAndObserveNthPlotSeriesNodeID(int n, const char *plotSeriesNodeID);

  ///
  /// Removes a plot data node from the chart.
  /// \sa SetAndObserverNthPlotSeriesNodeID(int, const char*)
  void RemovePlotSeriesNodeID(const char *plotSeriesNodeID);

  ///
  /// Removes n-th plot data node from the chart.
  /// \sa SetAndObserverNthPlotSeriesNodeID(int, const char*)
  void RemoveNthPlotSeriesNodeID(int n);

  ///
  /// Remove all plot data nodes from the chart.
  void RemoveAllPlotSeriesNodeIDs();

  ///
  /// Return true if PlotSeriesNodeID is in the plot node ID list.
  bool HasPlotSeriesNodeID(const char* plotSeriesNodeID);


  /// Title of the chart
  vtkSetStringMacro(Title);
  vtkGetStringMacro(Title);

  /// Title font size. Default: 20.
  vtkSetMacro(TitleFontSize, int);
  vtkGetMacro(TitleFontSize, int);

  /// Show title of the chart
  vtkBooleanMacro(TitleVisibility, bool);
  vtkGetMacro(TitleVisibility, bool);
  vtkSetMacro(TitleVisibility, bool);

  /// Show horizontal and vertical grid lines
  vtkBooleanMacro(GridVisibility, bool);
  vtkGetMacro(GridVisibility, bool);
  vtkSetMacro(GridVisibility, bool);

  /// Show horizontal and vertical grid lines
  vtkBooleanMacro(LegendVisibility, bool);
  vtkGetMacro(LegendVisibility, bool);
  vtkSetMacro(LegendVisibility, bool);

  /// Title font size. Default: 20.
  vtkSetMacro(LegendFontSize, int);
  vtkGetMacro(LegendFontSize, int);

  vtkGetMacro(XAxisRangeAuto, bool);
  vtkSetMacro(XAxisRangeAuto, bool);
  vtkBooleanMacro(XAxisRangeAuto, bool);

  vtkGetMacro(YAxisRangeAuto, bool);
  vtkSetMacro(YAxisRangeAuto, bool);
  vtkBooleanMacro(YAxisRangeAuto, bool);

  vtkGetVector2Macro(XAxisRange, double);
  vtkSetVector2Macro(XAxisRange, double);

  vtkGetVector2Macro(YAxisRange, double);
  vtkSetVector2Macro(YAxisRange, double);

  /// Enable logarithmic scaling of X axis.
  /// Only positive axis range can be displayed using logarithmic scale.
  vtkGetMacro(XAxisLogScale, bool);
  vtkSetMacro(XAxisLogScale, bool);
  vtkBooleanMacro(XAxisLogScale, bool);

  /// Enable logarithmic scaling of Y axis.
  /// Only positive axis range can be displayed using logarithmic scale.
  vtkGetMacro(YAxisLogScale, bool);
  vtkSetMacro(YAxisLogScale, bool);
  vtkBooleanMacro(YAxisLogScale, bool);

  /// Title of X axis
  vtkSetStringMacro(XAxisTitle);
  vtkGetStringMacro(XAxisTitle);

  /// Show title of X axis
  vtkBooleanMacro(XAxisTitleVisibility, bool);
  vtkGetMacro(XAxisTitleVisibility, bool);
  vtkSetMacro(XAxisTitleVisibility, bool);

  /// Title of Y axis
  vtkSetStringMacro(YAxisTitle);
  vtkGetStringMacro(YAxisTitle);

  /// Show title of Y axis
  vtkBooleanMacro(YAxisTitleVisibility, bool);
  vtkGetMacro(YAxisTitleVisibility, bool);
  vtkSetMacro(YAxisTitleVisibility, bool);

  /// Axis title font size. Default: 16.
  vtkSetMacro(AxisTitleFontSize, int);
  vtkGetMacro(AxisTitleFontSize, int);

  /// Axis label font size. Default: 12.
  vtkSetMacro(AxisLabelFontSize, int);
  vtkGetMacro(AxisLabelFontSize, int);

  /// Font type for all text in the chart: "Arial", "Times", "Courier"
  vtkSetStringMacro(FontType);
  vtkGetStringMacro(FontType);

  /// Enable moving of data points along X axis by click-and-drag in plot views.
  /// Points may only be moved in the view if moving is enabled in the plot view node as well.
  vtkBooleanMacro(EnablePointMoveAlongX, bool);
  vtkGetMacro(EnablePointMoveAlongX, bool);
  vtkSetMacro(EnablePointMoveAlongX, bool);

  /// Enable moving of data points along Y axis by click-and-drag in plot views.
  /// Points may only be moved in the view if moving is enabled in the plot view node as well.
  vtkBooleanMacro(EnablePointMoveAlongY, bool);
  vtkGetMacro(EnablePointMoveAlongY, bool);
  vtkSetMacro(EnablePointMoveAlongY, bool);

  /// Node reference role used for storing plot data node references
  virtual const char* GetPlotSeriesNodeReferenceRole();

  /// Helper function to set common properties for all associated plot data nodes
  void SetPropertyToAllPlotSeriesNodes(PlotSeriesNodeProperty plotProperty, const char* value);

  /// Helper function to get common properties from all associated plot data nodes.
  /// Returns false if property is not the same in all plots.
  /// value contains the value found in the first plot data node.
  bool GetPropertyFromAllPlotSeriesNodes(PlotSeriesNodeProperty plotProperty, std::string& value);

 protected:
  //----------------------------------------------------------------
  /// Constructor and destructor
  //----------------------------------------------------------------
  vtkMRMLPlotChartNode();
  ~vtkMRMLPlotChartNode() override;
  vtkMRMLPlotChartNode(const vtkMRMLPlotChartNode&);
  void operator=(const vtkMRMLPlotChartNode&);

  ///
  /// Called when a node reference ID is added (list size increased).
  void OnNodeReferenceAdded(vtkMRMLNodeReference *reference) override;

  ///
  /// Called when a node reference ID is modified.
  void OnNodeReferenceModified(vtkMRMLNodeReference *reference) override;

  ///
  /// Called after a node reference ID is removed (list size decreased).
  void OnNodeReferenceRemoved(vtkMRMLNodeReference *reference) override;

  static const char* PlotSeriesNodeReferenceRole;

  char *Title{nullptr};
  int TitleFontSize{20};
  bool TitleVisibility{true};
  bool GridVisibility{true};
  bool LegendVisibility{true};
  int LegendFontSize{16};
  bool XAxisRangeAuto{true};
  bool YAxisRangeAuto{true};
  double XAxisRange[2];
  double YAxisRange[2];
  bool XAxisLogScale{false};
  bool YAxisLogScale{false};
  char* XAxisTitle{nullptr};
  bool XAxisTitleVisibility{true};
  char* YAxisTitle{nullptr};
  bool YAxisTitleVisibility{true};
  int AxisTitleFontSize{16};
  int AxisLabelFontSize{12};
  char* FontType{nullptr};
  bool EnablePointMoveAlongX{true};
  bool EnablePointMoveAlongY{true};
};

#endif
