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

#ifndef __vtkMRMLPlotSeriesNode_h
#define __vtkMRMLPlotSeriesNode_h

#include <string>
#include <vector>

// MRML Includes
#include "vtkMRMLNode.h"

class vtkMRMLStorageNode;
class vtkMRMLTableNode;

// VTK Includes
class vtkColor4ub;
class vtkPlot;
class vtkTable;

/// \brief MRML node to represent a vtkPlot object
///
/// Plot nodes describe the properties of a single plot:
/// input data (one or two columns of a table node),
/// and display properties (plot type, marker style, color, etc).
class VTK_MRML_EXPORT vtkMRMLPlotSeriesNode : public vtkMRMLNode
{
public:
  static vtkMRMLPlotSeriesNode *New();
  vtkTypeMacro(vtkMRMLPlotSeriesNode,vtkMRMLNode);

  /// Enumerated values for SetPlot/GetPlot
  enum
    {
    PlotTypeLine,
    PlotTypeBar,
    PlotTypeScatter,
    PlotTypeScatterBar,
    PlotType_Last // must be last
    };

  /// Enumerated values for SetMarkerStyle/GetMarkerStyle
  enum
    {
    MarkerStyleNone,
    MarkerStyleCross,
    MarkerStylePlus,
    MarkerStyleSquare,
    MarkerStyleCircle,
    MarkerStyleDiamond,
    MarkerStyle_Last // must be last
    };

  /// Enumerated values for SetLineStyle/GetLineStyle
  enum
    {
    LineStyleNone,
    LineStyleSolid,
    LineStyleDash,
    LineStyleDot,
    LineStyleDashDot,
    LineStyleDashDotDot,
    LineStyle_Last // must be last
    };

  void PrintSelf(ostream& os, vtkIndent indent) override;

  //----------------------------------------------------------------
  /// Standard methods for MRML nodes
  //----------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes.
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Get node XML tag name (like Volume, Model).
  const char* GetNodeTagName() override { return "PlotSeries"; };

  ///
  /// Set and observe Table node ID.
  /// \sa TableNodeID, GetTableNodeID(), SetInputData()
  virtual void SetAndObserveTableNodeID(const char *tableNodeID);

  ///
  /// Set and observe Table node ID.
  /// Utility method that conveniently takes a string instead of a char*.
  /// \sa TableNodeID, GetTableNodeID(), SetInputData()
  virtual void SetAndObserveTableNodeID(const std::string& tableNodeID);

  ///
  /// Get associated Table MRML node.
  virtual vtkMRMLTableNode* GetTableNode();

  ///
  /// Method to propagate events generated in Plot nodes.
  void ProcessMRMLEvents (vtkObject *caller,
                                  unsigned long event,
                                  void *callData) override;

  ///
  /// TableModifiedEvent is send when the parent table is modified
  enum
    {
      TableModifiedEvent = 15000
    };

  ///
  /// Get referenced transform node id
  const char *GetTableNodeID();

  //----------------------------------------------------------------
  /// Get and Set Macros
  //----------------------------------------------------------------

  /// Get/Set the type of the plot (line, bar, scatter, scatter bar).
  /// PlotTypeLine: line points are equally spaced along X axis, Y column defines height of each point
  /// PlotTypeBar: vertical bars equally spaced along X axis, Y column defines height of each bar
  /// PlotTypeScatter: X and Y columns define X and Y coordinates of points, connected by line
  /// PlotTypeScatterBar: X and Y columns define bar position and height
  vtkSetMacro(PlotType, int);
  vtkGetMacro(PlotType, int);

  ///
  /// Convenience method to set the type of
  /// the plot from strings.
  virtual void SetPlotType(const char* type);

  /// Get the name of the Y column in the referenced table (for scatter plots).
  vtkGetMacro(XColumnName, std::string);
  vtkSetMacro(XColumnName, std::string);

  ///
  /// Get the name of the Y column in the referenced table.
  vtkGetMacro(YColumnName, std::string);
  vtkSetMacro(YColumnName, std::string);

  /// Get/Set the name of the X (for scatter plot)
  /// or labels (for line or bar plot) column in the referenced table.
  vtkGetMacro(LabelColumnName, std::string);
  vtkSetMacro(LabelColumnName, std::string);

  ///
  /// Convert between plot type ID and name
  static const char *GetPlotTypeAsString(int id);
  static int GetPlotTypeFromString(const char *name);

  ///
  /// Utility methods to set/get the marker style
  /// available for line plots.
  /// Valid values: MarkerStyleNone, MarkerStyleCross, MarkerStylePlus,
  /// MarkerStyleSquare, MarkerStyleCircle, MarkerStyleDiamond
  vtkSetMacro(MarkerStyle, int);
  vtkGetMacro(MarkerStyle, int);

  ///
  /// Convert between plot markers style ID and name
  static const char *GetMarkerStyleAsString(int id);
  static int GetMarkerStyleFromString(const char *name);

  ///
  /// Utility methods to set/get the marker size
  /// available for line plots.
  vtkGetMacro(MarkerSize, float);
  vtkSetMacro(MarkerSize, float);

  ///
  /// Set/get the line style.
  /// Valid values: LineStyleNone, LineStyleSolid, LineStyleDash,
  /// LineStyleDot, LineStyleDashDot, LineStyleDashDotDot.
  vtkSetMacro(LineStyle, int);
  vtkGetMacro(LineStyle, int);

  ///
  /// Convert between line style ID and name
  const char *GetLineStyleAsString(int id);
  int GetLineStyleFromString(const char *name);

  ///
  /// Utility methods to set/get the Line width
  /// available for Line Plots.
  vtkGetMacro(LineWidth, float);
  vtkSetMacro(LineWidth, float);

  ///
  /// Set/Get Color of the line and markers of the plot
  vtkGetVector3Macro(Color, double);
  vtkSetVector3Macro(Color, double);

  ///
  /// Utility function that generates a color that is not used by other plot series nodes.
  /// If colorTableNodeID is not specified then vtkMRMLColorTableNodeRandom is used.
  void SetUniqueColor(const char* colorTableNodeID = nullptr);

  ///
  /// Get set line opacity
  vtkGetMacro(Opacity, double);
  vtkSetMacro(Opacity, double);

  ///
  /// Returns true if X column is required (true for scatter plots)
  bool IsXColumnRequired();

  //----------------------------------------------------------------
  /// Constructor and destructor
  //----------------------------------------------------------------
protected:
  vtkMRMLPlotSeriesNode();
  ~vtkMRMLPlotSeriesNode() override;
  vtkMRMLPlotSeriesNode(const vtkMRMLPlotSeriesNode&);
  void operator=(const vtkMRMLPlotSeriesNode&);

  static const char* TableNodeReferenceRole;
  static const char* TableNodeReferenceMRMLAttributeName;

  virtual const char* GetTableNodeReferenceRole();
  virtual const char* GetTableNodeReferenceMRMLAttributeName();

  ///
  /// Called when a node reference ID is added (list size increased).
  void OnNodeReferenceAdded(vtkMRMLNodeReference *reference) override
  {
    Superclass::OnNodeReferenceAdded(reference);
    if (std::string(reference->GetReferenceRole()) == this->TableNodeReferenceRole)
      {
      this->InvokeCustomModifiedEvent(vtkMRMLPlotSeriesNode::TableModifiedEvent, reference->GetReferencedNode());
      }
  }

  ///
  /// Called when a node reference ID is modified.
  void OnNodeReferenceModified(vtkMRMLNodeReference *reference) override
  {
    Superclass::OnNodeReferenceModified(reference);
    if (std::string(reference->GetReferenceRole()) == this->TableNodeReferenceRole)
    {
      this->InvokeCustomModifiedEvent(vtkMRMLPlotSeriesNode::TableModifiedEvent, reference->GetReferencedNode());
    }
  }

  ///
  /// Called after a node reference ID is removed (list size decreased).
  void OnNodeReferenceRemoved(vtkMRMLNodeReference *reference) override
  {
    Superclass::OnNodeReferenceRemoved(reference);
    if (std::string(reference->GetReferenceRole()) == this->TableNodeReferenceRole)
    {
      this->InvokeCustomModifiedEvent(vtkMRMLPlotSeriesNode::TableModifiedEvent, reference->GetReferencedNode());
    }
  }

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLPlotSeriesNode);

  //----------------------------------------------------------------
  /// Data
  //----------------------------------------------------------------
 protected:

  ///
  /// Type of Plot (scatter, line, bar, ...).
  int PlotType{PlotTypeLine};

  std::string XColumnName;
  std::string YColumnName;
  std::string LabelColumnName;

  float LineWidth{2};
  int LineStyle{LineStyleSolid};

  float MarkerSize{7};
  int MarkerStyle{MarkerStyleCircle};

  double Color[3];
  double Opacity{1.0};
};

#endif
