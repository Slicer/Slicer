#ifndef __vtkSlicerPermissionPrompterWidget_h
#define __vtkSlicerPermissionPrompterWidget_h

#include "vtkKWFrame.h"
#include "vtkKWEntry.h"
#include "vtkKWLabel.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWMessageDialog.h"
#include "vtkPermissionPrompter.h"
#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerApplication.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerPermissionPrompterWidget : public vtkPermissionPrompter
{
 public:
  static vtkSlicerPermissionPrompterWidget *New();
  vtkTypeRevisionMacro ( vtkSlicerPermissionPrompterWidget, vtkPermissionPrompter );
  void PrintSelf ( ostream& os, vtkIndent indent );

  vtkGetObjectMacro ( PromptDialog, vtkKWMessageDialog );

  // Description:
  // Get the value of the remember check button
  vtkGetObjectMacro ( RememberCheck, vtkKWCheckButtonWithLabel );
  // Description:
  // Get the logo label
  vtkGetObjectMacro ( LogoLabel, vtkKWLabel );
  // Description:
  // Get the icon displayed on the logo label
  vtkGetObjectMacro ( LogoIcon, vtkKWIcon );
  // Description:
  // Get the entry widget for the User name
  vtkGetObjectMacro ( UserNameEntry, vtkKWEntryWithLabel );
  // Description:
  // Get the entry widget for the User password
  vtkGetObjectMacro ( PasswordEntry, vtkKWEntryWithLabel );

    
  // Description:
  // Convenience method to get the user name from the widget
  const char *GetUserFromWidget();
  // Description:
  // Convenience method to get the password from the widget
  const char *GetPasswordFromWidget();
  // Description:
  // Convenience method to get the user's preference for
  // remembering login information. 0 means don't remember,
  // 1 means do remember input values.
  int GetRememberStatusFromWidget();

  // Description:
  // Convenience method: assigns image data to  the Logo icon
  void SetLogoIcon( vtkKWIcon *icon);

  // Description:
  // Creates, configures and packs the widgets in the prompt.
  virtual void CreatePrompter( const char *messageText, const char *title);
  // Description:
  // Deletes widgets in the prompt.
  virtual void DestroyPrompter();

  // Description:
  // Raises the dialog and sets MRML values
  // Returns 1 if UserName and Password have been entered,
  // and 0 if either value is NULL;
  virtual int Prompt( const char *message );

  vtkGetObjectMacro ( Application, vtkSlicerApplication );
  vtkSetObjectMacro ( Application, vtkSlicerApplication );
  
 protected:
  vtkSlicerPermissionPrompterWidget ( );
  virtual ~vtkSlicerPermissionPrompterWidget ( );
  
  vtkKWMessageDialog *PromptDialog;
  vtkKWCheckButtonWithLabel *RememberCheck;
  vtkKWLabel *LogoLabel;
  vtkKWIcon *LogoIcon;
  vtkKWEntryWithLabel *UserNameEntry;
  vtkKWEntryWithLabel *PasswordEntry;
  vtkSlicerApplication *Application;

 private:
  vtkSlicerPermissionPrompterWidget(const vtkSlicerPermissionPrompterWidget&); // Not implemented
  void operator=(const vtkSlicerPermissionPrompterWidget&); // Not Implemented
};
#endif


