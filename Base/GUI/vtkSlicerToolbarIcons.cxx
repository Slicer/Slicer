
#include "vtkObjectFactory.h"
#include "vtkSlicerToolbarIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerToolbarIcons );
vtkCxxRevisionMacro ( vtkSlicerToolbarIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerToolbarIcons::vtkSlicerToolbarIcons ( )
{
        //--- toolbar icons
    this->HomeIcon = vtkKWIcon::New();
    this->DataIcon = vtkKWIcon::New();
    this->VolumeIcon = vtkKWIcon::New();
    this->ModelIcon = vtkKWIcon::New();
    this->EditorIcon = vtkKWIcon::New();
    //    this->EditorToolboxIcon = vtkKWIcon::New();
    this->TransformIcon = vtkKWIcon::New();
    this->ColorIcon = vtkKWIcon::New();
    this->FiducialsIcon = vtkKWIcon::New();
    //   this->MeasurementsIcon = vtkKWIcon::New();
    this->SaveSceneIcon = vtkKWIcon::New();
    this->LoadSceneIcon = vtkKWIcon::New();
    this->ConventionalViewIcon = vtkKWIcon::New();
    this->OneUp3DViewIcon = vtkKWIcon::New();
    this->OneUpSliceViewIcon = vtkKWIcon::New();
    this->FourUpViewIcon = vtkKWIcon::New();
    this->TabbedSliceViewIcon = vtkKWIcon::New();
    this->Tabbed3DViewIcon = vtkKWIcon::New();
    this->LightBoxViewIcon = vtkKWIcon::New();
    this->MousePickIcon = vtkKWIcon::New();
    this->MousePickIconLow = vtkKWIcon::New();
    this->MouseTransformViewIcon = vtkKWIcon::New();
    this->MouseTransformViewIconLow = vtkKWIcon::New();
    this->MousePlaceFiducialIcon = vtkKWIcon::New ( );
    this->MousePlaceFiducialIconLow = vtkKWIcon::New ( );
    this->UndoIcon = vtkKWIcon::New ( );
    this->RedoIcon = vtkKWIcon::New ( );
    this->AssignImageDataToIcons ( );
}



//---------------------------------------------------------------------------
vtkSlicerToolbarIcons::~vtkSlicerToolbarIcons ( )
{
  // toolbar icons
  if ( this->HomeIcon )
    {
    this->HomeIcon->Delete ( );
    this->HomeIcon = NULL;
    }
  if ( this->DataIcon )
    {
    this->DataIcon->Delete ( );
    this->DataIcon = NULL;
    }
  if ( this->VolumeIcon )
    {
    this->VolumeIcon->Delete ( );
    this->VolumeIcon = NULL;
    }
  if ( this->ModelIcon )
    {
    this->ModelIcon->Delete ( );
    this->ModelIcon = NULL;
    }
  if ( this->EditorIcon )
    {
    this->EditorIcon->Delete ( );
    this->EditorIcon = NULL;
    }
/*
  if ( this->EditorToolboxIcon )
    {
    this->EditorToolboxIcon->Delete ( );
    this->EditorToolboxIcon = NULL;
    }
*/
  if ( this->TransformIcon )
    {
    this->TransformIcon->Delete ( );
    this->TransformIcon = NULL;
    }
  if ( this->ColorIcon )
    {
    this->ColorIcon->Delete ( );
    this->ColorIcon = NULL;
    }
  if ( this->FiducialsIcon )
    {
    this->FiducialsIcon->Delete ( );
    this->FiducialsIcon = NULL;
    }
/*
  if ( this->MeasurementsIcon )
    {
    this->MeasurementsIcon->Delete ( );
    this->MeasurementsIcon = NULL;
    }
*/
  if ( this->SaveSceneIcon )
    {
    this->SaveSceneIcon->Delete ( );
    this->SaveSceneIcon = NULL;
    }
  if ( this->LoadSceneIcon )
    {
    this->LoadSceneIcon->Delete ( );
    this->LoadSceneIcon = NULL;
    }
  if ( this->ConventionalViewIcon )
    {
    this->ConventionalViewIcon->Delete ( );
    this->ConventionalViewIcon = NULL;
    }
  if ( this->OneUp3DViewIcon )
    {
    this->OneUp3DViewIcon->Delete ( );
    this->OneUp3DViewIcon = NULL;
    }
  if ( this->OneUpSliceViewIcon )
    {
    this->OneUpSliceViewIcon->Delete ( );
    this->OneUpSliceViewIcon = NULL;
    }
  if ( this->FourUpViewIcon )
    {
    this->FourUpViewIcon->Delete ( );
    this->FourUpViewIcon = NULL;
    }
  if ( this->Tabbed3DViewIcon )
    {
    this->Tabbed3DViewIcon->Delete ( );
    this->Tabbed3DViewIcon = NULL;
    }
  if ( this->TabbedSliceViewIcon )
    {
    this->TabbedSliceViewIcon->Delete ( );
    this->TabbedSliceViewIcon = NULL;
    }
  if ( this->LightBoxViewIcon )
    {
    this->LightBoxViewIcon->Delete ( );
    this->LightBoxViewIcon = NULL;
    }
  if ( this->MousePickIcon )
    {
    this->MousePickIcon->Delete ( );
    this->MousePickIcon = NULL;
    }
  if ( this->MousePlaceFiducialIcon )
    {
    this->MousePlaceFiducialIcon->Delete ( );
    this->MousePlaceFiducialIcon = NULL;
    }
  if ( this->MouseTransformViewIcon )
    {
    this->MouseTransformViewIcon->Delete ( );
    this->MouseTransformViewIcon = NULL;
    }
  if ( this->MousePickIconLow )
    {
    this->MousePickIconLow->Delete ( );
    this->MousePickIconLow = NULL;
    }
  if ( this->MousePlaceFiducialIconLow )
    {
    this->MousePlaceFiducialIconLow->Delete ( );
    this->MousePlaceFiducialIconLow = NULL;
    }
  if ( this->MouseTransformViewIconLow )
    {
    this->MouseTransformViewIconLow->Delete ( );
    this->MouseTransformViewIconLow = NULL;
    }

  if ( this->UndoIcon )
    {
    this->UndoIcon->Delete ( );
    this->UndoIcon = NULL;
    }
  if ( this->RedoIcon )
    {
    this->RedoIcon->Delete ( );
    this->RedoIcon = NULL;
    }
}



//---------------------------------------------------------------------------
void vtkSlicerToolbarIcons::AssignImageDataToIcons ( )
{
    this->HomeIcon->SetImage ( image_ToolbarHome,
                               image_ToolbarHome_width,
                               image_ToolbarHome_height,
                               image_ToolbarHome_pixel_size,
                               image_ToolbarHome_length, 0 );

    this->DataIcon->SetImage( image_ToolbarData,
                              image_ToolbarData_width,
                              image_ToolbarData_height,
                              image_ToolbarData_pixel_size,
                              image_ToolbarData_length, 0 );                              

    this->VolumeIcon->SetImage( image_ToolbarVolume,
                                image_ToolbarVolume_width,
                                image_ToolbarVolume_height,
                                image_ToolbarVolume_pixel_size,
                                image_ToolbarVolume_length, 0 );                                

    this->ModelIcon->SetImage( image_ToolbarModel,
                               image_ToolbarModel_width,
                               image_ToolbarModel_height,
                               image_ToolbarModel_pixel_size,
                               image_ToolbarModel_length, 0 );
    
    // use the toolbox image data resource for the editor.
    this->EditorIcon->SetImage( image_ToolbarEditorToolbox,
                                image_ToolbarEditorToolbox_width,
                                image_ToolbarEditorToolbox_height,
                                image_ToolbarEditorToolbox_pixel_size,
                                image_ToolbarEditorToolbox_length, 0 );
    
/*
  // if we make an editor toolbox later, change its icon.
    this->EditorToolboxIcon->SetImage( image_ToolbarEditorToolbox,
                                       image_ToolbarEditorToolbox_width,
                                       image_ToolbarEditorToolbox_height,
                                       image_ToolbarEditorToolbox_pixel_size,
                                       image_ToolbarEditorToolbox_length, 0 );
*/    
    this->TransformIcon->SetImage( image_ToolbarTransform,
                               image_ToolbarTransform_width,
                               image_ToolbarTransform_height,
                               image_ToolbarTransform_pixel_size,
                                   image_ToolbarTransform_length, 0 );
    
    this->ColorIcon->SetImage( image_ToolbarColor,
                               image_ToolbarColor_width,
                               image_ToolbarColor_height,
                               image_ToolbarColor_pixel_size,
                               image_ToolbarColor_length, 0);
    
    this->FiducialsIcon->SetImage( image_ToolbarFiducials,
                                   image_ToolbarFiducials_width,
                                   image_ToolbarFiducials_height,
                                   image_ToolbarFiducials_pixel_size,
                                   image_ToolbarFiducials_length, 0 );
/*    
    this->MeasurementsIcon->SetImage( image_ToolbarMeasurements,
                                   image_ToolbarMeasurements_width,
                                   image_ToolbarMeasurements_height,
                                   image_ToolbarMeasurements_pixel_size,
                                   image_ToolbarMeasurements_length, 0 );
*/                                     

    this->SaveSceneIcon->SetImage( image_ToolbarSaveScene,
                                   image_ToolbarSaveScene_width,
                                   image_ToolbarSaveScene_height,
                                   image_ToolbarSaveScene_pixel_size,
                                   image_ToolbarSaveScene_length, 0 );
    
    this->LoadSceneIcon->SetImage( image_ToolbarLoadScene,
                                   image_ToolbarLoadScene_width,
                                   image_ToolbarLoadScene_height,
                                   image_ToolbarLoadScene_pixel_size,
                                   image_ToolbarLoadScene_length, 0 );
    
    this->ConventionalViewIcon->SetImage( image_ToolbarConventionalView,
                                          image_ToolbarConventionalView_width,
                                          image_ToolbarConventionalView_height,
                                          image_ToolbarConventionalView_pixel_size,
                                          image_ToolbarConventionalView_length, 0);

    this->OneUp3DViewIcon->SetImage( image_ToolbarOneUp3DView,
                                     image_ToolbarOneUp3DView_width,
                                     image_ToolbarOneUp3DView_height,
                                     image_ToolbarOneUp3DView_pixel_size,
                                     image_ToolbarOneUp3DView_length, 0 );                                     
                                     
    this->OneUpSliceViewIcon->SetImage( image_ToolbarOneUpSliceView,
                                        image_ToolbarOneUpSliceView_width,
                                        image_ToolbarOneUpSliceView_height,
                                        image_ToolbarOneUpSliceView_pixel_size,
                                        image_ToolbarOneUpSliceView_length, 0 );
    
    this->FourUpViewIcon->SetImage( image_ToolbarFourUpView,
                                    image_ToolbarFourUpView_width,
                                    image_ToolbarFourUpView_height,
                                    image_ToolbarFourUpView_pixel_size,
                                    image_ToolbarFourUpView_length, 0 );
    
    this->Tabbed3DViewIcon->SetImage( image_ToolbarTabbed3DView,
                                    image_ToolbarTabbed3DView_width,
                                    image_ToolbarTabbed3DView_height,
                                    image_ToolbarTabbed3DView_pixel_size,
                                      image_ToolbarTabbed3DView_length, 0 );
    
    this->TabbedSliceViewIcon->SetImage( image_ToolbarTabbedSliceView,
                                    image_ToolbarTabbedSliceView_width,
                                    image_ToolbarTabbedSliceView_height,
                                    image_ToolbarTabbedSliceView_pixel_size,
                                         image_ToolbarTabbedSliceView_length, 0 );

    this->LightBoxViewIcon->SetImage( image_ToolbarLightBoxView,
                                      image_ToolbarLightBoxView_width,
                                      image_ToolbarLightBoxView_height,
                                      image_ToolbarLightBoxView_pixel_size,
                                      image_ToolbarLightBoxView_length, 0);
    
    this->MousePickIcon->SetImage (image_ToolbarMousePick,
                                   image_ToolbarMousePick_width,
                                   image_ToolbarMousePick_height,
                                   image_ToolbarMousePick_pixel_size,
                                   image_ToolbarMousePick_length, 0);
    this->MouseTransformViewIcon->SetImage (image_ToolbarMouseRotate,
                                   image_ToolbarMouseRotate_width,
                                   image_ToolbarMouseRotate_height,
                                   image_ToolbarMouseRotate_pixel_size,
                                            image_ToolbarMouseRotate_length, 0);
    this->MousePlaceFiducialIcon->SetImage (image_ToolbarMousePlaceFiducial,
                                   image_ToolbarMousePlaceFiducial_width,
                                   image_ToolbarMousePlaceFiducial_height,
                                   image_ToolbarMousePlaceFiducial_pixel_size,
                                            image_ToolbarMousePlaceFiducial_length, 0);

    this->MousePickIconLow->SetImage (image_ToolbarMousePickLow,
                                   image_ToolbarMousePickLow_width,
                                   image_ToolbarMousePickLow_height,
                                   image_ToolbarMousePickLow_pixel_size,
                                   image_ToolbarMousePickLow_length, 0);
    this->MouseTransformViewIconLow->SetImage (image_ToolbarMouseRotateLow,
                                   image_ToolbarMouseRotateLow_width,
                                   image_ToolbarMouseRotateLow_height,
                                   image_ToolbarMouseRotateLow_pixel_size,
                                            image_ToolbarMouseRotate_length, 0);
    this->MousePlaceFiducialIconLow->SetImage (image_ToolbarMousePlaceFiducialLow,
                                   image_ToolbarMousePlaceFiducialLow_width,
                                   image_ToolbarMousePlaceFiducialLow_height,
                                   image_ToolbarMousePlaceFiducialLow_pixel_size,
                                            image_ToolbarMousePlaceFiducialLow_length, 0);

    this->UndoIcon->SetImage ( image_ToolbarUndo,
                               image_ToolbarUndo_width,
                               image_ToolbarUndo_height,
                               image_ToolbarUndo_pixel_size,
                               image_ToolbarUndo_length, 0);
    this->RedoIcon->SetImage ( image_ToolbarRedo,
                               image_ToolbarRedo_width,
                               image_ToolbarRedo_height,
                               image_ToolbarRedo_pixel_size,
                               image_ToolbarUndo_length, 0);

}



//---------------------------------------------------------------------------
void vtkSlicerToolbarIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerToolbarIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "HomeIcon" << this->GetHomeIcon ( ) << "\n";
    os << indent << "DataIcon" << this->GetDataIcon ( ) << "\n";        
    os << indent << "VolumeIcon" << this->GetVolumeIcon ( ) << "\n";
    os << indent << "ModelIcon" << this->GetModelIcon ( ) << "\n";
    os << indent << "EditorIcon" << this->GetEditorIcon ( ) << "\n";
//    os << indent << "EditorToolboxIcon" << this->GetEditorToolboxIcon ( ) << "\n";
    os << indent << "TransformIcon" << this->GetTransformIcon ( ) << "\n";    
    os << indent << "ColorIcon" << this->GetColorIcon ( ) << "\n";
    os << indent << "FiducialsIcon" << this->GetFiducialsIcon ( ) << "\n";
//    os << indent << "MeasurementsIcon" << this->GetMeasurementsIcon ( ) << "\n";
    os << indent << "SaveSceneIcon" << this->GetSaveSceneIcon ( ) << "\n";
    os << indent << "LoadSceneIcon" << this->GetLoadSceneIcon ( ) << "\n";
    os << indent << "ConventionalViewIcon" << this->GetConventionalViewIcon ( ) << "\n";
    os << indent << "OneUp3DViewIcon" << this->GetOneUp3DViewIcon ( ) << "\n";
    os << indent << "OneUpSliceViewIcon" << this->GetOneUpSliceViewIcon ( ) << "\n";
    os << indent << "FourUpViewIcon" << this->GetFourUpViewIcon ( ) << "\n";
    os << indent << "TabbedSliceViewIcon" << this->GetTabbedSliceViewIcon ( ) << "\n";
    os << indent << "Tabbed3DViewIcon" << this->GetTabbed3DViewIcon ( ) << "\n";
    os << indent << "LightBoxViewIcon" << this->GetLightBoxViewIcon ( ) << "\n";
    os << indent << "MousePickIcon" << this->GetMousePickIcon ( ) << "\n";
    os << indent << "MouseTransformViewIcon" << this->GetMouseTransformViewIcon ( ) << "\n";
    os << indent << "MousePlaceFiducialIcon" << this->GetMousePlaceFiducialIcon ( ) << "\n";
    os << indent << "MousePickIconLow" << this->GetMousePickIconLow ( ) << "\n";
    os << indent << "MouseTransformViewIconLow" << this->GetMouseTransformViewIconLow ( ) << "\n";
    os << indent << "MousePlaceFiducialIconLow" << this->GetMousePlaceFiducialIconLow ( ) << "\n";
    os << indent << "UndoIcon" << this->GetUndoIcon ( ) << "\n";
    os << indent << "RedoIcon" << this->GetRedoIcon ( ) << "\n";
}

