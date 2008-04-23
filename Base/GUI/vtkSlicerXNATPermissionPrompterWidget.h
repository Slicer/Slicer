#ifndef __vtkSlicerXNATPermissionPrompterWidget_h
#define __vtkSlicerXNATPermissionPrompterWidget_h

#include "vtkMRMLScene.h"
#include "vtkPermissionPrompter.h"

#include "vtkSlicerPermissionPrompterWidget.h"
#include "vtkKWEntryWithLabel.h"


class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerXNATPermissionPrompterWidget : public vtkSlicerPermissionPrompterWidget
{
 public:
  static vtkSlicerXNATPermissionPrompterWidget *New();
  vtkTypeRevisionMacro ( vtkSlicerXNATPermissionPrompterWidget, vtkSlicerPermissionPrompterWidget );
  void PrintSelf ( ostream& os, vtkIndent indent );
  
  vtkGetObjectMacro ( HostNameEntry, vtkKWEntryWithLabel);
  vtkGetObjectMacro ( SceneNameEntry, vtkKWEntryWithLabel);
  vtkGetObjectMacro ( MRsessionIDEntry, vtkKWEntryWithLabel );

  vtkGetStringMacro ( SceneName );
  vtkSetStringMacro ( SceneName );

  vtkGetStringMacro ( MRsessionID );
  vtkSetStringMacro ( MRsessionID );

  // Description:
  // Convenience method for getting the host name string from widget.
  const char* GetHostFromWidget ( );

  // Description:
  // Convenience method for getting the scene name string from widget.
  const char* GetSceneNameFromWidget( );

  // Description:
  // Convenience method for getting the MRsessionID string from widget.
  const char* GetMRsessionIDFromWidget();
  
  // Description:
  // Creates, configures and packs the widgets in the prompt
  virtual void CreatePrompter( const char *messageText, const char *title);
  // Description:
  // Deletes the widgets in the prompt
  virtual void DestroyPrompter();

  // Description:
  // Raises the dialog and sets MRML values
  // Returns 1 if UserName, Password, and Host have been entered,
  // and 0 if any value is NULL;
  virtual int Prompt( const char *message, vtkMRMLScene *scene);
  
 protected:
  vtkSlicerXNATPermissionPrompterWidget ( );
  virtual ~vtkSlicerXNATPermissionPrompterWidget ( );

  vtkKWEntryWithLabel *HostNameEntry;
  vtkKWEntryWithLabel *SceneNameEntry;
  vtkKWEntryWithLabel *MRsessionIDEntry;
  char *SceneName;
  char *MRsessionID;
 
 private:
  vtkSlicerXNATPermissionPrompterWidget(const vtkSlicerXNATPermissionPrompterWidget&); // Not implemented
  void operator=(const vtkSlicerXNATPermissionPrompterWidget&); // Not Implemented
};
#endif


