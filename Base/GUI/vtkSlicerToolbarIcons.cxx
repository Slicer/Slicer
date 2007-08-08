
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
    this->EditorToolboxIcon = vtkKWIcon::New();
    this->TransformIcon = vtkKWIcon::New();
    this->ColorIcon = vtkKWIcon::New();
    this->FiducialsIcon = vtkKWIcon::New();
    //   this->MeasurementsIcon = vtkKWIcon::New();
    this->SaveSceneIcon = vtkKWIcon::New();
    this->LoadSceneIcon = vtkKWIcon::New();
    this->ChooseLayoutIcon = vtkKWIcon::New ( );
    this->ConventionalViewIcon = vtkKWIcon::New();
    this->OneUp3DViewIcon = vtkKWIcon::New();
    this->OneUpSliceViewIcon = vtkKWIcon::New();
    this->OneUpRedSliceViewIcon = vtkKWIcon::New();
    this->OneUpYellowSliceViewIcon = vtkKWIcon::New();
    this->OneUpGreenSliceViewIcon = vtkKWIcon::New();
    this->FourUpViewIcon = vtkKWIcon::New();
    this->TabbedSliceViewIcon = vtkKWIcon::New();
    this->Tabbed3DViewIcon = vtkKWIcon::New();
    this->LightBoxViewIcon = vtkKWIcon::New();

    this->MousePickOnIcon = vtkKWIcon::New();
    this->MousePickOffIcon = vtkKWIcon::New();
    this->MousePickDisabledIcon = vtkKWIcon::New();    
    this->MouseTransformViewOnIcon = vtkKWIcon::New();
    this->MouseTransformViewOffIcon = vtkKWIcon::New();
    this->MouseTransformViewDisabledIcon = vtkKWIcon::New();    
    this->MousePlaceOnIcon = vtkKWIcon::New ( );
    this->MousePlaceOffIcon = vtkKWIcon::New ( );
    this->MousePlaceDisabledIcon = vtkKWIcon::New ( );
    this->MouseManipulateOnIcon = vtkKWIcon::New ( );
    this->MouseManipulateOffIcon = vtkKWIcon::New ( );
    this->MouseManipulateDisabledIcon = vtkKWIcon::New ( );

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

  if ( this->EditorToolboxIcon )
    {
    this->EditorToolboxIcon->Delete ( );
    this->EditorToolboxIcon = NULL;
    }

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
  if ( this->ChooseLayoutIcon )
    {
    this->ChooseLayoutIcon->Delete();
    this->ChooseLayoutIcon = NULL;
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
  if ( this->OneUpRedSliceViewIcon )
    {
    this->OneUpRedSliceViewIcon->Delete ( );
    this->OneUpRedSliceViewIcon = NULL;
    }
  if ( this->OneUpYellowSliceViewIcon )
    {
    this->OneUpYellowSliceViewIcon->Delete ( );
    this->OneUpYellowSliceViewIcon = NULL;
    }
  if ( this->OneUpGreenSliceViewIcon )
    {
    this->OneUpGreenSliceViewIcon->Delete ( );
    this->OneUpGreenSliceViewIcon = NULL;
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
  if ( this->MousePickOnIcon )
    {
    this->MousePickOnIcon->Delete ( );
    this->MousePickOnIcon = NULL;
    }
  if ( this->MousePickOffIcon )
    {
    this->MousePickOffIcon->Delete ( );
    this->MousePickOffIcon = NULL;
    }
  if ( this->MousePickDisabledIcon )
    {
    this->MousePickDisabledIcon->Delete ( );
    this->MousePickDisabledIcon = NULL;
    }
  if ( this->MousePlaceOnIcon )
    {
    this->MousePlaceOnIcon->Delete ( );
    this->MousePlaceOnIcon = NULL;
    }
  if ( this->MousePlaceOffIcon )
    {
    this->MousePlaceOffIcon->Delete ( );
    this->MousePlaceOffIcon = NULL;
    }
  if ( this->MousePlaceDisabledIcon )
    {
    this->MousePlaceDisabledIcon->Delete ( );
    this->MousePlaceDisabledIcon = NULL;
    }
  if ( this->MouseManipulateOnIcon )
    {
    this->MouseManipulateOnIcon->Delete ( );
    this->MouseManipulateOnIcon = NULL;
    }
  if ( this->MouseManipulateOffIcon )
    {
    this->MouseManipulateOffIcon->Delete ( );
    this->MouseManipulateOffIcon = NULL;
    }
  if ( this->MouseManipulateDisabledIcon )
    {
    this->MouseManipulateDisabledIcon->Delete ( );
    this->MouseManipulateDisabledIcon = NULL;
    }
  if ( this->MouseTransformViewOnIcon )
    {
    this->MouseTransformViewOnIcon->Delete ( );
    this->MouseTransformViewOnIcon = NULL;
    }
  if ( this->MouseTransformViewOffIcon )
    {
    this->MouseTransformViewOffIcon->Delete ( );
    this->MouseTransformViewOffIcon = NULL;
    }
  if ( this->MouseTransformViewDisabledIcon )
    {
    this->MouseTransformViewDisabledIcon->Delete ( );
    this->MouseTransformViewDisabledIcon = NULL;
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
    
  // use the new toolbox image data for editor toolbox
    this->EditorToolboxIcon->SetImage( image_EditorToolbox,
                                       image_EditorToolbox_width,
                                       image_EditorToolbox_height,
                                       image_EditorToolbox_pixel_size,
                                       image_EditorToolbox_length, 0 );

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

    this->ChooseLayoutIcon->SetImage( image_ToolbarChooseView,
                                          image_ToolbarChooseView_width,
                                          image_ToolbarChooseView_height,
                                          image_ToolbarChooseView_pixel_size,
                                          image_ToolbarChooseView_length, 0);

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

    this->OneUpRedSliceViewIcon->SetImage( image_ToolbarOneUpRedSliceView,
                                        image_ToolbarOneUpRedSliceView_width,
                                        image_ToolbarOneUpRedSliceView_height,
                                        image_ToolbarOneUpRedSliceView_pixel_size,
                                        image_ToolbarOneUpRedSliceView_length, 0 );
    this->OneUpYellowSliceViewIcon->SetImage( image_ToolbarOneUpYellowSliceView,
                                        image_ToolbarOneUpYellowSliceView_width,
                                        image_ToolbarOneUpYellowSliceView_height,
                                        image_ToolbarOneUpYellowSliceView_pixel_size,
                                        image_ToolbarOneUpYellowSliceView_length, 0 );
    this->OneUpGreenSliceViewIcon->SetImage( image_ToolbarOneUpGreenSliceView,
                                        image_ToolbarOneUpGreenSliceView_width,
                                        image_ToolbarOneUpGreenSliceView_height,
                                        image_ToolbarOneUpGreenSliceView_pixel_size,
                                        image_ToolbarOneUpGreenSliceView_length, 0 );
    
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
    
    this->MousePickOnIcon->SetImage (image_MousePickOn,
                                   image_MousePickOn_width,
                                   image_MousePickOn_height,
                                   image_MousePickOn_pixel_size,
                                   image_MousePickOn_length, 0);
    this->MousePickOffIcon->SetImage (image_MousePickOff,
                                   image_MousePickOff_width,
                                   image_MousePickOff_height,
                                   image_MousePickOff_pixel_size,
                                   image_MousePickOff_length, 0);
    this->MousePickDisabledIcon->SetImage (image_MousePickDisabled,
                                   image_MousePickDisabled_width,
                                   image_MousePickDisabled_height,
                                   image_MousePickDisabled_pixel_size,
                                   image_MousePickDisabled_length, 0);


    this->MouseTransformViewOnIcon->SetImage (image_MouseRotateOn,
                                   image_MouseRotateOn_width,
                                   image_MouseRotateOn_height,
                                   image_MouseRotateOn_pixel_size,
                                            image_MouseRotateOn_length, 0);
    this->MouseTransformViewOffIcon->SetImage (image_MouseRotateOff,
                                   image_MouseRotateOff_width,
                                   image_MouseRotateOff_height,
                                   image_MouseRotateOff_pixel_size,
                                            image_MouseRotateOff_length, 0);
    this->MouseTransformViewDisabledIcon->SetImage (image_MouseRotateDisabled,
                                   image_MouseRotateDisabled_width,
                                   image_MouseRotateDisabled_height,
                                   image_MouseRotateDisabled_pixel_size,
                                            image_MouseRotateDisabled_length, 0);
    
    this->MousePlaceOnIcon->SetImage (image_MousePlaceOn,
                                   image_MousePlaceOn_width,
                                   image_MousePlaceOn_height,
                                   image_MousePlaceOn_pixel_size,
                                            image_MousePlaceOn_length, 0);
    this->MousePlaceOffIcon->SetImage (image_MousePlaceOff,
                                   image_MousePlaceOff_width,
                                   image_MousePlaceOff_height,
                                   image_MousePlaceOff_pixel_size,
                                            image_MousePlaceOff_length, 0);
    this->MousePlaceDisabledIcon->SetImage (image_MousePlaceDisabled,
                                   image_MousePlaceDisabled_width,
                                   image_MousePlaceDisabled_height,
                                   image_MousePlaceDisabled_pixel_size,
                                            image_MousePlaceDisabled_length, 0);

    this->MouseManipulateOnIcon->SetImage (image_MouseManipulateOn,
                                   image_MouseManipulateOn_width,
                                   image_MouseManipulateOn_height,
                                   image_MouseManipulateOn_pixel_size,
                                            image_MouseManipulateOn_length, 0);
    this->MouseManipulateOffIcon->SetImage (image_MouseManipulateOff,
                                   image_MouseManipulateOff_width,
                                   image_MouseManipulateOff_height,
                                   image_MouseManipulateOff_pixel_size,
                                            image_MouseManipulateOff_length, 0);
    this->MouseManipulateDisabledIcon->SetImage (image_MouseManipulateDisabled,
                                   image_MouseManipulateDisabled_width,
                                   image_MouseManipulateDisabled_height,
                                   image_MouseManipulateDisabled_pixel_size,
                                            image_MouseManipulateDisabled_length, 0);

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
    os << indent << "EditorToolboxIcon" << this->GetEditorToolboxIcon ( ) << "\n";
    os << indent << "TransformIcon" << this->GetTransformIcon ( ) << "\n";    
    os << indent << "ColorIcon" << this->GetColorIcon ( ) << "\n";
    os << indent << "FiducialsIcon" << this->GetFiducialsIcon ( ) << "\n";
//    os << indent << "MeasurementsIcon" << this->GetMeasurementsIcon ( ) << "\n";
    os << indent << "SaveSceneIcon" << this->GetSaveSceneIcon ( ) << "\n";
    os << indent << "LoadSceneIcon" << this->GetLoadSceneIcon ( ) << "\n";
    os << indent << "ConventionalViewIcon" << this->GetConventionalViewIcon ( ) << "\n";
    os << indent << "ChooseLayoutIcon" << this->GetChooseLayoutIcon ( ) << "\n";
    os << indent << "OneUp3DViewIcon" << this->GetOneUp3DViewIcon ( ) << "\n";
    os << indent << "OneUpSliceViewIcon" << this->GetOneUpSliceViewIcon ( ) << "\n";
    os << indent << "OneUpRedSliceViewIcon" << this->GetOneUpRedSliceViewIcon ( ) << "\n";
    os << indent << "OneUpYellowSliceViewIcon" << this->GetOneUpYellowSliceViewIcon ( ) << "\n";
    os << indent << "OneUpGreenSliceViewIcon" << this->GetOneUpGreenSliceViewIcon ( ) << "\n";
    os << indent << "FourUpViewIcon" << this->GetFourUpViewIcon ( ) << "\n";
    os << indent << "TabbedSliceViewIcon" << this->GetTabbedSliceViewIcon ( ) << "\n";
    os << indent << "Tabbed3DViewIcon" << this->GetTabbed3DViewIcon ( ) << "\n";
    os << indent << "LightBoxViewIcon" << this->GetLightBoxViewIcon ( ) << "\n";

    os << indent << "MousePickOnIcon" << this->GetMousePickOnIcon ( ) << "\n";
    os << indent << "MousePickOffIcon" << this->GetMousePickOffIcon ( ) << "\n";
    os << indent << "MousePickDisabledIcon" << this->GetMousePickDisabledIcon ( ) << "\n";

    os << indent << "MouseTransformViewOnIcon" << this->GetMouseTransformViewOnIcon ( ) << "\n";
    os << indent << "MouseTransformViewOffIcon" << this->GetMouseTransformViewOffIcon ( ) << "\n";
    os << indent << "MouseTransformViewDisabledIcon" << this->GetMouseTransformViewDisabledIcon ( ) << "\n";
    
    os << indent << "MousePlaceOnIcon" << this->GetMousePlaceOnIcon ( ) << "\n";
    os << indent << "MousePlaceOffIcon" << this->GetMousePlaceOffIcon ( ) << "\n";
    os << indent << "MousePlaceDisabledIcon" << this->GetMousePlaceDisabledIcon ( ) << "\n";
    
    os << indent << "MouseManipulateOnIcon" << this->GetMouseManipulateOnIcon ( ) << "\n";
    os << indent << "MouseManipulateOffIcon" << this->GetMouseManipulateOffIcon ( ) << "\n";
    os << indent << "MouseManipulateDisabledIcon" << this->GetMouseManipulateDisabledIcon ( ) << "\n";

    os << indent << "UndoIcon" << this->GetUndoIcon ( ) << "\n";
    os << indent << "RedoIcon" << this->GetRedoIcon ( ) << "\n";
}

