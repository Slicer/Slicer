#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "vtkObjectFactory.h"
#include "vtkImageChangeInformation.h"

#include "vtkIGTPlanningLogic.h"
#include "vtkIGTPlanning.h"

#include "vtkMRMLScene.h"

#include "vtkMath.h"

#include <math.h>
#include <exception>

#include <vtksys/SystemTools.hxx>

#define ERROR_NODE_VTKID 0


//----------------------------------------------------------------------------
vtkIGTPlanningLogic* vtkIGTPlanningLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkIGTPlanningLogic");
  if(ret)
    {
    return (vtkIGTPlanningLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkIGTPlanningLogic;
}


//----------------------------------------------------------------------------
vtkIGTPlanningLogic::vtkIGTPlanningLogic()
{
  this->ModuleName = NULL;

  //this->DebugOn();

  this->MRMLManager = NULL; // NB: must be set before SetMRMLManager is called
  vtkIGTPlanningMRMLManager* manager = vtkIGTPlanningMRMLManager::New();
  this->SetMRMLManager(manager);
  manager->Delete();
}

//----------------------------------------------------------------------------
vtkIGTPlanningLogic::~vtkIGTPlanningLogic()
{
  this->SetMRMLManager(NULL);
  this->SetModuleName(NULL);
}

//-----------------------------------------------------------------------------
vtkIntArray*
vtkIGTPlanningLogic::
NewObservableEvents()
{
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  return events;
}

//----------------------------------------------------------------------------
void vtkIGTPlanningLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  // !!! todo
}


