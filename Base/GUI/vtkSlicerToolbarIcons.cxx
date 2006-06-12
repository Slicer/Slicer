
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
    this->AlignIcon = vtkKWIcon::New();
    this->ColorIcon = vtkKWIcon::New();
    this->FiducialsIcon = vtkKWIcon::New();
    this->SaveSceneIcon = vtkKWIcon::New();
    this->LoadSceneIcon = vtkKWIcon::New();
    this->ConventionalViewIcon = vtkKWIcon::New();
    this->OneUp3DViewIcon = vtkKWIcon::New();
    this->OneUpSliceViewIcon = vtkKWIcon::New();
    this->FourUpViewIcon = vtkKWIcon::New();
    this->TabbedViewIcon = vtkKWIcon::New();
    this->LightBoxViewIcon = vtkKWIcon::New();
    this->AssignImageDataToIcons ( );
}



//---------------------------------------------------------------------------
vtkSlicerToolbarIcons::~vtkSlicerToolbarIcons ( )
{
    // toolbar icons
    if ( this->HomeIcon ) {
        this->HomeIcon->Delete ( );
        this->HomeIcon = NULL;
    }
    if ( this->DataIcon ) {
        this->DataIcon->Delete ( );
        this->DataIcon = NULL;
    }
    if ( this->VolumeIcon ) {
        this->VolumeIcon->Delete ( );
        this->VolumeIcon = NULL;
    }
    if ( this->ModelIcon ) {
        this->ModelIcon->Delete ( );
        this->ModelIcon = NULL;
    }
    if ( this->EditorIcon ) {
        this->EditorIcon->Delete ( );
        this->EditorIcon = NULL;
    }
    if ( this->EditorToolboxIcon ) {
        this->EditorToolboxIcon->Delete ( );
        this->EditorToolboxIcon = NULL;
    }
    if ( this->AlignIcon ) {
        this->AlignIcon->Delete ( );
        this->AlignIcon = NULL;
    }
    if ( this->ColorIcon ) {
        this->ColorIcon->Delete ( );
        this->ColorIcon = NULL;
    }
    if ( this->FiducialsIcon ) {
        this->FiducialsIcon->Delete ( );
        this->FiducialsIcon = NULL;
    }
    if ( this->SaveSceneIcon ) {
        this->SaveSceneIcon->Delete ( );
        this->SaveSceneIcon = NULL;
    }
    if ( this->LoadSceneIcon ) {
        this->LoadSceneIcon->Delete ( );
        this->LoadSceneIcon = NULL;
    }
    if ( this->ConventionalViewIcon ) {
        this->ConventionalViewIcon->Delete ( );
        this->ConventionalViewIcon = NULL;
    }
    if ( this->OneUp3DViewIcon ) {
        this->OneUp3DViewIcon->Delete ( );
        this->OneUp3DViewIcon = NULL;
    }
    if ( this->OneUpSliceViewIcon ) {
        this->OneUpSliceViewIcon->Delete ( );
        this->OneUpSliceViewIcon = NULL;
    }
    if ( this->FourUpViewIcon ) {
        this->FourUpViewIcon->Delete ( );
        this->FourUpViewIcon = NULL;
    }
    if ( this->TabbedViewIcon ) {
        this->TabbedViewIcon->Delete ( );
        this->TabbedViewIcon = NULL;
    }
    if ( this->LightBoxViewIcon ) {
        this->LightBoxViewIcon->Delete ( );
        this->LightBoxViewIcon = NULL;
    }
    
}



//---------------------------------------------------------------------------
void vtkSlicerToolbarIcons::AssignImageDataToIcons ( )
{
    this->HomeIcon->SetImage ( image_ToolbarHome,
                               image_ToolbarHome_width,
                               image_ToolbarHome_height,
                               image_ToolbarHome_pixel_size, 0, 0 );

    this->DataIcon->SetImage( image_ToolbarData,
                              image_ToolbarData_width,
                              image_ToolbarData_height,
                              image_ToolbarData_pixel_size, 0, 0 );                              

    this->VolumeIcon->SetImage( image_ToolbarVolume,
                                image_ToolbarVolume_width,
                                image_ToolbarVolume_height,
                                image_ToolbarVolume_pixel_size, 0, 0 );                                

    this->ModelIcon->SetImage( image_ToolbarModel,
                               image_ToolbarModel_width,
                               image_ToolbarModel_height,
                               image_ToolbarModel_pixel_size, 0, 0 );
    
    this->EditorIcon->SetImage( image_ToolbarEditor,
                                image_ToolbarEditor_width,
                                image_ToolbarEditor_height,
                                image_ToolbarEditor_pixel_size, 0, 0 );
    
    this->EditorToolboxIcon->SetImage( image_ToolbarEditorToolbox,
                                       image_ToolbarEditorToolbox_width,
                                       image_ToolbarEditorToolbox_height,
                                       image_ToolbarEditorToolbox_pixel_size, 0, 0 );
    
    this->AlignIcon->SetImage( image_ToolbarAlign,
                               image_ToolbarAlign_width,
                               image_ToolbarAlign_height,
                               image_ToolbarAlign_pixel_size, 0, 0 );
    
    this->ColorIcon->SetImage( image_ToolbarColor,
                               image_ToolbarColor_width,
                               image_ToolbarColor_height,
                               image_ToolbarColor_pixel_size, 0, 0);
    
    this->FiducialsIcon->SetImage( image_ToolbarFiducials,
                                   image_ToolbarFiducials_width,
                                   image_ToolbarFiducials_height,
                                   image_ToolbarFiducials_pixel_size, 0, 0 );
    
    this->SaveSceneIcon->SetImage( image_ToolbarSaveScene,
                                   image_ToolbarSaveScene_width,
                                   image_ToolbarSaveScene_height,
                                   image_ToolbarSaveScene_pixel_size, 0, 0 );
    
    this->LoadSceneIcon->SetImage( image_ToolbarLoadScene,
                                   image_ToolbarLoadScene_width,
                                   image_ToolbarLoadScene_height,
                                   image_ToolbarLoadScene_pixel_size, 0, 0 );
    
    this->ConventionalViewIcon->SetImage( image_ToolbarConventionalView,
                                          image_ToolbarConventionalView_width,
                                          image_ToolbarConventionalView_height,
                                          image_ToolbarConventionalView_pixel_size, 0, 0);

    this->OneUp3DViewIcon->SetImage( image_ToolbarOneUp3DView,
                                     image_ToolbarOneUp3DView_width,
                                     image_ToolbarOneUp3DView_height,
                                     image_ToolbarOneUp3DView_pixel_size, 0, 0 );                                     
                                     
    this->OneUpSliceViewIcon->SetImage( image_ToolbarOneUpSliceView,
                                        image_ToolbarOneUpSliceView_width,
                                        image_ToolbarOneUpSliceView_height,
                                        image_ToolbarOneUpSliceView_pixel_size, 0, 0 );
    
    this->FourUpViewIcon->SetImage( image_ToolbarFourUpView,
                                    image_ToolbarFourUpView_width,
                                    image_ToolbarFourUpView_height,
                                    image_ToolbarFourUpView_pixel_size, 0, 0 );
    
    this->TabbedViewIcon->SetImage( image_ToolbarTabbedView,
                                    image_ToolbarTabbedView_width,
                                    image_ToolbarTabbedView_height,
                                    image_ToolbarTabbedView_pixel_size, 0 , 0 );
    
    this->LightBoxViewIcon->SetImage( image_ToolbarLightBoxView,
                                      image_ToolbarLightBoxView_width,
                                      image_ToolbarLightBoxView_height,
                                      image_ToolbarLightBoxView_pixel_size, 0, 0);                                      
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
    os << indent << "AlignIcon" << this->GetAlignIcon ( ) << "\n";    
    os << indent << "ColorIcon" << this->GetColorIcon ( ) << "\n";
    os << indent << "FiducialsIcon" << this->GetFiducialsIcon ( ) << "\n";
    os << indent << "SaveSceneIcon" << this->GetSaveSceneIcon ( ) << "\n";
    os << indent << "LoadSceneIcon" << this->GetLoadSceneIcon ( ) << "\n";
    os << indent << "ConventionalViewIcon" << this->GetConventionalViewIcon ( ) << "\n";
    os << indent << "OneUp3DViewIcon" << this->GetOneUp3DViewIcon ( ) << "\n";
    os << indent << "OneUpSliceViewIcon" << this->GetOneUpSliceViewIcon ( ) << "\n";
    os << indent << "FourUpViewIcon" << this->GetFourUpViewIcon ( ) << "\n";
    os << indent << "TabbedViewIcon" << this->GetTabbedViewIcon ( ) << "\n";
    os << indent << "LightBoxViewIcon" << this->GetLightBoxViewIcon ( ) << "\n";
}

