/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
// .NAME vtkMRMLVolumeNode - MRML node for representing a volume (image stack).
// .SECTION Description
// Volume nodes describe data sets that can be thought of as stacks of 2D 
// images that form a 3D volume.  Volume nodes describe where the images 
// are stored on disk, how to render the data (window and level), and how 
// to read the files.  This information is extracted from the image 
// headers (if they exist) at the time the MRML file is generated.  
// Consequently, MRML files isolate MRML browsers from understanding how 
// to read the myriad of file formats for medical data. 

#ifndef __vtkMRMLScalarVolumeNode_h
#define __vtkMRMLScalarVolumeNode_h


#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"

class vtkImageData;
class vtkImageAccumulateDiscrete;
class vtkImageBimodalAnalysis;
class VTK_MRML_EXPORT vtkMRMLScalarVolumeNode : public vtkMRMLVolumeNode
{
  public:
  static vtkMRMLScalarVolumeNode *New();
  vtkTypeMacro(vtkMRMLScalarVolumeNode,vtkMRMLVolumeNode);
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
  virtual const char* GetNodeTagName() {return "Volume";};

  // Description:
  // Indicates if this volume is a label map, which is the output of 
  // segmentation that labels each voxel according to its tissue type.  
  // The alternative is a gray-level or color image.
  int GetLabelMap();
  void SetLabelMap(int);
  void LabelMapOn();
  void LabelMapOff();

  // Description:
  // Make a 'None' volume node with blank image data
  static void CreateNoneNode(vtkMRMLScene *scene);

  // Description:
  // Associated display MRML node
  virtual vtkMRMLScalarVolumeDisplayNode* GetScalarVolumeDisplayNode()
  {
    return vtkMRMLScalarVolumeDisplayNode::SafeDownCast(this->GetDisplayNode());
  }

  virtual void UpdateFromMRML();
  
  // Description:
  // Calculate good default viewing parameters, uses input image data if not
  // null, otherwise this node's image data
  void CalculateScalarAutoLevels( vtkMRMLScalarVolumeDisplayNode *refNode = NULL,
                                  vtkImageData *imageData = NULL);
  // Descriptoin:
  // special case for statistics volumes
  void CalculateStatisticsAutoLevels( vtkMRMLScalarVolumeDisplayNode *refNode = NULL,
                                      vtkImageData *imageData = NULL);
  
protected:
  vtkMRMLScalarVolumeNode();
  ~vtkMRMLScalarVolumeNode();
  vtkMRMLScalarVolumeNode(const vtkMRMLScalarVolumeNode&);
  void operator=(const vtkMRMLScalarVolumeNode&);

  // Description:
  // Used internally in CalculateScalarAutoLevels and CalculateStatisticsAutoLevels
  vtkImageAccumulateDiscrete *Accumulate;
  vtkImageBimodalAnalysis *Bimodal;
};

#endif


 

