#ifndef __vtkFetchMITagViewWidget_h
#define __vtkFetchMITagViewWidget_h

#include "vtkFetchMIWin32Header.h"
#include "vtkKWTopLevel.h"
#include "vtkKWText.h"
#include "vtkKWTextWithScrollbars.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"

#include "vtkSlicerWidget.h"

class VTK_FETCHMI_EXPORT vtkFetchMITagViewWidget : public vtkSlicerWidget
{
  
public:
  static vtkFetchMITagViewWidget* New();
  vtkTypeRevisionMacro(vtkFetchMITagViewWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Getting the window in which to display help text.
  vtkGetObjectMacro ( TagViewWindow, vtkKWTopLevel );
  vtkGetObjectMacro ( CloseButton, vtkKWPushButton );
  vtkGetObjectMacro ( TextBox, vtkKWTextWithScrollbars );  
  vtkGetObjectMacro ( TitleLabel,  vtkKWLabel);

// Description:
  // Use this method to set the help or information text.
  // Only simple formatting is now possible, for instance:
  // **bold** and
  //__underline__ and
  //~~italic~~
  // and that's about it.
  void SetTagText ( const char *text );

  // Description:
  // Use this method to set the text to appear in the title
  // of the help window. Best to use something brief, less
  // than 30 chars, for instance:
  void SetTagTitle ( const char *title );



// Description:
  // Internal Callbacks. do not use.
  void DisplayTagViewWindow ( );
  void DestroyTagViewWindow ( );
  
  void Bind ( );
  void UnBind ( );

 protected:
  vtkFetchMITagViewWidget();
  virtual ~vtkFetchMITagViewWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();


  vtkKWTopLevel *TagViewWindow;
  vtkKWPushButton *CloseButton;
  vtkKWLabel *TitleLabel;
  vtkKWTextWithScrollbars *TextBox;
  
private:
  vtkFetchMITagViewWidget(const vtkFetchMITagViewWidget&); // Not implemented
  void operator=(const vtkFetchMITagViewWidget&); // Not Implemented
};

#endif

