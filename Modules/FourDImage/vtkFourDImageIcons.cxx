/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFourDImageIcons.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkFourDImageIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkFourDImageIcons );
vtkCxxRevisionMacro ( vtkFourDImageIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkFourDImageIcons::vtkFourDImageIcons ( )
{
        //--- window level preset icons
  this->PlayIcon = vtkKWIcon::New();
  this->PauseIcon = vtkKWIcon::New();
  
  // now generate the icons from images
  this->AssignImageDataToIcons ( );
}

//---------------------------------------------------------------------------
vtkFourDImageIcons::~vtkFourDImageIcons ( )
{
  // window level presets icons
  if ( this->PlayIcon )
    {
    this->PlayIcon->Delete ( );
    this->PlayIcon = NULL;
    }
  if ( this->PauseIcon )
    {
    this->PauseIcon->Delete ( );
    this->PauseIcon = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkFourDImageIcons::AssignImageDataToIcons ( )
{
  this->PlayIcon->SetImage ( image_FourDImage_Play,
                             image_FourDImage_Play_width,
                             image_FourDImage_Play_height,
                             image_FourDImage_Play_pixel_size,
                             image_FourDImage_Play_length, 0 );
  this->PauseIcon->SetImage ( image_FourDImage_Pause,
                             image_FourDImage_Pause_width,
                             image_FourDImage_Pause_height,
                             image_FourDImage_Pause_pixel_size,
                             image_FourDImage_Pause_length, 0 );

}

//---------------------------------------------------------------------------
void vtkFourDImageIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "FourDImageIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "PlayIcon" << this->GetPlayIcon ( ) << "\n";
    os << indent << "PauseIcon" << this->GetPauseIcon ( ) << "\n";

}
