#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <vtksys/SystemTools.hxx>
#include <vtksys/Directory.hxx>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkIntArray.h"

#include "vtkMeasurementsLogic.h"
#include "vtkMeasurements.h"

#include "vtkMRMLScene.h"


//----------------------------------------------------------------------------
vtkMeasurementsLogic* vtkMeasurementsLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMeasurementsLogic");
  if(ret)
    {
      return (vtkMeasurementsLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMeasurementsLogic;
}


//----------------------------------------------------------------------------
vtkMeasurementsLogic::vtkMeasurementsLogic()
{
}



//----------------------------------------------------------------------------
vtkMeasurementsLogic::~vtkMeasurementsLogic()
{ 
  this->SetAndObserveMRMLScene ( NULL );
}


//----------------------------------------------------------------------------
void vtkMeasurementsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);  
}



