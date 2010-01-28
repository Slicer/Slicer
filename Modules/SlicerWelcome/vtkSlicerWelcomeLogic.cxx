#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <vtksys/SystemTools.hxx>
#include <vtksys/Directory.hxx>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkIntArray.h"

#include "vtkSlicerWelcomeLogic.h"
#include "vtkSlicerWelcome.h"

#include "vtkMRMLScene.h"


//----------------------------------------------------------------------------
vtkSlicerWelcomeLogic* vtkSlicerWelcomeLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSlicerWelcomeLogic");
  if(ret)
    {
      return (vtkSlicerWelcomeLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSlicerWelcomeLogic;
}


//----------------------------------------------------------------------------
vtkSlicerWelcomeLogic::vtkSlicerWelcomeLogic()
{
}



//----------------------------------------------------------------------------
vtkSlicerWelcomeLogic::~vtkSlicerWelcomeLogic()
{ 
  this->SetAndObserveMRMLScene ( NULL );
}


//----------------------------------------------------------------------------
void vtkSlicerWelcomeLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}



