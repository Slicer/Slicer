/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
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

#include "vtkMRMLTimeSeriesBundleNode.h"
#include "vtkMRMLScene.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTimeSeriesBundleNode);

//----------------------------------------------------------------------------
vtkMRMLTimeSeriesBundleNode::vtkMRMLTimeSeriesBundleNode()
{
  this->FrameNodeIDList.clear();
  this->TimeStampList.clear();
  //this->TransformNodeIDList.clear();

  this->DisplayBufferNodeIDList.resize(2);
  this->DisplayBufferNodeIDList[0] = "";
  this->DisplayBufferNodeIDList[1] = "";

  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLTimeSeriesBundleNode::~vtkMRMLTimeSeriesBundleNode()
{
  this->FrameNodeIDList.clear();
  this->TimeStampList.clear();
  //this->TransformNodeIDList.clear();
}

//----------------------------------------------------------------------------
void vtkMRMLTimeSeriesBundleNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  //---
  //--- DisplayBufferNodeList
  //---
  of << indent << " DisplayBuffer0=\"" << this->DisplayBufferNodeIDList[0] << "\"";
  of << indent << " DisplayBuffer1=\"" << this->DisplayBufferNodeIDList[1] << "\"";

  NodeIDListType::iterator iter;
  int index = 0;

  //---
  //--- FrameNodeIDList
  //---
  for (iter = this->FrameNodeIDList.begin(); iter != this->FrameNodeIDList.end(); iter ++)
    {
    of << indent << " Frame" << index << "=\"" << *iter << "\"";
    index ++;
    }

  //---
  //--- TimeStampList
  //---

  index = 0;
  TimeStampListType::iterator iter2;
  unsigned int sec, nanosec;
  for (iter2 = this->TimeStampList.begin(); iter2 != this->TimeStampList.end(); iter2 ++)
    {
    sec = this->TimeStampList[index].second;
    nanosec = this->TimeStampList[index].nanosecond;
    of << indent << " TimePoint_second_" << index << "=\"" << sec << "\"";
    of << indent << " TimePoint_nanosecond_" << index << "=\"" << nanosec << "\"";
    index ++;
    }

}


//----------------------------------------------------------------------------
void vtkMRMLTimeSeriesBundleNode::ReadXMLAttributes(const char** atts)
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
      
   //---
   //--- DisplayBufferNodeList
   //---
   if (!strcmp(attName, "DisplayBuffer0")) 
     {
     this->DisplayBufferNodeIDList[0] =attValue ;
     }
   if (!strcmp(attName, "DisplayBuffer1")) 
     {
     this->DisplayBufferNodeIDList[1] =attValue ;
     }

   //---
   //--- FrameNodeIDList
   //---
   if (!strncmp("Frame", attName, 5))
     {
     std::cerr << "Frames" << std::endl;
     const char* suffix = &attName[5];
     char** endptr = NULL;
     unsigned long index = strtol(suffix, endptr, 10);
     std::cerr << "index = " << index << std::endl;
     if (index >= this->FrameNodeIDList.size())
       {
       this->FrameNodeIDList.resize(index+1);
       }
     this->FrameNodeIDList[index] = attValue;
     }


   //---
   //--- TimeStampList
   //---

   if (!strncmp("TimePoint_second_", attName, 17))
     {
     std::cerr << "TimePoint_second_" << std::endl;
     const char* suffix = &attName[17];
     char** endptr = NULL;
     unsigned long index = strtol(suffix, endptr, 10);
     std::cerr << "index = " << index << std::endl;
     if (index >= this->TimeStampList.size())
       {
       this->TimeStampList.resize(index+1);
       }
     this->TimeStampList[index].second = (unsigned int)(atoi (attValue));
     }
   
   if (!strncmp("TimePoint_nanosecond_", attName, 21))
     {
     std::cerr << "TimePoint_second_" << std::endl;
     const char* suffix = &attName[21];
     char** endptr = NULL;
     unsigned long index = strtol(suffix, endptr, 10);
     std::cerr << "index = " << index << std::endl;
     if (index >= this->TimeStampList.size())
       {
       this->TimeStampList.resize(index+1);
       }
     this->TimeStampList[index].nanosecond = (unsigned int)(atoi (attValue));
     }
   }

 this->Modified();

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLTimeSeriesBundleNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLTimeSeriesBundleNode *node = (vtkMRMLTimeSeriesBundleNode *) anode;
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
void vtkMRMLTimeSeriesBundleNode::PrintSelf(ostream& os, vtkIndent indent)
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
void vtkMRMLTimeSeriesBundleNode::ProcessMRMLEvents ( vtkObject *caller,
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
int vtkMRMLTimeSeriesBundleNode::GetNumberOfFrames()
{

  return this->FrameNodeIDList.size();

}


//----------------------------------------------------------------------------
int vtkMRMLTimeSeriesBundleNode::InsertFrame(int i, const char* nodeID, TimeStamp* ts)
{
  unsigned int index;

  if (i < 0)
    {
    index = 0;
    }
  else if (i > (int)(this->FrameNodeIDList.size()))
    {
    index = this->FrameNodeIDList.size();
    }
  else
    {
    index = i;
    }


  // Add the ID node to the frame list
  NodeIDListType::iterator iter;
  iter = this->FrameNodeIDList.begin();
  iter += index;

  this->FrameNodeIDList.insert(iter, std::string(nodeID));
  
  // Add the time stamp to the frame list
  TimeStampListType::iterator titer;
  titer = this->TimeStampList.begin();
  titer += index;

  TimeStamp times;
  if (ts)
    {
    times.second     = ts->second;
    times.nanosecond = ts->nanosecond;
    }
  else
    {
    times.second     = 0;
    times.nanosecond = 0;
    }
  this->TimeStampList.insert(titer, times);

    
  this->Modified();

  return index;
  
}


//----------------------------------------------------------------------------
int vtkMRMLTimeSeriesBundleNode::InsertFrame(int i, const char* nodeID, unsigned int second, unsigned int microsecond)
{
  unsigned int index;

  if (i < 0)
    {
    index = 0;
    }
  else if (i > (int)(this->FrameNodeIDList.size()))
    {
    index = this->FrameNodeIDList.size();
    }
  else
    {
    index = i;
    }


  // Add the ID node to the frame list
  NodeIDListType::iterator iter;
  iter = this->FrameNodeIDList.begin();
  iter += index;

  this->FrameNodeIDList.insert(iter, std::string(nodeID));
  
  // Add the time stamp to the frame list
  TimeStampListType::iterator titer;
  titer = this->TimeStampList.begin();
  titer += index;

  TimeStamp times;
  times.second     = second + microsecond / 1000000;
  times.nanosecond = (microsecond % 1000000) * 1000;

  this->TimeStampList.insert(titer, times);

    
  this->Modified();

  return index;

}


//----------------------------------------------------------------------------
int vtkMRMLTimeSeriesBundleNode::AddFrame(const char* nodeID, TimeStamp* ts)
{
  this->FrameNodeIDList.push_back(std::string(nodeID));

  TimeStamp times;
  if (ts)
    {
    times.second     = ts->second;
    times.nanosecond = ts->nanosecond;
    }
  else
    {
    times.second     = 0;
    times.nanosecond = 0;
    }
  this->TimeStampList.push_back(times);

  this->Modified();

  return 1;

}


//----------------------------------------------------------------------------
int vtkMRMLTimeSeriesBundleNode::AddFrame(const char* nodeID, unsigned int second, unsigned int microsecond)
{
  this->FrameNodeIDList.push_back(std::string(nodeID));

  TimeStamp times;
  times.second     = second + microsecond / 1000000;
  times.nanosecond = (microsecond % 1000000) * 1000;
  this->TimeStampList.push_back(times);

  this->Modified();

  return 1;

}


//----------------------------------------------------------------------------
int vtkMRMLTimeSeriesBundleNode::RemoveFrame(int i)
{

  unsigned int index;

  if (i < 0)
    {
    index = 0;
    }
  else if (i > (int)(this->FrameNodeIDList.size()))
    {
    index = this->FrameNodeIDList.size();
    }
  else
    {
    index = i;
    }

  // Frame node id list.
  NodeIDListType::iterator iter;
  iter = this->FrameNodeIDList.begin();
  iter += index;

  /*
  vtkMRMLNode* node =
    vtkMRMLNode::SafeDownCast(this->GetScene()
                              ->GetNodeByID(iter->c_str()));
  */
  this->FrameNodeIDList.erase(iter);

  // Time stamp list
  TimeStampListType::iterator titer;
  titer = this->TimeStampList.begin();
  titer += index;
  this->TimeStampList.erase(titer);

  this->Modified();

  return index;

}


//----------------------------------------------------------------------------
int vtkMRMLTimeSeriesBundleNode::RemoveFrame(const char* nodeID)
{
  
  int index=0;

  NodeIDListType::iterator iter;
  for (iter = this->FrameNodeIDList.begin(); iter != this->FrameNodeIDList.end(); iter ++)
    {
    if (*iter == nodeID)
      {
      this->FrameNodeIDList.erase(iter);

      // Time stamp list
      TimeStampListType::iterator titer;
      titer = this->TimeStampList.begin();
      titer += index;
      this->TimeStampList.erase(titer);

      this->Modified();
      return index;

      }
    index ++;
    }

  return -1;
}


//----------------------------------------------------------------------------
void vtkMRMLTimeSeriesBundleNode::RemoveAllFrames()
{
  this->FrameNodeIDList.clear();
  this->TimeStampList.clear();
  this->Modified();

}


//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLTimeSeriesBundleNode::GetFrameNode(int i)
{
  if (i < 0 || i >= (int)(this->FrameNodeIDList.size()))
    {
    return NULL;
    }
  
  vtkMRMLNode* node =
    vtkMRMLNode::SafeDownCast(this->GetScene()
                              ->GetNodeByID(this->FrameNodeIDList[i].c_str()));

  return node;

}


//----------------------------------------------------------------------------
int vtkMRMLTimeSeriesBundleNode::GetTimeStamp(int i, TimeStamp* ts)
{
  if ( (this->TimeStampList.size() == 0) || (this->TimeStampList.empty()) )
    {
    vtkWarningMacro ( "Empty TimeStampList." );
    return -1;
    }
  if ( (i < 0) || (i >= (int)(this->FrameNodeIDList.size()) ) )
    {
    return 0;
    }
  TimeStamp& times = this->TimeStampList[i];
  ts->second     = times.second;
  ts->nanosecond = times.nanosecond;

  return 1;
}


//----------------------------------------------------------------------------
int vtkMRMLTimeSeriesBundleNode::SetTimeStamp(int i, TimeStamp* ts)
{
  if (i < 0 || i >= (int)(this->FrameNodeIDList.size()))
    {
    return 0;
    }
  
  TimeStamp& times = this->TimeStampList[i];
  times.second     = ts->second;
  times.nanosecond = ts->nanosecond;

  this->Modified();

  return 1;
}


//----------------------------------------------------------------------------
int vtkMRMLTimeSeriesBundleNode::SetTimeStamp(int i, unsigned int second, unsigned int microsecond)
{
  if (i < 0 || i >= (int)(this->FrameNodeIDList.size()))
    {
    return 0;
    }
  
  TimeStamp& times = this->TimeStampList[i];
  times.second     = second + microsecond / 1000000;
  times.nanosecond = (microsecond % 1000000) * 1000;

  this->Modified();

  return 1;
}


//----------------------------------------------------------------------------
unsigned int vtkMRMLTimeSeriesBundleNode::GetTimeStampSecondComponent(int i)
{
  if (i < 0 || i >= (int)(this->FrameNodeIDList.size()))
    {
    return 0;
    }
  
  TimeStamp& times = this->TimeStampList[i];

  return times.second;
}


//----------------------------------------------------------------------------
unsigned int vtkMRMLTimeSeriesBundleNode::GetTimeStampMicrosecondComponent(int i)
{
  if (i < 0 || i >= (int)(this->FrameNodeIDList.size()))
    {
    return 0;
    }
  
  TimeStamp& times = this->TimeStampList[i];

  return (times.nanosecond / 1000);
}


//----------------------------------------------------------------------------
int vtkMRMLTimeSeriesBundleNode::SetDisplayBufferNodeID(int bufferIndex, const char* nodeID)
{
  this->DisplayBufferNodeIDList[bufferIndex] = nodeID;

  this->Modified();

  return 1;
}


//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLTimeSeriesBundleNode::GetDisplayBufferNode(int bufferIndex)
{
  if (this->DisplayBufferNodeIDList[bufferIndex] != "")
    {
    vtkMRMLNode* node =
      vtkMRMLNode::SafeDownCast(this->GetScene()
                                ->GetNodeByID(this->DisplayBufferNodeIDList[bufferIndex].c_str()));
    
    return node;
    }
  else
    {
    return NULL;
    }
}
  

//----------------------------------------------------------------------------
void vtkMRMLTimeSeriesBundleNode::SwitchDisplayBuffer(int bufferIndex, int i)
  // this function should be implemented in the child class
{

  vtkMRMLScalarVolumeNode* frame = 
    vtkMRMLScalarVolumeNode::SafeDownCast(this->GetFrameNode(i));

  vtkMRMLScalarVolumeNode* displayBuffer = 
    vtkMRMLScalarVolumeNode::SafeDownCast(this->GetDisplayBufferNode(bufferIndex));

  if (frame && displayBuffer)
    {
    vtkImageData* imageData = displayBuffer->GetImageData();
    if ( imageData )
      {
      displayBuffer->SetAndObserveImageData(frame->GetImageData());
      displayBuffer->Modified();
      displayBuffer->InvokeEvent(vtkMRMLVolumeNode::ImageDataModifiedEvent);
      }
    }

}




// End
