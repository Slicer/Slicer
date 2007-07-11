#ifndef __vtkSlicerCheckButtonWidget_h
#define __vtkSlicerCheckButtonWidget_h

#include "vtkKWCheckButton.h"
#include "vtkSlicerCheckRadioButtonIcons.h"
#include "vtkKWIcon.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerCheckButtonWidget : public vtkKWCheckButton
{
 public:
  static vtkSlicerCheckButtonWidget * New ( );
  vtkTypeRevisionMacro ( vtkSlicerCheckButtonWidget, vtkKWCheckButton );
  void PrintSelf ( ostream& os, vtkIndent indent );
  
  vtkGetObjectMacro (Icons, vtkSlicerCheckRadioButtonIcons);
  vtkGetMacro (CustomIndicator, int);
  
  // Override superclass methods to switch indicator
  // indicator and/or icon, then call superclass method.
  virtual void SetDisabled ( );
  virtual void SetEnabled ( );

  // Description:
  // Turns Slicer's custom checkbutton indicator visible/invisible
  virtual void SetCheckboxVisibility ( int visibility );
  virtual int GetCheckboxVisibility ( );
  virtual void CheckboxVisibilityOn ( );
  virtual void CheckboxVisibilityOff ( );

 protected:
  vtkSlicerCheckButtonWidget();
  virtual ~vtkSlicerCheckButtonWidget();

  vtkSlicerCheckRadioButtonIcons *Icons;
  // Description:
  // Flag telling whether Slicer's custom indicator (bitmap) is on or off
  int CustomIndicator;
  
  // Description
  // Create the widget
  virtual void CreateWidget();

 private:
  vtkSlicerCheckButtonWidget ( const vtkSlicerCheckButtonWidget& ); // Not implemented
  void operator=(const vtkSlicerCheckButtonWidget& ); // Not implemented
  
};

#endif

