/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLMRAblationNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRMLMRAblationNode_h
#define __vtkMRMLMRAblationNode_h

#include <string>

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

#include "vtkMRAblation.h"

class vtkImageData;

class VTK_MRABLATION_EXPORT vtkMRMLMRAblationNode : public vtkMRMLNode
{
  public:
  static vtkMRMLMRAblationNode *New();
  vtkTypeMacro(vtkMRMLMRAblationNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "GADParameters";};

  const char *GetImageDirectory(); 
  void SetImageDirectory(const char *imgDir); 
  const char *GetWorkingDirectory(); 
  void SetWorkingDirectory(const char *workingDir); 

  // Description:
  // Get/Set timepoints (module parameter)
  vtkGetMacro(Timepoints, int);
  vtkSetMacro(Timepoints, int);

  // Description:
  // Get/Set slices (module parameter)
  vtkGetMacro(Slices, int);
  vtkSetMacro(Slices, int);

  // Description:
  // Get/Set TE (module parameter)
  vtkGetMacro(TE, double);
  vtkSetMacro(TE, double);
 
  // Description:
  // Get/Set w0 (module parameter)
  vtkGetMacro(w0, double);
  vtkSetMacro(w0, double);

  // Description:
  // Get/Set w0 (module parameter)
  vtkGetMacro(TC, double);
  vtkSetMacro(TC, double);
 
  // Description:
  // Get/Set output volume MRML Id
  vtkGetStringMacro(OutputVolumeRef);
  vtkSetStringMacro(OutputVolumeRef);


protected:
  vtkMRMLMRAblationNode();
  ~vtkMRMLMRAblationNode();
  vtkMRMLMRAblationNode(const vtkMRMLMRAblationNode&);
  void operator=(const vtkMRMLMRAblationNode&);

  int Timepoints;
  int Slices;
  double TE;
  double w0;
  double TC;

  char *ImageDirectory;
  char *WorkingDirectory;
 
  char* OutputVolumeRef;
};

#endif

