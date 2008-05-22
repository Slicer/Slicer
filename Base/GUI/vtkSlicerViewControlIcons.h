#ifndef __vtkSlicerViewControlIcons_h
#define __vtkSlicerViewControlIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerNavZoom_ImageData.h"
#include "./Resources/vtkSlicerViewControl_ImageData.h"

// Description:
// This class reads in the png format image data for logos and
// for icons in Slicer's main toolbar, and elsewhere in
// slicer's main window. It also creates vtkKWIcons and assigns
// the image data to them. (Elsewhere in the application,
// vtkKWLabels and vtkKWPushbuttons can display these
// vtkKWIcons instead of text.

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerViewControlIcons : public vtkSlicerIcons
{
 public:
    // Description:
    // Usual vtk functions
    static vtkSlicerViewControlIcons* New ();
    vtkTypeRevisionMacro ( vtkSlicerViewControlIcons, vtkSlicerIcons );
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro (ViewAxisAIconLO, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisAIconHI, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisPIconLO, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisPIconHI, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisRIconLO, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisRIconHI, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisLIconLO, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisLIconHI, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisSIconLO, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisSIconHI, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisIIconLO, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisIIconHI, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisBottomCornerIcon, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisTopCornerIcon, vtkKWIcon );
    vtkGetObjectMacro (ViewAxisCenterIcon, vtkKWIcon );    

    vtkGetObjectMacro (SpinOffButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (SpinOnButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (RockOnButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (RockOffButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (OrthoButtonIcon, vtkKWIcon );    
    vtkGetObjectMacro (PerspectiveButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (CenterButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (SelectViewButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (StereoButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (LookFromOffButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (LookFromOnButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundOffButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (RotateAroundOnButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (VisibilityButtonIcon, vtkKWIcon );
    vtkGetObjectMacro (SelectCameraButtonIcon, vtkKWIcon );

    vtkGetObjectMacro ( SceneSnapshotIcon, vtkKWIcon );
    vtkGetObjectMacro ( SelectSceneSnapshotIcon, vtkKWIcon );

    vtkGetObjectMacro (NavZoomInIcon, vtkKWIcon);
    vtkGetObjectMacro (NavZoomOutIcon, vtkKWIcon);

    vtkGetObjectMacro (PercentZoomIcon, vtkKWIcon );

    // Description:
    // Method for setting images for widgets in Slicer's ViewControlFrame
    void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerViewControlIcons ( );
    virtual ~vtkSlicerViewControlIcons ( );

    vtkKWIcon *ViewAxisAIconLO;
    vtkKWIcon *ViewAxisAIconHI;
    vtkKWIcon *ViewAxisPIconLO;
    vtkKWIcon *ViewAxisPIconHI;
    vtkKWIcon *ViewAxisRIconLO;
    vtkKWIcon *ViewAxisRIconHI;
    vtkKWIcon *ViewAxisLIconLO;
    vtkKWIcon *ViewAxisLIconHI;
    vtkKWIcon *ViewAxisSIconLO;
    vtkKWIcon *ViewAxisSIconHI;
    vtkKWIcon *ViewAxisIIconLO;
    vtkKWIcon *ViewAxisIIconHI;
    vtkKWIcon *ViewAxisBottomCornerIcon;
    vtkKWIcon *ViewAxisTopCornerIcon;
    vtkKWIcon *ViewAxisCenterIcon;

    vtkKWIcon *SpinOffButtonIcon;
    vtkKWIcon *SpinOnButtonIcon;
    vtkKWIcon *RockOffButtonIcon;
    vtkKWIcon *RockOnButtonIcon;
    vtkKWIcon *OrthoButtonIcon;
    vtkKWIcon *PerspectiveButtonIcon;
    vtkKWIcon *CenterButtonIcon;
    vtkKWIcon *SelectViewButtonIcon;
    vtkKWIcon *StereoButtonIcon;
    vtkKWIcon *LookFromOffButtonIcon;
    vtkKWIcon *LookFromOnButtonIcon;
    vtkKWIcon *RotateAroundOffButtonIcon;    
    vtkKWIcon *RotateAroundOnButtonIcon;    
    vtkKWIcon *VisibilityButtonIcon;
    vtkKWIcon *SelectCameraButtonIcon;

    vtkKWIcon *SceneSnapshotIcon;
    vtkKWIcon *SelectSceneSnapshotIcon;

    vtkKWIcon *NavZoomInIcon;
    vtkKWIcon *NavZoomOutIcon;
    vtkKWIcon *PercentZoomIcon;

    
 private:
    vtkSlicerViewControlIcons (const vtkSlicerViewControlIcons& ); // Not implemented
    void operator = (const vtkSlicerViewControlIcons& ); // Not implemented
    
};
#endif
