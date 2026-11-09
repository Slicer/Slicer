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

// Fixtures shared by the vector field tests that use MRML nodes: the nodes that hold a field and the display nodes
// that show them. The data fixtures are in vtkVectorFieldTestingHelpers.h.

#ifndef __vtkMRMLVectorFieldTestingHelpers_h
#define __vtkMRMLVectorFieldTestingHelpers_h

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLVectorFieldDisplayNode.h"
#include "vtkMRMLVectorFieldPipeline.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkVectorFieldSampler.h"
#include "vtkVectorFieldTestingHelpers.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkThinPlateSplineTransform.h>
#include <vtkTransform.h>
#include <vtkVector.h>

// STD includes
#include <algorithm>
#include <vector>

namespace vtkMRMLVectorFieldTesting
{
using namespace vtkVectorFieldTesting;

/// Register the node classes of the Vector Field Rendering module in a scene, as the module logic does
inline void RegisterVectorFieldNodes(vtkMRMLScene* scene)
{
  if (!scene->IsNodeClassRegistered("vtkMRMLVectorFieldDisplayNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLVectorFieldDisplayNode>::New());
  }
}

/// A new vector field display node in the scene, not added to any node yet
inline vtkMRMLVectorFieldDisplayNode* AddNewFieldDisplayNode(vtkMRMLScene* scene)
{
  RegisterVectorFieldNodes(scene);
  return vtkMRMLVectorFieldDisplayNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorFieldDisplayNode"));
}

/// Save a scene as XML and load that into another scene, as saving and loading a scene file
/// does. Returns false if either failed.
inline bool ReloadScene(vtkMRMLScene* scene, vtkMRMLScene* reloadedScene)
{
  scene->SetSaveToXMLString(1);
  if (!scene->Commit())
  {
    return false;
  }
  RegisterVectorFieldNodes(reloadedScene);
  reloadedScene->SetLoadFromXMLString(1);
  reloadedScene->SetSceneXMLString(scene->GetSceneXMLString());
  return (reloadedScene->Import() != 0);
}

/// A vector field display node for a node of the scene. It samples on a regular lattice:
/// the checks of the tests were written for that pattern.
inline vtkMRMLVectorFieldDisplayNode* AddFieldDisplayNode(vtkMRMLScene* scene, vtkMRMLDisplayableNode* displayableNode)
{
  vtkMRMLVectorFieldDisplayNode* displayNode = AddNewFieldDisplayNode(scene);
  displayableNode->AddAndObserveDisplayNodeID(displayNode->GetID());
  displayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);
  return displayNode;
}

/// A model node in the scene, with the given mesh and a vector field display node.
inline vtkMRMLVectorFieldDisplayNode* AddModelWithFieldDisplay(vtkMRMLScene* scene, vtkPointSet* mesh)
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  modelNode->SetAndObserveMesh(mesh);
  return AddFieldDisplayNode(scene, modelNode);
}

/// A vector volume node with a vector field display node, its image placed over a cube from -halfSizeMm to halfSizeMm
/// as CreateVectorImage() lays the voxels out (volume geometry is the node's, not the image's).
inline vtkMRMLVectorFieldDisplayNode* AddVolumeWithFieldDisplay(vtkMRMLScene* scene, vtkImageData* image, double halfSizeMm = 50.0)
{
  vtkMRMLVectorVolumeNode* volumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode"));
  volumeNode->SetAndObserveImageData(image);
  int dimensions[3] = { 2, 2, 2 };
  image->GetDimensions(dimensions);
  double spacingMm = 2.0 * halfSizeMm / (dimensions[0] - 1);
  volumeNode->SetSpacing(spacingMm, spacingMm, spacingMm);
  volumeNode->SetOrigin(-halfSizeMm, -halfSizeMm, -halfSizeMm);
  return AddFieldDisplayNode(scene, volumeNode);
}

//----------------------------------------------------------------------------
/// The pipelines that SharedPipeline() returned, held until ReleaseSharedPipelines()
inline std::vector<vtkSmartPointer<vtkMRMLVectorFieldPipeline>>& HeldSharedPipelines()
{
  static std::vector<vtkSmartPointer<vtkMRMLVectorFieldPipeline>> heldPipelines;
  return heldPipelines;
}

//----------------------------------------------------------------------------
/// The shared pipeline of a display node, as a view gets it. It is held until ReleaseSharedPipelines(), as a view holds
/// it while it shows the node, so that the connections it returns stay valid during a test.
inline vtkMRMLVectorFieldPipeline* SharedPipeline(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  vtkSmartPointer<vtkMRMLVectorFieldPipeline> pipeline = vtkMRMLVectorFieldPipeline::GetSharedPipeline(displayNode);
  std::vector<vtkSmartPointer<vtkMRMLVectorFieldPipeline>>& heldPipelines = HeldSharedPipelines();
  if (pipeline && std::find(heldPipelines.begin(), heldPipelines.end(), pipeline) == heldPipelines.end())
  {
    heldPipelines.push_back(pipeline);
  }
  return pipeline;
}

//----------------------------------------------------------------------------
/// Release the pipelines that SharedPipeline() returned. Called at the end of a test, before the leak check.
inline void ReleaseSharedPipelines()
{
  HeldSharedPipelines().clear();
}

//----------------------------------------------------------------------------
/// A slice view sampler configured by the display node; nullptr if the field cannot be resampled in a slice.
inline vtkSmartPointer<vtkVectorFieldSampler> CreateSliceFieldSampler(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  vtkSmartPointer<vtkVectorFieldSampler> sampler = vtkSmartPointer<vtkVectorFieldSampler>::New();
  return (SharedPipeline(displayNode)->UpdateSliceFieldSampler(sampler) ? sampler : nullptr);
}

/// A point list node in the scene with the given control points: where a field is sampled,
/// or streamlines start from.
inline vtkMRMLMarkupsFiducialNode* AddPointList(vtkMRMLScene* scene, const std::vector<vtkVector3d>& positions)
{
  vtkMRMLMarkupsFiducialNode* pointListNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode"));
  for (const vtkVector3d& position : positions)
  {
    pointListNode->AddControlPoint(position);
  }
  return pointListNode;
}

/// A region of interest in the scene: an axis-aligned box with the given center and half sizes.
inline vtkMRMLMarkupsROINode* AddROI(vtkMRMLScene* scene, const vtkVector3d& center, const vtkVector3d& radius)
{
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsROINode"));
  roiNode->SetXYZ(center[0], center[1], center[2]);
  roiNode->SetRadiusXYZ(radius[0], radius[1], radius[2]);
  return roiNode;
}

/// A linear transform node in the scene that moves everything by the given offset.
inline vtkMRMLLinearTransformNode* AddTranslation(vtkMRMLScene* scene, double x, double y, double z)
{
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLinearTransformNode"));
  vtkNew<vtkTransform> translation;
  translation->Translate(x, y, z);
  transformNode->SetMatrixTransformToParent(translation->GetMatrix());
  return transformNode;
}

/// A non-linear transform node that moves everything by offsetX along X
/// (a thin-plate spline reproduces the affine part of its landmark displacements exactly).
inline vtkMRMLTransformNode* AddNonLinearTranslation(vtkMRMLScene* scene, double offsetX)
{
  vtkNew<vtkPoints> sourceLandmarks;
  vtkNew<vtkPoints> targetLandmarks;
  const double landmarks[5][3] = { { 0, 0, 0 }, { 100, 0, 0 }, { 0, 100, 0 }, { 0, 0, 100 }, { 100, 100, 100 } };
  for (const double* landmark : landmarks)
  {
    sourceLandmarks->InsertNextPoint(landmark);
    targetLandmarks->InsertNextPoint(landmark[0] + offsetX, landmark[1], landmark[2]);
  }
  vtkNew<vtkThinPlateSplineTransform> thinPlateSpline;
  thinPlateSpline->SetBasisToR();
  thinPlateSpline->SetSourceLandmarks(sourceLandmarks);
  thinPlateSpline->SetTargetLandmarks(targetLandmarks);
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  transformNode->SetAndObserveTransformToParent(thinPlateSpline);
  return transformNode;
}

/// A transform node with a vector field display node, which samples it on a regular lattice in the given region
inline vtkMRMLVectorFieldDisplayNode* AddTransformWithFieldDisplay(vtkMRMLScene* scene, vtkAbstractTransform* transformToParent, vtkMRMLNode* samplingRegionNode)
{
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  transformNode->SetAndObserveTransformToParent(transformToParent);
  vtkMRMLVectorFieldDisplayNode* displayNode = AddFieldDisplayNode(scene, transformNode);
  displayNode->SetAndObserveSamplingRegionNode(samplingRegionNode);
  return displayNode;
}

} // namespace vtkMRMLVectorFieldTesting

#endif
