
#include "vtkChartViewInteractorStyle.h"

// VTK includes
#include "vtkCamera.h"
#include "vtkCallbackCommand.h"
#include "vtkMath.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkMRMLInteractionNode.h"

// for picking
#include "vtkMRMLModelDisplayableManager.h"

vtkCxxRevisionMacro(vtkChartViewInteractorStyle, "$Revision: 13328 $");
vtkStandardNewMacro(vtkChartViewInteractorStyle);

//----------------------------------------------------------------------------
vtkChartViewInteractorStyle::vtkChartViewInteractorStyle() 
{
}

//----------------------------------------------------------------------------
vtkChartViewInteractorStyle::~vtkChartViewInteractorStyle() 
{
 
}

//----------------------------------------------------------------------------
void vtkChartViewInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkChartViewInteractorStyle::SetModelDisplayableManager(
    vtkMRMLModelDisplayableManager * modelDisplayableManager)
{
  this->ModelDisplayableManager = modelDisplayableManager;
}

