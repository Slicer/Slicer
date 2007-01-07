#include "vtkIGTTracker.h"

#include "vtkObjectFactory.h"

vtkIGTTracker* vtkIGTTracker::New()
{
  vtkObject* ret=vtkObjectFactory::CreateInstance("vtkIGTTracker");
  if(ret)
    {
      return (vtkIGTTracker*) ret;
    }
  return new vtkIGTTracker;

}

vtkIGTTracker::vtkIGTTracker()
{
}


