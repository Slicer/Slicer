/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerFiberBundleLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerFiberBundleLogic - slicer logic class for fiber bundle manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the fiber bundles


#ifndef __vtkSlicerFiberBundleLogic_h
#define __vtkSlicerFiberBundleLogic_h

#include <stdlib.h>

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerModuleLogic.h"

#include "vtkMRML.h"
#include "vtkMRMLFiberBundleNode.h"


class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerFiberBundleLogic : public vtkSlicerModuleLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerFiberBundleLogic *New();
  vtkTypeRevisionMacro(vtkSlicerFiberBundleLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The currently active mrml fiber bundle node 
  vtkGetObjectMacro (ActiveFiberBundleNode, vtkMRMLFiberBundleNode);
  void SetActiveFiberBundleNode (vtkMRMLFiberBundleNode *ActiveFiberBundleNode);

  // Description:
  // Create new mrml fiber bundle node and read its polydata from a specified file.
  // Also create the logic object for its display.
  vtkMRMLFiberBundleNode* AddFiberBundle (char* filename);

  // Description:
  // Create fiber bundle nodes and
  // read their polydata from a specified directory
  int AddFiberBundles (const char* dirname, const char* suffix );

  // Description:
  // Write fiber bundle's polydata  to a specified file
  int SaveFiberBundle (char* filename, vtkMRMLFiberBundleNode *fiberBundleNode);

  // Description:
  // Read in a scalar overlay and add it to the fiber bundle node
  //int AddScalar(char* filename, vtkMRMLFiberBundleNode *fiberBundleNode);

  // Description:
  // Update logic state when MRML scene chenges
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                  unsigned long /*event*/, 
                                  void * /*callData*/ );    
protected:
  vtkSlicerFiberBundleLogic();
  ~vtkSlicerFiberBundleLogic();
  vtkSlicerFiberBundleLogic(const vtkSlicerFiberBundleLogic&);
  void operator=(const vtkSlicerFiberBundleLogic&);

  // Description:
  //
  vtkMRMLFiberBundleNode *ActiveFiberBundleNode;

  vtkCollection *DisplayLogicCollection;

};

#endif

