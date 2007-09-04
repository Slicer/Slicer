#ifndef __vtkQueryAtlasCollaboratorIcons_h
#define __vtkQueryAtlasCollaboratorIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkQueryAtlasCollaborators_ImageData.h"

class vtkQueryAtlasCollaboratorIcons : public vtkSlicerIcons
{
 public:
    static vtkQueryAtlasCollaboratorIcons* New ( );
    vtkTypeRevisionMacro ( vtkQueryAtlasCollaboratorIcons, vtkSlicerIcons);
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro ( BrainInfoLogo, vtkKWIcon);

    void AssignImageDataToIcons ( );
    
 protected:
    vtkQueryAtlasCollaboratorIcons ( );
    ~vtkQueryAtlasCollaboratorIcons ( );

    vtkKWIcon *BrainInfoLogo;
    
 private:
    vtkQueryAtlasCollaboratorIcons (const vtkQueryAtlasCollaboratorIcons&); // Not implemented
    void operator = ( const vtkQueryAtlasCollaboratorIcons& ); // Not implemented
};

#endif
