/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/
// .NAME vtkMRMLMarkupsPlaneDisplayNode - MRML node to represent display properties for markups Plane
// .SECTION Description
// Adjusts default display parameters for Plane such as fill opacity.
//

#ifndef __vtkMRMLMarkupsPlaneDisplayNode_h
#define __vtkMRMLMarkupsPlaneDisplayNode_h

// Markups MRML includes
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkSlicerMarkupsModuleMRMLExport.h"

/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsPlaneDisplayNode : public vtkMRMLMarkupsDisplayNode
{
public:
  static vtkMRMLMarkupsPlaneDisplayNode* New();
  vtkTypeMacro(vtkMRMLMarkupsPlaneDisplayNode, vtkMRMLMarkupsDisplayNode);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  // Get node XML tag name (like Volume, Markups)
  const char* GetNodeTagName() override { return "MarkupsPlaneDisplay"; };

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentDefaultMacro(vtkMRMLMarkupsPlaneDisplayNode);

  enum
  {
    ComponentPlane = vtkMRMLMarkupsDisplayNode::Component_Last,
    ComponentPlane_Last
  };

  /// Indexes of the scale handles
  enum
  {
    HandleLEdge,
    HandleREdge,
    HandlePEdge,
    HandleAEdge,

    HandleLPCorner,
    HandleRPCorner,
    HandleLACorner,
    HandleRACorner,

    HandlePlane_Last
  };

  //@{
  /// Get/Set the visibility of the plane normal arrow.
  vtkSetMacro(NormalVisibility, bool);
  vtkGetMacro(NormalVisibility, bool);
  vtkBooleanMacro(NormalVisibility, bool);
  //@}

  //@{
  /// Get/Set the opacity of the plane normal arrow.
  vtkSetMacro(NormalOpacity, double);
  vtkGetMacro(NormalOpacity, double);
  //@}

protected:

  bool NormalVisibility{ true };
  double NormalOpacity{ 1.0 };

  vtkMRMLMarkupsPlaneDisplayNode();
  ~vtkMRMLMarkupsPlaneDisplayNode() override;
  vtkMRMLMarkupsPlaneDisplayNode(const vtkMRMLMarkupsPlaneDisplayNode&);
  void operator= (const vtkMRMLMarkupsPlaneDisplayNode&);
};
#endif
