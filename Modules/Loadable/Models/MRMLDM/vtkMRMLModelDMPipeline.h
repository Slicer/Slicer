#pragma once

// Export
#include "vtkSlicerModelsModuleMRMLDisplayableManagerExport.h"

// Layer DM includes
#include "vtkMRMLLayerDMPipelineI.h"

// MRML includes
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelNode.h"

#include <vtkActor.h>
#include <vtkAlgorithm.h>
#include <vtkSmartPointer.h>

class vtkCapPolyData;
class vtkFeatureEdges;
class vtkGeneralTransform;
class vtkImageActor;
class vtkImplicitFunction;
class vtkMapper;
class vtkMRMLClipNode;
class vtkMRMLDisplayNode;
class vtkMRMLModelDisplayNode;
class vtkPolyData;
class vtkTransform;
class vtkTransformFilter;

/// \brief Pipeline for displaying a model in a 3D view.
class VTK_SLICER_MODELS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLModelDMPipeline : public vtkMRMLLayerDMPipelineI
{
public:
  static vtkMRMLModelDMPipeline* New();
  vtkTypeMacro(vtkMRMLModelDMPipeline, vtkMRMLLayerDMPipelineI);

  void SetDisplayNode(vtkMRMLNode* displayNode) override;
  vtkMRMLModelDisplayNode* GetModelDisplayNode() const;
  vtkMRMLModelNode* GetModelNode() const;

  /// \brief Returns props managed by the pipeline.
  /// The list of props doesn't change after instantiation of this pipeline.
  std::vector<vtkSmartPointer<vtkProp>> GetProps() const;

  void OnRendererAdded(vtkRenderer* renderer) override;
  void OnRendererRemoved(vtkRenderer* renderer) override;

  void UpdatePipeline() override;

protected:
  vtkMRMLModelDMPipeline();
  ~vtkMRMLModelDMPipeline() override;

  void OnUpdate(vtkObject* obj, unsigned long eventId, void* callData) override;

private:
  vtkMRMLModelDMPipeline(const vtkMRMLModelDMPipeline&) = delete;
  void operator=(const vtkMRMLModelDMPipeline&) = delete;

  template <typename T, typename U>
  T* EnsureTypeAs(vtkSmartPointer<U>& object)
  {
    if (!T::SafeDownCast(object))
    {
      object = T::New();
    }
    return T::SafeDownCast(object);
  }

  void UpdateModelNode();
  void SetModelNode(vtkMRMLModelNode* node);
  void UpdateClipper();
  bool HasClipping() const;
  void UpdateTransform() const;
  void UpdateMapperConnection();
  void UpdateDisplayProperty() const;
  void UpdateMapperProperties(vtkMapper* mapper) const;
  void UpdateActorProperties(vtkActor* actor, double opacity, bool visibility) const;
  void UpdateCapActorProperties(double opacity, bool visibility) const;

  vtkWeakPointer<vtkMRMLModelNode> ModelNode;
  vtkSmartPointer<vtkTransformFilter> TransformFilter;
  vtkSmartPointer<vtkTransform> Transform;
  vtkSmartPointer<vtkGeneralTransform> NonLinearTransform;
  vtkSmartPointer<vtkAlgorithm> Clipper;
  vtkSmartPointer<vtkAlgorithm> Capper;
  vtkSmartPointer<vtkMapper> Mapper;
  vtkSmartPointer<vtkActor> Actor;
  vtkSmartPointer<vtkImageActor> ImageActor;
  vtkSmartPointer<vtkMapper> CapMapper;
  vtkSmartPointer<vtkActor> CapActor;
};
