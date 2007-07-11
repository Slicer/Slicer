#ifndef __vtkSlicerRadioButtonWidget_h
#define __vtkSlicerRadioButtonWidget_h

#include "vtkKWRadioButton.h"
#include "vtkSlicerCheckRadioButtonIcons.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerRadioButtonWidget : public vtkKWRadioButton
{
 public:
  static vtkSlicerRadioButtonWidget * New ( );
  vtkTypeRevisionMacro ( vtkSlicerRadioButtonWidget, vtkKWRadioButton );
  void PrintSelf ( ostream& os, vtkIndent indent );
  
  vtkGetObjectMacro (Icons, vtkSlicerCheckRadioButtonIcons);
  vtkGetMacro (CustomIndicator, int);
  
  // Override superclass methods to switch indicator
  // indicator and/or icon, then call superclass method.
  virtual void SetDisabled ( );
  virtual void SetEnabled ( );

  // Description:
  // Turns Slicer's custom radiobutton indicator visible/invisible
  virtual void SetRadioboxVisibility ( int visibility );
  virtual int GetRadioboxVisibility ( );
  virtual void RadioboxVisibilityOn ( );
  virtual void RadioboxVisibilityOff ( );
  
 protected:
  vtkSlicerRadioButtonWidget();
  virtual ~vtkSlicerRadioButtonWidget();

  vtkSlicerCheckRadioButtonIcons *Icons;
  // Description:
  // Flag telling whether Slicer's custom indicator (bitmap) is on or off
  int CustomIndicator;
  
  // Description:
  // Create the widget
  virtual void CreateWidget();

 private:
  vtkSlicerRadioButtonWidget ( const vtkSlicerRadioButtonWidget& ); // Not implemented
  void operator=(const vtkSlicerRadioButtonWidget& ); // Not implemented
  
};

#endif

