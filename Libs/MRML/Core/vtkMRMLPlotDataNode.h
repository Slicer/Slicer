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

#ifndef __vtkMRMLPlotDataNode_h
#define __vtkMRMLPlotDataNode_h

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
/// Plot nodes describe the properties of a single plot.
/// In addition, takes care of references for the input
/// (one vtkTable from vtkMRMLTableNode).
/// To be viewed the node has to be added in the reference list
/// of a vtkMRMLPlotViewNode.
class VTK_MRML_EXPORT vtkMRMLPlotDataNode : public vtkMRMLNode
{
public:
  static vtkMRMLPlotDataNode *New();
  vtkTypeMacro(vtkMRMLPlotDataNode,vtkMRMLNode);

  // Description:
  // Enum of the available plot types
  enum {
    LINE,
    POINTS,
    BAR,
  };

  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  //----------------------------------------------------------------
  /// Standard methods for MRML nodes
  //----------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance() VTK_OVERRIDE;

  ///
  /// Set node attributes.
  virtual void ReadXMLAttributes( const char** atts) VTK_OVERRIDE;

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent) VTK_OVERRIDE;

  ///
  /// Get node XML tag name (like Volume, Model).
  virtual const char* GetNodeTagName() VTK_OVERRIDE { return "PlotData"; };

  ///
  /// Set and observe Table node ID.
  /// \sa TableNodeID, GetTableNodeID(), SetInputData()
  virtual bool SetAndObserveTableNodeID(const char *TableNodeID);

  ///
  /// Set and observe Table node ID.
  /// Utility method that conveniently takes a string instead of a char*.
  /// \sa TableNodeID, GetTableNodeID(), SetInputData()
  virtual bool SetAndObserveTableNodeID(const std::string& TableNodeID);

  ///
  /// Get associated Table MRML noide.
  virtual vtkMRMLTableNode* GetTableNode();

  ///
  /// Method to propagate events generated in Plot nodes.
  virtual void ProcessMRMLEvents (vtkObject *caller,
                                  unsigned long event,
                                  void *callData) VTK_OVERRIDE;

  ///
  /// TableModifiedEvent is send when the parent table is modified
  enum
    {
      TableModifiedEvent = 15000
    };

  ///
  /// Get referenced transform node id
  const char *GetTableNodeID();

  ///
  /// Updates other nodes in the scene depending on this node
  /// or updates this node if it depends on other nodes when the scene is read in
  /// This method is called automatically by XML parser after all nodes are created.
  virtual void UpdateScene(vtkMRMLScene * scene) VTK_OVERRIDE;

  ///
  /// Updates this node if it depends on other nodes when the scene is read in
  /// This method is called by scene when a node added to a scene.
  virtual void OnNodeAddedToScene() VTK_OVERRIDE;

  ///
  /// Update the stored reference to another node in the scene.
  virtual void UpdateReferenceID(const char *oldID, const char *newID) VTK_OVERRIDE;

  /// \brief Update the references of the node to the scene.
  ///
  /// \note You must unsure that a valid scene is set before calling
  /// SetSceneReferences().
  virtual void SetSceneReferences() VTK_OVERRIDE;

  //----------------------------------------------------------------
  /// Get and Set Macros
  //----------------------------------------------------------------

  ///
  /// Set and observe a vtkPlot.
  /// \sa SetInputData()
  virtual void SetAndObservePlot(vtkPlot* plot);

  ///
  /// \brief vtkGetObjectMacro
  /// Get observed plot
  vtkGetObjectMacro(Plot, vtkPlot);

  /// Get the type of the plot (line, scatter, bar).
  /// \brief vtkGetMacro
  vtkGetMacro(Type, int);

  ///
  /// Set the type of the plot (line, scatter, bar).
  virtual void SetType(int type);

  ///
  /// Utility method to eet the type of
  /// the plot (line, scatter, bar) from strings.
  virtual void SetType(const char* type);

  ///
  /// Get the name of the XColumn.
  /// \brief vtkGetMacro
  vtkGetMacro(XColumnName, vtkStdString);

  ///
  /// Set the name of the XColumn and assure the data connection.
  /// The value "Indexes" can not be used to point a column in a table.
  /// Such entry will be ignored and numeric indexes will used as deafult.
  /// \brief vtkSetMacro
  /// \sa SetInputData
  virtual void SetXColumnName(vtkStdString xColumnName);

  ///
  /// Get the name of the YColumn.
  /// \brief vtkGetMacro
  vtkGetMacro(YColumnName, vtkStdString);

  ///
  /// Set the name of the YColumn and assure the data connection.
  /// \brief vtkSetMacro
  /// \sa SetInputData
  virtual void SetYColumnName(vtkStdString yColumnName);

  ///
  /// Convert between plot type ID and name
  virtual const char *GetPlotTypeAsString(int id);
  virtual int GetPlotTypeFromString(const char *name);

  ///
  /// Utility methods to set/get the marker style
  /// available for Line and Points Plots.
  virtual void SetMarkerStyle(int style);
  virtual int GetMarkerStyle();

  ///
  /// Convert between plot markers style ID and name
  const char *GetMarkersStyleAsString(int id);
  int GetMarkersStyleFromString(const char *name);

  ///
  /// Utility methods to set/get the marker size
  /// available for Line and Points Plots.
  virtual void SetMarkerSize(float size);
  virtual float GetMarkerSize();

  ///
  /// Utility methods to set/get the Line width
  /// available for Line Plots.
  virtual void SetLineWidth(float width);
  virtual float GetLineWidth();

  ///
  /// Set/Get Color of the vtkPlot
  virtual void SetPlotColor(double color[4]);
  virtual void SetPlotColor(unsigned char color[4]);
  virtual void GetPlotColor(double color[4]);
  virtual void GetPlotColor(unsigned char color[4]);

  //----------------------------------------------------------------
  /// Constructor and destructor
  //----------------------------------------------------------------
protected:
  vtkMRMLPlotDataNode();
  ~vtkMRMLPlotDataNode();
  vtkMRMLPlotDataNode(const vtkMRMLPlotDataNode&);
  void operator=(const vtkMRMLPlotDataNode&);

  static const char* TableNodeReferenceRole;
  static const char* TableNodeReferenceMRMLAttributeName;

  virtual const char* GetTableNodeReferenceRole();
  virtual const char* GetTableNodeReferenceMRMLAttributeName();

  ///
  /// Called when a node reference ID is added (list size increased).
  virtual void OnNodeReferenceAdded(vtkMRMLNodeReference *reference) VTK_OVERRIDE
  {
    Superclass::OnNodeReferenceAdded(reference);
    if (std::string(reference->GetReferenceRole()) == this->TableNodeReferenceRole)
      {
      this->InvokeCustomModifiedEvent(vtkMRMLPlotDataNode::TableModifiedEvent, reference->GetReferencedNode());
      }
  }

  ///
  /// Called when a node reference ID is modified.
  virtual void OnNodeReferenceModified(vtkMRMLNodeReference *reference) VTK_OVERRIDE
  {
    Superclass::OnNodeReferenceModified(reference);
    if (std::string(reference->GetReferenceRole()) == this->TableNodeReferenceRole)
    {
      this->InvokeCustomModifiedEvent(vtkMRMLPlotDataNode::TableModifiedEvent, reference->GetReferencedNode());
    }
  }

  ///
  /// Called after a node reference ID is removed (list size decreased).
  virtual void OnNodeReferenceRemoved(vtkMRMLNodeReference *reference) VTK_OVERRIDE
  {
    Superclass::OnNodeReferenceRemoved(reference);
    if (std::string(reference->GetReferenceRole()) == this->TableNodeReferenceRole)
    {
      this->InvokeCustomModifiedEvent(vtkMRMLPlotDataNode::TableModifiedEvent, reference->GetReferencedNode());
    }
  }

  ///
  /// Copy the node's attributes to this object
  /// This is used only internally.
  /// Externally CopyWithScene has to be called.
  virtual void Copy(vtkMRMLNode *node) VTK_OVERRIDE;

  /// Set input data from a vtkTable.
  /// This method is called internally everytime
  /// that a new vtkPlot or vtkMRMLTable has been
  /// set and observed.
  /// \sa vtkPlot->SetInputData()
  virtual void SetInputData(vtkMRMLTableNode* tableNode,
                            vtkStdString xColumnName,
                            vtkStdString yColumnName);

  /// Utility method for setting InputData without
  /// providing the XColumnIndex and YColumnIndex parameters.
  /// \sa GetXColumnIndex(), GetYColumnIndex()
  /// \def default are 0, 1
  virtual void SetInputData(vtkMRMLTableNode* tableNode);

  //----------------------------------------------------------------
  /// Data
  //----------------------------------------------------------------
 protected:
  vtkPlot* Plot;

  ///
  /// Type of Plot (Line, Scatter, Bar).
  int Type;

  vtkStdString XColumnName;
  vtkStdString YColumnName;
};

#endif
