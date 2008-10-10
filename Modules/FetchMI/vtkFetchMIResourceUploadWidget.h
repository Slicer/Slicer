#ifndef __vtkFetchMIResourceUploadWidget_h
#define __vtkFetchMIResourceUploadWidget_h


#include "vtkFetchMIWin32Header.h"
#include "vtkFetchMIMulticolumnWidget.h"

class vtkKWPushButton;
class vtkFetchMIIcons;

class VTK_FETCHMI_EXPORT vtkFetchMIResourceUploadWidget : public vtkFetchMIMulticolumnWidget
{
  
public:
  static vtkFetchMIResourceUploadWidget* New();
  vtkTypeRevisionMacro(vtkFetchMIResourceUploadWidget,vtkFetchMIMulticolumnWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get methods on class members ( no Set methods required. )
  vtkGetObjectMacro ( SelectAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( DeselectAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( UploadSelectedButton, vtkKWPushButton );
  vtkGetObjectMacro ( ApplyTagsButton, vtkKWPushButton );
  vtkGetObjectMacro ( ShowTagsButton, vtkKWPushButton );
  vtkGetObjectMacro ( FetchMIIcons, vtkFetchMIIcons );
  
  virtual void AddNewItem ( const char *dataset );
  
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
  // Selection methods
  virtual void SelectRow ( int i );
  virtual void SelectAllItems();
  virtual void DeselectAllItems();

  // Description:
  // Methods to operate on selected items.
  virtual int IsItemSelected(int i );
  virtual void DeleteSelectedItems();
  virtual int GetNumberOfSelectedItems();
  virtual const char *GetNthSelectedSlicerDataType(int n);
  virtual const char *GetNthSelectedDataTarget(int n);
  
 protected:
  vtkFetchMIResourceUploadWidget();
  virtual ~vtkFetchMIResourceUploadWidget();

  // custom buttons and icons
  vtkKWPushButton *UploadSelectedButton;
  vtkKWPushButton *SelectAllButton;
  vtkKWPushButton *DeselectAllButton;
  vtkKWPushButton *ShowTagsButton;
  vtkKWPushButton *ApplyTagsButton;
  vtkFetchMIIcons *FetchMIIcons;
  
  // Description:
  // Create the widget.
  virtual void CreateWidget();

  void ShowTagViewCallback();

  // Description:
  // Update the widget, used when the color node id changes
  void UpdateWidget();
  
  void UpdateMRML();

  vtkFetchMIResourceUploadWidget(const vtkFetchMIResourceUploadWidget&); // Not implemented
  void operator=(const vtkFetchMIResourceUploadWidget&); // Not Implemented
};

#endif

