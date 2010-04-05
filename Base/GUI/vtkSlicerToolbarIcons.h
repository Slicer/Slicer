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
#include "./Resources/vtkSlicerUndoRedoToolbar_ImageData.h"
#include "./Resources/vtkSlicerUtilitiesToolbar_ImageData.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerToolbarIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerToolbarIcons* New ( );
    vtkTypeRevisionMacro ( vtkSlicerToolbarIcons, vtkSlicerIcons );
    void PrintSelf ( ostream& os, vtkIndent indent );

    /// 
    /// Get toolbar vtkKWIcons
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
    vtkGetObjectMacro (ChooseLayoutIcon, vtkKWIcon );
    vtkGetObjectMacro (TwinViewIcon, vtkKWIcon );
    vtkGetObjectMacro (ConventionalViewIcon, vtkKWIcon);
    vtkGetObjectMacro (OneUp3DViewIcon, vtkKWIcon);
    vtkGetObjectMacro (CompareViewIcon, vtkKWIcon );
    vtkGetObjectMacro (SideBySideCompareViewIcon, vtkKWIcon );
    vtkGetObjectMacro (OneUpSliceViewIcon, vtkKWIcon );
    vtkGetObjectMacro (OneUpRedSliceViewIcon, vtkKWIcon );
    vtkGetObjectMacro (OneUpGreenSliceViewIcon, vtkKWIcon );
    vtkGetObjectMacro (OneUpYellowSliceViewIcon, vtkKWIcon );    
    vtkGetObjectMacro (FourUpViewIcon, vtkKWIcon);
    vtkGetObjectMacro (TabbedSliceViewIcon, vtkKWIcon);
    vtkGetObjectMacro (Tabbed3DViewIcon, vtkKWIcon);
    vtkGetObjectMacro (LightBoxViewIcon, vtkKWIcon);
    vtkGetObjectMacro (SlicerOptionsOnIcon, vtkKWIcon );
    vtkGetObjectMacro (SlicerOptionsOffIcon, vtkKWIcon );
    vtkGetObjectMacro (SlicerOptionsDisabledIcon, vtkKWIcon );
    vtkGetObjectMacro (MousePickOnIcon, vtkKWIcon );
    vtkGetObjectMacro (MousePickOffIcon, vtkKWIcon );
    vtkGetObjectMacro (MousePickDisabledIcon, vtkKWIcon );
    vtkGetObjectMacro (MouseTransformViewOnIcon, vtkKWIcon );    
    vtkGetObjectMacro (MouseTransformViewOffIcon, vtkKWIcon );    
    vtkGetObjectMacro (MouseTransformViewDisabledIcon, vtkKWIcon );    
    vtkGetObjectMacro (MousePlaceOnIcon, vtkKWIcon );
    vtkGetObjectMacro (MousePlaceOffIcon, vtkKWIcon );
    vtkGetObjectMacro (MousePlaceDisabledIcon, vtkKWIcon );
    vtkGetObjectMacro (MouseManipulateOnIcon, vtkKWIcon );
    vtkGetObjectMacro (MouseManipulateOffIcon, vtkKWIcon );
    vtkGetObjectMacro (MouseManipulateDisabledIcon, vtkKWIcon );
    vtkGetObjectMacro (UndoIcon, vtkKWIcon );
    vtkGetObjectMacro (RedoIcon, vtkKWIcon );
    vtkGetObjectMacro (ScreenShotIcon, vtkKWIcon );

    virtual void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerToolbarIcons ( );
    virtual ~vtkSlicerToolbarIcons ( );
    
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
    vtkKWIcon *ChooseLayoutIcon;
    vtkKWIcon *TwinViewIcon;
    vtkKWIcon *ConventionalViewIcon;
    vtkKWIcon *OneUp3DViewIcon;
    vtkKWIcon *SideBySideCompareViewIcon;
    vtkKWIcon *CompareViewIcon;
    vtkKWIcon *OneUpSliceViewIcon;
    vtkKWIcon *OneUpRedSliceViewIcon;
    vtkKWIcon *OneUpGreenSliceViewIcon;
    vtkKWIcon *OneUpYellowSliceViewIcon;
    vtkKWIcon *FourUpViewIcon;
    vtkKWIcon *TabbedSliceViewIcon;
    vtkKWIcon *Tabbed3DViewIcon;
    vtkKWIcon *LightBoxViewIcon;
    vtkKWIcon *SlicerOptionsOnIcon;
    vtkKWIcon *SlicerOptionsOffIcon;
    vtkKWIcon *SlicerOptionsDisabledIcon;
    vtkKWIcon *MousePickOnIcon;
    vtkKWIcon *MousePickOffIcon;
    vtkKWIcon *MousePickDisabledIcon;
    vtkKWIcon *MouseManipulateOnIcon;
    vtkKWIcon *MouseManipulateOffIcon;
    vtkKWIcon *MouseManipulateDisabledIcon;
    vtkKWIcon *MouseTransformViewOnIcon;
    vtkKWIcon *MouseTransformViewOffIcon;
    vtkKWIcon *MouseTransformViewDisabledIcon;
    vtkKWIcon *MousePlaceOnIcon;
    vtkKWIcon *MousePlaceOffIcon;
    vtkKWIcon *MousePlaceDisabledIcon;
    vtkKWIcon *UndoIcon;
    vtkKWIcon *RedoIcon;    
    vtkKWIcon *ScreenShotIcon;


 private:
    vtkSlicerToolbarIcons (const vtkSlicerToolbarIcons&); /// Not implemented
    void operator = ( const vtkSlicerToolbarIcons& ); /// Not implemented
    
};

#endif
