/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerDaemonLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkSlicerDaemonLogic_h
#define __vtkSlicerDaemonLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"

#include "vtkSlicerDaemon.h"
#include "vtkMRMLSlicerDaemonNode.h"


class vtkITKSlicerDaemon;

class VTK_SLICERDAEMON_EXPORT vtkSlicerDaemonLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkSlicerDaemonLogic *New();
  vtkTypeMacro(vtkSlicerDaemonLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};

  // Description: Get/Set MRML node
  vtkGetObjectMacro (SlicerDaemonNode, vtkMRMLSlicerDaemonNode);
  vtkSetObjectMacro (SlicerDaemonNode, vtkMRMLSlicerDaemonNode);
  
  void Apply();
  
protected:
  vtkSlicerDaemonLogic();
  ~vtkSlicerDaemonLogic();
  vtkSlicerDaemonLogic(const vtkSlicerDaemonLogic&);
  void operator=(const vtkSlicerDaemonLogic&);

  vtkMRMLSlicerDaemonNode* SlicerDaemonNode;
  vtkITKSlicerDaemon* SlicerDaemon;


};

#endif

