#ifndef __vtkSlicerIcons_h
#define __vtkSlicerIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"

#include <map>
#include <string>

#include "vtkSlicerBaseGUI.h"

/// Description:
/// This class is defined in anticipation of superclass methods that
/// all Slicer Icons will use...

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerIcons : public vtkKWObject
{
 public:
    static vtkSlicerIcons* New ( );
    vtkTypeMacro ( vtkSlicerIcons, vtkKWObject);
    void PrintSelf ( ostream& os, vtkIndent indent );
    virtual vtkKWIcon *GetIconByName( const char *name) { return NULL; };
    virtual void SetIconName ( vtkKWIcon *icon, const char *name ) { };

 protected:
//BTX
    std::map<std::string, vtkKWIcon*> NamedIcons;
//ETX
    vtkSlicerIcons ( );
    virtual ~vtkSlicerIcons ( );
    vtkSlicerIcons (const vtkSlicerIcons& ); /// Not implemented
    void operator = ( const vtkSlicerIcons& ); /// Not implemented
    
 private:
    
};

#endif
