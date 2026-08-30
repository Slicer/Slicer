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

==============================================================================*/

#ifndef __vtkMRMLLayerDMPipelineCreateHelper_h
#define __vtkMRMLLayerDMPipelineCreateHelper_h

// Layer DM includes
#include "vtkMRMLLayerDMPipeline.h"

// VTK includes
#include <vtkSmartPointer.h>

namespace layer_dm
{
/// Helper template function to try and create the given pipeline if the input view node and node match the expected types.
/// Supports variadic calls with triplets <TView, TNode, TPipeline, TNode2, TPipeline2 ...>
/// \sa TryCreate
template <typename TExpView, typename TExpNode, typename TPipeline, typename... Rest>
vtkSmartPointer<vtkMRMLLayerDMPipeline> TryCreateForView(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node)
{
  if (TExpView::SafeDownCast(viewNode) && TExpNode::SafeDownCast(node))
  {
    auto pipeline = vtkSmartPointer<TPipeline>::New();
    pipeline->SetViewNode(viewNode);
    pipeline->SetDisplayNode(node);
    return pipeline;
  }
  if constexpr (sizeof...(Rest) > 0)
  {
    return TryCreateForView<TExpView, Rest...>(viewNode, node);
  }
  return nullptr;
}

/// Helper template function to try and create the given pipeline if the input view node and node match the expected types.
/// Supports variadic calls with triplets <TView, TNode, TPipeline, TView2, TNode2, TPipeline2 ...>
/// \sa TryCreateForView
template <typename TExpView, typename TExpNode, typename TPipeline, typename... Rest>
vtkSmartPointer<vtkMRMLLayerDMPipeline> TryCreate(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node)
{
  if (auto pipeline = TryCreateForView<TExpView, TExpNode, TPipeline>(viewNode, node))
  {
    return pipeline;
  }
  if constexpr (sizeof...(Rest) > 0)
  {
    return TryCreate<Rest...>(viewNode, node);
  }
  return nullptr;
}
}; // namespace layer_dm

#endif
