/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCameraNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
///  vtkMRMLClipModelsNode - MRML node to represent camera node
/// 
/// Camera node uses vtkCamera to store the state of the camera

#ifndef __vtkMRMLCameraNode_h
#define __vtkMRMLCameraNode_h

// MRML includes
#include "vtkMRMLTransformableNode.h"

// VTK includes
class vtkCamera;
class vtkMatrix4x4;

class VTK_MRML_EXPORT vtkMRMLCameraNode : public vtkMRMLTransformableNode
{
public:
  static vtkMRMLCameraNode *New();
  vtkTypeMacro(vtkMRMLCameraNode,vtkMRMLTransformableNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

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
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Camera";};

  /// 
  /// Set the camera active tag, i.e. the tag for which object (view) this
  /// camera is active.
  const char* GetActiveTag();
  virtual void SetActiveTag(const char *);

  /// 
  /// vtkCamera
  vtkGetObjectMacro(Camera, vtkCamera); 

  /// 
  /// Set camera ParallelProjection flag 
  void SetParallelProjection(int parallelProjection);
  
  /// 
  /// Set camera ParallelProjection flag   
  int GetParallelProjection();

  /// 
  /// Set camera Parallel Scale 
  void SetParallelScale(double scale);
  
  /// 
  /// Set camera Parallel Scale   
  double GetParallelScale();

  /// 
  /// Set camera Position 
  void SetPosition(double position[3]);
  
  /// 
  /// Get camera Position   
  double *GetPosition();

  /// 
  /// Set camera Focal Point 
  void SetFocalPoint(double focalPoint[3]);
  
  /// 
  /// Get camera Focal Point 
  double *GetFocalPoint();

  /// 
  /// Set camera Up vector
  void SetViewUp(double viewUp[3]);
  
  /// 
  /// Get camera Up vector
  double *GetViewUp();
  
  /// 
  /// alternative method to propagate events generated in Camera nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

  /// This is the transform that was last applied
  /// to the position, focal point, and up vector
  /// (for any new transforms, the incremental difference
  /// is calculated and applied to the parameters)
  vtkGetObjectMacro(AppliedTransform, vtkMatrix4x4);
  virtual void SetAppliedTransform(vtkMatrix4x4* appliedTransform);

  /// 
  /// Events
  enum
  {
    ActiveTagModifiedEvent = 30000
  };

  /// 
  /// Updates this node if it depends on other nodes 
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  /// 
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

protected:
  vtkMRMLCameraNode();
  ~vtkMRMLCameraNode();
  vtkMRMLCameraNode(const vtkMRMLCameraNode&);
  void operator=(const vtkMRMLCameraNode&);


  void SetCamera(vtkCamera* camera); 
  void SetAndObserveCamera(vtkCamera *camera);
  vtkCamera *Camera;

  vtkMRMLCameraNode* FindActiveTagInScene(const char *tag);

  void SetInternalActiveTag(const char* id);
  char *InternalActiveTag;

  vtkMatrix4x4 *AppliedTransform;
};

#endif
