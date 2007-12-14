/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLDisplayNode - a supercalss for other storage nodes
// .SECTION Description
// a supercalss for other storage nodes like volume and model

#ifndef __vtkMRMLDisplayNode_h
#define __vtkMRMLDisplayNode_h


#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLColorNode.h"

#include "vtkImageData.h"
#include "vtkPolyData.h"

class vtkMRMLDisplayableNode;

class VTK_MRML_EXPORT vtkMRMLDisplayNode : public vtkMRMLNode
{
  public:
  static vtkMRMLDisplayNode *New(){return NULL;};
  vtkTypeMacro(vtkMRMLDisplayNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance() = 0;
  
  // Description:
  // Gets PlyData converted from the real data in the node
  virtual vtkPolyData* GetPolyData() {return NULL;};
  
  // Description:
  // Gets ImageData converted from the real data in the node
  virtual vtkImageData* GetImageData() {return NULL;};

  // Description:
  // Gets associated dispayable node 
  virtual vtkMRMLDisplayableNode* GetDisplayableNode();

  // Description:
  // Update the pipeline based on this node attributes
  virtual void UpdatePolyDataPipeline() {};
 
  // Description:
  // Update the pipeline based on this node attributes
  virtual void UpdateImageDataPipeline() {};
 
  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);


  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName() = 0;


  // Description:
  // Propagate Progress Event generated in ReadData
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);
  
  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // Model's color (r,g,b)
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
  // Specifies whether to clip the surface with the slice planes
  vtkBooleanMacro(Clipping, int);
  vtkGetMacro(Clipping, int);
  vtkSetMacro(Clipping, int);

  // Description:
  // Indicates whether to cull (not render) the backface of the surface
  vtkBooleanMacro(BackfaceCulling, int);
  vtkGetMacro(BackfaceCulling, int);
  vtkSetMacro(BackfaceCulling, int);

  // Description:
  // Indicates whether to render the scalar value associated with each polygon vertex
  vtkBooleanMacro(ScalarVisibility, int);
  vtkGetMacro(ScalarVisibility, int);
  vtkSetMacro(ScalarVisibility, int);

  // Description:
  // Indicates whether to render the vector value associated with each polygon vertex
  vtkBooleanMacro(VectorVisibility, int);
  vtkGetMacro(VectorVisibility, int);
  vtkSetMacro(VectorVisibility, int);

  // Description:
  // Indicates whether to render the tensor value associated with each polygon vertex
  vtkBooleanMacro(TensorVisibility, int);
  vtkGetMacro(TensorVisibility, int);
  vtkSetMacro(TensorVisibility, int);

  // Description:
  // Range of scalar values to render rather than the single color designated by colorName
  vtkSetVector2Macro(ScalarRange, double);
  vtkGetVector2Macro(ScalarRange, double);


  // Description:
  // Associated ImageData
  vtkGetObjectMacro(TextureImageData, vtkImageData);
  void SetAndObserveTextureImageData(vtkImageData *ImageData);

  // Description:
  // Set a default color node
//  void SetDefaultColorMap();
  
  // Description:
  // String ID of the color MRML node
  virtual void SetAndObserveColorNodeID(const char *ColorNodeID);
  //BTX
  virtual void SetAndObserveColorNodeID(const std::string& ColorNodeID);
  //ETX
  vtkGetStringMacro(ColorNodeID);

  // Description:
  // Get associated color MRML node
  virtual vtkMRMLColorNode* GetColorNode();

  // Description:
  // the name of the currently active scalar field for this model
  vtkGetStringMacro(ActiveScalarName);
  // Description:
  // set the active scalar field name, and update the color table if necessary
  void SetActiveScalarName(const char *scalarName);
    
  
  
  
protected:
  vtkMRMLDisplayNode() ;
  ~vtkMRMLDisplayNode();
  vtkMRMLDisplayNode(const vtkMRMLDisplayNode&);
  void operator=(const vtkMRMLDisplayNode&);
  
  vtkSetObjectMacro(TextureImageData, vtkImageData);
  
  vtkImageData    *TextureImageData;
  
  char *ColorNodeID;

  char *ActiveScalarName;
  
  vtkSetReferenceStringMacro(ColorNodeID);

  vtkMRMLColorNode *ColorNode;

  // Numbers
  double Opacity;
  double Ambient;
  double Diffuse;
  double Specular;
  double Power;
  

  // Booleans
  int Visibility;
  int Clipping;
  int BackfaceCulling;
  int ScalarVisibility;
  int VectorVisibility;
  int TensorVisibility;

  // Arrays
  double ScalarRange[2];
  double Color[3];  

};

#endif

