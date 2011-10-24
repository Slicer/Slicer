#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "vtkObjectFactory.h"
#include "vtkImageChangeInformation.h"

#include "vtkBrainlabModuleLogic.h"
#include "vtkBrainlabModule.h"

#include "vtkMRMLScene.h"

#include "vtkMath.h"

#include <math.h>
#include <exception>

#include <vtksys/SystemTools.hxx>

#define ERROR_NODE_VTKID 0


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkBrainlabModuleLogic);


//----------------------------------------------------------------------------
vtkBrainlabModuleLogic::vtkBrainlabModuleLogic()
{
  this->ModuleName = NULL;

  //this->DebugOn();

  this->MRMLManager = NULL; // NB: must be set before SetMRMLManager is called
  vtkBrainlabModuleMRMLManager* manager = vtkBrainlabModuleMRMLManager::New();
  this->SetMRMLManager(manager);
  manager->Delete();
}

//----------------------------------------------------------------------------
vtkBrainlabModuleLogic::~vtkBrainlabModuleLogic()
{
  this->SetMRMLManager(NULL);
  this->SetModuleName(NULL);
}

//----------------------------------------------------------------------------
void vtkBrainlabModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  // !!! todo
}

//----------------------------------------------------------------------------
void vtkBrainlabModuleLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkSmartPointer<vtkIntArray> sceneEvents = vtkSmartPointer<vtkIntArray>::New();
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents);

  this->MRMLManager->SetMRMLScene(newScene);
}

//----------------------------------------------------------------------------
void vtkBrainlabModuleLogic::RegisterNodes()
{
  this->MRMLManager->RegisterMRMLNodesWithScene();
}

//----------------------------------------------------------------------------
void vtkBrainlabModuleLogic::ProcessMRMLSceneEvents(vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData)
{
  this->MRMLManager->ProcessMRMLEvents(caller, event, callData); 
}

