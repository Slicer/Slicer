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
  // Create new mrml node for a full list, make it the selected list, and clear up local pointers
  void AddFiducialListSelected();
  
  // Description:
  // Create new mrml node and associated display node for a full list,
  // return the node
  vtkMRMLFiducialListNode * AddFiducialList();

  // Description:
  // Add a fiducial to the currently selected list, as kept in the
  // vtkMRMLSelectionNode
  // Returns the index of the new fiducial in the list, -1 on failure
  // AddFiducialSelected includes a selected flag option
  int AddFiducial(float x, float y, float z);
  int AddFiducialSelected (float x, float y, float z, int selected);

  // Description:
  // Update logic state when MRML scene changes
  void ProcessMRMLEvents(); 
    
protected:
  vtkSlicerFiducialsLogic();
  ~vtkSlicerFiducialsLogic();
  vtkSlicerFiducialsLogic(const vtkSlicerFiducialsLogic&);
  void operator=(const vtkSlicerFiducialsLogic&);

};

#endif

