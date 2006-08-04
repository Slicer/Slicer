#ifndef __vtkSlicerToolbarIcons_h
#define __vtkSlicerToolbarIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"

#include "./Resources/vtkSlicerLoadSaveToolbar_ImageData.h"
#include "./Resources/vtkSlicerMouseModeToolbar_ImageData.h"
#include "./Resources/vtkSlicerViewToolbar_ImageData.h"
#include "./Resources/vtkSlicerModuleToolbar_ImageData.h"


class vtkSlicerToolbarIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerToolbarIcons* New ( );
    vtkTypeRevisionMacro ( vtkSlicerToolbarIcons, vtkSlicerIcons );
    void PrintSelf ( ostream& os, vtkIndent indent );

    // Description:
    // Get toolbar vtkKWIcons
    vtkGetObjectMacro (HomeIcon, vtkKWIcon);
    vtkGetObjectMacro (DataIcon, vtkKWIcon);
    vtkGetObjectMacro (VolumeIcon, vtkKWIcon);
    vtkGetObjectMacro (ModelIcon, vtkKWIcon);
    vtkGetObjectMacro (EditorIcon, vtkKWIcon);
    vtkGetObjectMacro (EditorToolboxIcon, vtkKWIcon);
    vtkGetObjectMacro (TransformIcon, vtkKWIcon);
    vtkGetObjectMacro (ColorIcon, vtkKWIcon);
    vtkGetObjectMacro (FiducialsIcon, vtkKWIcon);
    vtkGetObjectMacro (MeasurementsIcon, vtkKWIcon );
    vtkGetObjectMacro (SaveSceneIcon, vtkKWIcon);
    vtkGetObjectMacro (LoadSceneIcon, vtkKWIcon);
    vtkGetObjectMacro (ConventionalViewIcon, vtkKWIcon);
    vtkGetObjectMacro (OneUp3DViewIcon, vtkKWIcon);
    vtkGetObjectMacro (OneUpSliceViewIcon, vtkKWIcon );
    vtkGetObjectMacro (FourUpViewIcon, vtkKWIcon);
    vtkGetObjectMacro (TabbedSliceViewIcon, vtkKWIcon);
    vtkGetObjectMacro (Tabbed3DViewIcon, vtkKWIcon);
    vtkGetObjectMacro (LightBoxViewIcon, vtkKWIcon);
    vtkGetObjectMacro (MousePickIcon, vtkKWIcon );
    vtkGetObjectMacro (MousePanIcon, vtkKWIcon );
    vtkGetObjectMacro (MouseRotateIcon, vtkKWIcon );    
    vtkGetObjectMacro (MouseZoomIcon, vtkKWIcon );
    vtkGetObjectMacro (MousePlaceFiducialIcon, vtkKWIcon );

    virtual void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerToolbarIcons ( );
    ~vtkSlicerToolbarIcons ( );
    
    vtkKWIcon *HomeIcon;
    vtkKWIcon *DataIcon;
    vtkKWIcon *VolumeIcon;
    vtkKWIcon *ModelIcon;
    vtkKWIcon *EditorIcon;
    vtkKWIcon *EditorToolboxIcon;
    vtkKWIcon *TransformIcon;
    vtkKWIcon *ColorIcon;
    vtkKWIcon *FiducialsIcon;
    vtkKWIcon *MeasurementsIcon;
    vtkKWIcon *SaveSceneIcon;
    vtkKWIcon *LoadSceneIcon;
    vtkKWIcon *ConventionalViewIcon;
    vtkKWIcon *OneUp3DViewIcon;
    vtkKWIcon *OneUpSliceViewIcon;
    vtkKWIcon *FourUpViewIcon;
    vtkKWIcon *TabbedSliceViewIcon;
    vtkKWIcon *Tabbed3DViewIcon;
    vtkKWIcon *LightBoxViewIcon;
    vtkKWIcon *MousePickIcon;
    vtkKWIcon *MousePanIcon;
    vtkKWIcon *MouseRotateIcon;
    vtkKWIcon *MouseZoomIcon;
    vtkKWIcon *MousePlaceFiducialIcon;

 private:
    vtkSlicerToolbarIcons (const vtkSlicerToolbarIcons&); // Not implemented
    void operator = ( const vtkSlicerToolbarIcons& ); // Not implemented
    
};

#endif
