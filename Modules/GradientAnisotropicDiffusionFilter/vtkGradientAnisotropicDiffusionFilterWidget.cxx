#include "vtkGradientAnisotropicDiffusionFilterWidget.h"

#include "vtkObjectFactory.h"
#include "vtkKWScale.h"
#include "vtkCommand.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkGradientAnisotropicDiffusionFilterWidget );
vtkCxxRevisionMacro(vtkGradientAnisotropicDiffusionFilterWidget, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkGradientAnisotropicDiffusionFilterWidget::vtkGradientAnisotropicDiffusionFilterWidget()
{
  this->Scale = NULL;
}

//----------------------------------------------------------------------------
vtkGradientAnisotropicDiffusionFilterWidget::~vtkGradientAnisotropicDiffusionFilterWidget()
{
  if (this->Scale)
    {
    this->Scale->Delete();
    this->Scale = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterWidget::Create()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::Create();

  // Create our internal scale widget

  if (!this->Scale)
    {
    this->Scale = vtkKWScale::New();
    }
  this->Scale->SetParent(this);
  this->Scale->Create();
  this->Scale->SetResolution(0.5);

  this->Script("pack %s", this->Scale->GetWidgetName());

  // Our aim here is to use that scale widget to modify the application's value
  // First make sure the scale reflects the current app's value

  this->Scale->SetValue(6666);

  // Listen to the changes made to our internal scale widget through either
  // callbacks or events (for demonstration purposes, you would normally
  // just pick one), which will then modify the app's value accordingly
  
  this->Scale->SetCommand(this, "ScaleChangeNotifiedByCommandCallback");

  this->AddCallbackCommandObserver(
    this->Scale, vtkKWScale::ScaleValueChangingEvent);

  // Listen to the changes made to the application's value directly, so
  // that our scale is always in sync. There are different ways to do it,
  // depending on what mechanism the application uses to notify about
  // changes made to its values. Here, let's listen to the ModifiedEvent

  //this->AddCallbackCommandObserver(myapp, vtkCommand::ModifiedEvent);
}

//----------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterWidget::ScaleChangeNotifiedByCommandCallback(double value)
{

}

//----------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterWidget::ProcessCallbackCommandEvents(
  vtkObject *caller, unsigned long event, void *calldata)
{
 

  this->Superclass::ProcessCallbackCommandEvents(caller, event, calldata);
}
