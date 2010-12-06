/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceCompositeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLSliceCompositeNode - MRML node for storing a slice through RAS space
/// 
/// This node stores the information about how to composite two
/// vtkMRMLVolumes into a single display image
//

#ifndef __vtkMRMLSliceCompositeNode_h
#define __vtkMRMLSliceCompositeNode_h

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

#include "vtkMatrix4x4.h"

class VTK_MRML_EXPORT vtkMRMLSliceCompositeNode : public vtkMRMLNode
{
  public:
  static vtkMRMLSliceCompositeNode *New();
  vtkTypeMacro(vtkMRMLSliceCompositeNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "SliceComposite";};

  /// 
  /// Updates this node if it depends on other nodes 
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  /// 
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  /// 
  /// the ID of a MRMLVolumeNode
  vtkGetStringMacro (BackgroundVolumeID);
  vtkSetReferenceStringMacro (BackgroundVolumeID);
  void SetReferenceBackgroundVolumeID(const char *id) { this->SetBackgroundVolumeID(id); }

  /// 
  /// the ID of a MRMLVolumeNode
  /// TODO: make this an arbitrary list of layers
  vtkGetStringMacro (ForegroundVolumeID);
  vtkSetReferenceStringMacro (ForegroundVolumeID);
  void SetReferenceForegroundVolumeID(const char *id) { this->SetForegroundVolumeID(id); }

  /// 
  /// the ID of a MRMLVolumeNode
  /// TODO: make this an arbitrary list of layers
  vtkGetStringMacro (LabelVolumeID);
  vtkSetReferenceStringMacro (LabelVolumeID);
  void SetReferenceLabelVolumeID(const char *id) { this->SetLabelVolumeID(id); }

  /// 
  /// Compositing mode for foreground and background can be alpha
  /// blending, reverse alpha blending, addition, or subtraction
  vtkGetMacro (Compositing, int);
  vtkSetMacro (Compositing, int);
  
  /// 
  /// opacity of the Foreground for rendering over background
  /// TODO: make this an arbitrary list of layers
  /// TODO: make different composite types (checkerboard, etc)
  vtkGetMacro (ForegroundOpacity, double);
  vtkSetMacro (ForegroundOpacity, double);

  /// 
  /// opacity of the Label for rendering over background
  /// TODO: make this an arbitrary list of layers
  /// TODO: make different composite types (checkerboard, etc)
  vtkGetMacro (LabelOpacity, double);
  vtkSetMacro (LabelOpacity, double);

  /// 
  /// toggle that gangs control of slice viewers
  vtkGetMacro (LinkedControl, int );
  vtkSetMacro (LinkedControl, int );

  /// 
  /// toggles for grid in different slice layers.
  vtkGetMacro (ForegroundGrid, int );
  vtkSetMacro (ForegroundGrid, int );
  vtkGetMacro (BackgroundGrid, int );
  vtkSetMacro (BackgroundGrid, int );
  vtkGetMacro (LabelGrid, int );
  vtkSetMacro (LabelGrid, int );

  /// 
  /// toggles fiducial visibility in the slice viewer
  vtkGetMacro (FiducialVisibility, int );
  vtkSetMacro (FiducialVisibility, int );
  vtkGetMacro (FiducialLabelVisibility, int );
  vtkSetMacro (FiducialLabelVisibility, int );  

  /// 
  /// toggles visibility of intersections of other slices in the slice viewer
  vtkGetMacro (SliceIntersectionVisibility, int );
  vtkSetMacro (SliceIntersectionVisibility, int );  

  /// 
  /// configures the annotations
  vtkGetMacro ( AnnotationSpace, int );
  vtkSetMacro ( AnnotationSpace, int );
  vtkGetMacro ( AnnotationMode, int );
  vtkSetMacro ( AnnotationMode, int );
  
  /// 
  /// configures the behavior of PropagateVolumeSelection(): 
  /// if set to false, the background/label for slice views
  /// will not be reset. Default value is true
  vtkSetMacro (DoPropagateVolumeSelection, bool );
  vtkGetMacro (DoPropagateVolumeSelection, bool );
  
  /// 
  /// Name of the layout
  void SetLayoutName(const char *layoutName) {
    this->SetSingletonTag(layoutName);
  }
  char *GetLayoutName() {
    return this->GetSingletonTag();
  }

  /// Modes for annotation space and mode
  enum
    {
      XYZ = 0,
      IJK,
      RAS,
      IJKAndRAS
    };
  enum
    {
      NoAnnotation = 0,
      All,
      LabelValuesOnly,
      LabelAndVoxelValuesOnly
    };
  /// Modes for compositing
  enum
    {
      Alpha = 0,
      ReverseAlpha,
      Add,
      Subtract
    };

protected:
  vtkMRMLSliceCompositeNode();
  ~vtkMRMLSliceCompositeNode();
  vtkMRMLSliceCompositeNode(const vtkMRMLSliceCompositeNode&);
  void operator=(const vtkMRMLSliceCompositeNode&);

  char *BackgroundVolumeID;
  char *ForegroundVolumeID;
  char *LabelVolumeID;
  double ForegroundOpacity;

  int Compositing;
  
  double LabelOpacity;
  int LinkedControl;

  int ForegroundGrid;
  int BackgroundGrid;
  int LabelGrid;
  
  int FiducialVisibility;
  int FiducialLabelVisibility;

  int SliceIntersectionVisibility;
  
  int AnnotationSpace;
  int AnnotationMode;
  
  bool DoPropagateVolumeSelection;
};

#endif

