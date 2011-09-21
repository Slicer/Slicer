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

//-----------------------------------------------------------------------------
vtkIntArray*
vtkBrainlabModuleLogic::
NewObservableEvents()
{
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  return events;
}

//----------------------------------------------------------------------------
void vtkBrainlabModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  // !!! todo
}


