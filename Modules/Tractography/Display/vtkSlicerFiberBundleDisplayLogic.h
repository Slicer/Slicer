
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

#include "vtkSlicerModuleLogic.h"

#include "vtkSlicerTractographyDisplay.h"

#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"

#include "vtkDiffusionTensorGlyph.h"


class VTK_SLICERTRACTOGRAPHYDISPLAY_EXPORT vtkSlicerFiberBundleDisplayLogic : public vtkSlicerModuleLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerFiberBundleDisplayLogic *New();
  vtkTypeRevisionMacro(vtkSlicerFiberBundleDisplayLogic,vtkSlicerLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the fiber bundle display node that is observed by this class.
  vtkGetObjectMacro ( FiberBundleNode , vtkMRMLFiberBundleNode );

  // Description:
  // Set the fiber bundle node that is observed by this class. When the 
  // state of this node changes, this logic class modifies the models
  // that it has added to the slicer scene.
  void SetAndObserveFiberBundleNode( vtkMRMLFiberBundleNode *fiberBundleNode );

  // Description:
  // Update logic state when MRML scene changes. Observes the FiberBundleNode for PolyDataModified
  // and DisplayModified events. Either causes a display update.
  virtual void ProcessMRMLEvents ( vtkObject * caller, 
                                  unsigned long event, 
                                  void * callData );    
protected:
  vtkSlicerFiberBundleDisplayLogic();
  ~vtkSlicerFiberBundleDisplayLogic();
  vtkSlicerFiberBundleDisplayLogic(const vtkSlicerFiberBundleDisplayLogic&);
  void operator=(const vtkSlicerFiberBundleDisplayLogic&);

  // Description:
  // Create a vtkMRMLModelNode and place in slicer scene.
  // The model node is temporary. It is not saved with the scene and can't be edited or 
  // selected on menus.
  void CreateTemporaryModelNodeForDisplay ( vtkMRMLModelNode * & modelNode, vtkMRMLModelDisplayNode * & displayNode );

  // Description:
  // Delete a model from MRML scene.
  void DeleteTemporaryModelNodeForDisplay ( vtkMRMLModelNode * & modelNode, vtkMRMLModelDisplayNode * & displayNode );

  // Description:
  // Add a model to MRML scene.
  void AddTemporaryModelNodeToScene ( vtkMRMLModelNode * & modelNode, vtkMRMLModelDisplayNode * & displayNode);

  // Description:
  // Make one of our temporary models invisible (to correspond to visibility settings in fiber bundle 
  // display node).
  void VisibilityOffForTemporaryModel ( vtkMRMLModelNode * modelNode,  vtkMRMLModelDisplayNode * displayNode);

  void CreateLineModelNodes ( );
  void CreateTubeModelNodes ( );
  void CreateGlyphModelNodes ( );

  void DeleteLineModelNodes ( );
  void DeleteTubeModelNodes ( );
  void DeleteGlyphModelNodes ( );


  // Description:
  // Create a polyline model. The polydata is generated from our vtkMRMLFiberBundleNode, 
  // according to its vtkMRMLFiberDisplayNode settings.
  void CreateLineModel ( );

  // Description:
  // Create a tube model. The polydata is generated from our vtkMRMLFiberBundleNode, 
  // according to its vtkMRMLFiberDisplayNode settings.
  void CreateTubeModel ( );

  // Description:
  // Create a tensor glyph model. The polydata is generated from our vtkMRMLFiberBundleNode, 
  // according to its vtkMRMLFiberDisplayNode settings.
  void CreateGlyphModel ( );

  // Description:
  // Handle the logic to display various types of models (line/tube/glyph) and 
  // call the appropriate creation/deletion functions.
  void UpdateModelDisplay ( );
 
  vtkMRMLModelNode *LineModelNode;
  vtkMRMLModelNode *TubeModelNode;
  vtkMRMLModelNode *GlyphModelNode;

  vtkMRMLModelDisplayNode *LineModelDisplayNode;
  vtkMRMLModelDisplayNode *TubeModelDisplayNode;
  vtkMRMLModelDisplayNode *GlyphModelDisplayNode;

  vtkMRMLFiberBundleNode *FiberBundleNode;

  vtkDiffusionTensorGlyph *DiffusionTensorGlyphFilter;
};

#endif

