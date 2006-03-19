/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLVolumeDisplayNode - MRML node for representing a volume (image stack).
// .SECTION Description
// Volume nodes describe data sets that can be thought of as stacks of 2D 
// images that form a 3D volume.  Volume nodes describe where the images 
// are stored on disk, how to render the data (window and level), and how 
// to read the files.  This information is extracted from the image 
// headers (if they exist) at the time the MRML file is generated.  
// Consequently, MRML files isolate MRML browsers from understanding how 
// to read the myriad of file formats for medical data. 

#ifndef __vtkMRMLVolumeDisplayNode_h
#define __vtkMRMLVolumeDisplayNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLVolumeDisplayNode : public vtkMRMLNode
{
  public:
  static vtkMRMLVolumeDisplayNode *New();
  vtkTypeMacro(vtkMRMLVolumeDisplayNode,vtkMRMLNode);
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

  //--------------------------------------------------------------------------
  // Display Information
  //--------------------------------------------------------------------------
  
  // Description:
  // Numerical ID of the color lookup table to use for rendering the volume
  vtkSetStringMacro(LUTName);
  vtkGetStringMacro(LUTName);

  // Description:
  // Specifies whether windowing and leveling are to be performed automatically
  vtkBooleanMacro(AutoWindowLevel, int);
  vtkGetMacro(AutoWindowLevel, int);
  vtkSetMacro(AutoWindowLevel, int);

  // Description:
  // The window value to use when autoWindowLevel is 'no'
  vtkGetMacro(Window, vtkFloatingPointType);
  vtkSetMacro(Window, vtkFloatingPointType);

  // Description:
  // The level value to use when autoWindowLevel is 'no'
  vtkGetMacro(Level, vtkFloatingPointType);
  vtkSetMacro(Level, vtkFloatingPointType);

  // Description:
  // Specifies whether to apply the threshold
  vtkBooleanMacro(ApplyThreshold, int);
  vtkGetMacro(ApplyThreshold, int);
  vtkSetMacro(ApplyThreshold, int);

  // Description:
  // Specifies whether the threshold should be set automatically
  vtkBooleanMacro(AutoThreshold, int);
  vtkGetMacro(AutoThreshold, int);
  vtkSetMacro(AutoThreshold, int);

  // Description:
  // The upper threshold value to use when autoThreshold is 'no'
  // Warning:
  // XXX-MH Should be floating point....
  vtkGetMacro(UpperThreshold, vtkFloatingPointType);
  vtkSetMacro(UpperThreshold, vtkFloatingPointType);

  // Description:
  // The lower threshold value to use when autoThreshold is 'no'
  // Warning:
  // XXX-MH Should be floating point....
  vtkGetMacro(LowerThreshold, vtkFloatingPointType);
  vtkSetMacro(LowerThreshold, vtkFloatingPointType);

  // Description:
  // Set/Get interpolate reformated slices
  vtkGetMacro(Interpolate, int);
  vtkSetMacro(Interpolate, int);
  vtkBooleanMacro(Interpolate, int);

  
protected:
  vtkMRMLVolumeDisplayNode();
  ~vtkMRMLVolumeDisplayNode();
  vtkMRMLVolumeDisplayNode(const vtkMRMLVolumeDisplayNode&);
  void operator=(const vtkMRMLVolumeDisplayNode&);


  // Strings
  char *LUTName;

  vtkFloatingPointType Window;
  vtkFloatingPointType Level;
  vtkFloatingPointType UpperThreshold;
  vtkFloatingPointType LowerThreshold;


  // Booleans
  int Interpolate;
  int AutoWindowLevel;
  int ApplyThreshold;
  int AutoThreshold;

};

#endif

