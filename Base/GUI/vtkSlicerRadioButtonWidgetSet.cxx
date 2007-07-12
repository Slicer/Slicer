#include "vtkSlicerRadioButtonWidgetSet.h"

#include "vtkSlicerRadioButtonWidget.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------

vtkStandardNewMacro(vtkSlicerRadioButtonWidgetSet);
vtkCxxRevisionMacro(vtkSlicerRadioButtonWidgetSet, "$Revision: 1.23 $");

//----------------------------------------------------------------------------
vtkSlicerRadioButtonWidget* vtkSlicerRadioButtonWidgetSet::GetWidget(int id)
{
  return static_cast<vtkSlicerRadioButtonWidget*>(this->GetWidgetInternal(id));
}

//----------------------------------------------------------------------------
vtkSlicerRadioButtonWidget* vtkSlicerRadioButtonWidgetSet::AddWidget(int id)
{
  return static_cast<vtkSlicerRadioButtonWidget*>(this->InsertWidgetInternal(
                                          id, this->GetNumberOfWidgets()));
}

//----------------------------------------------------------------------------
vtkSlicerRadioButtonWidget* vtkSlicerRadioButtonWidgetSet::InsertWidget(int id, int pos)
{
  return static_cast<vtkSlicerRadioButtonWidget*>(this->InsertWidgetInternal(
                                          id, pos));
}

//----------------------------------------------------------------------------
vtkKWWidget* vtkSlicerRadioButtonWidgetSet::InsertWidgetInternal(
  int id, int pos)
{
  vtkSlicerRadioButtonWidget *widget = static_cast<vtkSlicerRadioButtonWidget*>(
    this->Superclass::InsertWidgetInternal(id, pos));
  if (widget)
    {
    widget->SetValueAsInt(id);
    }
  return widget;
}

//----------------------------------------------------------------------------
vtkKWWidget* vtkSlicerRadioButtonWidgetSet::AllocateAndCreateWidget()
{
  vtkSlicerRadioButtonWidget *widget = vtkSlicerRadioButtonWidget::New();
  widget->SetParent(this);
  widget->Create();

  // For convenience, all radiobuttons share the same var name

  if (this->GetNumberOfWidgets())
    {
    vtkSlicerRadioButtonWidget *first = this->GetWidget(this->GetIdOfNthWidget(0));
    if (first)
      {
      widget->SetVariableName(first->GetVariableName());
      }
    }

  return static_cast<vtkKWWidget*>(widget);
}

//----------------------------------------------------------------------------
void vtkSlicerRadioButtonWidgetSet::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
