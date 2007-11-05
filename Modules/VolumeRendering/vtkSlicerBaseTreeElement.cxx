#include "vtkSlicerBaseTreeElement.h"
#include "vtkObjectFactory.h"
vtkCxxRevisionMacro(vtkSlicerBaseTreeElement, "$Revision: 0.1 $");
vtkStandardNewMacro(vtkSlicerBaseTreeElement);
vtkSlicerBaseTreeElement::vtkSlicerBaseTreeElement(void)
{
}

vtkSlicerBaseTreeElement::~vtkSlicerBaseTreeElement(void)
{
}
void vtkSlicerBaseTreeElement::CreateWidget(void)
{
    Superclass::CreateWidget();
}
