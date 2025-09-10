#pragma once

// Layer DM includes
#include "vtkMRMLLayerDMPipelineI.h"

// VTK includes
#include <vtkSmartPointer.h>

namespace layer_dm
{
/// Helper template function to try and create the given pipeline if the input view node and node match the expected types.
/// Supports variadic calls with triplets <TView, TNode, TPipeline, TNode2, TPipeline2 ...>
/// \sa TryCreate
template <typename TExpView, typename TExpNode, typename TPipeline, typename... Rest>
vtkSmartPointer<vtkMRMLLayerDMPipelineI> TryCreateForView(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node)
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
vtkSmartPointer<vtkMRMLLayerDMPipelineI> TryCreate(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node)
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