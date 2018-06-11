/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLAbstractViewNode_h
#define __vtkMRMLAbstractViewNode_h

// VTK includes
#include <vtkSmartPointer.h>

// MRML includes
#include "vtkMRMLNode.h"

class vtkMRMLModelNode;
class vtkStringArray;

/// \brief Abstract MRML node to represent a view.
/// The class holds the properties common to any view type (3D, slice, chart..)
/// Views are not hidden from editors by default (HideFromEditor is 0)
class VTK_MRML_EXPORT vtkMRMLAbstractViewNode
  : public vtkMRMLNode
{
public:
  vtkTypeMacro(vtkMRMLAbstractViewNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts) VTK_OVERRIDE;

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent) VTK_OVERRIDE;

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node) VTK_OVERRIDE;

  /// \brief Reimplemented to preserve layout label when reset.
  /// \sa GetLayoutLabel()
  virtual void Reset(vtkMRMLNode* defaultNode) VTK_OVERRIDE;

  /// Name of the layout. Must be unique between all the view nodes of the
  /// same type because it is used as a singleton tag.
  /// Typical names can be colors "Red", "Green", "Yellow",...
  /// or numbers "1", "2"... to uniquely define the node.
  /// No name (i.e. "") by default.
  /// \sa SetSingletonTag(), SetViewLabel()
  inline void SetLayoutName(const char *layoutName);
  inline const char *GetLayoutName();

  /// An optional identifier to link groups of views. Views that have matching
  /// ViewGroup value are in the same group.
  /// ViewGroup is used for restricting scope of:
  /// \li Linked slice view property changes (is slices are linked, a property change
  ///     will only change views in the same group)
  /// \li Crosshair jump to slice (if crosshair is moved with shift+mousemove and slice
  ///     jump is enabled, only those slices will be moved that are in the same group as
  ///     the view where the mouse was)
  /// \li Slice intersection display (slice intersections will only shown of those slices
  ///     that are in the same group)
  vtkSetMacro(ViewGroup, int);
  vtkGetMacro(ViewGroup, int);

  /// Label for the view. Usually a 1 character label, e.g. R, 1, 2, etc.
  /// \sa SetLayoutName()
  vtkSetStringMacro(LayoutLabel);
  vtkGetStringMacro(LayoutLabel);

  /// Indicates whether or not the view is active
  vtkGetMacro(Active, int );
  vtkSetMacro(Active, int );

  /// \brief Indicates whether or not the view is visible.
  ///
  /// If it is not visible, then the view is not shown in any of the view
  /// layouts, but can be privately used by modules.
  ///
  /// \sa IsViewVisibleInLayout()
  /// \sa IsMappedInLayout()
  vtkGetMacro(Visibility, int);
  vtkSetMacro(Visibility, int);


  /// Indicates whether or not the view is mapped in the current layout.
  /// \sa GetVisibility()
  /// \sa IsViewVisibleInLayout()
  /// \sa vtkMRMLLayoutNode::SetViewArrangement()
  virtual int IsMappedInLayout();
  virtual void SetMappedInLayout(int value);

  /// Get parent layout node.
  /// Default is no reference, meaning that the view is managed by the main layout.
  /// Non-empty reference means standalone view or view managed by another layout.
  vtkMRMLNode* GetParentLayoutNode();
  /// Set parent layout node reference
  /// \sa GetParentLayoutNode
  bool SetAndObserveParentLayoutNodeID(const char *layoutNodeId);
  /// Set parent layout node reference
  /// \sa GetParentLayoutNode
  bool SetAndObserveParentLayoutNode(vtkMRMLNode* node);

  /// \brief Indicates whether or not the view is visible in the current layout.
  ///
  /// A view is visible in the current layout it is both mapped in layout
  /// and visible.
  ///
  /// \sa GetVisibility()
  /// \sa IsMappedInLayout()
  /// \sa vtkMRMLLayoutNode::SetViewArrangement()
  bool IsViewVisibleInLayout();

  /// 1st background color of the view.
  /// Black (0,0,0) by default.
  /// \sa SetBackgroundColor2()
  vtkGetVector3Macro(BackgroundColor, double);
  vtkSetVector3Macro(BackgroundColor, double);

  /// 2nd background color of the view
  /// Black (0,0,0) by default.
  /// \sa SetBackgroundColor2()
  vtkGetVector3Macro(BackgroundColor2, double);
  vtkSetVector3Macro(BackgroundColor2, double);


  /// Color for view header in layout as RGB
  /// Gray by default
  vtkSetVector3Macro(LayoutColor, double);
  vtkGetVector3Macro(LayoutColor, double);

  static double* GetRedColor();
  static double* GetYellowColor();
  static double* GetGreenColor();
  static double* GetCompareColor();
  static double* GetGrayColor();
  static double* GetThreeDViewBlueColor();

  /// Tells if it is meaningful to display orientation marker in this view.
  /// It is set statically in each specific view node class and cannot be changed dynamically.
  vtkGetMacro(OrientationMarkerEnabled, bool);

  /// Get/Set orientation marker type (e.g., not displayed, cube, human, coordinate system axes)
  vtkSetMacro(OrientationMarkerType, int);
  vtkGetMacro(OrientationMarkerType, int);

  /// Get/Set a custom human orientation marker model.
  /// If NULL or invalid node ID is specified then the default human model will be used.
  /// If the node has point data array with the name "Color" and 3 scalar components then
  /// it will be used to specify RGB color for the model. If no color point data is specified
  /// then the solid color specified in the model node's first display node will be used as color.
  void SetOrientationMarkerHumanModelNodeID(const char* modelNodeId);
  const char* GetOrientationMarkerHumanModelNodeID();
  vtkMRMLModelNode* GetOrientationMarkerHumanModelNode();

  /// Convert between orientation marker type ID and name
  static const char* GetOrientationMarkerTypeAsString(int id);
  static int GetOrientationMarkerTypeFromString(const char* name);

  /// Get/Set orientation marker is size. There are a few predefined marker sizes, defined by an enumerated value (e.g., small, medium, large).
  vtkSetMacro(OrientationMarkerSize, int);
  vtkGetMacro(OrientationMarkerSize, int);

  /// Convert between orientation marker type ID and name
  static const char* GetOrientationMarkerSizeAsString(int id);
  static int GetOrientationMarkerSizeFromString(const char* name);

  /// Enum to specify orientation marker types
  enum OrientationMarkerTypeType
  {
    OrientationMarkerTypeNone=0,
    OrientationMarkerTypeCube,
    OrientationMarkerTypeHuman,
    OrientationMarkerTypeAxes,
    OrientationMarkerType_Last // insert valid types above this line
  };

  enum OrientationMarkerSizeType
  {
    OrientationMarkerSizeSmall=0,
    OrientationMarkerSizeMedium,
    OrientationMarkerSizeLarge,
    OrientationMarkerSize_Last // insert valid types above this line
  };

  /// Tells if it is meaningful to display ruler in this view.
  /// It is set statically in each specific view node class and cannot be changed dynamically.
  vtkGetMacro(RulerEnabled, bool);

  /// Get/Set ruler type (e.g., not displayed, thin, thick)
  vtkSetMacro(RulerType, int);
  vtkGetMacro(RulerType, int);

  /// Convert between ruler type ID and name
  static const char* GetRulerTypeAsString(int id);
  static int GetRulerTypeFromString(const char* name);

  /// Enum to specify orientation marker types
  enum RulerTypeType
  {
    RulerTypeNone=0,
    RulerTypeThin,
    RulerTypeThick,
    RulerType_Last // insert valid types above this line
  };

  /// Get/Set labels of coordinate system axes.
  /// Order of labels: -X, +X, -Y, +Y, -Z, +Z.
  /// Default: L, R, P, A, I, S
  /// Note that these labels are used for display only (for example, showing organ specific
  /// directions, such as "Temporal" and "Nasal" instead of "Left" and "Right").
  /// Therefore, changing labels will not change orientation of displayed data in the view.
  const char* GetAxisLabel(int labelIndex);
  void SetAxisLabel(int labelIndex, const char* label);

  /// Total number of coordinate system axis labels
  static const int AxisLabelsCount;

protected:
  vtkMRMLAbstractViewNode();
  ~vtkMRMLAbstractViewNode();

  vtkMRMLAbstractViewNode(const vtkMRMLAbstractViewNode&);
  void operator=(const vtkMRMLAbstractViewNode&);

  ///
  /// Views with the same ViewGroup value are in the same group.
  int ViewGroup;

  ///
  /// Label to show for the view (shortcut for the name)
  char* LayoutLabel;

  ///
  /// Indicates whether or not the View is visible.
  /// Invisible (0) by default.
  int Visibility;

  ///
  /// Indicates whether or not the View is active.
  /// Inactive by default.
  int Active;

  ///
  /// Background colors
  double BackgroundColor[3];
  double BackgroundColor2[3];

  ///
  /// Color for view header in layout as RGB
  double LayoutColor[3];

  ///
  /// For views that supports orientation marker display (where OrientationMarkerEnabled=true)
  /// these parameters define how to display the orientation marker.
  bool OrientationMarkerEnabled;
  int OrientationMarkerType;
  int OrientationMarkerSize;

  static const char* OrientationMarkerHumanModelReferenceRole;

  ///
  /// For views that supports ruler display (where RulerEnabled=true)
  /// these parameters define how to display the ruler.
  bool RulerEnabled;
  int RulerType;

  ///
  /// Labels of coordinate system axes
  vtkSmartPointer<vtkStringArray> AxisLabels;

  static const char* ParentLayoutNodeReferenceRole;
};

//------------------------------------------------------------------------------
void vtkMRMLAbstractViewNode::SetLayoutName(const char *layoutName)
{
  this->SetSingletonTag(layoutName);
}

//------------------------------------------------------------------------------
const char *vtkMRMLAbstractViewNode::GetLayoutName()
{
  return this->GetSingletonTag();
}

#endif
