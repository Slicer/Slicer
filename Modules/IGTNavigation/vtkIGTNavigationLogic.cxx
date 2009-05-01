#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "vtkObjectFactory.h"
#include "vtkImageChangeInformation.h"

#include "vtkIGTNavigationLogic.h"
#include "vtkIGTNavigation.h"

#include "vtkMRMLScene.h"

#include "vtkMath.h"

#include <math.h>
#include <exception>

#include <vtksys/SystemTools.hxx>

#define ERROR_NODE_VTKID 0


//----------------------------------------------------------------------------
vtkIGTNavigationLogic* vtkIGTNavigationLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkIGTNavigationLogic");
  if(ret)
    {
    return (vtkIGTNavigationLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkIGTNavigationLogic;
}


//----------------------------------------------------------------------------
vtkIGTNavigationLogic::vtkIGTNavigationLogic()
{
  this->ModuleName = NULL;

  //this->DebugOn();

  this->MRMLManager = NULL; // NB: must be set before SetMRMLManager is called
  vtkIGTNavigationMRMLManager* manager = vtkIGTNavigationMRMLManager::New();
  this->SetMRMLManager(manager);
  manager->Delete();
}

//----------------------------------------------------------------------------
vtkIGTNavigationLogic::~vtkIGTNavigationLogic()
{
  this->SetMRMLManager(NULL);
  this->SetModuleName(NULL);
}

//-----------------------------------------------------------------------------
vtkIntArray*
vtkIGTNavigationLogic::
NewObservableEvents()
{
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  return events;
}

//----------------------------------------------------------------------------
void vtkIGTNavigationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  // !!! todo
}


