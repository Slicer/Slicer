
#include "vtkObjectFactory.h"
#include "vtkSlicerDiffusionTensorIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerDiffusionTensorIcons );

//---------------------------------------------------------------------------
vtkSlicerDiffusionTensorIcons::vtkSlicerDiffusionTensorIcons ( )
{
  
  this->LineIcon = vtkKWIcon::New();
  this->TubeIcon = vtkKWIcon::New();
  this->GlyphIcon = vtkKWIcon::New();
  
  this->AssignImageDataToIcons ( );
}



//---------------------------------------------------------------------------
vtkSlicerDiffusionTensorIcons::~vtkSlicerDiffusionTensorIcons ( )
{

  if ( this->LineIcon )
    {
    this->LineIcon->Delete ( );
    this->LineIcon = NULL;
    }
  if ( this->TubeIcon )
    {
    this->TubeIcon->Delete ( );
    this->TubeIcon = NULL;
    }
  if ( this->GlyphIcon )
    {
    this->GlyphIcon->Delete ( );
    this->GlyphIcon = NULL;
    }

}



//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorIcons::AssignImageDataToIcons ( )
{
    this->LineIcon->SetImage ( image_DTIlinesHigh,
                               image_DTIlinesHigh_width,
                               image_DTIlinesHigh_height,
                               image_DTIlinesHigh_pixel_size,
                               image_DTIlinesHigh_length, 0 );

    this->TubeIcon->SetImage( image_DTItubesHigh,
                              image_DTItubesHigh_width,
                              image_DTItubesHigh_height,
                              image_DTItubesHigh_pixel_size,
                              image_DTItubesHigh_length, 0 );                              

    this->GlyphIcon->SetImage( image_DTIglyphsHigh,
                                image_DTIglyphsHigh_width,
                                image_DTIglyphsHigh_height,
                                image_DTIglyphsHigh_pixel_size,
                                image_DTIglyphsHigh_length, 0 );                                

}



//---------------------------------------------------------------------------
void vtkSlicerDiffusionTensorIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerDiffusionTensorIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "LineIcon" << this->GetLineIcon ( ) << "\n";
    os << indent << "TubeIcon" << this->GetTubeIcon ( ) << "\n";        
    os << indent << "GlyphIcon" << this->GetGlyphIcon ( ) << "\n";

}

