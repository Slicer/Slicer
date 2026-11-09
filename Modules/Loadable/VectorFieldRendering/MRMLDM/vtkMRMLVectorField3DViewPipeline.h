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

#ifndef __vtkMRMLVectorField3DViewPipeline_h
#define __vtkMRMLVectorField3DViewPipeline_h

#include "vtkMRMLVectorFieldViewPipeline.h"

// VTK includes
#include <vtkNew.h>

class vtkActor;
class vtkGlyph3DMapper;
class vtkMapper;
class vtkMatrix4x4;
class vtkPolyDataMapper;

/// \brief Draws a vector field display node in a 3D view.
///
/// Maps the shared glyph points or poly data of vtkMRMLVectorFieldDisplayNode (of a model, vector volume, or
/// transform); the field is only sampled when the renderer pulls the data.
///
/// \sa vtkMRMLVectorField2DViewPipeline
class VTK_SLICER_VECTORFIELDRENDERING_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLVectorField3DViewPipeline : public vtkMRMLVectorFieldViewPipeline
{
public:
  static vtkMRMLVectorField3DViewPipeline* New();
  vtkTypeMacro(vtkMRMLVectorField3DViewPipeline, vtkMRMLVectorFieldViewPipeline);

  void OnRendererAdded(vtkRenderer* renderer) override;
  void OnRendererRemoved(vtkRenderer* renderer) override;

protected:
  vtkMRMLVectorField3DViewPipeline();
  ~vtkMRMLVectorField3DViewPipeline() override;

  void UpdatePipeline() override;

  /// Linear field-to-world transform applied to the actor. Identity if there is no parent transform, it is
  /// not linear (the display node moves the points then), or the node is a transform (evaluated in world).
  void GetLinearFieldToWorldMatrix(vtkMatrix4x4* linearTransformToWorld);
  void UpdateScalarColoring(vtkMapper* mapper);
  void UpdateActorProperties(vtkMRMLDisplayNode* effectiveDisplayNode, double hierarchyOpacity);

  /// The filters (GetGlyphPointsConnection(), GetOutputPolyDataConnection()) are in the shared pipeline of the
  /// display node (GetFieldPipeline()), used by all 3D views; a view only maps them.
  vtkNew<vtkGlyph3DMapper> Glypher;
  /// Draws the geometry of the grid, contour and streamline modes; the glyph mapper is
  /// only used in glyph mode.
  vtkNew<vtkPolyDataMapper> PolyDataMapper;
  vtkNew<vtkActor> Actor;
  /// The undeformed grid, which has a color of its own
  vtkNew<vtkPolyDataMapper> NonWarpedMapper;
  vtkNew<vtkActor> NonWarpedActor;

private:
  vtkMRMLVectorField3DViewPipeline(const vtkMRMLVectorField3DViewPipeline&) = delete;
  void operator=(const vtkMRMLVectorField3DViewPipeline&) = delete;
};

#endif
