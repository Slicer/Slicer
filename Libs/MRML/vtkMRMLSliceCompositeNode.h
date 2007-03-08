/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceCompositeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLSliceCompositeNode - MRML node for storing a slice through RAS space
// .SECTION Description
// This node stores the information about how to composite two
// vtkMRMLVolumes into a single display image
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
  virtual const char* GetNodeTagName() {return "SliceComposite";};

  // Description:
  // Updates other nodes in the scene depending on this node
  // or updates this node if it depends on other nodes when the scene is read in
  // This method is called automatically by XML parser after all nodes are created
  virtual void UpdateScene(vtkMRMLScene *);

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // the ID of a MRMLVolumeNode
  vtkGetStringMacro (BackgroundVolumeID);
  vtkSetReferenceStringMacro (BackgroundVolumeID);
  void SetReferenceBackgroundVolumeID(const char *id) { this->SetBackgroundVolumeID(id); }

  // Description:
  // the ID of a MRMLVolumeNode
  // TODO: make this an arbitrary list of layers
  vtkGetStringMacro (ForegroundVolumeID);
  vtkSetReferenceStringMacro (ForegroundVolumeID);
  void SetReferenceForegroundVolumeID(const char *id) { this->SetForegroundVolumeID(id); }

  // Description:
  // the ID of a MRMLVolumeNode
  // TODO: make this an arbitrary list of layers
  vtkGetStringMacro (LabelVolumeID);
  vtkSetReferenceStringMacro (LabelVolumeID);
  void SetReferenceLabelVolumeID(const char *id) { this->SetLabelVolumeID(id); }

  // Description:
  // opacity of the Foreground for rendering over background
  // TODO: make this an arbitrary list of layers
  // TODO: make different composite types (checkerboard, etc)
  vtkGetMacro (ForegroundOpacity, double);
  vtkSetMacro (ForegroundOpacity, double);

  // Description:
  // opacity of the Label for rendering over background
  // TODO: make this an arbitrary list of layers
  // TODO: make different composite types (checkerboard, etc)
  vtkGetMacro (LabelOpacity, double);
  vtkSetMacro (LabelOpacity, double);

  // Description:
  // toggle that gangs control of slice viewers
  vtkGetMacro (LinkedControl, int );
  vtkSetMacro (LinkedControl, int );

  // Description:
  // toggles for grid in different slice layers.
  vtkGetMacro (ForegroundGrid, int );
  vtkSetMacro (ForegroundGrid, int );
  vtkGetMacro (BackgroundGrid, int );
  vtkSetMacro (BackgroundGrid, int );
  vtkGetMacro (LabelGrid, int );
  vtkSetMacro (LabelGrid, int );

  // Description:
  // toggles fiducial visibility in the slice viewer
  vtkGetMacro (FiducialVisibility, int );
  vtkSetMacro (FiducialVisibility, int );
  vtkGetMacro (FiducialLabelVisibility, int );
  vtkSetMacro (FiducialLabelVisibility, int );  

  // Description:
  // configures the annotations
  vtkGetMacro ( AnnotationSpace, int );
  vtkSetMacro ( AnnotationSpace, int );
  vtkGetMacro ( AnnotationMode, int );
  vtkSetMacro ( AnnotationMode, int );
  
  // Description:
  // configures the crosshair appearance and behavior
  vtkGetMacro (CrosshairMode, int );
  vtkSetMacro (CrosshairMode, int );  
  vtkGetMacro (CrosshairBehavior, int );
  vtkSetMacro (CrosshairBehavior, int );  

  // Description:
  // Name of the layout
  void SetLayoutName(const char *layoutName) {
    this->SetSingletonTag(layoutName);
  }
  char *GetLayoutName() {
    return this->GetSingletonTag();
  }

  //BTX
  // Modes for annotation space and mode
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
  // Modes for crosshair display and behavior
  enum
    {
      NoCrosshair = 0,
      ShowBasic,
      ShowIntersection,
      ShowHashmarks,
      ShowAll
    };
  enum
    {
      Normal = 0,
      JumpSlice
    };
  //ETX

protected:
  vtkMRMLSliceCompositeNode();
  ~vtkMRMLSliceCompositeNode();
  vtkMRMLSliceCompositeNode(const vtkMRMLSliceCompositeNode&);
  void operator=(const vtkMRMLSliceCompositeNode&);

  char *BackgroundVolumeID;
  char *ForegroundVolumeID;
  char *LabelVolumeID;
  double ForegroundOpacity;

  double LabelOpacity;
  int LinkedControl;

  int ForegroundGrid;
  int BackgroundGrid;
  int LabelGrid;
  
  int FiducialVisibility;
  int FiducialLabelVisibility;
  
  int AnnotationSpace;
  int AnnotationMode;
  
  int CrosshairMode;
  int CrosshairBehavior;

};

#endif

