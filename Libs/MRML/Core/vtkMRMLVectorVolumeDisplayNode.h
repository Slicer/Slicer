/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVectorVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLVectorVolumeDisplayNode_h
#define __vtkMRMLVectorVolumeDisplayNode_h

#include "vtkMRMLGlyphableVolumeDisplayNode.h"

class vtkAlgorithmOutput;
class vtkImageData;
class vtkImageShiftScale;
class vtkImageExtractComponents;
class vtkImageRGBToHSI;

/// \brief MRML node for representing a volume (image stack).
///
/// Volume nodes describe data sets that can be thought of as stacks of 2D
/// images that form a 3D volume.  Volume nodes describe where the images
/// are stored on disk, how to render the data (window and level), and how
/// to read the files.  This information is extracted from the image
/// headers (if they exist) at the time the MRML file is generated.
/// Consequently, MRML files isolate MRML browsers from understanding how
/// to read the myriad of file formats for medical data.
class VTK_MRML_EXPORT vtkMRMLVectorVolumeDisplayNode : public vtkMRMLGlyphableVolumeDisplayNode
{
  public:
  static vtkMRMLVectorVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLVectorVolumeDisplayNode,vtkMRMLGlyphableVolumeDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "VectorVolumeDisplay";};

  //--------------------------------------------------------------------------
  /// Display Information
  //--------------------------------------------------------------------------

  enum
    {
    scalarModeMagnitude = 0
    };
  vtkGetMacro(ScalarMode, int);
  vtkSetMacro(ScalarMode, int);

  void SetScalarModeToMagnitude() {
    this->SetScalarMode(this->scalarModeMagnitude);
  };

  enum
    {
    glyphModeLines = 1,
    glyphModeTubes = 2
    };
  vtkGetMacro(GlyphMode, int);
  vtkSetMacro(GlyphMode, int);

  void SetGlyphModeToLines() {
    this->SetGlyphMode(this->glyphModeLines);
  };
  void SetGlyphModeToTubes() {
    this->SetGlyphMode(this->glyphModeTubes);
  };

  //virtual vtkPolyData* ExecuteGlyphPipeLineAndGetPolyData( vtkImageData* ) {return NULL;};

  virtual void SetDefaultColorMap() {};

  ///
  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );

  /// Get the input of the pipeline
  virtual vtkAlgorithmOutput* GetInputImageDataConnection();

  virtual void UpdateImageDataPipeline();

  ///
  /// get associated slice glyph display node
  /// TODO: return empty list for now, later add glyphs
  virtual std::vector< vtkMRMLGlyphableVolumeSliceDisplayNode*>
    GetSliceGlyphDisplayNodes( vtkMRMLVolumeNode* vtkNotUsed(node) )
    {
    return std::vector< vtkMRMLGlyphableVolumeSliceDisplayNode*>();
    }

  ///
  /// Access to this class's internal filter elements
  vtkGetObjectMacro (ShiftScale, vtkImageShiftScale);
  vtkGetObjectMacro (RGBToHSI, vtkImageRGBToHSI);
  vtkGetObjectMacro (ExtractIntensity, vtkImageExtractComponents);
  vtkGetObjectMacro (AppendComponents, vtkImageAppendComponents);
  vtkGetObjectMacro (Threshold, vtkImageThreshold);

protected:
  vtkMRMLVectorVolumeDisplayNode();
  ~vtkMRMLVectorVolumeDisplayNode();
  vtkMRMLVectorVolumeDisplayNode(const vtkMRMLVectorVolumeDisplayNode&);
  void operator=(const vtkMRMLVectorVolumeDisplayNode&);

  /// Set the input of the pipeline
  virtual void SetInputToImageDataPipeline(vtkAlgorithmOutput *imageDataConnection);
  virtual vtkAlgorithmOutput* GetScalarImageDataConnection();

  int ScalarMode;
  int GlyphMode;

  vtkImageShiftScale *ShiftScale;
  vtkImageRGBToHSI *RGBToHSI;
  vtkImageExtractComponents *ExtractIntensity;
};

#endif
