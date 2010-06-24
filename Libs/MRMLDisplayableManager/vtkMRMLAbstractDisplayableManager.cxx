
// VTK includes
#include <vtkObjectFactory.h>

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLAbstractDisplayableManager);

//-----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager::vtkMRMLAbstractDisplayableManager()
{

}

//-----------------------------------------------------------------------------
vtkMRMLAbstractDisplayableManager::~vtkMRMLAbstractDisplayableManager()
{

}

//----------------------------------------------------------------------------
void vtkMRMLAbstractDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkObject::PrintSelf(os,indent);
}
