/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerMRMLLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
// .NAME vtkSlicerMRMLLogic - the main logic to manage the application
// .SECTION Description
// This manages the logic associated with managing the MRML scene from
// the application perspective.  Transient information about the currently
// selected items in the scene is stored here, together with methods
// for manipulating them.
// There is a corresponding vtkSlicerMRMLGUI class that provides
// a user interface to this class by observing this class.
//

#ifndef __vtkSlicerMRMLLogic_h
#define __vtkSlicerMRMLLogic_h

#include "vtkSlicerLogic.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeNode.h"

#include "vtkCollection.h"


class VTK_SLICER_BASE_EXPORT vtkSlicerMRMLLogic : public vtkSlicerLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerMRMLLogic *New();
  vtkTypeRevisionMacro(vtkSlicerMRMLLogic,vtkSlicerLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
    

  // Description:
  // Get the current MRML Scene object
  vtkGetObjectMacro(MRMLScene,vtkMRMLScene);
  void SetMRMLScene (vtkMRMLScene *);

  // Description:
  // Additional methods here to manipulate the application:
 

  // Description:
  // The currently active volume (default target for 
  // viewing and filtering)
  // TODO: this should be dynamically maintained so that 
  // modules that define their own nodes can also get
  // automatic support for managing selection and active nodes
  vtkSetObjectMacro (ActiveVolume,vtkMRMLVolumeNode);
  vtkGetObjectMacro (ActiveVolume,vtkMRMLVolumeNode);

protected:

  vtkSlicerMRMLLogic();
  ~vtkSlicerMRMLLogic();
  vtkSlicerMRMLLogic(const vtkSlicerMRMLLogic&);
  void operator=(const vtkSlicerMRMLLogic&);

private:
  
  vtkMRMLScene *MRMLScene;

  vtkMRMLVolumeNode *ActiveVolume;
  
  vtkCallbackCommand *MRMLCallbackCommand;
};

#endif

