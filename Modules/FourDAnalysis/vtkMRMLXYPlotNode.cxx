/*=auto=========================================================================

Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGradientAnisotropicDiffusionFilterNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"

#include "vtkMRMLXYPlotNode.h"

#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLXYPlotNode* vtkMRMLXYPlotNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLXYPlotNode"); if(ret)
    {
      return (vtkMRMLXYPlotNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLXYPlotNode;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLXYPlotNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLXYPlotNode");
  if(ret)
    {
      return (vtkMRMLXYPlotNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLXYPlotNode;
}

//----------------------------------------------------------------------------
vtkMRMLXYPlotNode::vtkMRMLXYPlotNode()
{

  this->Title  = "";
  this->XLabel = "";
  this->YLabel = "";

  this->Data.clear();

  this->XRange[0] = 0.0;
  this->XRange[1] = 0.0;
  this->YRange[0] = 0.0;
  this->YRange[1] = 0.0;

}

//----------------------------------------------------------------------------
vtkMRMLXYPlotNode::~vtkMRMLXYPlotNode()
{
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotNode::WriteXML(ostream& of, int nIndent)
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
void vtkMRMLXYPlotNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;

  std::vector<double> valueX;
  std::vector<double> valueY;
  std::vector<double> valueYErr;

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
void vtkMRMLXYPlotNode::Copy(vtkMRMLNode *anode)
{

  Superclass::Copy(anode);
  vtkMRMLXYPlotNode *node = (vtkMRMLXYPlotNode *) anode;

  //int type = node->GetType();
  
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
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
void vtkMRMLXYPlotNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotNode::AddArrayNode(vtkMRMLDoubleArrayNode* node)
{
  PlotDataType data;

  // default color is black
  data.lineColor[0] = 0.0;
  data.lineColor[1] = 0.0;
  data.lineColor[2] = 0.0;

  data.visibility = 1;
  data.arrayNode = node;

  this->Data.push_back(data);

  this->Modified();
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotNode::RemoveArrayNode(vtkMRMLDoubleArrayNode* node)
{
  std::vector< PlotDataType >::iterator iter;

  for (iter = this->Data.begin();
       iter != this->Data.end();
       iter ++)
    {
    if ((*iter).arrayNode == node)
      {
      break;
      }
    }

  if (iter != this->Data.end())
    {
    this->Data.erase(iter);
    }

  this->Modified();
}


//----------------------------------------------------------------------------
unsigned int vtkMRMLXYPlotNode::GetNumberOfArrays()
{
  this->Data.size();
}


//----------------------------------------------------------------------------
vtkMRMLDoubleArrayNode* vtkMRMLXYPlotNode::GetArrayNode(unsigned int n)
{
  if (n < this->Data.size())
    {
    return this->Data[n].arrayNode;
    }
  else
    {
    return NULL;
    }
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotNode::SetColor(unsigned int n, double r, double g, double b)
{
  if (n < this->Data.size())
    {
    this->Data[n].lineColor[0] = r;
    this->Data[n].lineColor[1] = g;
    this->Data[n].lineColor[2] = b;
    this->Modified();
    }
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotNode::GetColor(unsigned int n, double* r, double* g, double* b)
{
  if (n < this->Data.size())
    {
    *r = this->Data[n].lineColor[0];
    *g = this->Data[n].lineColor[1];
    *b = this->Data[n].lineColor[2];
    }
  else
    {
    *r = 0.0;
    *g = 0.0;
    *b = 0.0;
    }

}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotNode::SetLineName(unsigned int n, const char* str)
{

  if (n < this->Data.size())
    {
    vtkMRMLDoubleArrayNode* anode = this->Data[n].arrayNode;
    if (anode)
      {
      anode->SetName(str);
      this->Modified();
      }
    }

}


//----------------------------------------------------------------------------
const char* vtkMRMLXYPlotNode::GetLineName(unsigned int n)
{

  if (n < this->Data.size())
    {
    vtkMRMLDoubleArrayNode* anode = this->Data[n].arrayNode;
    if (anode)
      {
      return anode->GetName();
      }
    }
  else
    {
    return NULL;
    }

}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotNode::SetVisibility(unsigned int n, int i)
{
  if (n < this->Data.size())
    {
    this->Data[n].visibility = i;
    }
}



//----------------------------------------------------------------------------
int vtkMRMLXYPlotNode::GetVisibility(unsigned int n)
{
  if (n < this->Data.size())
    {
    return this->Data[n].visibility;
    }
  else
    {
    return 0;
    }
}



//----------------------------------------------------------------------------
void vtkMRMLXYPlotNode::SetVisibilityAll(int i);
{
  std::vector< PlotDataType >::iterator iter;

  for (iter = this->Data.begin();
       iter != this->Data.end();
       iter ++)
    {
    (*iter).visibility = i;
    }

}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotNode::SetErrorBarPlot(unsigned int n, int i)
{
  if (n < this->Data.size())
    {
    this->Data[n].errorBar = i;
    }
}


//----------------------------------------------------------------------------
int vtkMRMLXYPlotNode::GetErrorBarPlot(unsigned int n)
{
  if (n < this->Data.size())
    {
    return this->Data[n].errorBar;
    }
  else
    {
    return 0;
    }
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotNode::SetErrorBarPlotAll(int i);
{
  std::vector< PlotDataType >::iterator iter;

  for (iter = this->Data.begin();
       iter != this->Data.end();
       iter ++)
    {
    (*iter).errorBar = i;
    }

}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotNode::SetTitle(const char* str)
{
  this->Title = str;
  this->Modified();
}


//----------------------------------------------------------------------------
const char* vtkMRMLXYPlotNode::GetTitle()
{
  return this->Title.c_str();
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotNode::SetXLabel(const char* str)
{
  this->XLabel = str;
  this->Modified();
}


//----------------------------------------------------------------------------
const char* vtkMRMLXYPlotNode::GetXLabel()
{
  return this->XLabel.c_str();
}


//----------------------------------------------------------------------------
void vtkMRMLXYPlotNode::SetYLabel(const char* str)
{
  this->YLabel = str;
  this->Modified();
}


//----------------------------------------------------------------------------
const char* vtkMRMLXYPlotNode::GetYLabel()
{
  return this->YLabel.c_str();
}



