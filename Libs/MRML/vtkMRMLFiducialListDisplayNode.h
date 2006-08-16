/*=auto=========================================================================

  Portions (c) Copyright 20%05 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiducialListDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLFiducialListDisplayNode - MRML node to represent a 3D surface model.
// .SECTION Description
// FiducialList nodes describe polygonal data.  They indicate where the model is 
// stored on disk, and how to render it (color, opacity, etc).  FiducialLists 
// are assumed to have been constructed with the orientation and voxel 
// dimensions of the original segmented volume.

#ifndef __vtkMRMLFiducialListDisplayNode_h
#define __vtkMRMLFiducialListDisplayNode_h

#include <string>

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"

class VTK_MRML_EXPORT vtkMRMLFiducialListDisplayNode : public vtkMRMLNode
{
public:
  static vtkMRMLFiducialListDisplayNode *New();
  vtkTypeMacro(vtkMRMLFiducialListDisplayNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

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
  // Get node XML tag name (like Volume, FiducialList)
  virtual const char* GetNodeTagName() {return "FiducialListDisplay";};

  // Description:
  // FiducialList's color (r,g,b)
  vtkSetVector3Macro(Color, double);
  vtkGetVector3Macro(Color, double);
  
  // Description:
  // Opacity of the surface expressed as a number from 0 to 1
  vtkSetMacro(Opacity, double);
  vtkGetMacro(Opacity, double);

  // Description:
  // Ambient of the surface expressed as a number from 0 to 100
  vtkSetMacro(Ambient, double);
  vtkGetMacro(Ambient, double);
  
  // Description:
  // Diffuse of the surface expressed as a number from 0 to 100
  vtkSetMacro(Diffuse, double);
  vtkGetMacro(Diffuse, double);
  
  // Description:
  // Specular of the surface expressed as a number from 0 to 100
  vtkSetMacro(Specular, double);
  vtkGetMacro(Specular, double);

  // Description:
  // Power of the surface expressed as a number from 0 to 100
  vtkSetMacro(Power, double);
  vtkGetMacro(Power, double);

  // Description:
  // Indicates if the surface is visible
  vtkBooleanMacro(Visibility, int);
  vtkGetMacro(Visibility, int);
  vtkSetMacro(Visibility, int);

  
  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
  
protected:
  vtkMRMLFiducialListDisplayNode();
  ~vtkMRMLFiducialListDisplayNode();
  vtkMRMLFiducialListDisplayNode(const vtkMRMLFiducialListDisplayNode&);
  void operator=(const vtkMRMLFiducialListDisplayNode&);

  // Strings

    
  // Numbers
  double Opacity;
  double Ambient;
  double Diffuse;
  double Specular;
  double Power;
  

  // Booleans
  int Visibility;

  // Arrays
  double Color[3];


};

#endif
