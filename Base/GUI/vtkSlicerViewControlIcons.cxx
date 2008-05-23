
#include "vtkObjectFactory.h"
#include "vtkSlicerViewControlIcons.h"
#include "vtkSlicerFoundationIcons.h"
#include "vtkKWIcon.h"
#include "vtkKWResourceUtilities.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerViewControlIcons );
vtkCxxRevisionMacro ( vtkSlicerViewControlIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerViewControlIcons::vtkSlicerViewControlIcons ( )
{

    //--- "View Axis" images in the ViewControlFrame
    this->ViewAxisAIconLO = vtkKWIcon::New ( );
    this->ViewAxisAIconHI = vtkKWIcon::New ( );
    this->ViewAxisPIconLO = vtkKWIcon::New ( );
    this->ViewAxisPIconHI = vtkKWIcon::New ( );    
    this->ViewAxisRIconLO = vtkKWIcon::New ( );
    this->ViewAxisRIconHI = vtkKWIcon::New ( );    
    this->ViewAxisLIconLO = vtkKWIcon::New ( );
    this->ViewAxisLIconHI = vtkKWIcon::New ( );    
    this->ViewAxisSIconLO = vtkKWIcon::New ( );
    this->ViewAxisSIconHI = vtkKWIcon::New ( );    
    this->ViewAxisIIconLO = vtkKWIcon::New ( );
    this->ViewAxisIIconHI = vtkKWIcon::New ( );    
    this->ViewAxisTopCornerIcon = vtkKWIcon::New ( );    
    this->ViewAxisBottomCornerIcon = vtkKWIcon::New ( );    
    this->ViewAxisCenterIcon = vtkKWIcon::New ( );    

    this->SpinOffButtonIcon = vtkKWIcon::New ( );
    this->SpinOnButtonIcon = vtkKWIcon::New ( );
    this->RockOffButtonIcon = vtkKWIcon::New ( );
    this->RockOnButtonIcon = vtkKWIcon::New ( );
    this->OrthoButtonIcon = vtkKWIcon::New ( );
    this->PerspectiveButtonIcon = vtkKWIcon::New ( );
    this->CenterButtonIcon = vtkKWIcon::New ( );
    this->ScreenGrabButtonIcon = vtkKWIcon::New ( );
    this->StereoButtonIcon = vtkKWIcon::New ( );
    this->LookFromOffButtonIcon = vtkKWIcon::New ( );
    this->LookFromOnButtonIcon = vtkKWIcon::New ( );
    this->RotateAroundOffButtonIcon = vtkKWIcon::New ( );
    this->RotateAroundOnButtonIcon = vtkKWIcon::New ( );
    this->VisibilityButtonIcon = vtkKWIcon::New ( );
    this->SelectCameraButtonIcon = vtkKWIcon::New ( );

    this->SceneSnapshotIcon = vtkKWIcon::New();
    this->SelectSceneSnapshotIcon = vtkKWIcon::New();
    
    //--- "Zoom in and out icons
    this->NavZoomInIcon = vtkKWIcon::New();
    this->NavZoomOutIcon = vtkKWIcon::New();
    this->PercentZoomIcon = vtkKWIcon::New();
    this->PitchIcon = vtkKWIcon::New();
    this->RollIcon = vtkKWIcon::New();
    this->YawIcon = vtkKWIcon::New();
    this->ZoomInIcon = vtkKWIcon::New();
    this->ZoomOutIcon = vtkKWIcon::New();

    //--- read in image data and assign to Icons.
    this->AssignImageDataToIcons ( );

}



//---------------------------------------------------------------------------
vtkSlicerViewControlIcons::~vtkSlicerViewControlIcons ( )
{

    // view from or rotate around icons
    if ( this->ViewAxisAIconLO ) {
        this->ViewAxisAIconLO->Delete ( );
        this->ViewAxisAIconLO = NULL;
    }
    if ( this->ViewAxisAIconHI ) {
        this->ViewAxisAIconHI->Delete ( );
        this->ViewAxisAIconHI = NULL;
    }
    if ( this->ViewAxisPIconLO ) {
        this->ViewAxisPIconLO->Delete ( );
        this->ViewAxisPIconLO = NULL;
    }
    if ( this->ViewAxisPIconHI ) {
        this->ViewAxisPIconHI->Delete ( );
        this->ViewAxisPIconHI = NULL;
    }
    if ( this->ViewAxisRIconLO ) {
        this->ViewAxisRIconLO->Delete ( );
        this->ViewAxisRIconLO = NULL;
    }
    if ( this->ViewAxisRIconHI ) {
        this->ViewAxisRIconHI->Delete ( );
        this->ViewAxisRIconHI = NULL;
    }
    if ( this->ViewAxisLIconLO ) {
        this->ViewAxisLIconLO->Delete ( );
        this->ViewAxisLIconLO = NULL;
    }
    if ( this->ViewAxisLIconHI ) {
        this->ViewAxisLIconHI->Delete ( );
        this->ViewAxisLIconHI = NULL;
    }
    if ( this->ViewAxisSIconLO ) {
        this->ViewAxisSIconLO->Delete ( );
        this->ViewAxisSIconLO = NULL;
    }
    if ( this->ViewAxisSIconHI ) {
        this->ViewAxisSIconHI->Delete ( );
        this->ViewAxisSIconHI = NULL;
    }
    if ( this->ViewAxisIIconLO ) {
        this->ViewAxisIIconLO->Delete ( );
        this->ViewAxisIIconLO = NULL;
    }
    if ( this->ViewAxisIIconHI ) {
        this->ViewAxisIIconHI->Delete ( );
        this->ViewAxisIIconHI = NULL;
    }
    if ( this->ViewAxisTopCornerIcon ) {
        this->ViewAxisTopCornerIcon->Delete ( );
        this->ViewAxisTopCornerIcon = NULL;
    }
    if ( this->ViewAxisBottomCornerIcon ) {
        this->ViewAxisBottomCornerIcon->Delete ( );
        this->ViewAxisBottomCornerIcon = NULL;
    }
    if ( this->ViewAxisCenterIcon ) {
        this->ViewAxisCenterIcon->Delete ( );
        this->ViewAxisCenterIcon = NULL;
    }
    if ( this->SpinOffButtonIcon )
      {
      this->SpinOffButtonIcon->Delete ( );
      this->SpinOffButtonIcon = NULL;
      }
    if ( this->RockOffButtonIcon )
      {
      this->RockOffButtonIcon->Delete ( );
      this->RockOffButtonIcon = NULL;
      }
    if ( this->SpinOnButtonIcon )
      {
      this->SpinOnButtonIcon->Delete ( );
      this->SpinOnButtonIcon = NULL;
      }
    if ( this->RockOnButtonIcon )
      {
      this->RockOnButtonIcon->Delete ( );
      this->RockOnButtonIcon = NULL;
      }
    if ( this->OrthoButtonIcon )
      {
      this->OrthoButtonIcon->Delete ( );
      this->OrthoButtonIcon = NULL;
      }
    if ( this->PerspectiveButtonIcon )
      {
      this->PerspectiveButtonIcon->Delete ( );
      this->PerspectiveButtonIcon = NULL;
      }
    if ( this->CenterButtonIcon )
      {
      this->CenterButtonIcon->Delete ( );
      this->CenterButtonIcon = NULL;
      }
    if ( this->ScreenGrabButtonIcon )
      {
      this->ScreenGrabButtonIcon->Delete ( );
      this->ScreenGrabButtonIcon = NULL;
      }
    if ( this->StereoButtonIcon )
      {
      this->StereoButtonIcon->Delete ( );
      this->StereoButtonIcon = NULL;
      }
    if ( this->LookFromOffButtonIcon )
      {
      this->LookFromOffButtonIcon->Delete ( );
      this->LookFromOffButtonIcon = NULL;
      }
    if ( this->RotateAroundOffButtonIcon )
      {
      this->RotateAroundOffButtonIcon->Delete ( );
      this->RotateAroundOffButtonIcon = NULL;
      }
    if ( this->LookFromOnButtonIcon )
      {
      this->LookFromOnButtonIcon->Delete ( );
      this->LookFromOnButtonIcon = NULL;
      }
    if ( this->RotateAroundOnButtonIcon )
      {
      this->RotateAroundOnButtonIcon->Delete ( );
      this->RotateAroundOnButtonIcon = NULL;
      }
    if ( this->VisibilityButtonIcon )
      {
      this->VisibilityButtonIcon->Delete ( );
      this->VisibilityButtonIcon = NULL;
      }
    if ( this->SelectCameraButtonIcon )
      {
      this->SelectCameraButtonIcon->Delete ( );
      this->SelectCameraButtonIcon = NULL;
      }

    if ( this->SceneSnapshotIcon )
      {
      this->SceneSnapshotIcon->Delete();
      this->SceneSnapshotIcon = NULL;
      }
    if ( this->SelectSceneSnapshotIcon)
      {
      this->SelectSceneSnapshotIcon->Delete();
      this->SelectSceneSnapshotIcon = NULL;
      }

    // zoom images
    if ( this->NavZoomInIcon ) {
        this->NavZoomInIcon->Delete ( );
        this->NavZoomInIcon = NULL;
    }
    if ( this->NavZoomOutIcon ) {
        this->NavZoomOutIcon->Delete ( );
        this->NavZoomOutIcon = NULL;
    }
    if ( this->PercentZoomIcon )
      {
      this->PercentZoomIcon->Delete();
      this->PercentZoomIcon = NULL;
      }
    if ( this->PitchIcon)
      {
      this->PitchIcon->Delete();
      this->PitchIcon = NULL;
      }
    if ( this->RollIcon)
      {
      this->RollIcon->Delete();
      this->RollIcon = NULL;
      }
    if ( this->YawIcon)
      {
      this->YawIcon->Delete();
      this->YawIcon = NULL;
      }
    if ( this->ZoomInIcon)
      {
      this->ZoomInIcon->Delete();
      this->ZoomInIcon = NULL;      
      }
    if ( this->ZoomOutIcon)
      {
      this->ZoomOutIcon->Delete();
      this->ZoomOutIcon = NULL;      
      }
}


                                                              
//---------------------------------------------------------------------------
void vtkSlicerViewControlIcons::AssignImageDataToIcons ( ) {
    // Rotate around Icons (9 of these tile one image in the view control frame;
    // six of them have a "LO" and "HI" state that changes
    // on mouseover.


  this->ViewAxisAIconHI->SetImage( image_ViewAxisAHi,
                                   image_ViewAxisAHi_width,
                                   image_ViewAxisAHi_height,
                                   image_ViewAxisAHi_pixel_size,
                                   image_ViewAxisAHi_length, 0);
  this->ViewAxisAIconLO->SetImage( image_ViewAxisALo,
                                   image_ViewAxisALo_width,
                                   image_ViewAxisALo_height,
                                   image_ViewAxisALo_pixel_size,
                                   image_ViewAxisALo_length, 0);
  this->ViewAxisPIconLO->SetImage( image_ViewAxisPLo,
                                   image_ViewAxisPLo_width,
                                   image_ViewAxisPLo_height,
                                   image_ViewAxisPLo_pixel_size,
                                   image_ViewAxisPLo_length, 0);
  this->ViewAxisPIconHI->SetImage( image_ViewAxisPHi,
                                   image_ViewAxisPHi_width,
                                   image_ViewAxisPHi_height,
                                   image_ViewAxisPHi_pixel_size,
                                   image_ViewAxisPHi_length, 0);
  this->ViewAxisRIconLO->SetImage( image_ViewAxisRLo,
                                   image_ViewAxisRLo_width,
                                   image_ViewAxisRLo_height,
                                   image_ViewAxisRLo_pixel_size,
                                   image_ViewAxisRLo_length, 0);
  this->ViewAxisRIconHI->SetImage( image_ViewAxisRHi,
                                   image_ViewAxisRHi_width,
                                   image_ViewAxisRHi_height,
                                   image_ViewAxisRHi_pixel_size,
                                   image_ViewAxisRHi_length, 0);
  this->ViewAxisLIconLO->SetImage( image_ViewAxisLLo,
                                   image_ViewAxisLLo_width,
                                   image_ViewAxisLLo_height,
                                   image_ViewAxisLLo_pixel_size,
                                   image_ViewAxisLLo_length, 0);
  this->ViewAxisLIconHI->SetImage( image_ViewAxisLHi,
                                   image_ViewAxisLHi_width,
                                   image_ViewAxisLHi_height,
                                   image_ViewAxisLHi_pixel_size,
                                   image_ViewAxisLHi_length, 0);
  this->ViewAxisSIconLO->SetImage( image_ViewAxisSLo,
                                   image_ViewAxisSLo_width,
                                   image_ViewAxisSLo_height,
                                   image_ViewAxisSLo_pixel_size,
                                   image_ViewAxisSLo_length, 0);
  this->ViewAxisSIconHI->SetImage( image_ViewAxisSHi,
                                   image_ViewAxisSHi_width,
                                   image_ViewAxisSHi_height,
                                   image_ViewAxisSHi_pixel_size,
                                   image_ViewAxisSHi_length, 0);
  this->ViewAxisIIconLO->SetImage( image_ViewAxisILo,
                                   image_ViewAxisILo_width,
                                   image_ViewAxisILo_height,
                                   image_ViewAxisILo_pixel_size,
                                   image_ViewAxisILo_length, 0 );
  this->ViewAxisIIconHI->SetImage( image_ViewAxisIHi,
                                   image_ViewAxisIHi_width,
                                   image_ViewAxisIHi_height,
                                   image_ViewAxisIHi_pixel_size,
                                   image_ViewAxisIHi_length, 0);
  this->ViewAxisBottomCornerIcon->SetImage( image_ViewAxisBottomCorner,
                                            image_ViewAxisBottomCorner_width,
                                            image_ViewAxisBottomCorner_height,
                                            image_ViewAxisBottomCorner_pixel_size,
                                            image_ViewAxisBottomCorner_length, 0);
  this->ViewAxisTopCornerIcon->SetImage( image_ViewAxisTopCorner,
                                         image_ViewAxisTopCorner_width,
                                         image_ViewAxisTopCorner_height,
                                         image_ViewAxisTopCorner_pixel_size,
                                         image_ViewAxisTopCorner_length, 0);
  this->ViewAxisCenterIcon->SetImage( image_ViewAxisMiddle,
                                      image_ViewAxisMiddle_width,
                                      image_ViewAxisMiddle_height,
                                      image_ViewAxisMiddle_pixel_size,
                                      image_ViewAxisMiddle_length, 0);
  this->SpinOffButtonIcon->SetImage ( image_ViewSpinRadioOff,
                                   image_ViewSpinRadioOff_width,
                                   image_ViewSpinRadioOff_height,
                                   image_ViewSpinRadioOff_pixel_size,
                                   image_ViewSpinRadioOff_length, 0 );                                   
  this->RockOffButtonIcon->SetImage ( image_ViewRockRadioOff,
                                   image_ViewRockRadioOff_width,
                                   image_ViewRockRadioOff_height,
                                   image_ViewRockRadioOff_pixel_size,
                                   image_ViewRockRadioOff_length, 0 );                                   
  this->SpinOnButtonIcon->SetImage ( image_ViewSpinRadioOn,
                                   image_ViewSpinRadioOn_width,
                                   image_ViewSpinRadioOn_height,
                                   image_ViewSpinRadioOn_pixel_size,
                                   image_ViewSpinRadioOn_length, 0 );                                   
  this->RockOnButtonIcon->SetImage ( image_ViewRockRadioOn,
                                   image_ViewRockRadioOn_width,
                                   image_ViewRockRadioOn_height,
                                   image_ViewRockRadioOn_pixel_size,
                                   image_ViewRockRadioOn_length, 0 );                                   
  this->OrthoButtonIcon->SetImage ( image_ViewOrtho,
                                   image_ViewOrtho_width,
                                   image_ViewOrtho_height,
                                   image_ViewOrtho_pixel_size,
                                   image_ViewOrtho_length, 0 );                                   
  this->PerspectiveButtonIcon->SetImage ( image_ViewPerspective,
                                   image_ViewPerspective_width,
                                   image_ViewPerspective_height,
                                   image_ViewPerspective_pixel_size,
                                   image_ViewPerspective_length, 0 );                                   
  this->CenterButtonIcon->SetImage ( image_ViewCenter,
                                   image_ViewCenter_width,
                                   image_ViewCenter_height,
                                   image_ViewCenter_pixel_size,
                                   image_ViewCenter_length, 0 );                                   
  this->ScreenGrabButtonIcon->SetImage ( image_ViewSelect,
                                   image_ViewSelect_width,
                                   image_ViewSelect_height,
                                   image_ViewSelect_pixel_size,
                                   image_ViewSelect_length, 0 );                                   
  this->StereoButtonIcon->SetImage ( image_ViewStereo,
                                   image_ViewStereo_width,
                                   image_ViewStereo_height,
                                   image_ViewStereo_pixel_size,
                                   image_ViewStereo_length, 0 );                                   
  this->LookFromOffButtonIcon->SetImage ( image_ViewAxisLookFromRadioOff,
                                   image_ViewAxisLookFromRadioOff_width,
                                   image_ViewAxisLookFromRadioOff_height,
                                   image_ViewAxisLookFromRadioOff_pixel_size,
                                   image_ViewAxisLookFromRadioOff_length, 0 );                                   
  this->RotateAroundOffButtonIcon->SetImage ( image_ViewAxisRotateAroundRadioOff,
                                   image_ViewAxisRotateAroundRadioOff_width,
                                   image_ViewAxisRotateAroundRadioOff_height,
                                   image_ViewAxisRotateAroundRadioOff_pixel_size,
                                   image_ViewAxisRotateAroundRadioOff_length, 0 );                                   
  this->LookFromOnButtonIcon->SetImage ( image_ViewAxisLookFromRadioOn,
                                   image_ViewAxisLookFromRadioOn_width,
                                   image_ViewAxisLookFromRadioOn_height,
                                   image_ViewAxisLookFromRadioOn_pixel_size,
                                   image_ViewAxisLookFromRadioOn_length, 0 );                                   
  this->RotateAroundOnButtonIcon->SetImage ( image_ViewAxisRotateAroundRadioOn,
                                   image_ViewAxisRotateAroundRadioOn_width,
                                   image_ViewAxisRotateAroundRadioOn_height,
                                   image_ViewAxisRotateAroundRadioOn_pixel_size,
                                   image_ViewAxisRotateAroundRadioOn_length, 0 );                                   
  this->VisibilityButtonIcon->SetImage ( image_ViewFeaturesVisible,
                                         image_ViewFeaturesVisible_width,
                                         image_ViewFeaturesVisible_height,
                                         image_ViewFeaturesVisible_pixel_size,
                                         image_ViewFeaturesVisible_length, 0 );
  this->SelectCameraButtonIcon->SetImage ( image_ViewCameraSelect,
                                           image_ViewCameraSelect_width,
                                           image_ViewCameraSelect_height,
                                           image_ViewCameraSelect_pixel_size,
                                           image_ViewCameraSelect_length, 0 );
  this->SceneSnapshotIcon->SetImage ( image_SlicerCamera,
                                      image_SlicerCamera_width,
                                      image_SlicerCamera_height,
                                      image_SlicerCamera_pixel_size,
                                      image_SlicerCamera_length, 0);                                      
  this->SelectSceneSnapshotIcon->SetImage ( image_ViewCameraSelect,
                                            image_ViewCameraSelect_width,
                                            image_ViewCameraSelect_height,
                                            image_ViewCameraSelect_pixel_size,
                                            image_ViewCameraSelect_length, 0);                                            

    this->NavZoomInIcon->SetImage( image_NavZoomIn,
                                   image_NavZoomIn_width,
                                   image_NavZoomIn_height,
                                   image_NavZoomIn_pixel_size, 0, 0);
    this->NavZoomOutIcon->SetImage( image_NavZoomOut,
                                    image_NavZoomOut_width,
                                    image_NavZoomOut_height,
                                    image_NavZoomOut_pixel_size, 0, 0);
    this->PercentZoomIcon->SetImage( image_PercentRelativeZoom,
                                    image_PercentRelativeZoom_width,
                                    image_PercentRelativeZoom_height,
                                    image_PercentRelativeZoom_pixel_size,
                                     image_PercentRelativeZoom_length, 0);
    this->ZoomInIcon->SetImage( image_SlicerZoomIn,
                                image_SlicerZoomIn_width,
                                image_SlicerZoomIn_height,
                                image_SlicerZoomIn_pixel_size,
                                image_SlicerZoomIn_length, 0);
    this->ZoomOutIcon->SetImage( image_SlicerZoomOut,
                                 image_SlicerZoomOut_width,
                                 image_SlicerZoomOut_height,
                                 image_SlicerZoomOut_pixel_size,
                                 image_SlicerZoomOut_length, 0);
    this->PitchIcon->SetImage ( image_ViewPitch,
                                image_ViewPitch_width,
                                image_ViewPitch_height,
                                image_ViewPitch_pixel_size,
                                image_ViewPitch_length, 0);
    this->RollIcon->SetImage ( image_ViewRoll,
                                image_ViewRoll_width,
                                image_ViewRoll_height,
                                image_ViewRoll_pixel_size,
                                image_ViewRoll_length, 0);
    this->YawIcon->SetImage ( image_ViewYaw,
                               image_ViewYaw_width,
                               image_ViewYaw_height,
                               image_ViewYaw_pixel_size,
                               image_ViewYaw_length, 0);
    
}



//---------------------------------------------------------------------------
void vtkSlicerViewControlIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerViewControlIcons: " << this->GetClassName ( ) << "\n";

    os << indent << "ViewAxisAIconLO: " << this->GetViewAxisAIconLO() << "\n";
    os << indent << "ViewAxisAIconHI: " << this->GetViewAxisAIconHI () << "\n";
    os << indent << "ViewAxisPIconLO: " << this->GetViewAxisPIconLO () << "\n";
    os << indent << "ViewAxisPIconHI: " << this->GetViewAxisPIconHI () << "\n";
    os << indent << "ViewAxisRIconLO: " << this->GetViewAxisRIconLO () << "\n";
    os << indent << "ViewAxisRIconHI: " << this->GetViewAxisRIconHI () << "\n";
    os << indent << "ViewAxisLIconLO: " << this->GetViewAxisLIconLO () << "\n";
    os << indent << "ViewAxisLIconHI: " << this->GetViewAxisLIconHI () << "\n";
    os << indent << "ViewAxisSIconLO: " << this->GetViewAxisSIconLO () << "\n";
    os << indent << "ViewAxisSIconHI: " << this->GetViewAxisSIconHI () << "\n";
    os << indent << "ViewAxisIIconLO: " << this->GetViewAxisIIconLO () << "\n";
    os << indent << "ViewAxisIIconHI: " << this->GetViewAxisIIconHI () << "\n";
    os << indent << "ViewAxisTopCornerIcon: " << this->GetViewAxisTopCornerIcon () << "\n";
    os << indent << "ViewAxisBottomCornerIcon: " << this->GetViewAxisBottomCornerIcon () << "\n";
    os << indent << "ViewAxisCenterIcon: " << this->GetViewAxisCenterIcon () << "\n";
    os << indent << "SpinOffButtonIcon: " << this->GetSpinOffButtonIcon () << "\n";
    os << indent << "RockOffButtonIcon: " << this->GetRockOffButtonIcon () << "\n";
    os << indent << "SpinOnButtonIcon: " << this->GetSpinOnButtonIcon () << "\n";
    os << indent << "RockOnButtonIcon: " << this->GetRockOnButtonIcon () << "\n";
    os << indent << "OrthoButtonIcon: " << this->GetOrthoButtonIcon () << "\n";
    os << indent << "PerspectiveButtonIcon: " << this->GetPerspectiveButtonIcon () << "\n";
    os << indent << "CenterButtonIcon: " << this->GetCenterButtonIcon () << "\n";
    os << indent << "ScreenGrabButtonIcon: " << this->GetScreenGrabButtonIcon () << "\n";
    os << indent << "StereoButtonIcon: " << this->GetStereoButtonIcon () << "\n";
    os << indent << "LookFromOffButtonIcon: " << this->GetLookFromOffButtonIcon () << "\n";
    os << indent << "RotateAroundOffButtonIcon: " << this->GetRotateAroundOffButtonIcon () << "\n";
    os << indent << "LookFromOnButtonIcon: " << this->GetLookFromOnButtonIcon () << "\n";
    os << indent << "RotateAroundOnButtonIcon: " << this->GetRotateAroundOnButtonIcon () << "\n";
    os << indent << "VisibilityButtonIcon: " << this->GetVisibilityButtonIcon () << "\n";
    os << indent << "SelectCameraButtonIcon: " << this->GetSelectCameraButtonIcon () << "\n";
    os << indent << "PercentZoomIcon: " << this->GetPercentZoomIcon () << "\n";
    os << indent << "SelectSceneSnapshotIcon: " << this->GetSelectSceneSnapshotIcon ( ) << "\n";
    os << indent << "SceneSnapshotIcon: " << this->GetSceneSnapshotIcon ( ) << "\n";
    os << indent << "PitchIcon: " << this->GetPitchIcon() << "\n";
    os << indent << "RollIcon: " << this->GetRollIcon() << "\n";
    os << indent << "YawIcon: " << this->GetYawIcon() << "\n";
    os << indent << "ZoomInIcon: " << this->GetZoomInIcon() << "\n";
    os << indent << "ZoomOutIcon: " << this->GetZoomOutIcon() << "\n";    
}
