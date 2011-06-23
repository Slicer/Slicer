/*=auto=========================================================================

Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkMRMLScene.h"

#include <sstream>
#include <string>


#include "vtkMRMLPlotNode.h"

#include "vtkDataObject.h"
#include "vtkFieldData.h"
#include "vtkDoubleArray.h"

//------------------------------------------------------------------------------
vtkMRMLPlotNode* vtkMRMLPlotNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLPlotNode"); if(ret)
    {
      return (vtkMRMLPlotNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLPlotNode;
}


//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLPlotNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLPlotNode");
  if(ret)
    {
      return (vtkMRMLPlotNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLPlotNode;
}


//----------------------------------------------------------------------------
vtkMRMLPlotNode::vtkMRMLPlotNode()
{
  this->Visible  = 1;
  this->Color[0] = 0.0;
  this->Color[1] = 0.0;
  this->Color[2] = 0.0;

  this->Legend   = "";
}


//----------------------------------------------------------------------------
vtkMRMLPlotNode::~vtkMRMLPlotNode()
{
}


//----------------------------------------------------------------------------
void vtkMRMLPlotNode::WriteXML(ostream& of, int nIndent)
{

  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  std::stringstream ssX;
  std::stringstream ssY;
  std::stringstream ssYerr;

  //int n = this->Array->GetNumberOfComponents();
  //double xy[3];

  //if (this->Array->GetNumberOfComponents() > 3)
  //  {
  //  // Put values to the string streams except the last values.
  //  n = this->Array->GetNumberOfTuples() - 1;
  //  for (int i = 0; i < n; i ++)
  //    {
  //    this->Array->GetTupleValue(i, xy);
  //    ssX    << xy[0] << ", ";
  //    ssY    << xy[1] << ", ";
  //    ssYerr << xy[2] << ", ";
  //    }
  //  this->Array->GetTupleValue(n, xy);
  //  // put the last values
  //  ssX    << xy[0];
  //  ssY    << xy[1];
  //  ssYerr << xy[2];
  //  }
  //
  //of << " valueX=\""    << ssX.str() << "\"";
  //of << " valueY=\""    << ssY.str() << "\"";
  //of << " valueYErr=\"" << ssYerr.str() << "\"";

}


//----------------------------------------------------------------------------
void vtkMRMLPlotNode::ReadXMLAttributes(const char** atts)
{
  //int disabledModify = this->StartModify();

  vtkMRMLNode::ReadXMLAttributes(atts);

  //const char* attName;
  //const char* attValue;
  //
  //std::vector<double> valueX;
  //std::vector<double> valueY;
  //std::vector<double> valueYErr;
  //
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
void vtkMRMLPlotNode::Copy(vtkMRMLNode *anode)
{

  Superclass::Copy(anode);
  //vtkMRMLPlotNode *node = (vtkMRMLPlotNode *) anode;

  //int type = node->GetType();
  
}


//----------------------------------------------------------------------------
void vtkMRMLPlotNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
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
void vtkMRMLPlotNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
}

