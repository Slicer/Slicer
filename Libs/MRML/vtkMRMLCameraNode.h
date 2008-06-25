/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLCameraNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/
// .NAME vtkMRMLClipModelsNode - MRML node to represent camera node
// .SECTION Description
// Camera node uses vtkCamera to store the state of the camera

#ifndef __vtkMRMLCameraNode_h
#define __vtkMRMLCameraNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

#include "vtkCamera.h"

class VTK_MRML_EXPORT vtkMRMLCameraNode : public vtkMRMLNode
{
public:
  static vtkMRMLCameraNode *New();
  vtkTypeMacro(vtkMRMLCameraNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

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
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Camera";};

  // Description:
  // Indicates if the camera is active
  vtkGetMacro(Active, int);

  // Description:
  // Make this camera node active/inactive 
  // NOTE it makes sure that only one node is active in the scene
  void SetActive(int active) 
  {
    if (active == this->Active)
      {
      return;
      }
    if (active == 1) 
      {
      this->MakeOthersInActive();
      }
    this->Active = active;
    this->Modified();
  };

  // Description:
  // vtkCamera
  vtkGetObjectMacro(Camera, vtkCamera); 

  // Description:
  // Set camera ParallelProjection flag 
  void SetParallelProjection(int parallelProjection) 
    {
    this->Camera->SetParallelProjection(parallelProjection);
    };
  
  // Description:
  // Set camera ParallelProjection flag   
  int GetParallelProjection()
    {
    return this->Camera->GetParallelProjection();
    };

  // Description:
  // Set camera Parallel Scale 
  void SetParallelScale(double scale) 
    {
    this->Camera->SetParallelScale(scale);
    };
  
  // Description:
  // Set camera Parallel Scale   
  double GetParallelScale()
    {
    return this->Camera->GetParallelScale();
    };

  // Description:
  // Set camera Position 
  void SetPosition(double position[3]) 
    {
    this->Camera->SetPosition(position);
    };
  
  // Description:
  // Get camera Position   
  double *GetPosition()
    {
    return this->Camera->GetPosition();
    };

  // Description:
  // Set camera Focal Point 
  void SetFocalPoint(double focalPoint[3]) 
    {
    this->Camera->SetFocalPoint(focalPoint);
    };
  
  // Description:
  // Get camera Focal Point 
  double *GetFocalPoint()
    {
    return this->Camera->GetFocalPoint();
    };

  // Description:
  // Set camera Up vector
  void SetViewUp(double viewUp[3]) 
    {
    this->Camera->SetViewUp(viewUp);
    };
  
  // Description:
  // Get camera Up vector
  double *GetViewUp()
    {
      return this->Camera->GetViewUp();
    };
  
  // Description:
  // alternative method to propagate events generated in Camera nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
protected:
  vtkMRMLCameraNode();
  ~vtkMRMLCameraNode();
  vtkMRMLCameraNode(const vtkMRMLCameraNode&);
  void operator=(const vtkMRMLCameraNode&);


  vtkSetObjectMacro(Camera, vtkCamera); 
  void SetAndObserveCamera(vtkCamera *camera);
  vtkCamera *Camera;

  void MakeOthersInActive();
  int Active;
};

#endif
