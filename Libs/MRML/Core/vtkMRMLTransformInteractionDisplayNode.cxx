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

// MRML includes
#include "vtkMRMLTransformInteractionDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTransformInteractionDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLTransformInteractionDisplayNode::vtkMRMLTransformInteractionDisplayNode()
{
  for (int i = 0; i < 3; ++i)
  {
    this->RotationHandleComponentVisibility3D[i] = true;
    this->ScaleHandleComponentVisibility3D[i] = true;
    this->TranslationHandleComponentVisibility3D[i] = true;

    this->RotationHandleComponentVisibilitySlice[i] = false;
    this->ScaleHandleComponentVisibilitySlice[i] = true;
    this->TranslationHandleComponentVisibilitySlice[i] = false;
  }
  this->RotationHandleComponentVisibility3D[3] = false;
  this->ScaleHandleComponentVisibility3D[3] = true;
  this->TranslationHandleComponentVisibility3D[3] = true;
  this->RotationHandleComponentVisibilitySlice[3] = true;
  this->ScaleHandleComponentVisibilitySlice[3] = true;
  this->TranslationHandleComponentVisibilitySlice[3] = true;

  this->TypeDisplayName = vtkMRMLTr("vtkMRMLTransformInteractionDisplayNode", "Transform Interaction Display");
}

//----------------------------------------------------------------------------
vtkMRMLTransformInteractionDisplayNode::~vtkMRMLTransformInteractionDisplayNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLTransformInteractionDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintBooleanMacro(EditorVisibility);
  vtkMRMLPrintBooleanMacro(EditorVisibility3D);
  vtkMRMLPrintBooleanMacro(EditorSliceIntersectionVisibility);
  vtkMRMLPrintBooleanMacro(EditorTranslationEnabled);
  vtkMRMLPrintBooleanMacro(EditorTranslationSliceEnabled);
  vtkMRMLPrintBooleanMacro(EditorTranslationSliceAnywhereEnabled);
  vtkMRMLPrintFloatMacro(EditorTranslationSliceAnywhereSensitivity);
  vtkMRMLPrintBooleanMacro(EditorRotationEnabled);
  vtkMRMLPrintBooleanMacro(EditorRotationSliceEnabled);
  vtkMRMLPrintBooleanMacro(EditorScalingEnabled);
  vtkMRMLPrintBooleanMacro(EditorScalingSliceEnabled);
  vtkMRMLPrintBooleanMacro(InteractionSizeAbsolute);
  vtkMRMLPrintFloatMacro(InteractionSizeMm);
  vtkMRMLPrintFloatMacro(InteractionScalePercent);
  vtkMRMLPrintFloatMacro(InteractionHandleOpacity);
  vtkMRMLPrintVectorMacro(TranslationHandleComponentVisibility3D, bool, 4);
  vtkMRMLPrintVectorMacro(RotationHandleComponentVisibility3D, bool, 4);
  vtkMRMLPrintVectorMacro(ScaleHandleComponentVisibility3D, bool, 4);
  vtkMRMLPrintVectorMacro(TranslationHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLPrintVectorMacro(RotationHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLPrintVectorMacro(ScaleHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformInteractionDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // The attribute names are the ones that vtkMRMLTransformDisplayNode used before these
  // properties moved into their own node, so that scenes saved by either version load.
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLBooleanMacro(EditorVisibility, EditorVisibility);
  vtkMRMLWriteXMLBooleanMacro(EditorVisibility3D, EditorVisibility3D);
  vtkMRMLWriteXMLBooleanMacro(EditorSliceIntersectionVisibility, EditorSliceIntersectionVisibility);
  vtkMRMLWriteXMLBooleanMacro(EditorTranslationEnabled, EditorTranslationEnabled);
  vtkMRMLWriteXMLBooleanMacro(EditorTranslationSliceEnabled, EditorTranslationSliceEnabled);
  vtkMRMLWriteXMLBooleanMacro(EditorTranslationSliceAnywhereEnabled, EditorTranslationSliceAnywhereEnabled);
  vtkMRMLWriteXMLFloatMacro(EditorTranslationSliceAnywhereSensitivity, EditorTranslationSliceAnywhereSensitivity);
  vtkMRMLWriteXMLBooleanMacro(EditorRotationEnabled, EditorRotationEnabled);
  vtkMRMLWriteXMLBooleanMacro(EditorRotationSliceEnabled, EditorRotationSliceEnabled);
  vtkMRMLWriteXMLBooleanMacro(EditorScalingEnabled, EditorScalingEnabled);
  vtkMRMLWriteXMLBooleanMacro(EditorScalingSliceEnabled, EditorScalingSliceEnabled);
  vtkMRMLWriteXMLBooleanMacro(InteractionSizeAbsolute, InteractionSizeAbsolute);
  vtkMRMLWriteXMLFloatMacro(InteractionSizeMm, InteractionSizeMm);
  vtkMRMLWriteXMLFloatMacro(InteractionScalePercent, InteractionScalePercent);
  vtkMRMLWriteXMLFloatMacro(InteractionHandleOpacity, InteractionHandleOpacity);
  vtkMRMLWriteXMLVectorMacro(TranslationHandleComponentVisibility3D, TranslationHandleComponentVisibility3D, bool, 4);
  vtkMRMLWriteXMLVectorMacro(RotationHandleComponentVisibility3D, RotationHandleComponentVisibility3D, bool, 4);
  vtkMRMLWriteXMLVectorMacro(ScaleHandleComponentVisibility3D, ScaleHandleComponentVisibility3D, bool, 4);
  vtkMRMLWriteXMLVectorMacro(TranslationHandleComponentVisibilitySlice, TranslationHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLWriteXMLVectorMacro(RotationHandleComponentVisibilitySlice, RotationHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLWriteXMLVectorMacro(ScaleHandleComponentVisibilitySlice, ScaleHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformInteractionDisplayNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLBooleanMacro(EditorVisibility, EditorVisibility);
  vtkMRMLReadXMLBooleanMacro(EditorVisibility3D, EditorVisibility3D);
  vtkMRMLReadXMLBooleanMacro(EditorSliceIntersectionVisibility, EditorSliceIntersectionVisibility);
  vtkMRMLReadXMLBooleanMacro(EditorTranslationEnabled, EditorTranslationEnabled);
  vtkMRMLReadXMLBooleanMacro(EditorTranslationSliceEnabled, EditorTranslationSliceEnabled);
  vtkMRMLReadXMLBooleanMacro(EditorTranslationSliceAnywhereEnabled, EditorTranslationSliceAnywhereEnabled);
  vtkMRMLReadXMLFloatMacro(EditorTranslationSliceAnywhereSensitivity, EditorTranslationSliceAnywhereSensitivity);
  vtkMRMLReadXMLBooleanMacro(EditorRotationEnabled, EditorRotationEnabled);
  vtkMRMLReadXMLBooleanMacro(EditorRotationSliceEnabled, EditorRotationSliceEnabled);
  vtkMRMLReadXMLBooleanMacro(EditorScalingEnabled, EditorScalingEnabled);
  vtkMRMLReadXMLBooleanMacro(EditorScalingSliceEnabled, EditorScalingSliceEnabled);
  vtkMRMLReadXMLBooleanMacro(InteractionSizeAbsolute, InteractionSizeAbsolute);
  vtkMRMLReadXMLFloatMacro(InteractionSizeMm, InteractionSizeMm);
  vtkMRMLReadXMLFloatMacro(InteractionScalePercent, InteractionScalePercent);
  vtkMRMLReadXMLFloatMacro(InteractionHandleOpacity, InteractionHandleOpacity);
  vtkMRMLReadXMLVectorMacro(TranslationHandleComponentVisibility3D, TranslationHandleComponentVisibility3D, bool, 4);
  vtkMRMLReadXMLVectorMacro(RotationHandleComponentVisibility3D, RotationHandleComponentVisibility3D, bool, 4);
  vtkMRMLReadXMLVectorMacro(ScaleHandleComponentVisibility3D, ScaleHandleComponentVisibility3D, bool, 4);
  vtkMRMLReadXMLVectorMacro(TranslationHandleComponentVisibilitySlice, TranslationHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLReadXMLVectorMacro(RotationHandleComponentVisibilitySlice, RotationHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLReadXMLVectorMacro(ScaleHandleComponentVisibilitySlice, ScaleHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformInteractionDisplayNode::CopyContent(vtkMRMLNode* anode, bool deepCopy /*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  vtkMRMLTransformInteractionDisplayNode* node = vtkMRMLTransformInteractionDisplayNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyBooleanMacro(EditorVisibility);
  vtkMRMLCopyBooleanMacro(EditorVisibility3D);
  vtkMRMLCopyBooleanMacro(EditorSliceIntersectionVisibility);
  vtkMRMLCopyBooleanMacro(EditorTranslationEnabled);
  vtkMRMLCopyBooleanMacro(EditorTranslationSliceEnabled);
  vtkMRMLCopyBooleanMacro(EditorTranslationSliceAnywhereEnabled);
  vtkMRMLCopyFloatMacro(EditorTranslationSliceAnywhereSensitivity);
  vtkMRMLCopyBooleanMacro(EditorRotationEnabled);
  vtkMRMLCopyBooleanMacro(EditorRotationSliceEnabled);
  vtkMRMLCopyBooleanMacro(EditorScalingEnabled);
  vtkMRMLCopyBooleanMacro(EditorScalingSliceEnabled);
  vtkMRMLCopyBooleanMacro(InteractionSizeAbsolute);
  vtkMRMLCopyFloatMacro(InteractionSizeMm);
  vtkMRMLCopyFloatMacro(InteractionScalePercent);
  vtkMRMLCopyFloatMacro(InteractionHandleOpacity);
  vtkMRMLCopyVectorMacro(TranslationHandleComponentVisibility3D, bool, 4);
  vtkMRMLCopyVectorMacro(RotationHandleComponentVisibility3D, bool, 4);
  vtkMRMLCopyVectorMacro(ScaleHandleComponentVisibility3D, bool, 4);
  vtkMRMLCopyVectorMacro(TranslationHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLCopyVectorMacro(RotationHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLCopyVectorMacro(ScaleHandleComponentVisibilitySlice, bool, 4);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformInteractionDisplayNode::UpdateEditorBounds()
{
  this->InvokeEvent(vtkMRMLTransformInteractionDisplayNode::TransformUpdateEditorBoundsEvent);
}
