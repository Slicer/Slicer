/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
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

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformableNode.h"
#include "vtkMatrix4x4.h"
#include "vtkCamera.h"

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
  void SetParallelProjection(int parallelProjection) 
    {
    this->Camera->SetParallelProjection(parallelProjection);
    };
  
  /// 
  /// Set camera ParallelProjection flag   
  int GetParallelProjection()
    {
    return this->Camera->GetParallelProjection();
    };

  /// 
  /// Set camera Parallel Scale 
  void SetParallelScale(double scale) 
    {
    this->Camera->SetParallelScale(scale);
    };
  
  /// 
  /// Set camera Parallel Scale   
  double GetParallelScale()
    {
    return this->Camera->GetParallelScale();
    };

  /// 
  /// Set camera Position 
  void SetPosition(double position[3]) 
    {
    this->Camera->SetPosition(position);
    };
  
  /// 
  /// Get camera Position   
  double *GetPosition()
    {
    return this->Camera->GetPosition();
    };

  /// 
  /// Set camera Focal Point 
  void SetFocalPoint(double focalPoint[3]) 
    {
    this->Camera->SetFocalPoint(focalPoint);
    };
  
  /// 
  /// Get camera Focal Point 
  double *GetFocalPoint()
    {
    return this->Camera->GetFocalPoint();
    };

  /// 
  /// Set camera Up vector
  void SetViewUp(double viewUp[3]) 
    {
    this->Camera->SetViewUp(viewUp);
    };
  
  /// 
  /// Get camera Up vector
  double *GetViewUp()
    {
      return this->Camera->GetViewUp();
    };
  
  /// 
  /// alternative method to propagate events generated in Camera nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

  ///
  /// This is the transform that was last applied
  /// to the position, focal point, and up vector
  /// (for any new transforms, the incremental difference
  /// is calculated and applied to the parameters)
  vtkGetObjectMacro(AppliedTransform, vtkMatrix4x4);
  vtkSetObjectMacro(AppliedTransform, vtkMatrix4x4);

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

  /// 
  /// Reimplement from parent
  virtual bool CanApplyNonLinearTransforms() { return false; }
  virtual void ApplyTransform(vtkAbstractTransform* transform);
  virtual void ApplyTransform(vtkMatrix4x4* transformMatrix)
    { Superclass::ApplyTransform(transformMatrix); }

protected:
  vtkMRMLCameraNode();
  ~vtkMRMLCameraNode();
  vtkMRMLCameraNode(const vtkMRMLCameraNode&);
  void operator=(const vtkMRMLCameraNode&);


  vtkSetObjectMacro(Camera, vtkCamera); 
  void SetAndObserveCamera(vtkCamera *camera);
  vtkCamera *Camera;

  vtkMRMLCameraNode* FindActiveTagInScene(const char *tag);

  vtkSetReferenceStringMacro(InternalActiveTag);
  char *InternalActiveTag;

  vtkMatrix4x4 *AppliedTransform;
};

#endif
