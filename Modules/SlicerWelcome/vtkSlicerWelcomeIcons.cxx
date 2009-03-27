
#include "vtkObjectFactory.h"
#include "vtkSlicerWelcomeIcons.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerWelcomeIcons );
vtkCxxRevisionMacro ( vtkSlicerWelcomeIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerWelcomeIcons::vtkSlicerWelcomeIcons ( )
{
    this->LogoIcon = vtkKWIcon::New();
    this->CommunityIcon = vtkKWIcon::New();
    this->MapIcon = vtkKWIcon::New();
    this->EditorIcon = vtkKWIcon::New();
    this->HelpIcon = vtkKWIcon::New();
    this->LoadIcon = vtkKWIcon::New();
    this->SaveIcon = vtkKWIcon::New();
    this->ModelsIcon = vtkKWIcon::New();
    this->FiducialsIcon = vtkKWIcon::New();
    this->SceneIcon = vtkKWIcon::New();
    this->SceneSnapshotsIcon = vtkKWIcon::New();
    this->SceneTreeIcon = vtkKWIcon::New();
    this->VolumesIcon = vtkKWIcon::New();

    this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkSlicerWelcomeIcons::~vtkSlicerWelcomeIcons ( )
{
    if ( this->LogoIcon)
      {
      this->LogoIcon->Delete();
      this->LogoIcon = NULL;
      }
   if ( this->CommunityIcon)
      {
      this->CommunityIcon->Delete();
      this->CommunityIcon = NULL;
      }
    if ( this->MapIcon)
      {
      this->MapIcon->Delete();
      this->MapIcon = NULL;
      }
    if ( this->EditorIcon)
      {
      this->EditorIcon->Delete();
      this->EditorIcon = NULL;
      }
    if ( this->HelpIcon)
      {
      this->HelpIcon->Delete();
      this->HelpIcon = NULL;
      }
    if ( this->LoadIcon)
      {
      this->LoadIcon->Delete();
      this->LoadIcon = NULL;
      }
    if ( this->SaveIcon)
      {
      this->SaveIcon->Delete();
      this->SaveIcon = NULL;
      }
    if ( this->FiducialsIcon)
      {
      this->FiducialsIcon->Delete();
      this->FiducialsIcon = NULL;
      }
    if ( this->ModelsIcon)
      {
      this->ModelsIcon->Delete();
      this->ModelsIcon = NULL;
      }
    if ( this->SceneIcon)
      {
      this->SceneIcon->Delete();
      this->SceneIcon = NULL;
      }
    if ( this->SceneSnapshotsIcon)
      {
      this->SceneSnapshotsIcon->Delete();
      this->SceneSnapshotsIcon = NULL;
      }
    if ( this->SceneTreeIcon)
      {
      this->SceneTreeIcon->Delete();
      this->SceneTreeIcon = NULL;
      }
    if ( this->VolumesIcon)
      {
      this->VolumesIcon->Delete();
      this->VolumesIcon = NULL;
      }
}

//---------------------------------------------------------------------------
void vtkSlicerWelcomeIcons::AssignImageDataToIcons ( )
{

  this->LogoIcon->SetImage ( image_WelcomeLogo,
                             image_WelcomeLogo_width,
                             image_WelcomeLogo_height,
                             image_WelcomeLogo_pixel_size,
                             image_WelcomeLogo_length, 0);
  this->CommunityIcon->SetImage ( image_WelcomeCommunity,
                        image_WelcomeCommunity_width,
                        image_WelcomeCommunity_height,
                        image_WelcomeCommunity_pixel_size,
                        image_WelcomeCommunity_length, 0);
  this->MapIcon->SetImage (image_WelcomeMap,
                           image_WelcomeMap_width,
                           image_WelcomeMap_height,
                           image_WelcomeMap_pixel_size,
                           image_WelcomeMap_length, 0);
  this->EditorIcon->SetImage (image_WelcomeEditor,
                              image_WelcomeEditor_width,
                              image_WelcomeEditor_height,
                              image_WelcomeEditor_pixel_size,
                              image_WelcomeEditor_length, 0);
  this->HelpIcon->SetImage ( image_WelcomeHelp,
                             image_WelcomeHelp_width,
                             image_WelcomeHelp_height,
                             image_WelcomeHelp_pixel_size,
                             image_WelcomeHelp_length, 0);
  this->LoadIcon->SetImage (image_WelcomeLoad,
                                image_WelcomeLoad_width,
                                image_WelcomeLoad_height,
                                image_WelcomeLoad_pixel_size,
                                image_WelcomeLoad_length, 0);
  this->SaveIcon->SetImage (image_WelcomeSave,
                                image_WelcomeSave_width,
                                image_WelcomeSave_height,
                                image_WelcomeSave_pixel_size,
                                image_WelcomeSave_length, 0);
  this->ModelsIcon->SetImage (image_WelcomeModels,
                               image_WelcomeModels_width,
                               image_WelcomeModels_height,
                               image_WelcomeModels_pixel_size,
                               image_WelcomeModels_length, 0);
  this->SceneIcon->SetImage (image_WelcomeScene,
                             image_WelcomeScene_width,
                             image_WelcomeScene_height,
                             image_WelcomeScene_pixel_size,
                             image_WelcomeScene_length, 0);                             
  this->SceneSnapshotsIcon->SetImage (image_WelcomeSceneSnapshots,
                            image_WelcomeSceneSnapshots_width,
                            image_WelcomeSceneSnapshots_height,
                            image_WelcomeSceneSnapshots_pixel_size,
                            image_WelcomeSceneSnapshots_length, 0);
  this->SceneTreeIcon->SetImage (image_WelcomeSceneTree,
                                 image_WelcomeSceneTree_width,
                                 image_WelcomeSceneTree_height,
                                 image_WelcomeSceneTree_pixel_size,
                                 image_WelcomeSceneTree_length, 0);
  this->FiducialsIcon->SetImage (image_WelcomeFiducials,
                                 image_WelcomeFiducials_width,
                                 image_WelcomeFiducials_height,
                                 image_WelcomeFiducials_pixel_size,
                                 image_WelcomeFiducials_length, 0);
  this->VolumesIcon->SetImage (image_WelcomeVolumes,
                               image_WelcomeVolumes_width,
                               image_WelcomeVolumes_height,
                               image_WelcomeVolumes_pixel_size,
                               image_WelcomeVolumes_length, 0);
}



//---------------------------------------------------------------------------
void vtkSlicerWelcomeIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerWelcomeIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "LogoIcon: " << this->GetLogoIcon() << "\n";
    os << indent << "CommunityIcon: " << this->GetCommunityIcon() << "\n";
    os << indent << "MapIcon: " << this->GetMapIcon() << "\n";
    os << indent << "EditorIcon: " << this->GetEditorIcon() << "\n";
    os << indent << "HelpIcon: " << this->GetHelpIcon() << "\n";
    os << indent << "SaveIcon: " << this->GetSaveIcon() << "\n";
    os << indent << "LoadIcon: " << this->GetLoadIcon() << "\n";
    os << indent << "FiducialsIcon: " << this->GetFiducialsIcon() << "\n";
    os << indent << "ModelsIcon: " << this->GetModelsIcon() << "\n";
    os << indent << "SceneIcon: " << this->GetSceneIcon() << "\n";
    os << indent << "SceneSnapshotsIcon: " << this->GetSceneSnapshotsIcon() << "\n";
    os << indent << "SceneTreeIcon: " << this->GetSceneTreeIcon() << "\n";
    os << indent << "VolumesIcon: " << this->GetVolumesIcon() << "\n";
}

