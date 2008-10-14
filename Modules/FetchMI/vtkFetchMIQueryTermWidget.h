#ifndef __vtkFetchMIQueryTermWidget_h
#define __vtkFetchMIQueryTermWidget_h


#include "vtkFetchMIWin32Header.h"
#include "vtkFetchMIMulticolumnWidget.h"
#include "vtkFetchMILogic.h"

class vtkKWPushButton;
class vtkFetchMIIcons;

class VTK_FETCHMI_EXPORT vtkFetchMIQueryTermWidget : public vtkFetchMIMulticolumnWidget
{
  
public:
  static vtkFetchMIQueryTermWidget* New();
  vtkTypeRevisionMacro(vtkFetchMIQueryTermWidget,vtkFetchMIMulticolumnWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get methods on class members ( no Set methods required. )
  vtkGetObjectMacro ( AddNewButton, vtkKWPushButton);
  vtkGetObjectMacro ( SelectAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( DeselectAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( ClearAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( ClearSelectedButton, vtkKWPushButton);
  vtkGetObjectMacro ( SearchButton, vtkKWPushButton );
  vtkGetObjectMacro ( FetchMIIcons, vtkFetchMIIcons );
  vtkGetObjectMacro ( Logic, vtkFetchMILogic );
  vtkSetObjectMacro ( Logic, vtkFetchMILogic );
  
  // Description:
  // Method to add a new keyword value pair to the tag list
  virtual void AddNewItem ( const char *keyword, const char *value);

  // Description:
  // returns the row number of that attribute.
  int GetRowForAttribute ( const char *attribute );

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
  // add observers on node
  virtual void AddMRMLObservers ( );

  // Description:
  // remove observers on  node
  virtual void RemoveMRMLObservers ( );

  // Description:
  // Selection methods
  virtual void SelectRow ( int i );
  virtual void SelectAllItems();
  virtual void DeselectAllItems();

  // Description:
  // Methods to operate on selected items.
  virtual int IsItemSelected(int i);
  virtual const char *GetAttributeOfItem (int i );
  virtual const char *GetValueOfItem (int i );
  virtual void DeleteSelectedItems();
  virtual int GetNumberOfSelectedItems();
  virtual const char *GetNthSelectedAttribute(int n);
  virtual const char *GetNthSelectedValue(int n);
  
  //BTX
  enum
    {
      TagChangedEvent = 11000,
    };
  //ETX

 protected:
  vtkFetchMIQueryTermWidget();
  virtual ~vtkFetchMIQueryTermWidget();

  // Custom buttons and icons
  vtkKWPushButton *AddNewButton;
  vtkKWPushButton *SelectAllButton;
  vtkKWPushButton *DeselectAllButton;
  vtkKWPushButton *ClearAllButton;
  vtkKWPushButton *ClearSelectedButton;
  vtkKWPushButton *SearchButton;
  vtkFetchMIIcons *FetchMIIcons;
  vtkFetchMILogic *Logic;

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Update the widget from MRML
  void UpdateWidget();
  
  void UpdateMRML();

  vtkFetchMIQueryTermWidget(const vtkFetchMIQueryTermWidget&); // Not implemented
  void operator=(const vtkFetchMIQueryTermWidget&); // Not Implemented
};

#endif

