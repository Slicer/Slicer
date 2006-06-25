#ifndef __vtkSlicerToolbarIcons_h
#define __vtkSlicerToolbarIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerToolbarTransform_ImageData.h"
#include "./Resources/vtkSlicerToolbarColor_ImageData.h"
#include "./Resources/vtkSlicerToolbarConventionalView_ImageData.h"
#include "./Resources/vtkSlicerToolbarData_ImageData.h"
#include "./Resources/vtkSlicerToolbarEditorToolbox_ImageData.h"
#include "./Resources/vtkSlicerToolbarEditor_ImageData.h"
#include "./Resources/vtkSlicerToolbarFiducials_ImageData.h"
#include "./Resources/vtkSlicerToolbarFourUpView_ImageData.h"
#include "./Resources/vtkSlicerToolbarHome_ImageData.h"
#include "./Resources/vtkSlicerToolbarLightBoxView_ImageData.h"
#include "./Resources/vtkSlicerToolbarLoadScene_ImageData.h"
#include "./Resources/vtkSlicerToolbarModel_ImageData.h"
#include "./Resources/vtkSlicerToolbarOneUp3DView_ImageData.h"
#include "./Resources/vtkSlicerToolbarOneUpSliceView_ImageData.h"
#include "./Resources/vtkSlicerToolbarSaveScene_ImageData.h"
#include "./Resources/vtkSlicerToolbarTabbedView_ImageData.h"
#include "./Resources/vtkSlicerToolbarVolume_ImageData.h"
#include "./Resources/vtkSlicerToolbarMouseModes_ImageData.h"

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
    vtkGetObjectMacro (SaveSceneIcon, vtkKWIcon);
    vtkGetObjectMacro (LoadSceneIcon, vtkKWIcon);
    vtkGetObjectMacro (ConventionalViewIcon, vtkKWIcon);
    vtkGetObjectMacro (OneUp3DViewIcon, vtkKWIcon);
    vtkGetObjectMacro (OneUpSliceViewIcon, vtkKWIcon );
    vtkGetObjectMacro (FourUpViewIcon, vtkKWIcon);
    vtkGetObjectMacro (TabbedViewIcon, vtkKWIcon);
    vtkGetObjectMacro (LightBoxViewIcon, vtkKWIcon);
    vtkGetObjectMacro (MousePickIcon, vtkKWIcon );
    vtkGetObjectMacro (MousePanIcon, vtkKWIcon );
    vtkGetObjectMacro (MouseRotateIcon, vtkKWIcon );    
    vtkGetObjectMacro (MouseZoomIcon, vtkKWIcon );

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
    vtkKWIcon *SaveSceneIcon;
    vtkKWIcon *LoadSceneIcon;
    vtkKWIcon *ConventionalViewIcon;
    vtkKWIcon *OneUp3DViewIcon;
    vtkKWIcon *OneUpSliceViewIcon;
    vtkKWIcon *FourUpViewIcon;
    vtkKWIcon *TabbedViewIcon;
    vtkKWIcon *LightBoxViewIcon;
    vtkKWIcon *MousePickIcon;
    vtkKWIcon *MousePanIcon;
    vtkKWIcon *MouseRotateIcon;
    vtkKWIcon *MouseZoomIcon;
    
 private:
    vtkSlicerToolbarIcons (const vtkSlicerToolbarIcons&); // Not implemented
    void operator = ( const vtkSlicerToolbarIcons& ); // Not implemented
    
};

#endif
