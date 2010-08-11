#include "MRMLDisplayableManager/vtkMRMLAnnotationClickCounter.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>


// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationClickCounter);
vtkCxxRevisionMacro (vtkMRMLAnnotationClickCounter, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
void vtkMRMLAnnotationClickCounter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

vtkMRMLAnnotationClickCounter::vtkMRMLAnnotationClickCounter()
{
  this->m_Clicks = 0;

}

vtkMRMLAnnotationClickCounter::~vtkMRMLAnnotationClickCounter()
{
  // TODO Auto-generated destructor stub
}

void vtkMRMLAnnotationClickCounter::Reset()
{
  this->m_Clicks = 0;
}

bool vtkMRMLAnnotationClickCounter::HasEnoughClicks(int clicks)
{
  this->m_Clicks++;

  if (this->m_Clicks==clicks)
    {
      this->Reset();
      return true;
    }

  return false;
}
