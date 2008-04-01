#ifndef __vtkSlicerXNATPermissionPrompterWidget_h
#define __vtkSlicerXNATPermissionPrompterWidget_h

#include "vtkSlicerWidget.h"
#include "vtkSlicerPermissionPrompterWidget.h"
#include "vtkKWEntryWithLabel.h"

//--- MRML storage 
#include "vtkXNATPermissionPrompter.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerXNATPermissionPrompterWidget : public vtkSlicerPermissionPrompterWidget
{
 public:
  static vtkSlicerXNATPermissionPrompterWidget *New();
  vtkTypeRevisionMacro ( vtkSlicerXNATPermissionPrompterWidget, vtkSlicerPermissionPrompterWidget );
  void PrintSelf ( ostream& os, vtkIndent indent );
  
  vtkGetObjectMacro ( HostNameEntry, vtkKWEntryWithLabel);

  // Description:
  // Get the MRML PermissionPrompter
  vtkGetObjectMacro ( XNATPermissionInfo, vtkXNATPermissionPrompter );
  vtkSetObjectMacro ( XNATPermissionInfo, vtkXNATPermissionPrompter );

  // Description:
  // methods that also call the superclass methods.
  void AddWidgetObservers();
  void RemoveWidgetObservers();
  void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData);
  void CreateWidget();
  
 protected:
  vtkSlicerXNATPermissionPrompterWidget ( );
  virtual ~vtkSlicerXNATPermissionPrompterWidget ( );

  vtkKWEntryWithLabel *HostNameEntry;
  vtkXNATPermissionPrompter *XNATPermissionInfo;

 
 private:
  vtkSlicerXNATPermissionPrompterWidget(const vtkSlicerXNATPermissionPrompterWidget&); // Not implemented
  void operator=(const vtkSlicerXNATPermissionPrompterWidget&); // Not Implemented
};
#endif


