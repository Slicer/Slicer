#ifndef __vtkSlicerHelpIcons_h
#define __vtkSlicerHelpIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerHelpAndInformation_ImageData.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerHelpIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerHelpIcons* New ( );
    vtkTypeRevisionMacro ( vtkSlicerHelpIcons, vtkSlicerIcons);
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro ( HelpAndInformationIcon, vtkKWIcon);

    void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerHelpIcons ( );
    ~vtkSlicerHelpIcons ( );
    vtkKWIcon *HelpAndInformationIcon;
    
 private:
    vtkSlicerHelpIcons (const vtkSlicerHelpIcons&); // Not implemented
    void operator = ( const vtkSlicerHelpIcons& ); // Not implemented
    
};

#endif
