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

#ifndef __vtkSlicerVectorFieldRenderingLogic_h
#define __vtkSlicerVectorFieldRenderingLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

#include "vtkSlicerVectorFieldRenderingModuleLogicExport.h"

// STD includes
#include <vector>

class vtkMRMLDisplayableNode;
class vtkMRMLNode;
class vtkMRMLTransformDisplayNode;
class vtkMRMLVectorFieldDisplayNode;
class vtkPolyData;

/// \brief Logic of the Vector Field Rendering module: adds, removes and shows the vector field display nodes of models, vector
/// volumes and transforms.
///
/// A node can have any number of vector field display nodes, each drawn on its own (for example glyphs and streamlines
/// of the same volume). They are added after the default display node of the node, which stays the first one.
///
/// Transform visualization settings of scenes saved by earlier Slicer versions (kept in vtkMRMLTransformDisplayNode) are
/// converted to vector field display nodes when a scene is imported or a scene view is restored.
class VTK_SLICER_VECTORFIELDRENDERING_MODULE_LOGIC_EXPORT vtkSlicerVectorFieldRenderingLogic : public vtkSlicerModuleLogic
{
public:
  static vtkSlicerVectorFieldRenderingLogic* New();
  vtkTypeMacro(vtkSlicerVectorFieldRenderingLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// True if the node holds a vector field that can be displayed: a model with a 3-component point or cell data array,
  /// a volume with at least 3 components, or a transform.
  static bool HasVectorField(vtkMRMLNode* node);

  /// Add a new vector field display node to a node that has a vector field (see HasVectorField()), with defaults that
  /// suit the kind of node, shown. The default display node of the node is created first if it has none, so that it
  /// stays the first display node. Returns nullptr if the node has no vector field or is not in a scene.
  static vtkMRMLVectorFieldDisplayNode* AddVectorFieldDisplayNode(vtkMRMLDisplayableNode* node);

  /// Remove a vector field display node from the scene, with the color legends that describe it
  static void RemoveVectorFieldDisplayNode(vtkMRMLVectorFieldDisplayNode* displayNode);

  /// The vector field display nodes of a node, in the order of its display nodes
  static std::vector<vtkMRMLVectorFieldDisplayNode*> GetVectorFieldDisplayNodes(vtkMRMLDisplayableNode* node);

  /// Show or hide a vector field display node. A shown display node is shown and hidden with its node by the eye icon of
  /// the subject hierarchy; a hidden one stays hidden when the node is shown (as volume rendering does).
  static void SetVectorFieldDisplayNodeShown(vtkMRMLVectorFieldDisplayNode* displayNode, bool shown);

  /// What 3D views show for a vector field display node, as a world-coordinate mesh with its coloring point data, for
  /// saving or export. Works whether or not a view shows the display node, using the filters that the views share.
  /// Returns false, leaving the mesh empty, if there is nothing to show.
  static bool GetVisualizationMesh(vtkMRMLVectorFieldDisplayNode* displayNode, vtkPolyData* mesh);

  /// Set the default display properties of a vector field display node of a transform: lengths in mm, glyphs on a
  /// lattice, displacement magnitude colored by a "Displacement to color" color map, which is added to the scene if the
  /// display node has no color node.
  static void SetDefaultTransformDisplayProperties(vtkMRMLVectorFieldDisplayNode* displayNode);

  /// Convert the transform visualization settings of a scene saved by an earlier Slicer version. If the visualization
  /// was in use (shown, or a region or glyph points node was set), a vector field display node is added to the transform
  /// with the same settings. The settings are then removed from the transform display node in either case.
  /// Returns the added display node, nullptr if none was added.
  static vtkMRMLVectorFieldDisplayNode* ConvertLegacyTransformDisplayNode(vtkMRMLTransformDisplayNode* transformDisplayNode);

  /// Convert the legacy settings of every transform display node of the scene (see ConvertLegacyTransformDisplayNode())
  void ConvertLegacyTransformDisplayNodes();

protected:
  vtkSlicerVectorFieldRenderingLogic();
  ~vtkSlicerVectorFieldRenderingLogic() override;
  vtkSlicerVectorFieldRenderingLogic(const vtkSlicerVectorFieldRenderingLogic&) = delete;
  void operator=(const vtkSlicerVectorFieldRenderingLogic&) = delete;

  void RegisterNodes() override;
  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  void OnMRMLSceneEndImport() override;
  void OnMRMLSceneEndRestore() override;
};

#endif
