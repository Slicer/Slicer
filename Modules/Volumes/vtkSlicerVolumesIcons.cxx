
#include "vtkObjectFactory.h"
#include "vtkSlicerVolumesIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerVolumesIcons );
vtkCxxRevisionMacro ( vtkSlicerVolumesIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerVolumesIcons::vtkSlicerVolumesIcons ( )
{
        //--- window level preset icons
  this->WindowLevelPresetCTAirIcon = vtkKWIcon::New();
  this->WindowLevelPresetCTBoneIcon = vtkKWIcon::New();
  this->WindowLevelPresetPETIcon = vtkKWIcon::New();

  // now generate the icons from images
  this->AssignImageDataToIcons ( );
}

//---------------------------------------------------------------------------
vtkSlicerVolumesIcons::~vtkSlicerVolumesIcons ( )
{
  // window level presets icons
  if ( this->WindowLevelPresetCTAirIcon )
    {
    this->WindowLevelPresetCTAirIcon->Delete ( );
    this->WindowLevelPresetCTAirIcon = NULL;
    }
  if ( this->WindowLevelPresetCTBoneIcon )
    {
    this->WindowLevelPresetCTBoneIcon->Delete ( );
    this->WindowLevelPresetCTBoneIcon = NULL;
    }
  if ( this->WindowLevelPresetPETIcon )
    {
    this->WindowLevelPresetPETIcon->Delete ( );
    this->WindowLevelPresetPETIcon = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumesIcons::AssignImageDataToIcons ( )
{
    this->WindowLevelPresetCTAirIcon->SetImage ( image_WindowLevelPreset_CT_air,
                               image_WindowLevelPreset_CT_air_width,
                               image_WindowLevelPreset_CT_air_height,
                               image_WindowLevelPreset_CT_air_pixel_size,
                               image_WindowLevelPreset_CT_air_length, 0 );
    this->WindowLevelPresetCTBoneIcon->SetImage ( image_WindowLevelPreset_CT_bone,
                               image_WindowLevelPreset_CT_bone_width,
                               image_WindowLevelPreset_CT_bone_height,
                               image_WindowLevelPreset_CT_bone_pixel_size,
                               image_WindowLevelPreset_CT_bone_length, 0 );
    this->WindowLevelPresetPETIcon->SetImage ( image_WindowLevelPreset_PET,
                               image_WindowLevelPreset_PET_width,
                               image_WindowLevelPreset_PET_height,
                               image_WindowLevelPreset_PET_pixel_size,
                               image_WindowLevelPreset_PET_length, 0 );

}

//---------------------------------------------------------------------------
void vtkSlicerVolumesIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerVolumesIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "WindowLevelPresetCTAirIcon" << this->GetWindowLevelPresetCTAirIcon ( ) << "\n";
    os << indent << "WindowLevelPresetCTBoneIcon" << this->GetWindowLevelPresetCTBoneIcon ( ) << "\n";
    os << indent << "WindowLevelPresetPETIcon" << this->GetWindowLevelPresetPETIcon ( ) << "\n";

}
