#ifndef __vtkFetchMIFlatResourceWidget_h
#define __vtkFetchMIFlatResourceWidget_h


#include "vtkFetchMIWin32Header.h"
#include "vtkFetchMIMulticolumnWidget.h"
#include "vtkFetchMILogic.h"
#include "vtkSlicerFoundationIcons.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerWindow.h"
class vtkKWPushButton;
class vtkFetchMIIcons;

class VTK_FETCHMI_EXPORT vtkFetchMIFlatResourceWidget : public vtkFetchMIMulticolumnWidget
{
  
public:
  static vtkFetchMIFlatResourceWidget* New();
  vtkTypeRevisionMacro(vtkFetchMIFlatResourceWidget,vtkFetchMIMulticolumnWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get methods on class members ( no Set methods required. )
  vtkGetObjectMacro ( SelectAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( DeselectAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( HelpButton, vtkKWPushButton );
  vtkGetObjectMacro ( ClearAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( ClearSelectedButton, vtkKWPushButton);
  vtkGetObjectMacro ( DownloadSelectedButton, vtkKWPushButton );
  vtkGetObjectMacro ( DeleteButton, vtkKWPushButton );
  vtkGetObjectMacro ( FetchMIIcons, vtkFetchMIIcons );
  vtkGetObjectMacro ( Logic, vtkFetchMILogic );
  vtkSetObjectMacro ( Logic, vtkFetchMILogic );


  // Description:
  // Method to add a new resource to the resource list.
  virtual void AddNewItem ( const char *uri, const char *dtype);
  //BTX
  using vtkFetchMIMulticolumnWidget::AddNewItem; 
  //ETX
  
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


// Description:
  // Display a window that has tips on using the interface.
  virtual void RaiseHelpWindow();

  // Description:
  // Convenience method that prevents other
  // listeners from acting on this event once
  // its callback has been dispatched.
  virtual void SwallowGUIEvent();
  
  // Description:
  // Selection methods
  virtual void SelectAllItems();
  virtual void DeselectAllItems();
  virtual void SelectRow ( int i );
  
  // Description:
  // Methods to operate on selected items.
  virtual int IsItemSelected(int i );
  virtual void DeleteSelectedItems();
  virtual int GetNumberOfSelectedItems();
  virtual const char *GetNthSelectedSlicerDataType(int n);
  virtual const char *GetNthSelectedURI(int n);
  virtual int GetRowForURI(const char *uri );
  virtual void SetStatusText (const char *txt);

  // Description:
  // editing callback for the multicolumn list
  void RightClickListCallback(int row, int col, int x, int y);

  //BTX
  enum
    {
      DeleteResourceEvent = 11000,
    };
  //ETX
  
 protected:
  vtkFetchMIFlatResourceWidget();
  virtual ~vtkFetchMIFlatResourceWidget();

  // Custom buttons and icons
  vtkKWPushButton *DownloadSelectedButton;
  vtkKWPushButton *DeleteButton;
  vtkKWPushButton *SelectAllButton;
  vtkKWPushButton *DeselectAllButton;
  vtkKWPushButton *HelpButton;
  vtkKWPushButton *ClearAllButton;
  vtkKWPushButton *ClearSelectedButton;
  vtkFetchMIIcons *FetchMIIcons;
  vtkFetchMILogic *Logic;

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Update the widget, used when the color node id changes
  void UpdateWidget();
  
  void UpdateMRML();


  vtkFetchMIFlatResourceWidget(const vtkFetchMIFlatResourceWidget&); // Not implemented
  void operator=(const vtkFetchMIFlatResourceWidget&); // Not Implemented
};

#endif

