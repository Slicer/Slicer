/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLGradientAnisotropicDiffusionFilterNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLGradientAnisotropicDiffusionFilterNode_h
#define __vtkMRMLGradientAnisotropicDiffusionFilterNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

#include "vtkGradientAnisotropicDiffusionFilter.h"

class vtkImageData;

class VTK_GRADIENTANISOTROPICDIFFUSIONFILTER_EXPORT vtkMRMLGradientAnisotropicDiffusionFilterNode : public vtkMRMLNode
{
  public:
  static vtkMRMLGradientAnisotropicDiffusionFilterNode *New();
  vtkTypeMacro(vtkMRMLGradientAnisotropicDiffusionFilterNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "GradientAnisotropicDiffusionFilter";};


  vtkGetMacro(NumberOfIterations, int);
  vtkSetMacro(NumberOfIterations, int);

  vtkGetMacro(Conductance, vtkFloatingPointType);
  vtkSetMacro(Conductance, vtkFloatingPointType);


  vtkGetMacro(TimeStep, vtkFloatingPointType);
  vtkSetMacro(TimeStep, vtkFloatingPointType);
 
  vtkGetStringMacro(InputVolumeRef);
  vtkSetStringMacro(InputVolumeRef);
  
  vtkGetStringMacro(OutputVolumeRef);
  vtkSetStringMacro(OutputVolumeRef);

 
protected:
  vtkMRMLGradientAnisotropicDiffusionFilterNode();
  ~vtkMRMLGradientAnisotropicDiffusionFilterNode();
  vtkMRMLGradientAnisotropicDiffusionFilterNode(const vtkMRMLGradientAnisotropicDiffusionFilterNode&);
  void operator=(const vtkMRMLGradientAnisotropicDiffusionFilterNode&);

  vtkFloatingPointType Conductance;
  vtkFloatingPointType TimeStep;
  int NumberOfIterations;
  
  char* InputVolumeRef;
  char* OutputVolumeRef;

};

#endif

