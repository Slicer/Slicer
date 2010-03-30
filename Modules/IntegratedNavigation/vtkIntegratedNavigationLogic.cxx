#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "vtkObjectFactory.h"
#include "vtkImageChangeInformation.h"

#include "vtkIntegratedNavigationLogic.h"
#include "vtkIntegratedNavigation.h"

#include "vtkMRMLScene.h"

#include "vtkMath.h"

#include <math.h>
#include <exception>

#include <vtksys/SystemTools.hxx>

#define ERROR_NODE_VTKID 0


//----------------------------------------------------------------------------
vtkIntegratedNavigationLogic* vtkIntegratedNavigationLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkIntegratedNavigationLogic");
  if(ret)
    {
    return (vtkIntegratedNavigationLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkIntegratedNavigationLogic;
}


//----------------------------------------------------------------------------
vtkIntegratedNavigationLogic::vtkIntegratedNavigationLogic()
{
  this->ModuleName = NULL;

  //this->DebugOn();

  this->MRMLManager = NULL; // NB: must be set before SetMRMLManager is called
  vtkIntegratedNavigationMRMLManager* manager = vtkIntegratedNavigationMRMLManager::New();
  this->SetMRMLManager(manager);
  manager->Delete();
}

//----------------------------------------------------------------------------
vtkIntegratedNavigationLogic::~vtkIntegratedNavigationLogic()
{
  this->SetMRMLManager(NULL);
  this->SetModuleName(NULL);
}

//-----------------------------------------------------------------------------
vtkIntArray*
vtkIntegratedNavigationLogic::
NewObservableEvents()
{
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);

  return events;
}

//----------------------------------------------------------------------------
void vtkIntegratedNavigationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  // !!! todo
}


