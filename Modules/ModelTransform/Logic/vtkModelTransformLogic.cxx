#include "vtkModelTransformLogic.h"


vtkModelTransformLogic* vtkModelTransformLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkModelTransformLogic");
  if(ret)
    {
      return (vtkModelTransformLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkModelTransformLogic;
}



//----------------------------------------------------------------------------
vtkModelTransformLogic::vtkModelTransformLogic()
{
   
}



//----------------------------------------------------------------------------
vtkModelTransformLogic::~vtkModelTransformLogic()
{ 
}
