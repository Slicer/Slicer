/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Modules/OpenIGTLinkIF/vtkIGTLToMRMLBase.h $
  Date:      $Date: 2009-01-05 13:28:20 -0500 (Mon, 05 Jan 2009) $
  Version:   $Revision: 8267 $

==========================================================================*/

#ifndef __vtkRobotToImageRegistration_h
#define __vtkRobotToImageRegistration_h

#include "vtkObject.h"
#include "vtkProstateNavWin32Header.h" 

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include <vector>
#include <string>

class VTK_PROSTATENAV_EXPORT vtkRobotToImageRegistration : public vtkObject
{

 public:

  static vtkRobotToImageRegistration *New();
  vtkTypeRevisionMacro(vtkRobotToImageRegistration,vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetObjectMacro ( FiducialVolume, vtkMRMLScalarVolumeNode );
  vtkSetObjectMacro ( FiducialVolume, vtkMRMLScalarVolumeNode );
  vtkGetObjectMacro ( RobotToImageTransform, vtkMRMLLinearTransformNode );
  vtkSetObjectMacro ( RobotToImageTransform, vtkMRMLLinearTransformNode );

  virtual int DoRegistration() { return 0; };

 protected:
  vtkRobotToImageRegistration();
  ~vtkRobotToImageRegistration();

 protected:

  vtkMRMLScalarVolumeNode*    FiducialVolume;
  vtkMRMLLinearTransformNode* RobotToImageTransform;

};


#endif //__vtkRobotToImageRegistration_h
