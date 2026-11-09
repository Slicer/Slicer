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

#include "vtkMRMLVectorField3DViewPipeline.h"

// MRMLDisplayableManager includes
#include <vtkMRMLModelDisplayableManager.h>

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLVectorFieldDisplayNode.h>
#include <vtkMRMLVectorFieldPipeline.h>
#include <vtkVectorFieldGlyph.h>

// VTK includes
#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkGlyph3DMapper.h>
#include <vtkLookupTable.h>
#include <vtkMapper.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLVectorField3DViewPipeline);

//---------------------------------------------------------------------------
vtkMRMLVectorField3DViewPipeline::vtkMRMLVectorField3DViewPipeline()
{
  this->Actor->SetMapper(this->Glypher);
  this->Actor->SetVisibility(0);
  // Not pickable: markups would snap onto arrowheads, and every pick would have to test all glyphs.
  this->Actor->SetPickable(0);

  this->NonWarpedMapper->ScalarVisibilityOff();
  this->NonWarpedActor->SetMapper(this->NonWarpedMapper);
  this->NonWarpedActor->SetVisibility(0);
  this->NonWarpedActor->SetPickable(0);
}

//---------------------------------------------------------------------------
vtkMRMLVectorField3DViewPipeline::~vtkMRMLVectorField3DViewPipeline() = default;

//---------------------------------------------------------------------------
void vtkMRMLVectorField3DViewPipeline::OnRendererAdded(vtkRenderer* renderer)
{
  renderer->AddViewProp(this->Actor);
  renderer->AddViewProp(this->NonWarpedActor);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorField3DViewPipeline::OnRendererRemoved(vtkRenderer* renderer)
{
  renderer->RemoveViewProp(this->Actor);
  renderer->RemoveViewProp(this->NonWarpedActor);
}

//---------------------------------------------------------------------------
void vtkMRMLVectorField3DViewPipeline::GetLinearFieldToWorldMatrix(vtkMatrix4x4* linearTransformToWorld)
{
  linearTransformToWorld->Identity();
  vtkMRMLDisplayableNode* node = this->GetDisplayableNode();
  vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(node);
  // A transform is evaluated in the world, with its parent transforms, so its parent
  // transform is not applied a second time
  if (!transformableNode || vtkMRMLTransformNode::SafeDownCast(node))
  {
    return;
  }
  vtkMRMLTransformNode* transformNode = transformableNode->GetParentTransformNode();
  if (transformNode && transformNode->IsTransformToWorldLinear())
  {
    transformNode->GetMatrixTransformToWorld(linearTransformToWorld);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorField3DViewPipeline::UpdatePipeline()
{
  this->NonWarpedActor->SetVisibility(false);

  double hierarchyOpacity = 1.0;
  vtkMRMLDisplayNode* effectiveDisplayNode = this->GetEffectiveDisplayNode(hierarchyOpacity);
  if (!effectiveDisplayNode)
  {
    this->Actor->SetVisibility(false);
    return;
  }
  vtkMRMLVectorFieldDisplayNode* fieldDisplayNode = this->GetFieldDisplayNode();

  // Parent transform: a linear transform is applied to the actor, which is much cheaper than
  // moving every point. The display node moves the points under a non-linear transform.
  vtkNew<vtkMatrix4x4> linearTransformToWorld;
  this->GetLinearFieldToWorldMatrix(linearTransformToWorld);
  this->Actor->SetUserMatrix(linearTransformToWorld);

  vtkMapper* mapper = nullptr;
  if (fieldDisplayNode->IsPolyDataVisualizationMode())
  {
    // The modes that draw poly data (grid, contour, streamline): the display node builds their
    // geometry and the actor uses a plain mapper
    vtkAlgorithmOutput* polyDataConnection = this->GetFieldPipeline()->GetOutputPolyDataConnection();
    if (!polyDataConnection)
    {
      this->Actor->SetVisibility(false);
      return;
    }
    this->Glypher->SetInputConnection(nullptr);
    this->PolyDataMapper->SetInputConnection(polyDataConnection);
    mapper = this->PolyDataMapper;
  }
  else
  {
    // Glyph mode: the display node selects the points, the view places the glyphs at them
    vtkAlgorithmOutput* glyphPointsConnection = this->GetFieldPipeline()->GetGlyphPointsConnection();
    if (!glyphPointsConnection)
    {
      this->Actor->SetVisibility(false);
      return;
    }
    this->PolyDataMapper->SetInputConnection(nullptr);
    this->Glypher->SetInputConnection(glyphPointsConnection);
    // Glyph shape, orientation and scale, as the glyph filter describes them: the geometry that
    // the display node exports is built with the same settings
    this->GetFieldPipeline()->GetGlyphFilter()->ConfigureMapper(this->Glypher);
    mapper = this->Glypher;
  }
  this->Actor->SetMapper(mapper);
  this->UpdateScalarColoring(mapper);
  this->UpdateActorProperties(effectiveDisplayNode, hierarchyOpacity);

  // Looks like the deformed grid, except for its color
  vtkAlgorithmOutput* nonWarpedConnection = this->GetFieldPipeline()->GetNonWarpedGridConnection();
  this->NonWarpedMapper->SetInputConnection(nonWarpedConnection);
  if (nonWarpedConnection)
  {
    this->NonWarpedActor->SetUserMatrix(linearTransformToWorld);
    this->NonWarpedActor->GetProperty()->DeepCopy(this->Actor->GetProperty());
    this->NonWarpedActor->GetProperty()->SetColor(fieldDisplayNode->GetGridNonWarpedColor());
    this->NonWarpedActor->SetVisibility(true);
  }
}

//---------------------------------------------------------------------------
void vtkMRMLVectorField3DViewPipeline::UpdateScalarColoring(vtkMapper* mapper)
{
  vtkMRMLVectorFieldDisplayNode* displayNode = this->GetFieldDisplayNode();
  // Coloring: either by the active point data scalar array through the color node's
  // lookup table, or by the display node's solid color.
  if (!displayNode->GetScalarVisibility() || !HasRenderedActiveScalar(displayNode))
  {
    mapper->ScalarVisibilityOff();
    return;
  }
  // Multi-component arrays are colored by the chosen component or by their magnitude
  int component = displayNode->GetRenderedActiveScalarComponent();
  bool multiComponent = displayNode->IsRenderedActiveScalarMultiComponent();

  mapper->ScalarVisibilityOn();
  mapper->SetScalarModeToUsePointFieldData();
  mapper->SelectColorArray(displayNode->GetRenderedActiveScalarName());
  mapper->SetColorModeToMapScalars();
  // Component -1 lets the lookup table turn the vector into a scalar (magnitude);
  // single-component arrays are mapped directly.
  mapper->SetArrayComponent(multiComponent ? component : 0);
  vtkMRMLColorNode* colorNode = displayNode->GetColorNode();
  if (multiComponent)
  {
    // The vector-to-scalar mode is a property of the lookup table, so a table of this view's
    // own is used
    vtkSmartPointer<vtkLookupTable> lookupTable = CreateLookupTable(colorNode);
    vtkMRMLModelDisplayableManager::SetLookupTableComponent(lookupTable, component);
    mapper->SetLookupTable(lookupTable);
  }
  else if (colorNode && colorNode->GetLookupTable())
  {
    mapper->SetLookupTable(colorNode->GetLookupTable());
  }
  mapper->UseLookupTableScalarRangeOff();
  mapper->SetScalarRange(displayNode->GetScalarRange());
}

//---------------------------------------------------------------------------
void vtkMRMLVectorField3DViewPipeline::UpdateActorProperties(vtkMRMLDisplayNode* effectiveDisplayNode, double hierarchyOpacity)
{
  vtkMRMLVectorFieldDisplayNode* displayNode = this->GetFieldDisplayNode();
  vtkProperty* actorProperties = this->Actor->GetProperty();
  actorProperties->SetColor(effectiveDisplayNode->GetColor());
  double opacity = hierarchyOpacity * effectiveDisplayNode->GetOpacity();
  if (displayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeContour)
  {
    // Nested isosurfaces are only readable if they can be seen through
    opacity *= displayNode->GetContourOpacity3D();
  }
  actorProperties->SetOpacity(opacity);
  actorProperties->SetAmbient(displayNode->GetAmbient());
  actorProperties->SetDiffuse(displayNode->GetDiffuse());
  actorProperties->SetSpecular(displayNode->GetSpecular());
  actorProperties->SetSpecularPower(displayNode->GetPower());
  actorProperties->SetRepresentation(displayNode->GetRepresentation());
  actorProperties->SetBackfaceCulling(displayNode->GetBackfaceCulling());
  actorProperties->SetFrontfaceCulling(displayNode->GetFrontfaceCulling());

  this->Actor->SetVisibility(true);
}
