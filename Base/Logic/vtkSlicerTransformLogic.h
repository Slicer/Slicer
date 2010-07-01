/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerTransformLogic.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/

///  vtkSlicerTransformLogic - slicer logic class for volumes manipulation
/// 
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the volumes


#ifndef __vtkSlicerTransformLogic_h
#define __vtkSlicerTransformLogic_h

#include <stdlib.h>

#include "vtkSlicerLogic.h"

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"


class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerTransformLogic : public vtkSlicerLogic 
{
  public:
  
  /// The Usual vtk class functions
  static vtkSlicerTransformLogic *New();
  vtkTypeRevisionMacro(vtkSlicerTransformLogic,vtkSlicerLogic);
  void PrintSelf(ostream& os, vtkIndent indent) { Superclass::PrintSelf(os, indent); };

  /// 
  /// Read transform from file 
  vtkMRMLTransformNode* AddTransform (const char* filename, vtkMRMLScene *scene);

  // Register transform types with ITK factory
  // TODO: revisit this after the next (3.20?) release of ITK is out!!!
  void RegisterITKTransforms();

  /// 
  /// Write transform's data to a specified file
  int SaveTransform (const char* filename, vtkMRMLTransformNode *transformNode);
  
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                  unsigned long /*event*/, 
                                   void * /*callData*/ ) { };
  
  
  /// 
  /// Update logic events
  virtual void ProcessLogicEvents ( vtkObject * /*caller*/, 
                                  unsigned long /*event*/, 
                                  void * /*callData*/ ) {};  
  virtual void ProcessLogicEvents() {}

protected:
  vtkSlicerTransformLogic();
  virtual ~vtkSlicerTransformLogic();
  vtkSlicerTransformLogic(const vtkSlicerTransformLogic&);
  void operator=(const vtkSlicerTransformLogic&);
};

#endif

