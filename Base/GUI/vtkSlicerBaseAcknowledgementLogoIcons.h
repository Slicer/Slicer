#ifndef __vtkSlicerBaseAcknowledgementLogoIcons_h
#define __vtkSlicerBaseAcknowledgementLogoIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerBaseAcknowledgementLogos_ImageData.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerBaseAcknowledgementLogoIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerBaseAcknowledgementLogoIcons* New ( );
    vtkTypeRevisionMacro ( vtkSlicerBaseAcknowledgementLogoIcons, vtkSlicerIcons);
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro ( NAMICLogo, vtkKWIcon);
    vtkGetObjectMacro ( NCIGTLogo, vtkKWIcon);
    vtkGetObjectMacro ( NACLogo, vtkKWIcon);
    vtkGetObjectMacro ( BIRNLogo, vtkKWIcon);

    void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerBaseAcknowledgementLogoIcons ( );
    ~vtkSlicerBaseAcknowledgementLogoIcons ( );
    vtkKWIcon *NAMICLogo;
    vtkKWIcon *NCIGTLogo;
    vtkKWIcon *NACLogo;
    vtkKWIcon *BIRNLogo;    
    
 private:
    vtkSlicerBaseAcknowledgementLogoIcons (const vtkSlicerBaseAcknowledgementLogoIcons&); // Not implemented
    void operator = ( const vtkSlicerBaseAcknowledgementLogoIcons& ); // Not implemented
    
};

#endif
