/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRMLLogic includes
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceLayerLogic.h"

// MRML includes
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkImageBlend.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

#include "vtkMRMLCoreTestingMacros.h"

// STD includes
#include <iostream>

class vtkMRMLSliceLogicObserverProbe : public vtkMRMLSliceLogic
{
public:
  static vtkMRMLSliceLogicObserverProbe* New();
  vtkTypeMacro(vtkMRMLSliceLogicObserverProbe, vtkMRMLSliceLogic);

  int LogicEventCount{ 0 };
  vtkMRMLSliceLayerLogic* TrackedLayer{ nullptr };
  int TrackedLayerEventCount{ 0 };
  bool SawPartiallyConfiguredLayer{ false };
  vtkObserverManager* GetLayerObserverManager() const { return this->GetMRMLLogicsObserverManager(); }

protected:
  void ProcessMRMLLogicsEvents(vtkObject* caller, unsigned long, void*) override
  {
    ++this->LogicEventCount;
    if (caller == this->TrackedLayer)
    {
      ++this->TrackedLayerEventCount;
      bool sceneReady = this->TrackedLayer->GetMRMLScene() == this->GetMRMLScene();
      bool sliceNodeReady = this->TrackedLayer->GetSliceNode() == this->GetSliceNode();
      this->SawPartiallyConfiguredLayer |= !(sceneReady && sliceNodeReady);
    }
  }
};

vtkStandardNewMacro(vtkMRMLSliceLogicObserverProbe);

struct LayerReentryCallbackState
{
  vtkMRMLSliceLogicObserverProbe* Logic;
  int Slot;
  vtkMRMLSliceLayerLogic* Replacement;
  int CallCount{ 0 };
};

void OnLayerConfigurationEvent(vtkObject*, unsigned long, void* clientData, void*)
{
  auto* state = static_cast<LayerReentryCallbackState*>(clientData);
  if (++state->CallCount == 1)
  {
    state->Logic->SetNthLayer(state->Slot, state->Replacement);
  }
}

int vtkMRMLSliceLogicTest1(int, char*[])
{
  vtkNew<vtkMRMLSliceLogicObserverProbe> observerLogic;
  vtkNew<vtkMRMLSliceLayerLogic> originalLayer;
  vtkNew<vtkMRMLSliceLayerLogic> replacementLayer;
  vtkObserverManager* observerManager = observerLogic->GetLayerObserverManager();
  int originalReferenceCount = originalLayer->GetReferenceCount();
  int replacementReferenceCount = replacementLayer->GetReferenceCount();

  observerLogic->SetBackgroundLayer(originalLayer.GetPointer());
  if (observerManager->GetObservationsCount(originalLayer.GetPointer(), vtkCommand::ModifiedEvent) != 1)
  {
    std::cerr << "Background layer is not observed exactly once" << std::endl;
    return EXIT_FAILURE;
  }
  if (originalLayer->GetReferenceCount() != originalReferenceCount + 1)
  {
    std::cerr << "The observer manager did not retain the background layer" << std::endl;
    return EXIT_FAILURE;
  }
  observerLogic->SetBackgroundLayer(originalLayer.GetPointer());
  if (observerManager->GetObservationsCount(originalLayer.GetPointer(), vtkCommand::ModifiedEvent) != 1)
  {
    std::cerr << "Setting the same layer added another observation" << std::endl;
    return EXIT_FAILURE;
  }
  originalLayer->Modified();
  if (observerLogic->LogicEventCount != 1)
  {
    std::cerr << "Background layer modification was not delivered" << std::endl;
    return EXIT_FAILURE;
  }

  observerLogic->SetBackgroundLayer(replacementLayer.GetPointer());
  if (observerManager->GetObservationsCount(originalLayer.GetPointer(), vtkCommand::ModifiedEvent) != 0
      || observerManager->GetObservationsCount(replacementLayer.GetPointer(), vtkCommand::ModifiedEvent) != 1)
  {
    std::cerr << "Replacing the background layer left a stale observation" << std::endl;
    return EXIT_FAILURE;
  }
  if (originalLayer->GetReferenceCount() != originalReferenceCount)
  {
    std::cerr << "Replacing the background layer retained the old layer" << std::endl;
    return EXIT_FAILURE;
  }
  originalLayer->Modified();
  replacementLayer->Modified();
  if (observerLogic->LogicEventCount != 2)
  {
    std::cerr << "Only the replacement layer should notify the slice logic" << std::endl;
    return EXIT_FAILURE;
  }

  observerLogic->SetForegroundLayer(replacementLayer.GetPointer());
  observerLogic->SetBackgroundLayer(nullptr);
  if (observerManager->GetObservationsCount(replacementLayer.GetPointer(), vtkCommand::ModifiedEvent) != 1
      || replacementLayer->GetReferenceCount() != replacementReferenceCount + 1)
  {
    std::cerr << "A layer shared by two slots lost its observation or ownership" << std::endl;
    return EXIT_FAILURE;
  }
  replacementLayer->Modified();
  if (observerLogic->LogicEventCount != 3)
  {
    std::cerr << "The shared layer should still notify the slice logic" << std::endl;
    return EXIT_FAILURE;
  }

  observerLogic->SetForegroundLayer(nullptr);
  if (observerManager->GetObservationsCount(replacementLayer.GetPointer(), vtkCommand::ModifiedEvent) != 0 || replacementLayer->GetReferenceCount() != replacementReferenceCount)
  {
    std::cerr << "Clearing the last layer slot left a stale observation or reference" << std::endl;
    return EXIT_FAILURE;
  }
  replacementLayer->Modified();
  if (observerLogic->LogicEventCount != 3)
  {
    std::cerr << "A cleared layer still notified the slice logic" << std::endl;
    return EXIT_FAILURE;
  }

  observerLogic->SetLabelLayer(originalLayer.GetPointer());
  observerLogic->SetNthLayer(vtkMRMLSliceLogic::Layer_Last, replacementLayer.GetPointer());
  observerLogic->SetLabelLayer(replacementLayer.GetPointer());
  if (observerManager->GetObservationsCount(originalLayer.GetPointer(), vtkCommand::ModifiedEvent) != 0
      || observerManager->GetObservationsCount(replacementLayer.GetPointer(), vtkCommand::ModifiedEvent) != 1)
  {
    std::cerr << "Replacing the label layer did not update its observation" << std::endl;
    return EXIT_FAILURE;
  }
  observerLogic->SetNthLayer(vtkMRMLSliceLogic::Layer_Last, nullptr);
  observerLogic->SetLabelLayer(nullptr);
  if (observerManager->GetObservationsCount(replacementLayer.GetPointer(), vtkCommand::ModifiedEvent) != 0)
  {
    std::cerr << "Clearing label and additional layers left a stale observation" << std::endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkMRMLScene> orderingScene;
  vtkMRMLSliceNode::AddDefaultSliceOrientationPresets(orderingScene.GetPointer());
  vtkNew<vtkMRMLSliceLogicObserverProbe> orderingLogic;
  orderingLogic->SetMRMLScene(orderingScene.GetPointer());
  CHECK_NOT_NULL(orderingLogic->AddSliceNode("Ordering"));
  vtkNew<vtkMRMLSliceLayerLogic> orderingLayer;
  orderingLogic->TrackedLayer = orderingLayer.GetPointer();

  orderingLogic->SetBackgroundLayer(orderingLayer.GetPointer());
  if (orderingLogic->TrackedLayerEventCount != 0 || orderingLogic->SawPartiallyConfiguredLayer)
  {
    std::cerr << "Replacement layer notified its logic before configuration was complete" << std::endl;
    return EXIT_FAILURE;
  }
  if (orderingLogic->GetBackgroundLayer() != orderingLayer.GetPointer() || orderingLayer->GetMRMLScene() != orderingScene.GetPointer()
      || orderingLayer->GetSliceNode() != orderingLogic->GetSliceNode())
  {
    std::cerr << "Replacement layer was not configured for the slice logic" << std::endl;
    return EXIT_FAILURE;
  }
  orderingLayer->Modified();
  if (orderingLogic->TrackedLayerEventCount != 1 || orderingLogic->SawPartiallyConfiguredLayer)
  {
    std::cerr << "Configured replacement layer did not notify its logic correctly" << std::endl;
    return EXIT_FAILURE;
  }

  LayerReentryCallbackState clearPeer{};
  vtkNew<vtkMRMLSliceLogicObserverProbe> sharedReentryLogic;
  sharedReentryLogic->SetMRMLScene(orderingScene.GetPointer());
  CHECK_NOT_NULL(sharedReentryLogic->AddSliceNode("SharedReentry"));
  vtkNew<vtkMRMLSliceLayerLogic> sharedReentryLayer;
  int sharedReferenceCount = sharedReentryLayer->GetReferenceCount();
  sharedReentryLogic->SetForegroundLayer(sharedReentryLayer.GetPointer());
  // Force configuration to emit an event while the layer is shared.
  sharedReentryLayer->SetSliceNode(nullptr);
  sharedReentryLayer->SetMRMLScene(nullptr);
  clearPeer.Logic = sharedReentryLogic.GetPointer();
  clearPeer.Slot = vtkMRMLSliceLogic::LayerForeground;
  vtkNew<vtkCallbackCommand> clearPeerCommand;
  clearPeerCommand->SetClientData(&clearPeer);
  clearPeerCommand->SetCallback(OnLayerConfigurationEvent);
  unsigned long clearPeerTag = sharedReentryLayer->AddObserver(vtkCommand::ModifiedEvent, clearPeerCommand.GetPointer());

  sharedReentryLogic->SetBackgroundLayer(sharedReentryLayer.GetPointer());
  if (clearPeer.CallCount < 1 || sharedReentryLogic->GetForegroundLayer() != nullptr || sharedReentryLogic->GetBackgroundLayer() != sharedReentryLayer.GetPointer()
      || sharedReentryLayer->GetMRMLScene() != orderingScene.GetPointer() || sharedReentryLayer->GetSliceNode() != sharedReentryLogic->GetSliceNode()
      || sharedReentryLogic->GetLayerObserverManager()->GetObservationsCount(sharedReentryLayer.GetPointer(), vtkCommand::ModifiedEvent) != 1
      || sharedReentryLayer->GetReferenceCount() != sharedReferenceCount + 1)
  {
    std::cerr << "Clearing a shared slot during configuration lost the replacement layer" << std::endl;
    return EXIT_FAILURE;
  }
  sharedReentryLogic->TrackedLayer = sharedReentryLayer.GetPointer();
  sharedReentryLayer->Modified();
  if (sharedReentryLogic->TrackedLayerEventCount != 1 || sharedReentryLogic->SawPartiallyConfiguredLayer)
  {
    std::cerr << "Layer cleared from a peer slot was not observed after configuration" << std::endl;
    return EXIT_FAILURE;
  }
  sharedReentryLayer->RemoveObserver(clearPeerTag);

  LayerReentryCallbackState changeTarget{};
  vtkNew<vtkMRMLSliceLogicObserverProbe> targetReentryLogic;
  targetReentryLogic->SetMRMLScene(orderingScene.GetPointer());
  CHECK_NOT_NULL(targetReentryLogic->AddSliceNode("TargetReentry"));
  vtkNew<vtkMRMLSliceLayerLogic> targetReentryLayer;
  vtkNew<vtkMRMLSliceLayerLogic> interveningLayer;
  int targetReferenceCount = targetReentryLayer->GetReferenceCount();
  int interveningReferenceCount = interveningLayer->GetReferenceCount();
  changeTarget.Logic = targetReentryLogic.GetPointer();
  changeTarget.Slot = vtkMRMLSliceLogic::LayerBackground;
  changeTarget.Replacement = interveningLayer.GetPointer();
  vtkNew<vtkCallbackCommand> changeTargetCommand;
  changeTargetCommand->SetClientData(&changeTarget);
  changeTargetCommand->SetCallback(OnLayerConfigurationEvent);
  unsigned long changeTargetTag = targetReentryLayer->AddObserver(vtkCommand::ModifiedEvent, changeTargetCommand.GetPointer());
  targetReentryLogic->TrackedLayer = targetReentryLayer.GetPointer();

  targetReentryLogic->SetBackgroundLayer(targetReentryLayer.GetPointer());
  if (changeTarget.CallCount < 1 || targetReentryLogic->GetBackgroundLayer() != targetReentryLayer.GetPointer() || targetReentryLayer->GetMRMLScene() != orderingScene.GetPointer()
      || targetReentryLayer->GetSliceNode() != targetReentryLogic->GetSliceNode()
      || targetReentryLogic->GetLayerObserverManager()->GetObservationsCount(targetReentryLayer.GetPointer(), vtkCommand::ModifiedEvent) != 1
      || targetReentryLogic->GetLayerObserverManager()->GetObservationsCount(interveningLayer.GetPointer(), vtkCommand::ModifiedEvent) != 0
      || targetReentryLayer->GetReferenceCount() != targetReferenceCount + 1 || interveningLayer->GetReferenceCount() != interveningReferenceCount)
  {
    std::cerr << "Changing the target slot during configuration corrupted layer ownership" << std::endl;
    return EXIT_FAILURE;
  }
  if (targetReentryLogic->TrackedLayerEventCount != 0 || targetReentryLogic->SawPartiallyConfiguredLayer)
  {
    std::cerr << "Target layer was observed before configuration completed" << std::endl;
    return EXIT_FAILURE;
  }
  int eventCountBeforeStaleLayerModified = targetReentryLogic->LogicEventCount;
  interveningLayer->Modified();
  targetReentryLayer->Modified();
  if (targetReentryLogic->LogicEventCount != eventCountBeforeStaleLayerModified + 1 || targetReentryLogic->TrackedLayerEventCount != 1
      || targetReentryLogic->SawPartiallyConfiguredLayer)
  {
    std::cerr << "Target replacement observation was not exclusive after reentry" << std::endl;
    return EXIT_FAILURE;
  }
  targetReentryLayer->RemoveObserver(changeTargetTag);

  vtkNew<vtkMRMLSliceLogic> logic;
  EXERCISE_BASIC_OBJECT_METHODS(logic.GetPointer());

  vtkNew<vtkMRMLScene> scene;

  // Add default slice orientation presets
  vtkMRMLSliceNode::AddDefaultSliceOrientationPresets(scene.GetPointer());

  logic->SetMRMLScene(scene.GetPointer());
  CHECK_NOT_NULL(logic->AddSliceNode("Green"));

  vtkNew<vtkMRMLSliceNode> SliceNode;
  TEST_SET_GET_VALUE(logic, SliceNode, SliceNode.GetPointer());

  vtkNew<vtkMRMLSliceLayerLogic> LabelLayer;
  TEST_SET_GET_VALUE(logic, LabelLayer, LabelLayer.GetPointer());

  vtkNew<vtkMRMLSliceCompositeNode> SliceCompositeNode;
  TEST_SET_GET_VALUE(logic, SliceCompositeNode, SliceCompositeNode.GetPointer());

  vtkNew<vtkMRMLSliceLayerLogic> ForegroundLayer;
  TEST_SET_GET_VALUE(logic, ForegroundLayer, ForegroundLayer.GetPointer());

  vtkNew<vtkMRMLSliceLayerLogic> BackgroundLayer;
  TEST_SET_GET_VALUE(logic, BackgroundLayer, BackgroundLayer.GetPointer());

  // TODO: need to fix the test.
  // The problem here is that the current node of the logic is wrong
  // it hasn't been added to the mrml scene. So when modified,
  // the logic realizes it and create a new node (losing the props).
  // TEST_SET_GET_VALUE(logic, SliceOffset, 1);

  logic->DeleteSliceModel();
  logic->CreateSliceModel();
  TEST_GET_OBJECT(logic, SliceModelNode);
  TEST_GET_OBJECT(logic, SliceModelDisplayNode);
  TEST_GET_OBJECT(logic, SliceModelTransformNode);
  TEST_GET_OBJECT(logic, Blend);

  logic->Print(std::cout);
  return EXIT_SUCCESS;
}
