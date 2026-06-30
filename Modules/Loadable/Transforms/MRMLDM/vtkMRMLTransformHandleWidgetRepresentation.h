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

///
/// \class vtkMRMLTransformHandleWidgetRepresentation
/// \brief Abstract representation for the transform handle widgets
///
/// \sa vtkMRMLInteractionWidgetRepresentation
///

#ifndef vtkMRMLTransformHandleWidgetRepresentation_h
#define vtkMRMLTransformHandleWidgetRepresentation_h

#include "vtkSlicerTransformsModuleMRMLDisplayableManagerExport.h"

// MRMLDM includes
#include "vtkMRMLInteractionWidgetRepresentation.h"

// MRML includes
#include <vtkMRMLTransformDisplayNode.h>

// VTK includes
#include <vtkArrayCalculator.h>
#include <vtkOutlineSource.h>

class VTK_SLICER_TRANSFORMS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLTransformHandleWidgetRepresentation : public vtkMRMLInteractionWidgetRepresentation
{
public:
  /// Instantiate this class.
  static vtkMRMLTransformHandleWidgetRepresentation* New();

  ///@{
  /// Standard VTK class macros.
  vtkTypeMacro(vtkMRMLTransformHandleWidgetRepresentation, vtkMRMLInteractionWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  ///@}

  /// Indices used for handle axes within a handle group (rotation, scale, translation).
  /// X, Y, Z are the per-axis handles; ViewPlane is the free-movement handle (e.g. used for grabbing
  /// the transform anywhere in the slice view, when wide translation is enabled).
  enum
  {
    HandleIndexX,
    HandleIndexY,
    HandleIndexZ,
    HandleIndexViewPlane
  };

  virtual int GetActiveComponentType() override;
  virtual void SetActiveComponentType(int type) override;
  virtual int GetActiveComponentIndex() override;
  virtual void SetActiveComponentIndex(int type) override;

  virtual vtkMRMLTransformDisplayNode* GetDisplayNode();
  virtual void SetDisplayNode(vtkMRMLTransformDisplayNode* displayNode);

  virtual vtkMRMLTransformNode* GetTransformNode();

  void UpdateInteractionPipeline() override;

  using Superclass::UpdateHandleToWorldTransform;
  void UpdateHandleToWorldTransform(vtkTransform* handleToWorldTransform) override;

  /// Size relative to screen
  double GetInteractionScalePercent() override;
  /// Size in mm
  double GetInteractionSizeMm() override;
  /// True -> size in mm; false -> relative to screen
  bool GetInteractionSizeAbsolute() override;
  double GetInteractionHandleOpacity() override;

  /// Scales the translation vector by vtkMRMLTransformDisplayNode::GetEditorTranslationSliceAnywhereSensitivity()
  /// when the most recent translate interaction was started from a "wide" hit (see LastInteractionWasWideHit).
  /// Otherwise (e.g. dragging the small translation handle itself) no scaling is applied.
  double GetTranslationScaleFactor() override;

  bool GetHandleVisibility(int type, int index) override;

  /// Renders the view plane translation handle as a crosshair (instead of the default filled circle)
  /// when vtkMRMLTransformDisplayNode::GetEditorTranslationSliceAnywhereEnabled() is true in a slice view,
  /// so that the transform center underneath the handle is not occluded.
  int GetHandleGlyphType(int type, int index) override;

  /// Renders the view plane translation handle (whether shown as a circle or as a crosshair, see
  /// GetHandleGlyphType()) and the in-plane (view plane) rotation handle in cyan, instead of their
  /// default color, so that these "free"/view-plane handles are easy to recognize and tell apart
  /// from the axis-constrained (red/green/blue) handles.
  void GetHandleColor(int type, int index, double color[4]) override;

  bool IsDisplayable() override;

  /// In addition to the default handle hit-testing performed by the superclass, when
  /// vtkMRMLTransformDisplayNode::GetEditorTranslationSliceAnywhereEnabled() is true, clicking anywhere
  /// in a slice view is treated as grabbing the view plane translation handle, so the transform can be
  /// dragged from anywhere in the slice view, not just from the small handle at the center of the transform.
  void CanInteract(vtkMRMLInteractionEventData* interactionEventData, int& foundComponentType, int& foundComponentIndex, double& closestDistance2) override;

  /// True if the component found by the most recent call to CanInteract() was found using the "wide"
  /// hit-test area (clicking anywhere in the slice view) rather than a precise handle grab.
  /// Used by vtkMRMLTransformHandleWidget to give the wide hit-test a lower interaction priority.
  vtkGetMacro(LastInteractionWasWideHit, bool);

  /// Squared display-coordinate distance between the event position and the transform center, computed
  /// by the most recent "wide" hit-test performed by CanInteract(). Used by vtkMRMLTransformHandleWidget
  /// so that, when multiple transforms can be dragged from anywhere in the same slice view, the one
  /// whose center is closest to the cursor is the one that is picked.
  vtkGetMacro(LastSliceWideHitDistance2, double);

protected:
  vtkMRMLTransformHandleWidgetRepresentation();
  ~vtkMRMLTransformHandleWidgetRepresentation() override;

  vtkSmartPointer<vtkMRMLTransformDisplayNode> DisplayNode;
  bool LastInteractionWasWideHit{ false };
  double LastSliceWideHitDistance2{ 0.0 };

private:
  vtkMRMLTransformHandleWidgetRepresentation(const vtkMRMLTransformHandleWidgetRepresentation&) = delete;
  void operator=(const vtkMRMLTransformHandleWidgetRepresentation&) = delete;
};

#endif
