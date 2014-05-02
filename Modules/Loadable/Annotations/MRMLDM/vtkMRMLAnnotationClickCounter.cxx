
// Annotation MRMLDisplayableManager includes
#include "vtkMRMLAnnotationClickCounter.h"

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationClickCounter);

//---------------------------------------------------------------------------
void vtkMRMLAnnotationClickCounter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationClickCounter::vtkMRMLAnnotationClickCounter()
{
  this->m_Clicks = 0;
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationClickCounter::~vtkMRMLAnnotationClickCounter()
{
  // TODO Auto-generated destructor stub
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationClickCounter::Reset()
{
  this->m_Clicks = 0;
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationClickCounter::Click()
{
  return ++this->m_Clicks;
}

//---------------------------------------------------------------------------
bool vtkMRMLAnnotationClickCounter::HasEnoughClicks(int clicks)
{
  this->Click();

  if (this->m_Clicks==clicks)
    {
    this->Reset();
    return true;
    }

  return false;
}
