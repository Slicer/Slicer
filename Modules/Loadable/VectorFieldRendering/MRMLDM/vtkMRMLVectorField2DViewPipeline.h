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

#ifndef __vtkMRMLVectorField2DViewPipeline_h
#define __vtkMRMLVectorField2DViewPipeline_h

#include "vtkMRMLVectorFieldViewPipeline.h"

class vtkAlgorithmOutput;
class vtkMRMLSliceNode;

/// \brief Draws a vector field display node in a slice view.
///
/// Resampleable fields (volume, transform, volumetric mesh) are sampled in the slice plane; surface mesh points are
/// shown within the slice thickness. Glyphs are drawn flat in the slice, other modes are cut or projected to it.
///
/// \sa vtkMRMLVectorField3DViewPipeline
class VTK_SLICER_VECTORFIELDRENDERING_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLVectorField2DViewPipeline : public vtkMRMLVectorFieldViewPipeline
{
public:
  static vtkMRMLVectorField2DViewPipeline* New();
  vtkTypeMacro(vtkMRMLVectorField2DViewPipeline, vtkMRMLVectorFieldViewPipeline);

  void OnRendererAdded(vtkRenderer* renderer) override;
  void OnRendererRemoved(vtkRenderer* renderer) override;

protected:
  vtkMRMLVectorField2DViewPipeline();
  ~vtkMRMLVectorField2DViewPipeline() override;

  void OnUpdate(vtkObject* caller, unsigned long event, void* callData) override;
  void UpdatePipeline() override;

  /// Draws the streamlines that the display node traced for all views, the ones that cross
  /// this slice, fading out with the distance from it.
  void UpdateStreamlines(vtkMRMLDisplayNode* effectiveDisplayNode, double hierarchyOpacity, double sliceHalfThicknessMm);
  void UpdateScalarColoring(vtkMRMLDisplayNode* effectiveDisplayNode, double hierarchyOpacity);
  /// Adds the magnitude of a multi-component color array as ColorMagnitudeArrayName and returns the connection
  /// to color from (the input connection itself for single-component arrays).
  vtkAlgorithmOutput* UpdateColorMagnitude(vtkAlgorithmOutput* connection);
  void UpdateActorProperties(vtkMRMLDisplayNode* effectiveDisplayNode, double hierarchyOpacity);

  /// The objects of the pipeline, which are not wrapped
  class vtkInternal;
  vtkInternal* Internal;

private:
  vtkMRMLVectorField2DViewPipeline(const vtkMRMLVectorField2DViewPipeline&) = delete;
  void operator=(const vtkMRMLVectorField2DViewPipeline&) = delete;
};

#endif
