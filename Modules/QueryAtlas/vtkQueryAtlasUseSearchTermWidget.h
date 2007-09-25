#ifndef __vtkQueryAtlasUseSearchTermWidget_h
#define __vtkQueryAtlasUseSearchTermWidget_h


#include "vtkQueryAtlasWin32Header.h"
#include "vtkSlicerWidget.h"

class vtkKWPushButton;
class vtkKWMultiColumnListWithScrollbars;
class vtkQueryAtlasIcons;
class vtkKWFrame;


class VTK_QUERYATLAS_EXPORT vtkQueryAtlasUseSearchTermWidget : public vtkSlicerWidget
{
  
public:
  static vtkQueryAtlasUseSearchTermWidget* New();
  vtkTypeRevisionMacro(vtkQueryAtlasUseSearchTermWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get methods on class members ( no Set methods required. )
  vtkGetObjectMacro ( AddNewButton, vtkKWPushButton);
  vtkGetObjectMacro ( SelectAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( DeselectAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( ClearAllButton, vtkKWPushButton);
  vtkGetObjectMacro ( ClearSelectedButton, vtkKWPushButton);
  vtkGetObjectMacro ( ToggleQuotesButton, vtkKWPushButton );
  vtkGetObjectMacro ( MultiColumnList, vtkKWMultiColumnListWithScrollbars );
  vtkGetObjectMacro ( QueryAtlasIcons, vtkQueryAtlasIcons );
  vtkGetObjectMacro ( ContainerFrame, vtkKWFrame );
  vtkGetMacro (NumberOfColumns, int );
  
  // Description:
  // Method that gets all terms in the multicolumn list
  virtual void GetAllSearchTerms ( );
  // Description:
  // Method that gets all terms in the multicolumn list
  virtual void GetSearchTermsToUse ( );

  virtual void SelectAllSearchTerms ( );
  virtual void DeselectAllSearchTerms ( );
  virtual void DeleteAllSearchTerms ( );
  virtual void AddNewSearchTerm ( const char *term);
  virtual void DeleteSelectedSearchTerms ( );
  virtual void ToggleQuotesOnSelectedSearchTerms ( );

  virtual int GetNumberOfSearchTermsToUse( ) {
  return useTerms.size();
  }
  virtual const char *GetNthSearchTermToUse ( int ind ) {
  return useTerms[ind].c_str();
  }
  
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
  vtkQueryAtlasUseSearchTermWidget();
  virtual ~vtkQueryAtlasUseSearchTermWidget();

  vtkKWPushButton *AddNewButton;
  vtkKWPushButton *SelectAllButton;
  vtkKWPushButton *DeselectAllButton;
  vtkKWPushButton *ClearAllButton;
  vtkKWPushButton *ClearSelectedButton;
  vtkKWPushButton *ToggleQuotesButton;
  vtkKWMultiColumnListWithScrollbars *MultiColumnList;
  vtkQueryAtlasIcons *QueryAtlasIcons;
  vtkKWFrame *ContainerFrame;

  //BTX
  std::vector<std::string> useTerms;
  //ETX
  
  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Update the widget, used when the color node id changes
  void UpdateWidget();
  
  void UpdateMRML();

  // Description:
  // Called when the selected row changes, just update the label, called from UpdateWidget
  void UpdateSelectedColor();
  
  //BTX
  // Description:
  // The column orders in the list box
  enum
    {
      SearchTermColumn = 0,
    };
  //ETX

  int NumberOfColumns;

  vtkQueryAtlasUseSearchTermWidget(const vtkQueryAtlasUseSearchTermWidget&); // Not implemented
  void operator=(const vtkQueryAtlasUseSearchTermWidget&); // Not Implemented
};

#endif

