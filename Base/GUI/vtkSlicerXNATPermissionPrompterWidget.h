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
  vtkGetObjectMacro ( ResourceSetNameEntry, vtkKWEntryWithLabel );
  vtkGetObjectMacro ( ResourceTagEntry, vtkKWEntryWithLabel);

  vtkGetStringMacro ( ResourceSetName);
  vtkSetStringMacro ( ResourceSetName);  

  vtkGetStringMacro ( ResourceTag );
  vtkSetStringMacro ( ResourceTag );
  
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
  // Convenience method for getting the name of the Resource Set from widget.
  const char* GetResourceSetNameFromWidget ( );

  // Description:
  // Convenience method for getting the Resource Tags from widget.
  const char* GetResourceTagFromWidget ( );
  
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
  vtkKWEntryWithLabel *ResourceSetNameEntry;
  vtkKWEntryWithLabel *ResourceTagEntry;
  char *SceneName;
  char *MRsessionID;
  char *ResourceSetName;
  char *ResourceTag;
 
 private:
  vtkSlicerXNATPermissionPrompterWidget(const vtkSlicerXNATPermissionPrompterWidget&); // Not implemented
  void operator=(const vtkSlicerXNATPermissionPrompterWidget&); // Not Implemented
};
#endif


