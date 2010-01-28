#ifndef __vtkFetchMIResourceUploadWidget_h
#define __vtkFetchMIResourceUploadWidget_h


#include "vtkFetchMIWin32Header.h"
#include "vtkFetchMIMulticolumnWidget.h"
#include "vtkFetchMILogic.h"
#include <string>

class vtkKWPushButtonWithLabel;
class vtkKWPushButton;
class vtkKWMenuButton;
class vtkFetchMIIcons;
class vtkKWLabel;
class vtkKWTopLevel;
class vtkKWEntry;

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
  vtkGetObjectMacro ( UploadButton, vtkKWPushButton );
  vtkGetObjectMacro ( ApplyTagsButton, vtkKWPushButton );
  vtkGetObjectMacro ( RemoveTagsButton, vtkKWPushButton );
  vtkGetObjectMacro ( ShowTagsForAllButton, vtkKWPushButton );
  vtkGetObjectMacro ( FetchMIIcons, vtkFetchMIIcons );
  vtkGetObjectMacro ( Logic, vtkFetchMILogic );
  vtkSetObjectMacro ( Logic, vtkFetchMILogic );
  vtkGetObjectMacro ( CurrentTagLabel, vtkKWLabel );
  vtkGetObjectMacro (TaggingHelpButton, vtkKWPushButton );
  vtkGetObjectMacro (SelectTagMenuButton, vtkKWMenuButton );

  vtkGetObjectMacro (NewTagWindow, vtkKWTopLevel );
  vtkGetObjectMacro (AddNewTagEntry, vtkKWEntry);
  vtkGetObjectMacro (AddNewTagButton, vtkKWPushButton);
  vtkGetObjectMacro (AddNewValueEntry, vtkKWEntry);
  vtkGetObjectMacro (AddNewValueButton, vtkKWPushButton);
  vtkGetObjectMacro (AddNewTagLabel, vtkKWLabel );
  vtkGetObjectMacro (CloseNewTagWindowButton, vtkKWPushButton);

  // Description:
  // Get/Set on a new tag name added by user.
  vtkGetStringMacro (NewUserTag);
  vtkSetStringMacro (NewUserTag);
  // Description:
  // Get/Set on a new value added for a tag by user.
  vtkGetStringMacro (NewUserValue);
  vtkSetStringMacro (NewUserValue);  

  vtkGetStringMacro (TagToAddValuesTo);
  vtkSetStringMacro (TagToAddValuesTo);
  
  
  void RaiseTaggingHelpWindow();
  
  virtual void AddNewItem ( const char *dataset, const char *dtype);
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
  // Selection methods
  virtual void SelectRow ( int i );
  virtual void DeselectRow ( int i );
  virtual void SelectAllItems();
  virtual void DeselectAllItems();

  // Description:
  // Each time the widget is clicked (select or deselect)
  // this method maintains a current vector of strings
  // of selected nodes (and scene).
  virtual void UpdateSelectedStorableNodes();

  // Description:
  // Methods to operate on selected items.
  virtual int IsItemSelected(int i );
  virtual void DeleteSelectedItems();
  virtual int GetNumberOfSelectedItems();
  virtual int GetNumberOfItems();
  virtual const char *GetNthSelectedSlicerDataType(int n);
  virtual const char *GetNthSelectedDataTarget(int n);
  virtual const char *GetNthDataTarget(int n );
  virtual const char *GetNthSlicerDataType(int n);
  void ShowTagViewCallback( );

  void ResetCurrentTagLabel ( );
  // Description:
  // Get/Set on CurrentTagAttribute.
  // CurrentTagAttribute holds the value of the tag's attribute
  // that will be applied to data if the ApplyTagsButton is  selected.
  void SetCurrentTagAttribute(const char* att);
  const char *GetCurrentTagAttribute();
  // Description:
  // Get/Set on CurrentTagValue.
  // CurrentTagValue holds the value of the tag's value
  // that will be applied to data if the ApplyTagsButton is  selected.
  void SetCurrentTagValue(const char* val);
  const char *GetCurrentTagValue ( );
  
  void PopulateTagMenuButtonCallback ();

  void DestroyNewTagWindow();
  void WithdrawNewTagWindow();
  void RaiseNewTagWindow( const char *att );


  // Description:
  // When a user adds a new value for an existing or new tag,
  // The appropriate tag table (for a selected web service) is
  // updated with that keyword/value pair. Also, the values for
  // CurrentTagAttribute and CurrentTagValue on the
  // TaggedDataList are updated.
  void UpdateNewUserTag (const char *att, const char *val);

  void ShowTagsForSelection();
  void SetStatusText(const char *txt);

  //BTX
  enum
    {
      TagSelectedDataEvent = 11000,
      RemoveTagSelectedDataEvent,
      ShowAllTagViewEvent,
      ShowSelectionTagViewEvent,
      UploadRequestedEvent,
    };

  std::string CurrentTagAttribute;
  std::string CurrentTagValue;
  //ETX

  // Description:
  // editing callback for the multicolumn list
  void RightClickListCallback(int row, int col, int x, int y);

  
 protected:
  vtkFetchMIResourceUploadWidget();
  virtual ~vtkFetchMIResourceUploadWidget();

  // custom buttons and icons
  vtkKWPushButton *UploadButton;
  vtkKWPushButton *SelectAllButton;
  vtkKWPushButton *DeselectAllButton;
  vtkKWPushButton *ShowTagsForAllButton;
  vtkKWPushButton *ApplyTagsButton;
  vtkKWPushButton *RemoveTagsButton;
  vtkKWPushButton *TaggingHelpButton;
  vtkKWMenuButton *SelectTagMenuButton;
  vtkKWLabel *CurrentTagLabel;
  vtkFetchMIIcons *FetchMIIcons;
  vtkFetchMILogic *Logic;
  
  //--- Add new Tag pop-up window
  vtkKWTopLevel *NewTagWindow;
  vtkKWEntry *AddNewTagEntry;
  vtkKWLabel *AddNewTagLabel;
  vtkKWPushButton *AddNewTagButton;
  vtkKWEntry *AddNewValueEntry;
  vtkKWPushButton *AddNewValueButton;
  vtkKWPushButton *CloseNewTagWindowButton;

  char *NewUserTag;
  char *NewUserValue;
  char *TagToAddValuesTo;

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Update the widget, used when the color node id changes
  void UpdateWidget();
  
  void UpdateMRML();

  vtkFetchMIResourceUploadWidget(const vtkFetchMIResourceUploadWidget&); // Not implemented
  void operator=(const vtkFetchMIResourceUploadWidget&); // Not Implemented
};

#endif

