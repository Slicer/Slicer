/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerFiberBundleDisplayLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerFiberBundleDisplayLogic - slicer logic class for fiber bundle display
// .SECTION Description
// This class manages the logic associated with creating standard slicer
// models for display of requested fiber bundle attributes
// (tubes, centerlines, glyphs, etc.)   These standard slicer models
// are then added to the scene as "hidden models" for display.

#ifndef __vtkSlicerFiberBundleDisplayLogic_h
#define __vtkSlicerFiberBundleDisplayLogic_h

#include <stdlib.h>

#include "vtkSlicerLogic.h"

#include "vtkSlicerTractographyDisplayWin32Header.h"

#include "vtkMRML.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"


class VTK_SLICERTRACTOGRAPHYDISPLAY_EXPORT vtkSlicerFiberBundleDisplayLogic : public vtkSlicerLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerFiberBundleDisplayLogic *New();
  vtkTypeRevisionMacro(vtkSlicerFiberBundleDisplayLogic,vtkSlicerLogic);
  void PrintSelf(ostream& os, vtkIndent indent);


  // Description:
  // The fiber bundle display node that is observed by this class. When the 
  // state of this node changes, this logic class modifies the models
  // that it has added to the slicer scene.
  vtkSetObjectMacro ( FiberBundleNode , vtkMRMLFiberBundleNode );
  vtkGetObjectMacro ( FiberBundleNode , vtkMRMLFiberBundleNode );

  void SetAndObserveFiberBundleNode( vtkMRMLFiberBundleNode *fiberBundleNode );

  // Description:
  // Update logic state when MRML scene chenges
  virtual void ProcessMRMLEvents ( vtkObject * caller, 
                                  unsigned long event, 
                                  void * callData );    
protected:
  vtkSlicerFiberBundleDisplayLogic();
  ~vtkSlicerFiberBundleDisplayLogic();
  vtkSlicerFiberBundleDisplayLogic(const vtkSlicerFiberBundleDisplayLogic&);
  void operator=(const vtkSlicerFiberBundleDisplayLogic&);

  vtkPolyData *LineModel;
  vtkPolyData *TubeModel;
  vtkPolyData *GlyphModel;

  vtkMRMLFiberBundleNode *FiberBundleNode;

};

#endif

