/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDiffusionTensorVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLDiffusionTensorVolumeDisplayNode - MRML node for representing a volume (image stack).
// .SECTION Description
// Volume nodes describe data sets that can be thought of as stacks of 2D 
// images that form a 3D volume.  Volume nodes describe where the images 
// are stored on disk, how to render the data (window and level), and how 
// to read the files.  This information is extracted from the image 
// headers (if they exist) at the time the MRML file is generated.  
// Consequently, MRML files isolate MRML browsers from understanding how 
// to read the myriad of file formats for medical data. 

#ifndef __vtkMRMLDiffusionTensorVolumeDisplayNode_h
#define __vtkMRMLDiffusionTensorVolumeDisplayNode_h

#include "vtkMRML.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLColorNode.h"

#include "vtkMatrix4x4.h"
#include "vtkImageData.h"

class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLDiffusionTensorVolumeDisplayNode : public vtkMRMLVolumeDisplayNode
{
  public:
  static vtkMRMLDiffusionTensorVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLDiffusionTensorVolumeDisplayNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "DiffusionWeightedVolumeDisplay";};

  //--------------------------------------------------------------------------
  // Display Information
  //--------------------------------------------------------------------------

  // Description:
  // Set/Get visualization Mode
#define VTK_VISMODE_SCALAR 0
#define VTK_VISMODE_GLYPH 1
#define VTK_VISMODE_BOTH 2

  vtkGetMacro(VisualizationMode, int);
  vtkSetMacro(VisualizationMode, int);

  void SetVisualizationModeToScalarVolume() {
    this->SetVisualizationMode(VTK_VISMODE_SCALAR);
  };  
  void SetVisualizationModeToGlyphs() {
    this->SetVisualizationMode(VTK_VISMODE_GLYPH);
  };  
  void SetVisualizationModeToBoth() {
    this->SetVisualizationMode(VTK_VISMODE_BOTH);
  };  

#define VTK_SCALARMODE_TRACE                  0
#define VTK_SCALARMODE_DETERMINANT            1
#define VTK_SCALARMODE_RELATIVE_ANISOTROPY    2
#define VTK_SCALARMODE_FRACTIONAL_ANISOTROPY  3
#define VTK_SCALARMODE_MAX_EIGENVALUE         4
#define VTK_SCALARMODE_MID_EIGENVALUE         5
#define VTK_SCALARMODE_MIN_EIGENVALUE         6
#define VTK_SCALARMODE_LINEAR_MEASURE         7
#define VTK_SCALARMODE_PLANAR_MEASURE         8
#define VTK_SCALARMODE_SPHERICAL_MEASURE      9
#define VTK_SCALARMODE_COLOR_ORIENTATION     10
#define VTK_SCALARMODE_D11                   11
#define VTK_SCALARMODE_D22                   12
#define VTK_SCALARMODE_D33                   13
#define VTK_SCALARMODE_MODE                  14
#define VTK_SCALARMODE_COLOR_MODE            15
#define VTK_SCALARMODE_MAX_EIGENVALUE_PROJX 16
#define VTK_SCALARMODE_MAX_EIGENVALUE_PROJY 17
#define VTK_SCALARMODE_MAX_EIGENVALUE_PROJZ 18

  vtkGetMacro(ScalarMode, int);
  vtkSetMacro(ScalarMode, int);
 
  void SetScalarModeToTrace() {
    this->SetScalarMode(VTK_SCALARMODE_TRACE);
  };
  void SetScalarModeToFractionalAnisotropy() {
    this->SetScalarMode(VTK_SCALARMODE_FRACTIONAL_ANISOTROPY);
  };  
  //and so on.

#define VTK_GLYPHMODE_LINES 1
#define VTK_GLYPHMODE_TUBES 2
#define VTK_GLYPHMODE_ELLIPSOIDS 3
#define VTK_GLYPHMODE_SUPERQUADRICS 4
  
  vtkGetMacro(GlyphMode, int);
  vtkSetMacro(GlyphMode, int);
  
  void SetGlyphModeToLines() {
    this->SetGlyphMode(VTK_GLYPHMODE_LINES);
  };
  void SetGlyphModeToTubes() {
    this->SetGlyphMode(VTK_GLYPHMODE_TUBES);
  };
  void SetGlyphModeToEllipsoids() {
    this->SetGlyphMode(VTK_GLYPHMODE_ELLIPSOIDS);
  };
  void SetGlyphModeToSuperquadrics() {
    this->SetGlyphMode(VTK_GLYPHMODE_SUPERQUADRICS);
  };
  
protected:
  vtkMRMLDiffusionTensorVolumeDisplayNode();
  ~vtkMRMLDiffusionTensorVolumeDisplayNode();
  vtkMRMLDiffusionTensorVolumeDisplayNode(const vtkMRMLDiffusionTensorVolumeDisplayNode&);
  void operator=(const vtkMRMLDiffusionTensorVolumeDisplayNode&);

  int VisualizationMode;
  int ScalarMode;
  int GlyphMode;
};

#endif

