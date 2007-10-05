// .NAME vtkSlicerApplicationSettingsInterface
// .SECTION Description
// Slicer specific ApplicationSettingsInterface. Provides a user
// interface to set/view application settings.
//

#ifndef __vtkSlicerApplicationSettingsInterface_h
#define __vtkSlicerApplicationSettingsInterface_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWApplicationSettingsInterface.h"

class vtkKWFrameWithLabel;
class vtkKWEntryWithLabel;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWCheckButton;
class vtkKWRadioButtonSet;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerApplicationSettingsInterface
  : public vtkKWApplicationSettingsInterface
{
public:
  static vtkSlicerApplicationSettingsInterface* New();
  vtkTypeRevisionMacro(vtkSlicerApplicationSettingsInterface,vtkKWApplicationSettingsInterface);
  vtkGetObjectMacro (FontSizeButtons, vtkKWRadioButtonSet );
  vtkGetObjectMacro (FontFamilyButtons, vtkKWRadioButtonSet );

  // Description:
  // Create the widget.
  virtual void Create();
  
  // Description:
  // Refresh the interface given the current value of the Window and its
  // views/composites/widgets.
  virtual void Update();

  // Description:
  // Callbacks for the Module application settings. Internal, do not use.

  virtual void ModulePathCallback(char *);
  virtual void ModuleCachePathCallback(char *);
  virtual void HomeModuleCallback(char *name);
  virtual void TemporaryDirectoryCallback();
  virtual void BrowserSelectCallback();
  virtual void ZipSelectCallback();
  virtual void UnzipSelectCallback();
  virtual void RmSelectCallback();
  virtual void LoadCommandLineModulesCallback(int state);
  virtual void EnableDaemonCallback(int state);

  // Description:
  // Callbacks for the Slicer application settings
  virtual void ConfirmDeleteCallback(int state);
  // Description:
  // Callbacks for the Slicer font settings.
  virtual void SetFontFamilyCallback ( );
  virtual void SetFontSizeCallback ( );

protected:
  vtkSlicerApplicationSettingsInterface();
  ~vtkSlicerApplicationSettingsInterface();

  
private:
  vtkSlicerApplicationSettingsInterface(const vtkSlicerApplicationSettingsInterface&); // Not implemented
  void operator=(const vtkSlicerApplicationSettingsInterface&); // Not Implemented

  vtkKWFrameWithLabel *SlicerSettingsFrame;
  vtkKWCheckButton  *ConfirmDeleteCheckButton;
  
  vtkKWFrameWithLabel *ModuleSettingsFrame;
  vtkKWFrameWithLabel *FontSettingsFrame;
  vtkKWEntryWithLabel *HomeModuleEntry;
  vtkKWEntryWithLabel *ModulePathEntry;
  vtkKWEntryWithLabel *ModuleCachePathEntry;
  vtkKWLoadSaveButtonWithLabel *TemporaryDirectoryButton;
  vtkKWLoadSaveButtonWithLabel *BrowserSelectButton;
  vtkKWLoadSaveButtonWithLabel *ZipSelectButton;
  vtkKWLoadSaveButtonWithLabel *UnzipSelectButton;
  vtkKWLoadSaveButtonWithLabel *RmSelectButton;
  vtkKWRadioButtonSet *FontSizeButtons;
  vtkKWRadioButtonSet *FontFamilyButtons;
  vtkKWCheckButton  *LoadCommandLineModulesCheckButton;
  vtkKWCheckButton  *EnableDaemonCheckButton;
};


#endif
