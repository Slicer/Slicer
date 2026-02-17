#pragma once

// LayerDM includes
#include "vtkSlicerLayerDMModuleMRMLExport.h"

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkMRMLInteractionNode;
class vtkMRMLNode;
class vtkMRMLScene;
class vtkMRMLSelectionNode;
class vtkMRMLLayerDMObjectEventObserver;
class vtkMRMLApplicationLogic;

/// Helper class to observe changes to the interaction / selection singleton pairs.
/// When either selection or interaction change, triggers a modified event.
///
/// Provides helper methods to check if a given node is currently in place mode.
class VTK_SLICER_LAYERDM_MODULE_MRML_EXPORT vtkMRMLLayerDMSelectionObserver : public vtkObject
{
public:
  static vtkMRMLLayerDMSelectionObserver* New();
  vtkTypeMacro(vtkMRMLLayerDMSelectionObserver, vtkObject);

  /// @{
  /// \brief Updates selection and interaction nodes from the input scene
  /// Uses the singleton instances defined in the scene.
  /// For finer granularity, use the direct setters.
  void SetScene(vtkMRMLScene* scene);
  void UpdateNodesFromScene(vtkMRMLScene* scene);
  /// @}

  /// \brief Updates selection and interaction nodes from the input application logic
  /// Doesn't store the application logic instance.
  void UpdateNodesFromApplicationLogic(vtkMRMLApplicationLogic* logic);

  bool SetInteractionNode(vtkMRMLInteractionNode* interactionNode);
  vtkMRMLInteractionNode* GetInteractionNode() const;

  bool SetSelectionNode(vtkMRMLSelectionNode* selectionNode);
  vtkMRMLSelectionNode* GetSelectionNode() const;

  /// \returns true if the interaction is in place mode and selection active node is the input mrml node
  bool IsPlacing(vtkMRMLNode* node) const;

  /// \returns true if the interaction is in place mode
  bool IsPlacing() const;

  /// \brief Starts placing the input node (selection to node ID + interaction in place mode)
  void StartPlace(vtkMRMLNode* node, bool isPersistent);

  /// \brief Sets the interaction mode to view.
  void StopPlace() const;

  std::string GetActivePlaceNodeID() const;
  void SetInteractionMode(int interactionMode) const;
  int GetCurrentInteractionMode() const;

  /// \returns true if currently placing and in persistent mode.
  bool GetPlaceModePersistence() const;

protected:
  vtkMRMLLayerDMSelectionObserver();
  ~vtkMRMLLayerDMSelectionObserver() override;
  vtkMRMLLayerDMSelectionObserver(const vtkMRMLLayerDMSelectionObserver&) = delete;
  void operator=(const vtkMRMLLayerDMSelectionObserver&) = delete;

private:
  vtkSmartPointer<vtkMRMLLayerDMObjectEventObserver> m_obs;
  vtkWeakPointer<vtkMRMLScene> m_scene;
  vtkWeakPointer<vtkMRMLInteractionNode> m_interactionNode;
  vtkWeakPointer<vtkMRMLSelectionNode> m_selectionNode;
};
