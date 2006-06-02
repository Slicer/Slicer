#ifndef __vtkSlicerIcons_h
#define __vtkSlicerIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"

class vtkSlicerIcons : public vtkKWObject
{
 public:
    static vtkSlicerIcons* New ( );
    vtkTypeRevisionMacro ( vtkSlicerIcons, vtkKWObject);
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetStringMacro ( SlicerImagePath );

    virtual void DeleteIcon ( vtkKWIcon *icon );
 protected:
    vtkSlicerIcons ( );
    ~vtkSlicerIcons ( );

    char *SlicerImagePath;
    
 private:
    vtkSlicerIcons (const vtkSlicerIcons& ); // Not implemented
    void operator = ( const vtkSlicerIcons& ); // Not implemented
    
};

#endif
