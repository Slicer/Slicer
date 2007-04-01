#ifndef __vtkSlicerFileBrowserDialog_h
#define __vtkSlicerFileBrowserDialog_h

#include "vtkKWFileBrowserDialog.h"
#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerFileBrowserIcons.h"


class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerFileBrowserDialog : public vtkKWFileBrowserDialog
{
public:
  static vtkSlicerFileBrowserDialog* New();
  vtkTypeRevisionMacro(vtkSlicerFileBrowserDialog,vtkKWFileBrowserDialog);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  vtkGetObjectMacro ( Icons, vtkSlicerFileBrowserIcons );
    
protected:
  vtkSlicerFileBrowserDialog();
  ~vtkSlicerFileBrowserDialog();

  // Description:
  // Create the widget.
  virtual void CreateWidget();
  vtkSlicerFileBrowserIcons *Icons;

private:
  vtkSlicerFileBrowserDialog(const vtkSlicerFileBrowserDialog&); // Not implemented
  void operator=(const vtkSlicerFileBrowserDialog&); // Not implemented
};
#endif
