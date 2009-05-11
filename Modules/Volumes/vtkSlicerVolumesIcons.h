#ifndef __vtkSlicerVolumesIcons_h
#define __vtkSlicerVolumesIcons_h

#include "vtkVolumes.h"
#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"

#include "./Resources/vtkKWWindowLevelThresholdEditor_ImageData.h"

class VTK_VOLUMES_EXPORT vtkSlicerVolumesIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerVolumesIcons* New ( );
    vtkTypeRevisionMacro ( vtkSlicerVolumesIcons, vtkSlicerIcons );
    void PrintSelf ( ostream& os, vtkIndent indent );

    // Description:
    // Get toolbar vtkKWIcons
    vtkGetObjectMacro (WindowLevelPresetCTAirIcon, vtkKWIcon);
    vtkGetObjectMacro (WindowLevelPresetCTBoneIcon, vtkKWIcon);
    vtkGetObjectMacro (WindowLevelPresetPETIcon, vtkKWIcon);
    virtual void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerVolumesIcons ( );
    virtual ~vtkSlicerVolumesIcons ( );
    
    vtkKWIcon *WindowLevelPresetCTAirIcon;
    vtkKWIcon *WindowLevelPresetCTBoneIcon;
    vtkKWIcon *WindowLevelPresetPETIcon;
 private:
    vtkSlicerVolumesIcons (const vtkSlicerVolumesIcons&); // Not implemented
    void operator = ( const vtkSlicerVolumesIcons& ); // Not implemented
    
};

#endif
