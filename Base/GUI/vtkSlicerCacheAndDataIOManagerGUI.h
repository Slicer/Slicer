#ifndef __vtkSlicerCacheAndDataIOManagerGUI_h
#define __vtkSlicerCacheAndDataIOManagerGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"

#include "vtkCacheManager.h"

#include "vtkObserverManager.h"
#include "vtkMRMLScene.h"
#include "vtkDataIOManager.h"

#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWLabel.h"
#include "vtkKWTopLevel.h"
#include "vtkKWFrameWithScrollbar.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerCacheAndDataIOManagerGUI : public vtkSlicerComponentGUI
{
public:
  // Description: 
  // Usual vtk class functions 
  static vtkSlicerCacheAndDataIOManagerGUI* New (  );
  vtkTypeRevisionMacro ( vtkSlicerCacheAndDataIOManagerGUI, vtkSlicerComponentGUI );
  void PrintSelf ( ostream& os, vtkIndent indent );

  // Description:
  // tracks all data transfers in and out of scene
  vtkGetObjectMacro ( DataIOManager, vtkDataIOManager );

  // Description:
  // tracks cache
  vtkGetObjectMacro ( CacheManager, vtkCacheManager );
  
  vtkGetObjectMacro ( CacheFullLabel, vtkKWLabel );
  vtkGetObjectMacro ( CloseButton, vtkKWPushButton );
  vtkGetObjectMacro ( ClearCacheButton, vtkKWPushButton );
  vtkGetObjectMacro ( ForceReloadCheckButton, vtkKWCheckButton );
  vtkGetObjectMacro ( OverwriteCacheCheckButton, vtkKWCheckButton );
  vtkGetObjectMacro ( AsynchronousCheckButton, vtkKWCheckButton );
  vtkGetObjectMacro ( TimeOutCheckButton, vtkKWCheckButton );
  vtkGetObjectMacro ( ManagerTopLevel, vtkKWTopLevel );
  vtkGetObjectMacro ( ControlFrame, vtkKWFrame );
  vtkGetObjectMacro ( TransfersFrame, vtkKWFrameWithScrollbar );
  vtkSetMacro ( Built, bool );
  vtkGetMacro ( Built, bool );

  // Description:
  // This method builds the DataIOManager module GUI
  virtual void BuildGUI ( ) ;

  // Description:
  // This method releases references and key-bindings,
  // and optionally removes observers.
  virtual void TearDownGUI ( );

  // Description:
  // Methods for adding module-specific key bindings and
  // removing them.
  virtual void CreateModuleEventBindings ( );
  virtual void ReleaseModuleEventBindings ( );

  // Description:
  // Add/Remove observers on widgets in the GUI
  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );

  // Description:
  // Class's mediator methods for processing events invoked by
  // either the Logic, MRML or GUI.
  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );


  // Description:
  // Methods describe behavior at module enter and exit.
  virtual void Enter ( );
  virtual void Exit ( );

  // Description:
  // Update the gui from the currently selected DataIOManager node, called on Enter
  void UpdateGUI();

  // Description:
  // API for setting DataIOManager, and for
  // setting and observing it.
  void SetDataIOManager ( vtkDataIOManager *iomanager);
  void SetAndObserveDataIOManager ( vtkDataIOManager *iomanager);

  // Description:
  // API for setting CacheManager, and for
  // setting and observing it.
  void SetCacheManager ( vtkCacheManager *cmanager);
  void SetAndObserveCacheManager ( vtkCacheManager *cmanager );
  
  // Description:
  // Methods for displaying and hiding the Cache and DataIO Manager
  // Internal callbacks; do not use.
  virtual void DisplayManagerWindow ( );
  virtual void WithdrawManagerWindow ( );

protected:
  vtkSlicerCacheAndDataIOManagerGUI ( );
  virtual ~vtkSlicerCacheAndDataIOManagerGUI ( );

  // Widgets for the DataIOManager module
  // add a point
  vtkKWLabel *CacheFullLabel;
  vtkKWPushButton *CloseButton;
  vtkKWPushButton *ClearCacheButton;
  vtkKWCheckButton *ForceReloadCheckButton;
  vtkKWCheckButton *OverwriteCacheCheckButton;
  vtkKWCheckButton *AsynchronousCheckButton;
  vtkKWCheckButton *TimeOutCheckButton;
  vtkKWTopLevel *ManagerTopLevel;
  vtkKWFrame *ControlFrame;
  vtkKWFrameWithScrollbar *TransfersFrame;
  
  vtkDataIOManager *DataIOManager;
  vtkCacheManager *CacheManager;

    // Description:
    // Describes whether the GUI has been built or not.
    bool Built;

private:
  vtkSlicerCacheAndDataIOManagerGUI ( const vtkSlicerCacheAndDataIOManagerGUI& ); // Not implemented.
  void operator = ( const vtkSlicerCacheAndDataIOManagerGUI& ); //Not implemented.
};
#endif
