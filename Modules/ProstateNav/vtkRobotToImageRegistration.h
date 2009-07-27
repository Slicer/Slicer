/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.slicer.org/Slicer3/trunk/Modules/OpenIGTLinkIF/vtkIGTLToMRMLBase.h $
  Date:      $Date: 2009-01-05 13:28:20 -0500 (Mon, 05 Jan 2009) $
  Version:   $Revision: 8267 $

==========================================================================*/

#ifndef __vtkIGTLToMRMLBase_h
#define __vtkIGTLToMRMLBase_h

#include "vtkObject.h"
#include "vtkProstateNavWin32Header.h" 

#include "vtkImageData.h"
#include "vtkMatrix4x4.h"

#include <vector>
#include <string>

class VTK_PROSTATENAV_EXPORT vtkRobotToImageRegistration : public vtkObject
{

 public:

  static vtkRobotToImageRegistration *New();
  vtkTypeRevisionMacro(vtkRobotToImageRegistration,vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetObjectMacro ( FiducialImage, vtkImageData );
  vtkSetObjectMacro ( FiducialImage, vtkImageData );
  vtkGetObjectMacro ( RobotToImageTransformMatrix, vtkMatrix4x4 );
  vtkSetObjectMacro ( RobotToImageTransformMatrix, vtkMatrix4x4 );

  virtual int DoRegistration() {};

 protected:
  vtkRobotToImageRegistration();
  ~vtkRobotToImageRegistration();

 protected:

  vtkImageData* FiducialImage;
  vtkMatrix4x4* RobotToImageTransformMatrix;

};


#endif //__vtkRobotToImageRegistration_h
