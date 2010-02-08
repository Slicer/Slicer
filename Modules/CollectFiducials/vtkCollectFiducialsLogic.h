/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

// .NAME vtkCollectFiducialsLogic - slicer logic class for Locator module 
// .SECTION Description
// This class manages the logic associated with tracking device for
// IGT. 


#ifndef __vtkCollectFiducialsLogic_h
#define __vtkCollectFiducialsLogic_h

#include "vtkCollectFiducialsWin32Header.h"

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLSliceNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLLinearTransformNode.h"

class vtkIGTLConnector;

class VTK_CollectFiducials_EXPORT vtkCollectFiducialsLogic : public vtkSlicerModuleLogic 
{
 public:
  //BTX
  enum {  // Events
    //LocatorUpdateEvent      = 50000,
    StatusUpdateEvent       = 50001,
  };
  //ETX

 public:
  
  static vtkCollectFiducialsLogic *New();
  
  vtkTypeRevisionMacro(vtkCollectFiducialsLogic,vtkObject);
  void PrintSelf(ostream&, vtkIndent);

  void ResetAllFiducials(const char *ID);
  void AddFiducial(const char *ID, char *name, float x, float y, float z);
  void GetNewFiducialMeasure(const char *ID, float *px, float *py, float *pz);

 protected:
  
  vtkCollectFiducialsLogic();
  ~vtkCollectFiducialsLogic();

  void operator=(const vtkCollectFiducialsLogic&);
  vtkCollectFiducialsLogic(const vtkCollectFiducialsLogic&);

  static void DataCallback(vtkObject*, unsigned long, void *, void *);
  void UpdateAll();

  vtkCallbackCommand *DataCallbackCommand;

 private:


};

#endif


  
