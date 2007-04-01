#ifndef __vtkSlicerLoadSaveDialog_h
#define __vtkSlicerLoadSaveDialog_h

#include "vtkSlicerFileBrowserDialog.h"
#include "vtkSlicerBaseGUIWin32Header.h"

class vtkStringArray;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerLoadSaveDialog : public vtkSlicerFileBrowserDialog
{
public:
  static vtkSlicerLoadSaveDialog* New();
  vtkTypeRevisionMacro(vtkSlicerLoadSaveDialog,vtkSlicerFileBrowserDialog);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkSlicerLoadSaveDialog() {};
  ~vtkSlicerLoadSaveDialog() {};

private:
  vtkSlicerLoadSaveDialog(const vtkSlicerLoadSaveDialog&); // Not implemented
  void operator=(const vtkSlicerLoadSaveDialog&); // Not implemented
};


#endif



