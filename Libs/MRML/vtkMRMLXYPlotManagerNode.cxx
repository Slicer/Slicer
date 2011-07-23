/*=auto=========================================================================

Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLXYPlotManagerNode.h"
#include "vtkMRMLArrayPlotNode.h"
#include "vtkMRMLDoubleArrayNode.h"
#include "vtkMRMLPlotNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkIntArray.h>
#include <vtkObjectFactory.h>


//------------------------------------------------------------------------------
vtkMRMLXYPlotManagerNode* vtkMRMLXYPlotManagerNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLXYPlotManagerNode"); if(ret)
    {
      return (vtkMRMLXYPlotManagerNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLXYPlotManagerNode;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLXYPlotManagerNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLXYPlotManagerNode");
  if(ret)
    {
      return (vtkMRMLXYPlotManagerNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLXYPlotManagerNode;
}

//----------------------------------------------------------------------------
vtkMRMLXYPlotManagerNode::vtkMRMLXYPlotManagerNode()
{

  this->Title  = "";
  this->XLabel = "";
  this->YLabel = "";

  this->Data.clear();
  this->LastArrayID = -1;

  this->XRange[0] = 0.0;
  this->XRange[1] = 0.0;
  this->YRange[0] = 0.0;
  this->YRange[1] = 0.0;

  this->AutoXRange = 0;
  this->AutoYRange = 0;

}


//----------------------------------------------------------------------------
vtkMRMLXYPlotManagerNode::~vtkMRMLXYPlotManagerNode()
{
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotManagerNode::WriteXML(ostream& of, int nIndent)
{

  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  //std::stringstream ssX;
  //std::stringstream ssY;
  //std::stringstream ssYerr;
  //
  //int n = this->Array->GetNumberOfComponents();
  //double xy[3];
  //
  //if (this->Array->GetNumberOfComponents() > 3)
  //  {
  //  // Put values to the string streams except the last values.
  //  n = this->Array->GetSize() - 1;
  //  for (int i = 0; i < n; i ++)
  //    {
  //    this->Array->GetTupleValue(i, xy);
  //    ssX    << xy[0] << ", ";
  //    ssY    << xy[1] << ", ";
  //    ssYerr << xy[2] << ", ";
  //    }
  //  this->Array->GetTupleValue(n);
  //  // put the last values
  //  ssX    << xy[0];
  //  ssY    << xy[1];
  //  ssYerr << xy[2];
  //  }
  //
  //of << " valueX=\"" << ss.str() << "\"";
  //of << " valueY=\"" << ss.str() << "\"";
  //of << " valueYErr=\"" << ss.str() << "\"";

}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotManagerNode::ReadXMLAttributes(const char** atts)
{
  //int disabledModify = this->StartModify();

  vtkMRMLNode::ReadXMLAttributes(atts);

  //const char* attName;
  //const char* attValue;
  //
  //std::vector<double> valueX;
  //std::vector<double> valueY;
  //std::vector<double> valueYErr;

  //valueX.clear();
  //valueY.clear();
  //valueYErr.clear();
  //
  //while (*atts != NULL) 
  //  {
  //  attName = *(atts++);
  //  attValue = *(atts++);
  //
  //  if (!strcmp(attName, "valueX")) 
  //    {
  //    std::stringstream ss;
  //    std::string s;
  //    double val;
  //
  //    ss << attValue;
  //
  //    while (std::getline(ss, s, ','))
  //      {
  //      double v = atof(s.c_str());
  //      valueX.push_back(v);
  //      }
  //    }
  //  else if (!strcmp(attName, "valueY"))
  //    {
  //    std::stringstream ss;
  //    std::string s;
  //    double val;
  //
  //    ss << attValue;
  //
  //    while (std::getline(ss, s, ','))
  //      {
  //      double v = atof(s.c_str());
  //      valueY.push_back(v);
  //      }
  //    }
  //  else if (!strcmp(attName, "valueYErr"))
  //    {
  //    std::stringstream ss;
  //    std::string s;
  //    double val;
  //
  //    ss << attValue;
  //
  //    while (std::getline(ss, s, ','))
  //      {
  //      double v = atof(s.c_str());
  //      valueYErr.push_back(v);
  //      }
  //    }
  //  }
  //
  //if (valueYErr.size() > 0)  // if Y error values have loaded
  //  {
  //  if (valueX.size() == valueY.size() && valueY.size() == valueYErr.size())
  //    {
  //    int n = valueX.size();
  //    this->Array->SetNumberOfComponents(3);
  //    this->Array->SetNumberOfTuples(n);
  //    for (int i = 0; i < n; i ++)
  //      {
  //      double xy[3];
  //      xy[0] = valueX[i];
  //      xy[1] = valueY[i];
  //      xy[2] = valueYErr[i];
  //      this->Array->SetTupleValue(i, xy);
  //      }
  //    }
  //  }
  //else
  //  {
  //  if (valueX.size() == valueY.size())
  //    {
  //    int n = valueX.size();
  //    this->Array->SetNumberOfComponents(3);
  //    this->Array->SetNumberOfTuples(n);
  //    for (int i = 0; i < n; i ++)
  //      {
  //      double xy[3];
  //      xy[0] = valueX[i];
  //      xy[1] = valueY[i];
  //      xy[2] = 0.0;
  //      this->Array->SetTupleValue(i, xy);
  //      }
  //    }
  //  }
  //
  //this->EndModify(disabledModify);

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLXYPlotManagerNode::Copy(vtkMRMLNode *anode)
{

  Superclass::Copy(anode);
  //vtkMRMLXYPlotManagerNode *node = (vtkMRMLXYPlotManagerNode *) anode;

  //int type = node->GetType();
  
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotManagerNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  //if (this->TargetPlanList && this->TargetPlanList == vtkMRMLFiducialListNode::SafeDownCast(caller) &&
  //  event ==  vtkCommand::ModifiedEvent)
  //  {
  //  //this->ModifiedSinceReadOn();
  //  //this->InvokeEvent(vtkMRMLVolumeNode::ImageDataModifiedEvent, NULL);
  //  //this->UpdateFromMRML();
  //  return;
  //  }
  //
  //if (this->TargetCompletedList && this->TargetCompletedList == vtkMRMLFiducialListNode::SafeDownCast(caller) &&
  //  event ==  vtkCommand::ModifiedEvent)
  //  {
  //  //this->ModifiedSinceReadOn();
  //  //this->InvokeEvent(vtkMRMLVolumeNode::ImageDataModifiedEvent, NULL);
  //  //this->UpdateFromMRML();
  //  return;
  //  }

  return;
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotManagerNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
int vtkMRMLXYPlotManagerNode::AddPlotNode(vtkMRMLPlotNode* node)
{

  // Check if the number of arrays hasn't reached to the maximum
  
  if (this->Data.size() >= 10000)
    {
    return -1;
    }

  // Find unique ID
  int next_id = (this->LastArrayID + 1) % 10000;
  std::map< int, vtkMRMLPlotNode* >::iterator iter;
  while (1)
    {
    iter = this->Data.find(next_id);
    if (iter == this->Data.end())
      {
      break;
      }
    next_id = (next_id + 1) % 10000;
    }
  this->LastArrayID = next_id;
  this->Data[this->LastArrayID] = node;

  this->Modified();

  return this->LastArrayID;
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotManagerNode::RemovePlotNode(int id)
{
  
  std::map< int, vtkMRMLPlotNode* >::iterator iter;
  iter = this->Data.find(id);

  if (iter != this->Data.end())
    {
    this->Data.erase(iter);
    }

  this->Modified();
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotManagerNode::RemovePlotNodeByNodeID(const char* nodeID)
{

  std::map< int, vtkMRMLPlotNode* >::iterator iter;

  for (iter = this->Data.begin(); iter != this->Data.end(); iter ++)
    {
    vtkMRMLPlotNode* anode = iter->second;
    if (anode && strcmp(anode->GetID(), nodeID) == 0)
      {
      this->Data.erase(iter);
      }
    }

  this->Modified();
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotManagerNode::ClearPlotNodes()
{
  std::map< int, vtkMRMLPlotNode* >::iterator iter;
  for (iter = this->Data.begin(); iter != this->Data.end(); iter ++)
    {
    if (iter->second)
      {
      iter->second->Delete();
      iter->second = NULL;
      }
    }

  this->Data.clear();
  this->LastArrayID = -1;
  this->Modified();
}


//----------------------------------------------------------------------------
unsigned int vtkMRMLXYPlotManagerNode::GetNumberOfPlotNodes()
{
  return static_cast<unsigned int>(this->Data.size());
}


//----------------------------------------------------------------------------
vtkIntArray* vtkMRMLXYPlotManagerNode::GetPlotNodeIDList()
{

  vtkIntArray* list;
    
  list = vtkIntArray::New();
  
  std::map< int, vtkMRMLPlotNode* >::iterator iter;
  for (iter = this->Data.begin(); iter != this->Data.end(); iter ++)
    {
    
    list->InsertNextValue(iter->first);
    }

  return list;

}


//----------------------------------------------------------------------------
vtkMRMLPlotNode* vtkMRMLXYPlotManagerNode::GetPlotNode(int id)
{

  std::map< int, vtkMRMLPlotNode* >::iterator iter;
  iter = this->Data.find(id);

  if (iter != this->Data.end())
    {
    return iter->second;
    }
  else
    {
    return NULL;
    }
}


//----------------------------------------------------------------------------
vtkCollection* vtkMRMLXYPlotManagerNode::GetPlotNodes(const char* tag)
{

  vtkCollection* nodes = vtkCollection::New();  
  
  std::map< int, vtkMRMLPlotNode* >::iterator iter;
  for (iter = this->Data.begin(); iter != this->Data.end(); iter ++)
    {
    if (iter->second)
      {
      if (tag == NULL ||
          strcmp(tag, iter->second->GetNodeTagName()) == 0)
        {
        nodes->AddItem(iter->second);
        }
      }
    }

  return nodes;

}



//----------------------------------------------------------------------------
void vtkMRMLXYPlotManagerNode::SetVisibilityAll(int i)
{
  std::map< int, vtkMRMLPlotNode* >::iterator iter;

  for (iter = this->Data.begin();
       iter != this->Data.end();
       iter ++)
    {
    vtkMRMLPlotNode* node = (iter->second);
    if (node)
      {
      node->SetVisible(i);
      }
    }

}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotManagerNode::SetErrorBarAll(int i)
{
  std::map< int, vtkMRMLPlotNode* >::iterator iter;
  
  for (iter = this->Data.begin();
       iter != this->Data.end();
       iter ++)
    {
    vtkMRMLPlotNode* node = (iter->second);
    if (node)
      {
      vtkMRMLArrayPlotNode* cnode = vtkMRMLArrayPlotNode::SafeDownCast(node);
      if (cnode)
        {
        cnode->SetErrorBar(i);
        }
      }
    }

}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotManagerNode::Refresh()
{
  this->InvokeEvent(UpdateGraphEvent);
}

