#ifndef __vtkFetchMIQueryTermWidget_h
#define __vtkFetchMIQueryTermWidget_h


#include "vtkFetchMIWin32Header.h"
#include "vtkFetchMIMulticolumnWidget.h"
#include "vtkFetchMILogic.h"
#include <string>
#include <vector>
#include <map>
#include <iterator>

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
  vtkGetObjectMacro ( SelectAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( DeselectAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( HelpButton, vtkKWPushButton );
  vtkGetObjectMacro ( ClearAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( ClearSelectedButton, vtkKWPushButton);
  vtkGetObjectMacro ( SearchButton, vtkKWPushButton );
  vtkGetObjectMacro ( RefreshButton, vtkKWPushButton );
  vtkGetObjectMacro ( FetchMIIcons, vtkFetchMIIcons );
  vtkGetObjectMacro ( Logic, vtkFetchMILogic );
  vtkSetObjectMacro ( Logic, vtkFetchMILogic );
  vtkGetMacro ( InPopulateWidget, int );
  vtkSetMacro ( InPopulateWidget, int );
  
  // Description:
  // not used. New method AddNewTagForQuery is used instead.
  virtual void AddNewItem ( const char *keyword, const char *value);
  //BTX
  using vtkFetchMIMulticolumnWidget::AddNewItem; 
  //ETX
  
  //BTX
  // Description:
  // Method to add a new keyword and list of possible values to the tag list
  virtual void AddNewTagForQuery ( const char *keyword,
                            std::vector<std::string> values );
  //ETX

  // Description:
  // Method to add all metadata known by the selected webservice in one shot.
  virtual void PopulateFromServer ( );

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
  virtual void SetStatusText(const char *txt);

  // Description:
  // Methods to operate on selected items.
  virtual int IsItemSelected(int i);
  virtual const char *GetAttributeOfItem (int i );
  virtual const char *GetValueOfItem (int i );
  virtual void DeleteSelectedItems();
  virtual int GetNumberOfSelectedItems();
  virtual const char *GetNthSelectedAttribute(int n);
  virtual const char *GetNthSelectedValue(int n);
  virtual void SelectValueOfItem(int i, const char *val );

  //BTX
  enum
    {
      TagChangedEvent = 11000,
      QuerySubmittedEvent,
    };
  //ETX

  // Description:
  // editing callback for the multicolumn list
  void RightClickListCallback(int row, int col, int x, int y);
  
 protected:
  vtkFetchMIQueryTermWidget();
  virtual ~vtkFetchMIQueryTermWidget();

  // Custom buttons and icons
  vtkKWPushButton *SelectAllButton;
  vtkKWPushButton *DeselectAllButton;
  vtkKWPushButton *ClearAllButton;
  vtkKWPushButton *ClearSelectedButton;
  vtkKWPushButton *SearchButton;
  vtkKWPushButton *HelpButton;
  vtkKWPushButton *RefreshButton;
  vtkFetchMIIcons *FetchMIIcons;
  vtkFetchMILogic *Logic;
  int InPopulateWidget;

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Display a window that has tips on using the interface.
  virtual void RaiseHelpWindow();
  
  // Description:
  // Update the widget from MRML
  void UpdateWidget();
  
  void UpdateMRML();

  vtkFetchMIQueryTermWidget(const vtkFetchMIQueryTermWidget&); // Not implemented
  void operator=(const vtkFetchMIQueryTermWidget&); // Not Implemented
};

#endif

