#ifndef __vtkFetchMIMulticolumnWidget_h
#define __vtkFetchMIMulticolumnWidget_h

#include "vtkFetchMIWin32Header.h"
#include "vtkSlicerWidget.h"

class vtkKWMultiColumnListWithScrollbars;
class vtkKWFrame;

class VTK_FETCHMI_EXPORT vtkFetchMIMulticolumnWidget : public vtkSlicerWidget
{
  
public:
  static vtkFetchMIMulticolumnWidget* New();
  vtkTypeRevisionMacro(vtkFetchMIMulticolumnWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get methods on class members ( no Set methods required. )
  vtkGetObjectMacro ( MultiColumnList, vtkKWMultiColumnListWithScrollbars );
  vtkGetObjectMacro ( ContainerFrame, vtkKWFrame );
  vtkGetMacro (NumberOfColumns, int );
  
  virtual void AddNewItem () { };
  virtual void SelectAllItems ( );
  virtual void DeselectAllItems ( );
  virtual void DeleteAllItems ( );
  virtual void DeleteSelectedItems ( );
  
  // Description:
  // Method that gets all terms in the multicolumn list
  virtual void GetAllItems ( );

  // Description:
  // Method that gets all terms in the multicolumn list
  virtual void GetItemsToUse ( );

  virtual int GetNumberOfItemsToUse( ) {
    return (int)useTerms.size();
  }
  virtual const char *GetNthItemToUse ( int ind ) {
  return useTerms[ind].c_str();
  }
  
  //BTX
  std::vector<std::string> useTerms;
  //ETX

 protected:
  vtkFetchMIMulticolumnWidget();
  virtual ~vtkFetchMIMulticolumnWidget();

  vtkKWMultiColumnListWithScrollbars *MultiColumnList;
  vtkKWFrame *ContainerFrame;
  
  // Description:
  // Create the widget.
  virtual void CreateWidget();
  
  //BTX
  // Description:
  // The column orders in the list box
  enum
    {
      MulticolumnColumn = 0,
    };
  //ETX

  int NumberOfColumns;

  vtkFetchMIMulticolumnWidget(const vtkFetchMIMulticolumnWidget&); // Not implemented
  void operator=(const vtkFetchMIMulticolumnWidget&); // Not Implemented
};

#endif

