/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiberBundleGlyphDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

  =========================================================================auto=*/
// .NAME vtkMRMLDiffusionTensorVolumeSliceDisplayNode - MRML node to represent display properties for tractography.
// .SECTION Description
// vtkMRMLDiffusionTensorVolumeSliceDisplayNode nodes store display properties of trajectories 
// from tractography in diffusion MRI data, including color type (by bundle, by fiber, 
// or by scalar invariants), display on/off for tensor glyphs and display of 
// trajectory as a line or tube.
//

#ifndef __vtkMRMLDiffusionTensorVolumeSliceDisplayNode_h
#define __vtkMRMLDiffusionTensorVolumeSliceDisplayNode_h

#include <string>

#include "vtkPolyData.h"
#include "vtkMatrix4x4.h"

#include "vtkMRML.h"
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"

class vtkDiffusionTensorGlyph;
class vtkTransform;
class vtkTransformPolyDataFilter;

class VTK_MRML_EXPORT vtkMRMLDiffusionTensorVolumeSliceDisplayNode : public vtkMRMLFiberBundleDisplayNode
{
 public:
  static vtkMRMLDiffusionTensorVolumeSliceDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLDiffusionTensorVolumeSliceDisplayNode,vtkMRMLFiberBundleDisplayNode );
  void PrintSelf ( ostream& os, vtkIndent indent );
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance (  );

  // Description:
  // Read node attributes from XML (MRML) file
  virtual void ReadXMLAttributes ( const char** atts );

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML ( ostream& of, int indent );


  // Description:
  // Copy the node's attributes to this object
  virtual void Copy ( vtkMRMLNode *node );
  
  // Description:
  // Get node XML tag name (like Volume, UnstructuredGrid)
  virtual const char* GetNodeTagName ( ) {return "DiffusionTensorVolumeSliceDisplayNode";};

  // Description:
  // Sets polydata for glyph input (usually stored in FiberBundle node)
  void SetPolyData(vtkPolyData *glyphPolyData);

  // Description:
  // Gets resulting glyph PolyData 
  virtual vtkPolyData* GetPolyData();
  
  // Description:
  // Gets resulting glyph PolyData transfomed to slice XY
  virtual vtkPolyData* GetPolyDataTransformedToSlice();
   
  // Description:
  // Update the pipeline based on this node attributes
  virtual void UpdatePolyDataPipeline();

  // Description:
  // Set ImageData for a volume slice
  virtual void SetSliceImage(vtkImageData *image);
 
  // Description:
  // Set slice to RAS transformation
  virtual void SetSlicePositionMatrix(vtkMatrix4x4 *matrix);

  // Description:
  // Set slice to IJK transformation
  virtual void SetSliceTensorRotationMatrix(vtkMatrix4x4 *matrix);

 protected:
  vtkMRMLDiffusionTensorVolumeSliceDisplayNode ( );
  ~vtkMRMLDiffusionTensorVolumeSliceDisplayNode ( );
  vtkMRMLDiffusionTensorVolumeSliceDisplayNode ( const vtkMRMLDiffusionTensorVolumeSliceDisplayNode& );
  void operator= ( const vtkMRMLDiffusionTensorVolumeSliceDisplayNode& );

    vtkDiffusionTensorGlyph  *DiffusionTensorGlyphFilter;
    vtkTransform             *SliceToXYTransform;
    vtkTransformPolyDataFilter *SliceToXYTransformer;
    vtkMatrix4x4             *SliceToXYMatrix;
};

#endif
