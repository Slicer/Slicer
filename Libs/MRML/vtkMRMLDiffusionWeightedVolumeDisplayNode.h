/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDiffusionWeightedVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLDiffusionWeightedVolumeDisplayNode - MRML node for representing a volume (image stack).
// .SECTION Description
// Volume nodes describe data sets that can be thought of as stacks of 2D 
// images that form a 3D volume.  Volume nodes describe where the images 
// are stored on disk, how to render the data (window and level), and how 
// to read the files.  This information is extracted from the image 
// headers (if they exist) at the time the MRML file is generated.  
// Consequently, MRML files isolate MRML browsers from understanding how 
// to read the myriad of file formats for medical data. 

#ifndef __vtkMRMLDiffusionWeightedVolumeDisplayNode_h
#define __vtkMRMLDiffusionWeightedVolumeDisplayNode_h

#include "vtkMRML.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLColorNode.h"

#include "vtkMatrix4x4.h"
#include "vtkImageData.h"
#include "vtkImageExtractComponents.h"

class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLDiffusionWeightedVolumeDisplayNode : public vtkMRMLScalarVolumeDisplayNode
{
  public:
  static vtkMRMLDiffusionWeightedVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLDiffusionWeightedVolumeDisplayNode,vtkMRMLScalarVolumeDisplayNode);
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

    // Description:
  // Sets vtkImageData to be converted to displayable vtkImageData
  virtual void SetImageData(vtkImageData *imageData)
    {
    this->ExtractComponent->SetInput( imageData);
    };

  virtual void UpdateImageDataPipeline()
    {
    this->ExtractComponent->SetComponents(this->GetDiffusionComponent());
    Superclass::UpdateImageDataPipeline();
    };

  //--------------------------------------------------------------------------
  // Display Information
  //--------------------------------------------------------------------------

  // Description:
  // Set/Get interpolate reformated slices
  vtkGetMacro(DiffusionComponent, int);
  vtkSetMacro(DiffusionComponent, int);
 
protected:
  vtkMRMLDiffusionWeightedVolumeDisplayNode();
  ~vtkMRMLDiffusionWeightedVolumeDisplayNode();
  vtkMRMLDiffusionWeightedVolumeDisplayNode(const vtkMRMLDiffusionWeightedVolumeDisplayNode&);
  void operator=(const vtkMRMLDiffusionWeightedVolumeDisplayNode&);

  int DiffusionComponent;

  vtkImageExtractComponents *ExtractComponent;


};

#endif

