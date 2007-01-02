/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVectorVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLVectorVolumeDisplayNode - MRML node for representing a volume (image stack).
// .SECTION Description
// Volume nodes describe data sets that can be thought of as stacks of 2D 
// images that form a 3D volume.  Volume nodes describe where the images 
// are stored on disk, how to render the data (window and level), and how 
// to read the files.  This information is extracted from the image 
// headers (if they exist) at the time the MRML file is generated.  
// Consequently, MRML files isolate MRML browsers from understanding how 
// to read the myriad of file formats for medical data. 

#ifndef __vtkMRMLVectorVolumeDisplayNode_h
#define __vtkMRMLVectorVolumeDisplayNode_h

#include "vtkMRML.h"
#include "vtkMRMLVolumeDisplayNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLColorNode.h"

#include "vtkMatrix4x4.h"
#include "vtkImageData.h"

class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLVectorVolumeDisplayNode : public vtkMRMLVolumeDisplayNode
{
  public:
  static vtkMRMLVectorVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLVectorVolumeDisplayNode,vtkMRMLVolumeDisplayNode);
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
  //BTX
  enum 
    {
    visModeScalar = 0,
    visModeGlyph = 1,
    visModeBoth = 2,
    };
  //ETX

  vtkGetMacro(VisualizationMode, int);
  vtkSetMacro(VisualizationMode, int);

  void SetVisualizationModeToScalarVolume() {
    this->SetVisualizationMode(this->visModeScalar);
  };  
  void SetVisualizationModeToGlyphs() {
    this->SetVisualizationMode(this->visModeGlyph);
  };  
  void SetVisualizationModeToBoth() {
    this->SetVisualizationMode(this->visModeBoth);
  };  

  //BTX
  enum
    {
    scalarModeMagnitude = 0,
    };
  //ETX

  vtkGetMacro(ScalarMode, int);
  vtkSetMacro(ScalarMode, int);
 
  void SetScalarModeToMagnitude() {
    this->SetScalarMode(this->scalarModeMagnitude);
  };

  //BTX
  enum
    {
    glyphModeLines = 1,
    glyphModeTubes = 2,
    };
  //ETX
  vtkGetMacro(GlyphMode, int);
  vtkSetMacro(GlyphMode, int);
  
  void SetGlyphModeToLines() {
    this->SetGlyphMode(this->glyphModeLines);
  };
  void SetGlyphModeToTubes() {
    this->SetGlyphMode(this->glyphModeTubes);
  };
  
protected:
  vtkMRMLVectorVolumeDisplayNode();
  ~vtkMRMLVectorVolumeDisplayNode();
  vtkMRMLVectorVolumeDisplayNode(const vtkMRMLVectorVolumeDisplayNode&);
  void operator=(const vtkMRMLVectorVolumeDisplayNode&);

  int VisualizationMode;
  int ScalarMode;
  int GlyphMode;
};

#endif

