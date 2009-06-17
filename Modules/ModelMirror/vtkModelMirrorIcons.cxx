#include "vtkObjectFactory.h"
#include "vtkModelMirrorIcons.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkModelMirrorIcons );
vtkCxxRevisionMacro ( vtkModelMirrorIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkModelMirrorIcons::vtkModelMirrorIcons ( )
{
  this->AxialLOIcon = vtkKWIcon::New();
  this->SaggitalLOIcon = vtkKWIcon::New();
  this->CoronalLOIcon = vtkKWIcon::New();
  this->AxialHIIcon = vtkKWIcon::New();
  this->SaggitalHIIcon = vtkKWIcon::New();
  this->CoronalHIIcon = vtkKWIcon::New();
  
  this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkModelMirrorIcons::~vtkModelMirrorIcons ( )
{

  if ( this->AxialLOIcon )
    {
    this->AxialLOIcon->Delete();
    this->AxialLOIcon = NULL;
    }
  if ( this->SaggitalLOIcon)
    {
    this->SaggitalLOIcon->Delete();
    this->SaggitalLOIcon = NULL;
    }
  if ( this->CoronalLOIcon)
    {
    this->CoronalLOIcon->Delete();
    this->CoronalLOIcon = NULL;
    }
  if ( this->AxialHIIcon )
    {
    this->AxialHIIcon->Delete();
    this->AxialHIIcon = NULL;
    }
  if ( this->SaggitalHIIcon)
    {
    this->SaggitalHIIcon->Delete();
    this->SaggitalHIIcon = NULL;
    }
  if ( this->CoronalHIIcon)
    {
    this->CoronalHIIcon->Delete();
    this->CoronalHIIcon = NULL;
    }

}

//---------------------------------------------------------------------------
void vtkModelMirrorIcons::AssignImageDataToIcons ( )
{

  this->AxialLOIcon->SetImage ( image_AxialMirrorLO,
                              image_AxialMirrorLO_width,
                              image_AxialMirrorLO_height,
                              image_AxialMirrorLO_pixel_size,
                              image_AxialMirrorLO_length, 0);

  this->SaggitalLOIcon->SetImage (image_SaggitalMirrorLO,
                                       image_SaggitalMirrorLO_width,
                                       image_SaggitalMirrorLO_height,
                                       image_SaggitalMirrorLO_pixel_size,
                                       image_SaggitalMirrorLO_length, 0);

  this->CoronalLOIcon->SetImage (image_CoronalMirrorLO,
                              image_CoronalMirrorLO_width,
                              image_CoronalMirrorLO_height,
                              image_CoronalMirrorLO_pixel_size,
                              image_CoronalMirrorLO_length, 0 );

  this->AxialHIIcon->SetImage ( image_AxialMirrorHI,
                              image_AxialMirrorHI_width,
                              image_AxialMirrorHI_height,
                              image_AxialMirrorHI_pixel_size,
                              image_AxialMirrorHI_length, 0);

  this->SaggitalHIIcon->SetImage (image_SaggitalMirrorHI,
                                       image_SaggitalMirrorHI_width,
                                       image_SaggitalMirrorHI_height,
                                       image_SaggitalMirrorHI_pixel_size,
                                       image_SaggitalMirrorHI_length, 0);

  this->CoronalHIIcon->SetImage (image_CoronalMirrorHI,
                              image_CoronalMirrorHI_width,
                              image_CoronalMirrorHI_height,
                              image_CoronalMirrorHI_pixel_size,
                              image_CoronalMirrorHI_length, 0 );

}



//---------------------------------------------------------------------------
void vtkModelMirrorIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "AxialLOIcon: " << this->GetAxialLOIcon() << "\n";
    os << indent << "SaggitalLOIcon: " << this->GetSaggitalLOIcon() << "\n";
    os << indent << "CoronalLOIcon: " << this->GetCoronalLOIcon() << "\n";
    os << indent << "AxialHIIcon: " << this->GetAxialHIIcon() << "\n";
    os << indent << "SaggitalHIIcon: " << this->GetSaggitalHIIcon() << "\n";
    os << indent << "CoronalHIIcon: " << this->GetCoronalHIIcon() << "\n";


}
