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
    /// 
    /// Usual vtk functions
    static vtkSlicerModuleNavigator* New ();
    vtkTypeRevisionMacro ( vtkSlicerModuleNavigator, vtkObject);
    void PrintSelf ( ostream& os, vtkIndent indent );

    /// 
    /// Returns name of first module in the module navigation list
    virtual char *GetFirstModuleName ( );
    /// 
    /// Returns name of the next module in the module navigation list
    virtual char *GetNextModuleName ( );
    /// 
    /// Returns name of the previous module in the module navigation list
    virtual char *GetPreviousModuleName ( );
    /// 
    /// Returns name of the current module in the module navigation list
    virtual char *GetCurrentModuleName ( );
    /// 
    /// Returns name of the last module in the module navigation list
    virtual char *GetLastModuleName ( );
    /// 
    /// Adds a module's name to the module navigation list for fwd and back
    virtual void AddModuleNameToNavigationList ( const char *name );
    /// 
    /// Adds a module's name to the module history list
    virtual void AddModuleNameToHistoryList ( const char *name );
    /// 
    /// Makes the previous module the current module and returns its name
    virtual char * NavigateBack ( );
    /// 
    /// Makes the next module the current module and returns its name
    virtual char * NavigateForward ( );

    virtual void ShowHistory ( );
    virtual void InitializeLists ( );
    virtual void DeleteLists ( );
    /// 
    /// Returns the list containing the modules to navigate forward and back
    ModuleNameEntry *GetModuleNavigationList ( );
    /// 
    /// Returns the list of modules visited.
    ModuleNameEntry *GetModuleHistoryList  ( );
    
 protected:
    vtkSlicerModuleNavigator ( );
    virtual ~vtkSlicerModuleNavigator ( );

    /// 
    /// Lists all modules visited by user.
    ModuleNameEntry *ModuleHistoryList;

    /// 
    /// Keeps track of the list of modules visited
    /// for forward/backward navigation, like a
    /// web browser.
    ModuleNameEntry *ModuleNavigationList;
    ModuleNameEntry *LastModule;
    ModuleNameEntry *CurrentModule;
    ModuleNameEntry *NextModule;
    ModuleNameEntry *PreviousModule;
    char *HomeModule;
    
 private:
    vtkSlicerModuleNavigator (const vtkSlicerModuleNavigator& ); /// Not implemented
    void operator = (const vtkSlicerModuleNavigator& ); /// Not implemented
    
};


#endif
