#ifndef __vtkSlicerModuleNavigator_h
#define __vtkSlicerModuleNavigator_h

#include "vtkObject.h"
#include "vtkSlicerBaseGUIWin32Header.h"

struct  ModuleNameEntry
{
  struct ModuleNameEntry *Next;
  struct ModuleNameEntry *Prev;
  char *ModuleName;

  ModuleNameEntry ( const char *str )
  {
    Next = NULL;
    Prev = NULL;
    ModuleName = new char [ strlen (str) + 1 ];
    strcpy ( ModuleName, str );
  }
  ~ModuleNameEntry ( )
  {
    Next = NULL;
    Prev = NULL;
    if ( ModuleName != NULL )
      {
      delete [] ModuleName;
      }
  }
};


class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerModuleNavigator : public vtkObject
{
 public:
    // Description:
    // Usual vtk functions
    static vtkSlicerModuleNavigator* New ();
    vtkTypeRevisionMacro ( vtkSlicerModuleNavigator, vtkObject);
    void PrintSelf ( ostream& os, vtkIndent indent );

    // Description:
    // Returns name of first module in the module navigation list
    virtual char *GetFirstModuleName ( );
    // Description:
    // Returns name of the next module in the module navigation list
    virtual char *GetNextModuleName ( );
    // Description:
    // Returns name of the previous module in the module navigation list
    virtual char *GetPreviousModuleName ( );
    // Description:
    // Returns name of the current module in the module navigation list
    virtual char *GetCurrentModuleName ( );
    // Description:
    // Returns name of the last module in the module navigation list
    virtual char *GetLastModuleName ( );
    // Description:
    // Adds a module's name to the module navigation list for fwd and back
    virtual void AddModuleNameToNavigationList ( const char *name );
    // Description:
    // Adds a module's name to the module history list
    virtual void AddModuleNameToHistoryList ( const char *name );
    // Description:
    // Makes the previous module the current module and returns its name
    virtual char * NavigateBack ( );
    // Description:
    // Makes the next module the current module and returns its name
    virtual char * NavigateForward ( );

    virtual void ShowHistory ( );
    virtual void InitializeLists ( );
    virtual void DeleteLists ( );
    // Description:
    // Returns the list containing the modules to navigate forward and back
    ModuleNameEntry *GetModuleNavigationList ( );
    // Description:
    // Returns the list of modules visited.
    ModuleNameEntry *GetModuleHistoryList  ( );
    
 protected:
    vtkSlicerModuleNavigator ( );
    virtual ~vtkSlicerModuleNavigator ( );

    // Description:
    // Lists all modules visited by user.
    ModuleNameEntry *ModuleHistoryList;

    // Description:
    // Keeps track of the list of modules visited
    // for forward/backward navigation, like a
    // web browser.
    ModuleNameEntry *ModuleNavigationList;
    ModuleNameEntry *LastModule;
    ModuleNameEntry *CurrentModule;
    ModuleNameEntry *NextModule;
    ModuleNameEntry *PreviousModule;
    char *HomeModule;
    
 private:
    vtkSlicerModuleNavigator (const vtkSlicerModuleNavigator& ); // Not implemented
    void operator = (const vtkSlicerModuleNavigator& ); // Not implemented
    
};


#endif
