
// VTK includes
#include <vtkObjectFactory.h>

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractLogic.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLAbstractLogic);

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic::vtkMRMLAbstractLogic()
{

}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic::~vtkMRMLAbstractLogic()
{

}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkObject::PrintSelf(os,indent);
}
