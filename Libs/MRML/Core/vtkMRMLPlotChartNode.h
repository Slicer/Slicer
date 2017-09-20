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
class vtkMRMLPlotDataNode;
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

  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  virtual vtkMRMLNode* CreateNodeInstance() VTK_OVERRIDE;

  ///
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts) VTK_OVERRIDE;

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent) VTK_OVERRIDE;

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node) VTK_OVERRIDE;

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() VTK_OVERRIDE {return "PlotChart";};

  ///
  /// Method to propagate events generated in mrml
  virtual void ProcessMRMLEvents(vtkObject *caller,
                                 unsigned long event,
                                 void *callData) VTK_OVERRIDE;

  /// PlotModifiedEvent is fired when:
  ///  - a new plot node is observed
  ///  - a plot node is not longer observed
  ///  - an associated plot node is modified
  /// Note that when SetAndObserve(Nth)NodeID() is called with an ID that
  /// has not yet any associated plot node in the scene, then
  /// plotModifiedEvent is not fired until found for the first time in
  /// the scene, e.g. Get(Nth)PlotDataNode(), UpdateScene()...
  enum
    {
    PlotModifiedEvent = 17000,
    };

  //----------------------------------------------------------------
  /// Access methods
  //----------------------------------------------------------------

  ///
  /// Convenience method that sets the first plot node ID.
  /// \sa SetAndObserverNthPlotDataNodeID(int, const char*)
  void SetAndObservePlotDataNodeID(const char *plotDataNodeID);

  ///
  /// Convenience method that adds a plot node ID at the end of the list.
  /// \sa SetAndObserverNthPlotDataNodeID(int, const char*)
  void AddAndObservePlotDataNodeID(const char *plotDataNodeID);

  ///
  /// Convenience method that removes the plot node ID from the list
  /// \sa SetAndObserverNthPlotDataNodeID(int, const char*)
  void RemovePlotDataNodeID(const char *plotDataNodeID);

  ///
  /// Convenience method that removes the Nth plot node ID from the list
  /// \sa SetAndObserverNthPlotDataNodeID(int, const char*)
  void RemoveNthPlotDataNodeID(int n);

  ///
  /// Remove all plot node IDs and associated plot nodes.
  void RemoveAllPlotDataNodeIDs();

  ///
  /// Set and observe the Nth plot node ID in the list.
  /// If n is larger than the number of plot nodes, the plot node ID
  /// is added at the end of the list. If PlotDataNodeID is 0, the node ID is
  /// removed from the list.
  /// When a node ID is set (added or changed), its corresponding node is
  /// searched (slow) into the scene and cached for fast future access.
  /// It is possible however that the node is not yet into the scene (due to
  /// some temporary state (at loading time for example). UpdateScene() can
  /// later be called to retrieve the plot nodes from the scene
  /// (automatically done when loading a scene). Get(Nth)PlotDataNode() also
  /// scan the scene if the node was not yet cached.
  /// \sa SetAndObservePlotDataNodeID(const char*),
  /// AddAndObservePlotDataNodeID(const char *), RemoveNthPlotDataNodeID(int)
  void SetAndObserveNthPlotDataNodeID(int n, const char *plotDataNodeID);

  ///
  /// Return true if PlotDataNodeID is in the plot node ID list.
  bool HasPlotDataNodeID(const char* plotDataNodeID);

  ///
  /// Return the number of plot node IDs (and plot nodes as they always
  /// have the same size).
  int GetNumberOfPlotDataNodes();

  ///
  /// Return the string of the Nth plot node ID. Or 0 if no such
  /// node exist.
  /// Warning, a temporary char generated from a std::string::c_str()
  /// is returned.
  const char *GetNthPlotDataNodeID(int n);

  ///
  /// Return the index of the Nth plot node ID.
  /// If not found, it return '-1'
  int GetNthPlotIdexFromID(const char* plotDataNodeID);

  ///
  /// Return an index to color each Plot.
  /// If not found, it return '-1'
  vtkIdType GetColorPlotIdexFromID(const char* plotDataNodeID);

  ///
  /// Utility function that returns the first plot node id.
  /// \sa GetNthPlotDataNodeID(int), GetPlotDataNode()
  const char *GetPlotDataNodeID();

  ///
  /// Get associated plot MRML node. Can be 0 in temporary states; e.g. if
  /// the plot node has no scene, or if the associated plot is not
  /// yet into the scene.
  /// If not cached, it internally scans (slow) the scene to search for the
  /// associated plot node ID.
  /// If the plot node is no longer in the scene (GetScene() == 0), it
  /// happens after the node is removed from the scene (scene->RemoveNode(dn),
  /// the returned plot node is 0.
  /// \sa GetNthPlotDataNodeByClass()
  vtkMRMLPlotDataNode* GetNthPlotDataNode(int n);

  ///
  /// Utility function that returns the first plot node.
  /// \sa GetNthPlotDataNode(int), GetPlotDataNodeID()
  vtkMRMLPlotDataNode* GetPlotDataNode();

  ///
  /// Get the list of Plot names
  virtual int GetPlotNames(std::vector<std::string> &plotDataNodeNames);

  ///
  /// Get the list of Plot ids
  virtual int GetPlotIDs(std::vector<std::string> &plotDataNodeIDs);

  ///
  /// Set the Type for all the referenced Plots.
  /// Type is also an attribute (see below),
  /// but this method has to be used to properly set all the referenced Plots.
  virtual void SetPlotType(const char* Type);

  ///
  /// In addition a set of properties are available for a PlotChart.
  /// These are stored as Attributes of PlotChartNode.
  /// Available properties are:
  ///
  /// \li  "Type" - "Line", "Line and Scatter", "Scatter", "Bar"
  /// \li  "TitleName" - title ploted on the PlotChart
  /// \li  "ShowTitle" - show title "on" or "off"
  /// \li  "XAxisLabelName" - label ploted on the x-axis
  /// \li  "ShowXAxisLabel" - show x-axis label "on" or "off"
  /// \li  "ClickAndDragAlongX" - set the action along x-axis "on" or "off"
  /// \li  "ClickAndDragAlongY" - set the action along y-axis "on" or "off"
  /// \li  "YAxisLabelName" - label ploted on the y-axis
  /// \li  "ShowYAxisLabel" - show y-axis label "on" or "off"
  /// \li  "ShowGrid" - show grid "on" or "off"
  /// \li  "ShowLegend" - show legend "on" or "off"
  /// \li  "FontType" - global Font for the PlotChart: "Arial", "Times", "Courier"
  /// \li  "TitleFontSize" - default: "20"
  /// \li  "AxisTitleFontSize" - default: "16"
  /// \li  "AxisLabelFontSize" - default: "12"
  /// \li  "LookupTable" colorNodeID default: NULL
  ///
  ///  NOTE: To change the PlotType SetPlotType has to be used.
  ///

  virtual const char* GetPlotDataNodeReferenceRole();

 protected:
  //----------------------------------------------------------------
  /// Constructor and destroctor
  //----------------------------------------------------------------
  vtkMRMLPlotChartNode();
  ~vtkMRMLPlotChartNode();
  vtkMRMLPlotChartNode(const vtkMRMLPlotChartNode&);
  void operator=(const vtkMRMLPlotChartNode&);

  static const char* PlotDataNodeReferenceRole;
  static const char* PlotDataNodeReferenceMRMLAttributeName;

  virtual const char* GetPlotDataNodeReferenceMRMLAttributeName();

  ///
  /// Called when a node reference ID is added (list size increased).
  virtual void OnNodeReferenceAdded(vtkMRMLNodeReference *reference) VTK_OVERRIDE;

  ///
  /// Called when a node reference ID is modified.
  virtual void OnNodeReferenceModified(vtkMRMLNodeReference *reference) VTK_OVERRIDE;

  ///
  /// Called after a node reference ID is removed (list size decreased).
  virtual void OnNodeReferenceRemoved(vtkMRMLNodeReference *reference) VTK_OVERRIDE;
};

#endif
