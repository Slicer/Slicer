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
class vtkKWSpinBoxWithLabel;

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
  virtual void LoadCommandLineModulesCallback(int state);
 
  // Description:
  // Callbacks for the Slicer application settings. Internal, do not use.
  virtual void BrowserSelectCallback();
  virtual void ZipSelectCallback();
  virtual void UnzipSelectCallback();
  virtual void RmSelectCallback();
  virtual void EnableDaemonCallback(int state);
  virtual void ConfirmDeleteCallback(int state);
  
  // Description:
  // Callbacks for the Slicer font settings.
  virtual void SetFontFamilyCallback ( );
  virtual void SetFontSizeCallback ( );

  // Description:
  // Callbacks for the Remote Data Handling settings.
  // Description:
  // Enable and disable asychronous IO, if false use blocking IO, if true,
  // downloads and loading are forked off into a separate thread
  virtual void EnableAsynchronousIOCallback(int state);
  // Description:
  // If true, even if the file is in the cache, download it again. If false,
  // check for and use the cached copy.
  virtual void EnableForceRedownloadCallback(int state);
  // Description:
  // If true, over write copies in the cache when downloading a remote
  // file. If false, rename the old copy.
//  virtual void EnableRemoteCacheOverwritingCallback(int state);
  // Description:
  // Set the cache directory on disk
  virtual void RemoteCacheDirectoryCallback();
  // Description:
  // Set the upper limit on the size of the cache directory
  virtual void RemoteCacheLimitCallback(int size);
  // Description:
  // Set the amount of space in the cache directory that should remain free
  virtual void RemoteCacheFreeBufferSizeCallback(int size);
  
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

  vtkKWFrameWithLabel *RemoteCacheSettingsFrame;
  vtkKWCheckButton *EnableAsynchronousIOCheckButton;
  vtkKWCheckButton *EnableForceRedownloadCheckButton;
//  vtkKWCheckButton *EnableRemoteCacheOverwritingCheckButton;
  vtkKWLoadSaveButtonWithLabel *RemoteCacheDirectoryButton;
  vtkKWSpinBoxWithLabel *RemoteCacheLimitSpinBox;
  vtkKWSpinBoxWithLabel * RemoteCacheFreeBufferSizeSpinBox;
  
};

#endif
