/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLDisplayNode - a supercalss for other storage nodes
/// 
/// a supercalss for other storage nodes like volume and model

#ifndef __vtkMRMLDisplayNode_h
#define __vtkMRMLDisplayNode_h

// MRML includes
#include "vtkMRMLNode.h"
class vtkMRMLColorNode;
class vtkMRMLDisplayableNode;

// VTK includes
class vtkImageData;
class vtkPolyData;

// STD includes
#include <vector>

class VTK_MRML_EXPORT vtkMRMLDisplayNode : public vtkMRMLNode
{
public:
  vtkTypeMacro(vtkMRMLDisplayNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance() = 0;

  ///
  /// Returns the first displayable node that is associated to this display node
  /// Warning: This function is slow as it browses the entire scene to find the
  /// displayable node.
  virtual vtkMRMLDisplayableNode* GetDisplayableNode();

  /// 
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);


  /// 
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName() = 0;


  /// 
  /// Propagate Progress Event generated in ReadData
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  /// 
  /// Updates this node if it depends on other nodes 
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  /// 
  /// Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);
  
  /// 
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  /// Model's color (r,g,b).
  /// Gray (0.5, 0.5, 0.5) by default.
  /// \sa SelectedColor, Ambient, Diffuse, Specular, Power
  vtkSetVector3Macro(Color, double);
  vtkGetVector3Macro(Color, double);

  /// Node's selected ambient color (r,g,b).
  /// Red (1., 0., 0.) by default.
  /// \sa Color, SelectedAmbient, SelectedSpecular
  vtkSetVector3Macro(SelectedColor, double);
  vtkGetVector3Macro(SelectedColor, double);

  /// Node's selected ambient.
  /// 0.4 by default.
  /// \sa SelectedColor, Ambient, SelectedSpecular
  vtkSetMacro(SelectedAmbient, double);
  vtkGetMacro(SelectedAmbient, double);

  /// Node's selected specular.
  /// 0.5 by default.
  /// \sa SelectedColor, SelectedAmbient, Specular
  vtkSetMacro(SelectedSpecular, double);
  vtkGetMacro(SelectedSpecular, double);

  /// Opacity of the surface expressed as a number from 0 to 1.
  /// Opaque (1.) by default.
  /// \sa Color, Visibility, Clipping
  vtkSetMacro(Opacity, double);
  vtkGetMacro(Opacity, double);

  /// Ambient of the surface expressed as a number from 0 to 1.
  /// 0. by default.
  /// \sa Color, SelectedAmbient, Diffuse, Specular, Power
  vtkSetMacro(Ambient, double);
  vtkGetMacro(Ambient, double);

  /// Diffuse of the surface expressed as a number from 0 to 1.
  /// 1. by default.
  /// \sa Color, Ambient, Specular, Power
  vtkSetMacro(Diffuse, double);
  vtkGetMacro(Diffuse, double);

  /// Specular of the surface expressed as a number from 0 to 1.
  /// 0. by default.
  /// \sa Color, Ambient, Diffuse, Power
  vtkSetMacro(Specular, double);
  vtkGetMacro(Specular, double);

  /// Power of the surface specularity expressed as a number from 0 to 100.
  /// 1. by default.
  /// \sa Color, Ambient, Diffuse, Specular
  vtkSetMacro(Power, double);
  vtkGetMacro(Power, double);

  /// Indicates if the surface is visible
  /// True by default.
  /// \sa Color, Opacity, Clipping
  vtkBooleanMacro(Visibility, int);
  vtkGetMacro(Visibility, int);
  vtkSetMacro(Visibility, int);

  /// Specifies whether to clip the surface with the slice planes.
  /// 0 by default.
  /// \sa Visibility, SliceIntersectionVisibility
  vtkBooleanMacro(Clipping, int);
  vtkGetMacro(Clipping, int);
  vtkSetMacro(Clipping, int);

  /// Specifies whether to show model intersections on slice planes.
  /// 0 by default.
  /// \sa Visibility, Clipping
  vtkBooleanMacro(SliceIntersectionVisibility, int);
  vtkGetMacro(SliceIntersectionVisibility, int);
  vtkSetMacro(SliceIntersectionVisibility, int);

  /// Indicates whether to cull (not render) the backface of the surface.
  /// 1 by default.
  /// \sa Visibility, Clipping
  vtkBooleanMacro(BackfaceCulling, int);
  vtkGetMacro(BackfaceCulling, int);
  vtkSetMacro(BackfaceCulling, int);

  /// Indicates whether to render the scalar value associated with each polygon
  /// vertex.
  /// 0 by default.
  /// \sa Visibility, VectorVisibility, TensorVisibility
  vtkBooleanMacro(ScalarVisibility, int);
  vtkGetMacro(ScalarVisibility, int);
  vtkSetMacro(ScalarVisibility, int);

  /// Indicates whether to render the vector value associated with each polygon
  /// vertex.
  /// 0 by default.
  /// \sa Visibility, ScalarVisibility, TensorVisibility
  vtkBooleanMacro(VectorVisibility, int);
  vtkGetMacro(VectorVisibility, int);
  vtkSetMacro(VectorVisibility, int);

  /// Indicates whether to render the tensor value associated with each polygon
  /// vertex.
  /// 0 by default.
  /// \sa Visibility, ScalarVisibility, VectorVisibility
  vtkBooleanMacro(TensorVisibility, int);
  vtkGetMacro(TensorVisibility, int);
  vtkSetMacro(TensorVisibility, int);

  /// Indicates whether to use scalar range from polydata or the one specidied
  /// by ScalarRange.
  /// 1 by default.
  /// \sa ScalarRange
  vtkBooleanMacro(AutoScalarRange, int);
  vtkGetMacro(AutoScalarRange, int);
  vtkSetMacro(AutoScalarRange, int);

  /// Range of scalar values to render rather than the single color designated
  /// by colorName.
  /// (0, 100) by default.
  vtkSetVector2Macro(ScalarRange, double);
  vtkGetVector2Macro(ScalarRange, double);

  /// Associated ImageData to apply as texture.
  /// No texture (NULL) by default.
  /// \sa InterpolateTexture
  vtkGetObjectMacro(TextureImageData, vtkImageData);
  void SetAndObserveTextureImageData(vtkImageData *ImageData);

  /// Indicates whether to use interpolate texture.
  /// 0 by default.
  /// \sa TextureImageData
  vtkBooleanMacro(InterpolateTexture, int);
  vtkGetMacro(InterpolateTexture, int);
  vtkSetMacro(InterpolateTexture, int);

  /// 
  /// Set a default color node
///  void SetDefaultColorMap();
  
  /// 
  /// String ID of the color MRML node
  /// Note that anytime the color node is modified, the observing display node
  /// fires a Modified event.
  virtual void SetAndObserveColorNodeID(const char *ColorNodeID);
  void SetAndObserveColorNodeID(const std::string& ColorNodeID);
  vtkGetStringMacro(ColorNodeID);

  /// 
  /// Get associated color MRML node
  virtual vtkMRMLColorNode* GetColorNode();

  /// 
  /// the name of the currently active scalar field for this model
  vtkGetStringMacro(ActiveScalarName);
  /// 
  /// set the active scalar field name, and update the color table if necessary
  void SetActiveScalarName(const char *scalarName);
    
  /// Add View Node ID for the view to display this node in.
  void AddViewNodeID(const char* viewNodeID);

  /// Remove View Node ID for the view to display this node in.
  void RemoveViewNodeID(char* viewNodeID);

  /// Remove All View Node IDs for the views to display this node in.
  void RemoveAllViewNodeIDs();

  /// Get number of View Node ID's for the view to display this node in.
  /// If 0, display in all views
  int GetNumberOfViewNodeIDs()const
  {
    return static_cast<int>(ViewNodeIDs.size());
  }

  /// Get View Node ID's for the view to display this node in.
  /// If NULL, display in all views
  const char* GetNthViewNodeID(unsigned int index);
  
  /// Get all View Node ID's for the view to display this node in.
  /// If empty, display in all views
  std::vector< std::string > GetViewNodeIDs()const
  {
    return ViewNodeIDs;
  }
  /// True if the view node id is present in the viewnodeid list
  /// false if not found
  bool IsViewNodeIDPresent(const char* viewNodeID)const;

  /// Returns true if the viewNodeID is present in the ViewNodeId list
  /// or there is no ViewNodeId in the list (meaning all the views display the
  /// node)
  bool IsDisplayableInView(const char* viewNodeID)const;

protected:
  vtkMRMLDisplayNode() ;
  ~vtkMRMLDisplayNode();
  vtkMRMLDisplayNode(const vtkMRMLDisplayNode&);
  void operator=(const vtkMRMLDisplayNode&);
  
  virtual void SetColorNodeInternal(vtkMRMLColorNode* newColorNode);
  
  virtual void SetTextureImageData(vtkImageData* imageData);
  
  vtkImageData    *TextureImageData;
  
  char *ColorNodeID;

  char *ActiveScalarName;
  
  vtkMRMLColorNode *ColorNode;

  /// Numbers
  double Opacity;
  double Ambient;
  double Diffuse;
  double Specular;
  double Power;
  double SelectedAmbient;
  double SelectedSpecular;

  /// Booleans
  int Visibility;
  int Clipping;
  int SliceIntersectionVisibility;
  int BackfaceCulling;
  int ScalarVisibility;
  int VectorVisibility;
  int TensorVisibility;
  int AutoScalarRange;
  int InterpolateTexture;

  /// Arrays
  double ScalarRange[2];
  double Color[3];
  double SelectedColor[3];

  /// View node ID's
  std::vector< std::string > ViewNodeIDs;

private:
  void SetColorNodeID(const char* id);
};

#endif

