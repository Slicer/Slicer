#ifndef __vtkFetchMIDescribeDataWidget_h
#define __vtkFetchMIDescribeDataWidget_h


#include "vtkFetchMIWin32Header.h"
#include "vtkFetchMIMulticolumnWidget.h"

class vtkKWPushButton;
class vtkFetchMIIcons;

class VTK_FETCHMI_EXPORT vtkFetchMIDescribeDataWidget : public vtkFetchMIMulticolumnWidget
{
  
public:
  static vtkFetchMIDescribeDataWidget* New();
  vtkTypeRevisionMacro(vtkFetchMIDescribeDataWidget,vtkFetchMIMulticolumnWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get methods on class members ( no Set methods required. )
  vtkGetObjectMacro ( AddNewButton, vtkKWPushButton);
  vtkGetObjectMacro ( CopyTagsButton, vtkKWPushButton );
  vtkGetObjectMacro ( ShowTagsButton, vtkKWPushButton );
  vtkGetObjectMacro ( SelectAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( DeselectAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( ClearAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( ClearSelectedButton, vtkKWPushButton);
  vtkGetObjectMacro ( FetchMIIcons, vtkFetchMIIcons );
  
  virtual void AddNewItem ( const char *keyword, const char *value);

  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );
  // Description:
  // adds observers on widgets in the class
  virtual void AddWidgetObservers ( );

  // Description:
  // add observers on color node
  virtual void AddMRMLObservers ( );

  // Description:
  // remove observers on color node
  virtual void RemoveMRMLObservers ( );

  
 protected:
  vtkFetchMIDescribeDataWidget();
  virtual ~vtkFetchMIDescribeDataWidget();

  // custom buttons and icons
  vtkKWPushButton *AddNewButton;
  vtkKWPushButton *CopyTagsButton;
  vtkKWPushButton *ShowTagsButton;  
  vtkKWPushButton *SelectAllButton;
  vtkKWPushButton *DeselectAllButton;
  vtkKWPushButton *ClearAllButton;
  vtkKWPushButton *ClearSelectedButton;
  vtkFetchMIIcons *FetchMIIcons;

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Update the widget, used when the color node id changes
  void UpdateWidget();
  
  void UpdateMRML();

  vtkFetchMIDescribeDataWidget(const vtkFetchMIDescribeDataWidget&); // Not implemented
  void operator=(const vtkFetchMIDescribeDataWidget&); // Not Implemented
};

#endif

