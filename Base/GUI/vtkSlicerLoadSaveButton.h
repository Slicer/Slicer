#ifndef __vtkSlicerLoadSaveButton_h
#define __vtkSlicerLoadSaveButton_h

#include "vtkKWPushButton.h"
#include "vtkSlicerBaseGUIWin32Header.h"

class vtkSlicerLoadSaveDialog;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerLoadSaveButton : public vtkKWPushButton
{
public:
  static vtkSlicerLoadSaveButton* New();
  vtkTypeRevisionMacro(vtkSlicerLoadSaveButton, vtkKWPushButton);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Access to sub-widgets.
  vtkGetObjectMacro(LoadSaveDialog, vtkSlicerLoadSaveDialog);

  // Description:
  // Retrieve the filename. This method only query the GetFileName method
  // on the LoadSaveDialog member.
  virtual const char* GetFileName();

  // Description:
  // Set/Get the length of the filename when displayed in the button.
  // If set to 0, do not shorten the filename.
  virtual void SetMaximumFileNameLength(int);
  vtkGetMacro(MaximumFileNameLength, int);

  // Description:
  // Set/Get if the path of the filename should be trimmed when displayed in
  // the button.
  virtual void SetTrimPathFromFileName(int);
  vtkBooleanMacro(TrimPathFromFileName, int);
  vtkGetMacro(TrimPathFromFileName, int);
  
  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

  // Description:
  // Add all the default observers needed by that object, or remove
  // all the observers that were added through AddCallbackCommandObserver.
  // Subclasses can override these methods to add/remove their own default
  // observers, but should call the superclass too.
  virtual void AddCallbackCommandObservers();
  virtual void RemoveCallbackCommandObservers();

protected:
  vtkSlicerLoadSaveButton();
  ~vtkSlicerLoadSaveButton();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  vtkSlicerLoadSaveDialog *LoadSaveDialog;

  int TrimPathFromFileName;
  int MaximumFileNameLength;
  virtual void UpdateTextFromFileName();

  virtual void InvokeCommand();

  // Description:
  // Processes the events that are passed through CallbackCommand (or others).
  // Subclasses can oberride this method to process their own events, but
  // should call the superclass too.
  virtual void ProcessCallbackCommandEvents(
    vtkObject *caller, unsigned long event, void *calldata);
  
private:
  vtkSlicerLoadSaveButton(const vtkSlicerLoadSaveButton&); // Not implemented
  void operator=(const vtkSlicerLoadSaveButton&); // Not implemented
};

#endif

