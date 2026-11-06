/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

#ifndef __vtkMRMLVectorFieldViewPipeline_h
#define __vtkMRMLVectorFieldViewPipeline_h

// LayerDM includes
#include "vtkMRMLLayerDMPipeline.h"

#include "vtkSlicerVectorFieldRenderingModuleMRMLDisplayableManagerExport.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkLookupTable;
class vtkMRMLColorNode;
class vtkMRMLDisplayableNode;
class vtkMRMLDisplayNode;
class vtkMRMLVectorFieldDisplayNode;
class vtkMRMLVectorFieldPipeline;

/// \brief Draws a vector field display node in a view, as a pipeline of the LayerDM module.
///
/// There is one pipeline for each view and vector field display node; the LayerDM displayable manager creates and
/// removes them. The filters that all views share are in the shared vtkMRMLVectorFieldPipeline of the display node,
/// which the view holds while it shows the display node; a view pipeline only owns what is specific to its view.
/// This base class keeps the pipeline up to date with the display node, its displayable node, the view node and the
/// scene; the subclasses draw the field in 3D views and in slice views.
///
/// \sa vtkMRMLVectorField3DViewPipeline, vtkMRMLVectorField2DViewPipeline
class VTK_SLICER_VECTORFIELDRENDERING_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLVectorFieldViewPipeline : public vtkMRMLLayerDMPipeline
{
public:
  vtkTypeMacro(vtkMRMLVectorFieldViewPipeline, vtkMRMLLayerDMPipeline);

  /// Makes the LayerDM displayable manager draw vector field display nodes in 3D and slice views. Calling it again has
  /// no effect.
  static void RegisterPipelineCreator();
  /// Removes the pipelines of vector field display nodes from all views.
  static void UnregisterPipelineCreator();

  void SetDisplayNode(vtkMRMLNode* displayNode) override;
  void SetScene(vtkMRMLScene* scene) override;

  /// Updates the observations, then the pipeline, unless the scene is batch processing: the pipeline is updated when
  /// it ends.
  void UpdateFromMRML() override;

protected:
  vtkMRMLVectorFieldViewPipeline();
  ~vtkMRMLVectorFieldViewPipeline() override;

  void OnUpdate(vtkObject* caller, unsigned long event, void* callData) override;

  /// Updates what is drawn in the view
  virtual void UpdatePipeline() = 0;

  vtkMRMLVectorFieldDisplayNode* GetFieldDisplayNode() const;
  /// The filters of the display node that all views share
  vtkMRMLVectorFieldPipeline* GetFieldPipeline() const;
  /// The node whose field is drawn
  vtkMRMLDisplayableNode* GetDisplayableNode() const;

  /// Returns the display node whose display properties apply (the overriding folder display node, if any) and the
  /// opacity of the folders. Returns nullptr if the field is not shown in this view.
  vtkMRMLDisplayNode* GetEffectiveDisplayNode(double& hierarchyOpacity) const;

  /// True if the display node names an array to color the field by
  static bool HasRenderedActiveScalar(vtkMRMLVectorFieldDisplayNode* displayNode);
  /// Copy of the color node's table (or a default table if there is none), so that setting range, alpha
  /// and vector mode does not affect other mappers sharing the color node's table.
  static vtkSmartPointer<vtkLookupTable> CreateLookupTable(vtkMRMLColorNode* colorNode);

private:
  vtkMRMLVectorFieldViewPipeline(const vtkMRMLVectorFieldViewPipeline&) = delete;
  void operator=(const vtkMRMLVectorFieldViewPipeline&) = delete;

  /// Updates the observation of the displayable node, which may be set after the display node is added to the scene.
  void UpdateDisplayableNodeObservation();

  vtkWeakPointer<vtkMRMLDisplayableNode> ObservedDisplayableNode;
  vtkSmartPointer<vtkMRMLVectorFieldPipeline> FieldPipeline;
  vtkWeakPointer<vtkMRMLScene> ObservedScene;
};

#endif
