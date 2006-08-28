/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerFiducialsLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerFiducialsLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerFiducialsLogic_h
#define __vtkSlicerFiducialsLogic_h

#include <stdlib.h>

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerLogic.h"

#include "vtkMRML.h"
#include "vtkMRMLFiducial.h"
#include "vtkMRMLFiducialListNode.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerFiducialsLogic : public vtkSlicerLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerFiducialsLogic *New();
  vtkTypeRevisionMacro(vtkSlicerFiducialsLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The currently active mrml volume node 
  vtkGetObjectMacro (ActiveFiducialListNode, vtkMRMLFiducialListNode);
  void SetActiveFiducialListNode (vtkMRMLFiducialListNode *ActiveFiducialListNode);

  // Description:
  // Create new mrml node and associated display node for a full list
  vtkMRMLFiducialListNode* AddFiducials();
  // Create a new mrml node and associated display node for a new point on
  // a list
  vtkMRMLFiducial *AddFiducial();

  // Description:
  // Update logic state when MRML scene chenges
  void ProcessMRMLEvents(); 
    
protected:
  vtkSlicerFiducialsLogic();
  ~vtkSlicerFiducialsLogic();
  vtkSlicerFiducialsLogic(const vtkSlicerFiducialsLogic&);
  void operator=(const vtkSlicerFiducialsLogic&);

  // Description:
  //
  vtkMRMLFiducialListNode *ActiveFiducialListNode;
};

#endif

