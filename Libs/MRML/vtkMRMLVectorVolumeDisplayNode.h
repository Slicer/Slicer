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
#include "vtkMRMLVolumeGlyphDisplayNode.h"
#include "vtkMRMLColorNode.h"

#include "vtkMatrix4x4.h"
#include "vtkImageData.h"
#include "vtkImageShiftScale.h"
#include "vtkImageRGBToHSI.h"

class vtkImageData;
class vtkImageShiftScale;
class vtkImageExtractComponents;
class vtkImageRGBToHSI;

class VTK_MRML_EXPORT vtkMRMLVectorVolumeDisplayNode : public vtkMRMLVolumeGlyphDisplayNode
{
  public:
  static vtkMRMLVectorVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLVectorVolumeDisplayNode,vtkMRMLVolumeGlyphDisplayNode);
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
  virtual const char* GetNodeTagName() {return "VectorVolumeDisplay";};

  //--------------------------------------------------------------------------
  // Display Information
  //--------------------------------------------------------------------------


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

  virtual vtkPolyData* ExecuteGlyphPipeLineAndGetPolyData( vtkImageData* ) {return NULL;};

  virtual void SetDefaultColorMap() {};

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
  // Description:
  // Sets vtkImageData to be converted to displayable vtkImageData
  virtual void SetImageData(vtkImageData *imageData)
    {
    this->ShiftScale->SetInput( imageData );
    this->RGBToHSI->SetInput( imageData );

    this->AppendComponents->RemoveAllInputs();
    //this->AppendComponents->SetInputConnection(0, this->ShiftScale->GetOutput()->GetProducerPort());
    this->AppendComponents->SetInput(0, imageData);
    this->AppendComponents->SetInput(1, this->Threshold->GetOutput());
    };

  // Description:
  // Sets ImageData for background mask 
  virtual void SetBackgroundImageData(vtkImageData *imageData)
    {
    // TODO: what is this for?  The comment above is unhelpful!
    this->ResliceAlphaCast->SetInput(imageData);
    };

  // Description:
  // Gets ImageData converted from the real data in the node
  virtual vtkImageData* GetImageData() 
    {
    if ( this->RGBToHSI->GetInput() != NULL )
      {
      this->UpdateImageDataPipeline();
      this->AppendComponents->Update();
      return this->AppendComponents->GetOutput();
      }
    return NULL;
    };

  virtual void UpdateImageDataPipeline();

  // Description:
  // Access to this class's internal filter elements
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

  int ScalarMode;
  int GlyphMode;

  vtkImageShiftScale *ShiftScale;
  vtkImageRGBToHSI *RGBToHSI;
  vtkImageExtractComponents *ExtractIntensity;
};

#endif

