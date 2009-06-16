/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: $
Date:      $Date: $
Version:   $Revision: $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkLinearTransform.h"

#include "vtkMRML4DBundleNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRML4DBundleNode* vtkMRML4DBundleNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRML4DBundleNode");
  if(ret)
    {
    return (vtkMRML4DBundleNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRML4DBundleNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRML4DBundleNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRML4DBundleNode");
  if(ret)
    {
    return (vtkMRML4DBundleNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRML4DBundleNode;
}

//----------------------------------------------------------------------------
vtkMRML4DBundleNode::vtkMRML4DBundleNode()
{
  this->MatrixTransformToParent = NULL;

  vtkMatrix4x4 *matrix  = vtkMatrix4x4::New();
  matrix->Identity();
  this->SetAndObserveMatrixTransformToParent(matrix);
  matrix->Delete();

  this->FrameNodeIDList.clear();
  this->TransformNodeIDList.clear();

  this->DisplayBufferNodeIDList.resize(2);
  this->DisplayBufferNodeIDList[0] = "";
  this->DisplayBufferNodeIDList[1] = "";

  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRML4DBundleNode::~vtkMRML4DBundleNode()
{
  if (this->MatrixTransformToParent) 
    {
    this->SetAndObserveMatrixTransformToParent(NULL);
    }

  this->FrameNodeIDList.clear();
  this->TransformNodeIDList.clear();
}

//----------------------------------------------------------------------------
void vtkMRML4DBundleNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " DisplayBuffer0=\"" << this->DisplayBufferNodeIDList[0] << "\"";
  of << indent << " DisplayBuffer1=\"" << this->DisplayBufferNodeIDList[1] << "\"";

  NodeIDListType::iterator iter;
  int index = 0;

  for (iter = this->FrameNodeIDList.begin(); iter != this->FrameNodeIDList.end(); iter ++)
    {
    of << indent << " Frame" << index << "=\"" << *iter << "\"";
    index ++;
    }

}


//----------------------------------------------------------------------------
void vtkMRML4DBundleNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    std::cerr << "attName = " << attName << "." << std::endl;
    std::cerr << "attValue = " << attValue << "." << std::endl;
      
    if (!strcmp(attName, "DisplayBuffer0")) 
      {
      this->DisplayBufferNodeIDList[0] =attValue ;
      }
    if (!strcmp(attName, "DisplayBuffer1")) 
      {
      this->DisplayBufferNodeIDList[1] =attValue ;
      }
    if (!strncmp("Frame", attName, 5))
      {
      std::cerr << "Frames" << std::endl;
      const char* suffix = &attName[5];
      char** endptr;
      long index = strtol(suffix, endptr, 10);
      std::cerr << "index = " << index << std::endl;
      if (index >= this->FrameNodeIDList.size())
        {
        this->FrameNodeIDList.resize(index+1);
        }
      this->FrameNodeIDList[index] = attValue;
      }
    }
  this->FrameNodeIDList.clear();
  this->DisplayBufferNodeIDList.clear();
  this->Modified();

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRML4DBundleNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRML4DBundleNode *node = (vtkMRML4DBundleNode *) anode;
  for (int i=0; i<4; i++) 
    {
    for (int j=0; j<4; j++)
      {
      this->GetMatrixTransformToParent()
        ->SetElement(i,j,(node->MatrixTransformToParent->GetElement(i,j)));
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRML4DBundleNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  if (this->MatrixTransformToParent != NULL) 
    {
    os << indent << "MatrixTransformToParent: " << "\n";
    for (int row=0; row<4; row++) 
      {
      for (int col=0; col<4; col++) 
        {
        os << this->MatrixTransformToParent->GetElement(row, col);
        if (!(row==3 && col==3)) 
          {
          os << " ";
          }
        else 
          {
          os << "\n";
          }
        } // for (int col
      } // for (int row
    }
}


//---------------------------------------------------------------------------
void vtkMRML4DBundleNode::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event, 
                                                    void *callData )
{
  Superclass::ProcessMRMLEvents ( caller, event, callData );

//  if (this->MatrixTransformToParent != NULL && this->MatrixTransformToParent == vtkMatrix4x4::SafeDownCast(caller) &&
//      event ==  vtkCommand::ModifiedEvent)
//    {
//    this->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent, NULL);
//    }
}


//----------------------------------------------------------------------------
int vtkMRML4DBundleNode::GetNumberOfFrames()
{

  return this->FrameNodeIDList.size();

}


//----------------------------------------------------------------------------
int vtkMRML4DBundleNode::InsertFrame(int i, const char* nodeID)
{
  int index;

  if (i < 0)
    {
    index = 0;
    }
  else if (i > this->FrameNodeIDList.size())
    {
    index = this->FrameNodeIDList.size();
    }
  else
    {
    index = i;
    }

  NodeIDListType::iterator iter;
  iter = this->FrameNodeIDList.begin();
  iter += index;

  this->FrameNodeIDList.insert(iter, std::string(nodeID));

  this->Modified();

  return index;
  
}


//----------------------------------------------------------------------------
int vtkMRML4DBundleNode::AddFrame(const char* nodeID)
{
  this->FrameNodeIDList.push_back(std::string(nodeID));
  this->Modified();
  return 1;
}


//----------------------------------------------------------------------------
int vtkMRML4DBundleNode::RemoveFrame(int i)
{

  int index;

  if (i < 0)
    {
    index = 0;
    }
  else if (i > this->FrameNodeIDList.size())
    {
    index = this->FrameNodeIDList.size();
    }
  else
    {
    index = i;
    }

  NodeIDListType::iterator iter;
  iter = this->FrameNodeIDList.begin();
  iter += index;

  /*
  vtkMRMLNode* node =
    vtkMRMLNode::SafeDownCast(this->GetScene()
                              ->GetNodeByID(iter->c_str()));
  */
  this->FrameNodeIDList.erase(iter);

  this->Modified();

  return index;

}


//----------------------------------------------------------------------------
int vtkMRML4DBundleNode::RemoveFrame(const char* nodeID)
{
  
  int index;

  NodeIDListType::iterator iter;
  for (iter = this->FrameNodeIDList.begin(); iter != this->FrameNodeIDList.end(); iter ++)
    {
    if (*iter == nodeID)
      {
      this->FrameNodeIDList.erase(iter);
      return index;
      }
    index ++;
    }

  return -1;
}


//----------------------------------------------------------------------------
void vtkMRML4DBundleNode::RemoveAllFrames()
{
  this->FrameNodeIDList.clear();
}


//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRML4DBundleNode::GetFrameNode(int i)
{
  if (i < 0 || i >= this->FrameNodeIDList.size())
    {
    return NULL;
    }
  
  vtkMRMLNode* node =
    vtkMRMLNode::SafeDownCast(this->GetScene()
                              ->GetNodeByID(this->FrameNodeIDList[i].c_str()));

  return node;

}


//----------------------------------------------------------------------------
int vtkMRML4DBundleNode::SetDisplayBufferNodeID(int bufferIndex, const char* nodeID)
{
  this->DisplayBufferNodeIDList[bufferIndex] = nodeID;
  return 1;
}


//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRML4DBundleNode::GetDisplayBufferNode(int bufferIndex)
{
  vtkMRMLNode* node =
    vtkMRMLNode::SafeDownCast(this->GetScene()
                              ->GetNodeByID(this->DisplayBufferNodeIDList[bufferIndex].c_str()));

  return node;
}
  

//----------------------------------------------------------------------------
void vtkMRML4DBundleNode::SwitchDisplayBuffer(int bufferIndex, int i)
  // this function should be implemented in the child class
{
  vtkMRMLScalarVolumeNode* frame = 
    vtkMRMLScalarVolumeNode::SafeDownCast(this->GetFrameNode(i));

  vtkMRMLScalarVolumeNode* displayBuffer = 
    vtkMRMLScalarVolumeNode::SafeDownCast(this->GetDisplayBufferNode(bufferIndex));

  if (frame && displayBuffer)
    {
    vtkImageData* imageData = displayBuffer->GetImageData();
    imageData->DeepCopy(frame->GetImageData());
    displayBuffer->Modified();
    displayBuffer->InvokeEvent(vtkMRMLVolumeNode::ImageDataModifiedEvent);
    }

}



// End
