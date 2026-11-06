/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

  This file was further developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

#ifndef __vtkMRMLTransformInteractionDisplayNode_h
#define __vtkMRMLTransformInteractionDisplayNode_h

#include "vtkMRMLDisplayNode.h"

/// \brief MRML node for the interactive editing handles of a transform: which are shown, what they do, their size.
/// How the transform is drawn is stored separately, in the vector field display node of the same transform node.
///
/// \sa vtkMRMLTransformDisplayNode, vtkMRMLTransformNode
class VTK_MRML_EXPORT vtkMRMLTransformInteractionDisplayNode : public vtkMRMLDisplayNode
{
public:
  static vtkMRMLTransformInteractionDisplayNode* New();
  vtkTypeMacro(vtkMRMLTransformInteractionDisplayNode, vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "TransformInteractionDisplay"; }

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLTransformInteractionDisplayNode);

  ///@{
  /// Show the interaction handles that allow the transform to be edited by dragging.
  vtkGetMacro(EditorVisibility, bool);
  vtkSetMacro(EditorVisibility, bool);
  vtkBooleanMacro(EditorVisibility, bool);
  vtkGetMacro(EditorVisibility3D, bool);
  vtkSetMacro(EditorVisibility3D, bool);
  vtkBooleanMacro(EditorVisibility3D, bool);
  vtkGetMacro(EditorSliceIntersectionVisibility, bool);
  vtkSetMacro(EditorSliceIntersectionVisibility, bool);
  vtkBooleanMacro(EditorSliceIntersectionVisibility, bool);
  ///@}

  ///@{
  /// Which of the interactions the handles offer.
  vtkGetMacro(EditorTranslationEnabled, bool);
  vtkSetMacro(EditorTranslationEnabled, bool);
  vtkBooleanMacro(EditorTranslationEnabled, bool);
  vtkGetMacro(EditorTranslationSliceEnabled, bool);
  vtkSetMacro(EditorTranslationSliceEnabled, bool);
  vtkBooleanMacro(EditorTranslationSliceEnabled, bool);
  vtkGetMacro(EditorRotationEnabled, bool);
  vtkSetMacro(EditorRotationEnabled, bool);
  vtkBooleanMacro(EditorRotationEnabled, bool);
  vtkGetMacro(EditorRotationSliceEnabled, bool);
  vtkSetMacro(EditorRotationSliceEnabled, bool);
  vtkBooleanMacro(EditorRotationSliceEnabled, bool);
  vtkGetMacro(EditorScalingEnabled, bool);
  vtkSetMacro(EditorScalingEnabled, bool);
  vtkBooleanMacro(EditorScalingEnabled, bool);
  vtkGetMacro(EditorScalingSliceEnabled, bool);
  vtkSetMacro(EditorScalingSliceEnabled, bool);
  vtkBooleanMacro(EditorScalingSliceEnabled, bool);
  ///@}

  ///@{
  /// If enabled, the transform can be translated in slice views by click-and-dragging
  /// anywhere in the slice view, not at the small view plane translation handle.
  /// Dragging the center moves the center of rotation point instead of changing the translation.
  /// Only affects slice views (it has no effect in 3D views).
  vtkGetMacro(EditorTranslationSliceAnywhereEnabled, bool);
  vtkSetMacro(EditorTranslationSliceAnywhereEnabled, bool);
  vtkBooleanMacro(EditorTranslationSliceAnywhereEnabled, bool);
  ///@}

  ///@{
  /// Determines how much the transform translates in response to click-and-drag, when dragging
  /// anywhere in the slice view (see EditorTranslationSliceAnywhereEnabled). 1.0 means the transform
  /// moves in lock-step with the mouse cursor; lower values make the drag less sensitive, which makes
  /// high-precision alignment easier. Default is 0.2.
  /// Does not affect dragging transform handles (as a sensitivity != 1 would move the mouse pointer off the handle).
  vtkSetClampMacro(EditorTranslationSliceAnywhereSensitivity, double, 0.0, 1.0);
  vtkGetMacro(EditorTranslationSliceAnywhereSensitivity, double);
  ///@}

  ///@{
  /// Absolute size of the interaction handle widget in mm.
  vtkSetMacro(InteractionSizeMm, double);
  vtkGetMacro(InteractionSizeMm, double);
  ///@}

  ///@{
  /// Relative size of the interaction handle widget as a percent of the view size.
  vtkSetMacro(InteractionScalePercent, double);
  vtkGetMacro(InteractionScalePercent, double);
  ///@}

  ///@{
  /// If true, uses the absolute size for the interaction handle, otherwise uses the relative size.
  vtkSetMacro(InteractionSizeAbsolute, bool);
  vtkGetMacro(InteractionSizeAbsolute, bool);
  vtkBooleanMacro(InteractionSizeAbsolute, bool);
  ///@}

  ///@{
  /// Opacity of the interaction handles, in the range (0, 1].
  /// This is combined with the fade effect that is already applied when a handle is viewed edge-on.
  vtkSetClampMacro(InteractionHandleOpacity, double, 0.0, 1.0);
  vtkGetMacro(InteractionHandleOpacity, double);
  ///@}

  ///@{
  /// The type and index of the active interaction handle.
  vtkSetMacro(ActiveInteractionType, int);
  vtkGetMacro(ActiveInteractionType, int);
  vtkSetMacro(ActiveInteractionIndex, int);
  vtkGetMacro(ActiveInteractionIndex, int);
  ///@}

  ///@{
  /// Get/Set the visibility of the individual handle axes
  /// The order of the vector is: [X, Y, Z, ViewPlane]
  /// "ViewPlane" scale/translation allows transformations to take place along the active view plane.
  /// (ex. center translation point and ROI corner scale handles.
  vtkSetVector4Macro(RotationHandleComponentVisibility3D, bool);
  vtkGetVector4Macro(RotationHandleComponentVisibility3D, bool);
  vtkSetVector4Macro(ScaleHandleComponentVisibility3D, bool);
  vtkGetVector4Macro(ScaleHandleComponentVisibility3D, bool);
  vtkSetVector4Macro(TranslationHandleComponentVisibility3D, bool);
  vtkGetVector4Macro(TranslationHandleComponentVisibility3D, bool);
  vtkSetVector4Macro(RotationHandleComponentVisibilitySlice, bool);
  vtkGetVector4Macro(RotationHandleComponentVisibilitySlice, bool);
  vtkSetVector4Macro(ScaleHandleComponentVisibilitySlice, bool);
  vtkGetVector4Macro(ScaleHandleComponentVisibilitySlice, bool);
  vtkSetVector4Macro(TranslationHandleComponentVisibilitySlice, bool);
  vtkGetVector4Macro(TranslationHandleComponentVisibilitySlice, bool);
  ///@}

  /// Ask the editor to recompute its bounds by invoking the
  /// TransformUpdateEditorBoundsEvent event.
  void UpdateEditorBounds();
  enum
  {
    TransformUpdateEditorBoundsEvent = 2750
  };

protected:
  vtkMRMLTransformInteractionDisplayNode();
  ~vtkMRMLTransformInteractionDisplayNode() override;
  vtkMRMLTransformInteractionDisplayNode(const vtkMRMLTransformInteractionDisplayNode&);
  void operator=(const vtkMRMLTransformInteractionDisplayNode&);

  bool EditorVisibility{ false };
  bool EditorVisibility3D{ true };
  bool EditorSliceIntersectionVisibility{ true };
  bool EditorTranslationEnabled{ true };
  bool EditorTranslationSliceEnabled{ true };
  bool EditorTranslationSliceAnywhereEnabled{ false };
  double EditorTranslationSliceAnywhereSensitivity{ 0.2 };
  bool EditorRotationEnabled{ true };
  bool EditorRotationSliceEnabled{ true };
  bool EditorScalingEnabled{ false };
  bool EditorScalingSliceEnabled{ false };

  int ActiveInteractionType{ -1 };
  int ActiveInteractionIndex{ -1 };
  bool InteractionSizeAbsolute{ false };
  double InteractionSizeMm{ 5.0 };
  double InteractionScalePercent{ 15.0 };
  double InteractionHandleOpacity{ 1.0 };

  bool RotationHandleComponentVisibility3D[4];
  bool ScaleHandleComponentVisibility3D[4];
  bool TranslationHandleComponentVisibility3D[4];

  bool RotationHandleComponentVisibilitySlice[4];
  bool ScaleHandleComponentVisibilitySlice[4];
  bool TranslationHandleComponentVisibilitySlice[4];
};

#endif
