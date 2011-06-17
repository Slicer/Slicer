/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

///  vtkSlicerTransformLogic - slicer logic class for volumes manipulation
///
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the volumes

#ifndef __vtkSlicerTransformLogic_h
#define __vtkSlicerTransformLogic_h

// SlicerLogic includes
#include "vtkSlicerBaseLogic.h"

// MRMLLogic includes
#include <vtkMRMLAbstractLogic.h>

// MRML includes
class vtkMRMLScene;
class vtkMRMLTransformableNode;
class vtkMRMLTransformNode;

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerTransformLogic : public vtkMRMLAbstractLogic
{
  public:
  
  /// The Usual vtk class functions
  static vtkSlicerTransformLogic *New();
  vtkTypeRevisionMacro(vtkSlicerTransformLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) { Superclass::PrintSelf(os, indent); };

  ///
  /// Apply the associated transform to the transformable node. Return true
  /// on success, false otherwise.
  static bool hardenTransform(vtkMRMLTransformableNode* node);
  
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

