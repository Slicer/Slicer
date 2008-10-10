#ifndef __vtkFetchMITagViewWidget_h
#define __vtkFetchMITagViewWidget_h

#include "vtkFetchMIWin32Header.h"
#include "vtkKWTopLevel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
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
  vtkGetObjectMacro ( NodeLabel, vtkKWLabel );
  vtkGetObjectMacro ( TagTable, vtkKWMultiColumnListWithScrollbars );  

  // Description:
  // Internal Callbacks. do not use.
  void DisplayTagViewWindow ( );
  void DestroyTagViewWindow ( );
  
 protected:
  vtkFetchMITagViewWidget();
  virtual ~vtkFetchMITagViewWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  void Bind ( );
  void UnBind ( );

  vtkKWTopLevel *TagViewWindow;
  vtkKWPushButton *CloseButton;
  vtkKWLabel *NodeLabel;
  vtkKWMultiColumnListWithScrollbars *TagTable;
  
private:
  vtkFetchMITagViewWidget(const vtkFetchMITagViewWidget&); // Not implemented
  void operator=(const vtkFetchMITagViewWidget&); // Not Implemented
};

#endif

