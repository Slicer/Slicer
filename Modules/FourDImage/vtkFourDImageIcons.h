/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFourDImageIcons.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkFourDImageIcons_h
#define __vtkFourDImageIcons_h

#include "vtkFourDImageWin32Header.h"
#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "vtkFourDImageIcons.h"

#include "./Resources/vtkFourDImage_ImageData.h"

class VTK_FourDImage_EXPORT vtkFourDImageIcons : public vtkSlicerIcons
{
 public:
    static vtkFourDImageIcons* New ( );
    vtkTypeRevisionMacro ( vtkFourDImageIcons, vtkSlicerIcons );
    void PrintSelf ( ostream& os, vtkIndent indent );

    // Description:
    // Get toolbar vtkKWIcons
    vtkGetObjectMacro (PlayIcon, vtkKWIcon);
    vtkGetObjectMacro (PauseIcon, vtkKWIcon);
    virtual void AssignImageDataToIcons ( );
    
 protected:
    vtkFourDImageIcons ( );
    virtual ~vtkFourDImageIcons ( );
    
    vtkKWIcon *PlayIcon;
    vtkKWIcon *PauseIcon;

 private:
    vtkFourDImageIcons (const vtkFourDImageIcons&); // Not implemented
    void operator = ( const vtkFourDImageIcons& ); // Not implemented
    
};

#endif



