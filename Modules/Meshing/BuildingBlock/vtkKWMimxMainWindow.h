/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMainWindow.h,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.18.4.2 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// .NAME vtkKWMimxMainUserInterfacePanel - Main window containing user
// interface panel and view window.
//
// .SECTION Description
// This is the main window containing user interface panels for operations
// and the view window to view the objects. Also contains access to a pop-up
// window to change the display settings.


#ifndef __vtkKWMimxMainWindow_h
#define __vtkKWMimxMainWindow_h

#include "vtkKWWindow.h"
#include "vtkKWRegistryHelper.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWWindow.h"
#include "vtkBuildingBlockWin32Header.h"
//#include "vtkKWMimxViewProperties.h"


class vtkCallbackCommand;
class vtkPVAxesActor;
class vtkRenderer;
class vtkKWIcon;
class vtkKWLoadSaveDialog;
class vtkKWRenderWidget;
class vtkMimxErrorCallback;
class vtkKWMimxDisplayPropertiesGroup;
class vtkKWMimxMainNotebook;
class vtkKWMimxViewProperties;
class vtkLinkedListWrapperTree;
class vtkKWFrameWithLabel;
class vtkKWCheckButtonWithLabel;
class vtkKWScaleWithLabel;
class vtkKWCheckButtonWithLabel;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWEntryWithLabel;
class vtkKWRadioButtonSet;
class vtkSlicerTheme;
class vtkKWTheme;
class vtkObject;
class vtkKWFrameWithScrollbar;

// *** collabsible frame definition
class vtkSlicerModuleCollapsibleFrame;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxMainWindow : public vtkKWCompositeWidget
{
public:
  static vtkKWMimxMainWindow* New();
  vtkTypeRevisionMacro(vtkKWMimxMainWindow,vtkKWCompositeWidget);

  // Description:
  // Get/Set the KWRenderWidget used to render the scene. The
  // RenderWidget must be set before calling the Create() method.
  vtkGetObjectMacro(RenderWidget, vtkKWRenderWidget);
  vtkSetObjectMacro(RenderWidget, vtkKWRenderWidget);

  // Description:
  // Get/Set the KWWindow used by the application. The Window must
  // be set before calling the Create() method.
  vtkGetObjectMacro(MainWindow, vtkKWWindow);
  vtkSetObjectMacro(MainWindow, vtkKWWindow);

  // Description:
  // Get/Set the mode for the user interface. The default mode (true)
  // is a standalone application. This will create options to
  // allow the user to set the font size and type. If the value
  // is false the main application is expected to manage this.
  // The mode must be set before calling the Create() method.
  vtkGetMacro(StandAloneApplication, bool);
  vtkSetMacro(StandAloneApplication, bool);

  // Description:
  // Get the Error dialog used to report errors
  vtkGetObjectMacro(ErrorCallback, vtkMimxErrorCallback);

  // Description:
  // Get the ViewProperties dialog box
  vtkGetObjectMacro(ViewProperties, vtkKWMimxViewProperties);

  // Description:
  // Get Axis Actor that is placed in the lower left.
  vtkGetObjectMacro(PVAxesActor, vtkPVAxesActor);

  // Description:
  // Get the user interface panel
  vtkGetObjectMacro(
          MainUserInterfacePanel, vtkKWMimxMainNotebook);

  // once the main window this will be drawing into is set, this method will
  // add and remove an orientation actor from the corner
  void RemoveOrientationAxis();
  void AddOrientationAxis();

  // Description:
  // Set the status text in the lower left of the application. This
  // is a convience function that calls the SetStatusText() in KWWindow.
  // The method is provided here for convience.It could be
  // eliminated with code refactoring in the other MIMX classes.
  void SetStatusText( const char * );

  // Description:
  // Get the UserInterfaceManager that is managed by the window. This
  // is a convience function that calls the GetMainUserInterfaceManager()
  // in KWWindow. The method is provided here for convience. It could be
  // eliminated with code refactoring in the other MIMX classes
  vtkKWUserInterfaceManager* GetMainUserInterfaceManager();

  // Description:
  // Update the Enabled state of the Window. This is a
  // convience function that calls the UpdateEnableState()
  // in KWWindow. The method is provided here for convience. It could be
  // eliminated with code refactoring in the other MIMX classes
  void UpdateEnableState();


  // Description:
  // Launch a window to containing display property change options
  void DisplayPropertyCallback();

  // Description:
  // Set/Get the application font family
  void SetApplicationFontFamily ( const char *family);
  const char *GetApplicationFontFamily ( ) const;

  // Description:
  // Set/Get the application font size
  void SetApplicationFontSize ( const char *size );
  const char *GetApplicationFontSize ( ) const;

  // Description:
  // Callback for autosave. Saves the active instance of the FE Mesh,
  // Building blocks and Surfaces in the object list
  void AutoSaveCallback( );
  void EnableAutoSave( );
  void DisableAutoSave( );

  // Description:
  // Set/Get the Current Render Window Text Color
  void SetTextColor(double color[3]);
  double *GetTextColor();

  // Description:
  // Set/Get the Current Render Window Background Color
  void SetBackgroundColor(double color[3]);
  using vtkKWFrame::SetBackgroundColor; 
  double *GetBackgroundColor();
  using vtkKWFrame::GetBackgroundColor; 

  // Description:
  // Set/Get the default average edge length for building blocks when created
  double GetAverageElementLength( );
  void SetAverageElementLength(double length);

  // Description:
  // Set/Get the default precision for material properties when written to ABAQUS
  int GetABAQUSPrecision( );
  void SetABAQUSPrecision(int precision);

  // Description:
  // Set/Get the application Auto Save Flag
  bool GetAutoSaveFlag( );
  void SetAutoSaveFlag(bool saveFlag);

  // Description:
  // Set/Get the Auto Save Working Directory Flag
  bool GetAutoSaveWorkDirFlag( );
  void SetAutoSaveWorkDirFlag(bool saveFlag);

  // Description:
  // Set/Get the application Auto Save Frequency
  int  GetAutoSaveTime( );
  void SetAutoSaveTime(int saveTime);

  // Description:
  // Set/Get the Auto Save Directory
  const char *GetWorkingDirectory();
  void SetWorkingDirectory(const char *dirName);
  void InitializeWorkingDirectory(const char *dirName);

  // Description:
  // Set/Get the Auto Save Directory
  const char *GetAutoSaveDirectory() const;
  void SetAutoSaveDirectory(const char *dirName);

  // Description:
  // Get Apply/Cancel Icons used througout the application
  vtkKWIcon* GetApplyButtonIcon();
  vtkKWIcon* GetCancelButtonIcon();

  // Description:
  // Get the Application Settings Interface as well as the Application
  // Settings User Interface Manager.
  //virtual vtkKWApplicationSettingsInterface *GetApplicationSettingsInterface();

  // Description:
  // Callbacks for the Module application settings. Internal, do not use.

  // Description:
  // Callbacks for font settings. Internal, do not use.
  virtual void SetFontFamilyCallback ( );
  virtual void SetFontSizeCallback ( );

  // Description:
  // Callbacks for the Application settings. Internal, do not use.
  virtual void AutoSaveModeCallback(int mode);
  virtual void AutoSaveScaleCallback(double value);
  virtual void AutoSaveDirectoryModeCallback(int mode);
  virtual void AutoSaveDirectoryCallback( );
  virtual void AverageElementLengthCallback(char *value);
  virtual void ABAQUSPrecisionCallback(double value);

  // Description:
  // Get the directory to be used for automated backups of the work
  const char *GetSaveDirectory ( );

  // Description:
  // Refresh the interface given the current value of the Window and its
  // views/composites/widgets.
  virtual void Update();

  // Description:
  // Load the previous set Registry Settings
  void LoadRegistryApplicationSettings();

  // Description:
  // Get/Set the Theme to be used for the Application
  virtual void InstallTheme ( vtkKWTheme *theme );
  virtual void InstallDefaultTheme ( );
  vtkSlicerTheme *GetSlicerTheme ( );


  // *** Force redraw since in slicer the panels weren't redrawing automatically
  void ForceWidgetRedraw(void);

  // add and remove the ApplicationSettings menus
  void CustomApplicationSettingsModuleEntry();
  void CustomApplicationSettingsModuleExit();

  // clear and display the objects from the lists.  This is used when the
  // module is entered and exited, so the actors don't display in the slicer
  // window when we are not in the module.  This traverses the list and stores
  // state in the MRML nodes.
  void SaveVisibilityStateOfObjectLists(void);
  void RestoreVisibilityStateOfObjectLists(void);

  // Widgets that must be public
  vtkKWRenderWidget *RenderWidget;
  vtkRenderer *AxesRenderer;
  vtkKWMimxMainNotebook *MainUserInterfacePanel;

protected:
  vtkKWMimxMainWindow();
  ~vtkKWMimxMainWindow();
  virtual void CreateWidget();
  vtkMimxErrorCallback *ErrorCallback;
  vtkKWMimxViewProperties *ViewProperties;
  vtkLinkedListWrapperTree *DoUndoTree;
  vtkKWMimxDisplayPropertiesGroup *DisplayPropertyDialog;
  vtkSlicerModuleCollapsibleFrame *MainFrame;

        
private:
  vtkKWMimxMainWindow(const vtkKWMimxMainWindow&);   // Not implemented.
  void operator=(const vtkKWMimxMainWindow&);  // Not implemented.

  void AddCustomApplicationSettingsPanel();
  void AddFontApplicationSettingsPanel();

  vtkKWIcon *applyIcon;
  vtkKWIcon *cancelIcon;

  char autoSaveEventId[256];

  /* Application Settings Interface Widgets */
  vtkPVAxesActor *PVAxesActor;
  vtkCallbackCommand *CallbackCommand;
  vtkKWWindow *MainWindow;
  vtkKWFrameWithLabel *MimxSettingsFrame;
  vtkKWCheckButtonWithLabel *AutoSaveButton;
  vtkKWScaleWithLabel *AutoSaveScale;
  vtkKWCheckButtonWithLabel *WorkingDirButton;
  vtkKWLoadSaveButtonWithLabel *AutoSaveDir;
  vtkKWEntryWithLabel *AverageElementLengthEntry;
  vtkKWScaleWithLabel *PropertyPrecisionScale;
  
  /* Hold Current Application Settings */
  /* Font Configuration */
  vtkKWFrameWithLabel *FontSettingsFrame;
  vtkKWRadioButtonSet *FontSizeButtons;
  vtkKWRadioButtonSet *FontFamilyButtons;
  vtkKWFrameWithScrollbar *FontScrollFrame;
  
  void RemoveFontApplicationSettingsPanel();
  void RemoveCustomApplicationSettingsPanel();
  
  vtkSlicerTheme *SlicerTheme;
  char ApplicationFontSize [vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char ApplicationFontFamily [vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  bool AutoSaveFlag;
  int  AutoSaveTime;
  char AutoSaveDirectory[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  bool AutoSaveWorkDirFlag; 
  char WorkingDirectory[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char DateTimeString[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  char SaveDirectory[vtkKWRegistryHelper::RegistryKeyValueSizeMax];
  double AverageElementLength;
  int ABAQUSPrecision;
  double TextColor[3];
  double BackgroundColor[3];     
  bool StandAloneApplication;            

};

void updateAxis(vtkObject* caller, unsigned long , void* arg, void* );

#endif
