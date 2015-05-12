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

// MRML includes
#include "vtkMRMLNode.h"

/// \brief Abstract MRML node to represent a view.
/// The class holds the properties common to any view type (3D, slice, chart..)
/// Views are not hidden from editors by default (HideFromEditor is 0)
class VTK_MRML_EXPORT vtkMRMLAbstractViewNode
  : public vtkMRMLNode
{
public:
  vtkTypeMacro(vtkMRMLAbstractViewNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  ///
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// \brief Reimplemented to preserve layout label when reset.
  /// \sa GetLayoutLabel()
  virtual void Reset();

  ///
  /// Name of the layout. Must be unique between all the view nodes of the
  /// same type because it is used as a singleton tag.
  /// Typical names can be colors "Red", "Green", "Yellow",...
  /// or numbers "1", "2"... to uniquely define the node.
  /// No name (i.e. "") by default.
  /// \sa SetSingletonTag(), SetViewLabel()
  inline void SetLayoutName(const char *layoutName);
  inline const char *GetLayoutName();

  ///
  /// Label for the view. Usually a 1 character label, e.g. R, 1, 2, etc.
  /// \sa SetLayoutName()
  vtkSetStringMacro(LayoutLabel);
  vtkGetStringMacro(LayoutLabel);

  ///
  /// Indicates whether or not the view is active
  vtkGetMacro (Active, int );
  vtkSetMacro (Active, int );

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

  /// \brief Indicates whether or not the view is visible in the current layout.
  ///
  /// A view is visible in the current layout it is both mapped in layout
  /// and visible.
  ///
  /// \sa GetVisibility()
  /// \sa IsMappedInLayout()
  /// \sa vtkMRMLLayoutNode::SetViewArrangement()
  bool IsViewVisibleInLayout();

  ///
  /// 1st background color of the view.
  /// Black (0,0,0) by default.
  /// \sa SetBackgroundColor2()
  vtkGetVector3Macro (BackgroundColor, double);
  vtkSetVector3Macro (BackgroundColor, double);

  ///
  /// 2nd background color of the view
  /// Black (0,0,0) by default.
  /// \sa SetBackgroundColor2()
  vtkGetVector3Macro (BackgroundColor2, double);
  vtkSetVector3Macro (BackgroundColor2, double);

protected:
  vtkMRMLAbstractViewNode();
  ~vtkMRMLAbstractViewNode();

  vtkMRMLAbstractViewNode(const vtkMRMLAbstractViewNode&);
  void operator=(const vtkMRMLAbstractViewNode&);

  ///
  /// Label to show for the view (shortcut for the name)
  char * LayoutLabel;

  ///
  /// Indicates whether or not the View is visible.
  /// Invisible (0) by default.
  int Visibility;

  ///
  /// Indicates whether or not the View is active.
  /// Inactive (1) by default.
  int Active;

  ///
  /// When a view is set Active, make other views inactive.
  virtual void RemoveActiveFlagInScene();

  ///
  /// Background colors
  double BackgroundColor[3];
  double BackgroundColor2[3];
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
