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
#include "vtkMRMLDoubleArrayNode.h"

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkMRMLDoubleArrayNode, Array, vtkDoubleArray)

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLDoubleArrayNode);

//----------------------------------------------------------------------------
vtkMRMLDoubleArrayNode::vtkMRMLDoubleArrayNode()
{
  this->Array = vtkDoubleArray::New();
  this->Array->SetNumberOfComponents(3);

  this->Unit.resize(3);

  this->HideFromEditorsOff();
}

//----------------------------------------------------------------------------
vtkMRMLDoubleArrayNode::~vtkMRMLDoubleArrayNode()
{
  if (this->Array)
    {
    this->Array->Delete();
    this->Array = NULL;
    }
}


//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayNode::WriteXML(ostream& of, int nIndent)
{

  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  std::stringstream ssX;
  std::stringstream ssY;
  std::stringstream ssYerr;

  if (this->Array->GetNumberOfComponents() >= 3 &&
      this->Array->GetNumberOfTuples() > 0)
    {
    // Put values to the string streams except the last values.
    int n = this->Array->GetNumberOfTuples() - 1;
    double xy[3];
    for (int i = 0; i < n; i ++)
      {
      this->Array->GetTupleValue(i, xy);
      ssX    << xy[0] << ", ";
      ssY    << xy[1] << ", ";
      ssYerr << xy[2] << ", ";
      }
    this->Array->GetTupleValue(n, xy);
    // put the last values
    ssX    << xy[0];
    ssY    << xy[1];
    ssYerr << xy[2];
    }

  of << " valueX=\""    << ssX.str() << "\"";
  of << " valueY=\""    << ssY.str() << "\"";
  of << " valueYErr=\"" << ssYerr.str() << "\"";

}


//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;

  std::vector<double> valueX;
  std::vector<double> valueY;
  std::vector<double> valueYErr;

  valueX.clear();
  valueY.clear();
  valueYErr.clear();

  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "valueX")) 
      {
      std::stringstream ss;
      std::string s;
      //double val;

      ss << attValue;

      while (std::getline(ss, s, ','))
        {
        double v = atof(s.c_str());
        valueX.push_back(v);
        }
      }
    else if (!strcmp(attName, "valueY"))
      {
      std::stringstream ss;
      std::string s;
      //double val;

      ss << attValue;

      while (std::getline(ss, s, ','))
        {
        double v = atof(s.c_str());
        valueY.push_back(v);
        }
      }
    else if (!strcmp(attName, "valueYErr"))
      {
      std::stringstream ss;
      std::string s;
      //double val;

      ss << attValue;

      while (std::getline(ss, s, ','))
        {
        double v = atof(s.c_str());
        valueYErr.push_back(v);
        }
      }
    }

  if (valueYErr.size() > 0)  // if Y error values have loaded
    {
    if (valueX.size() == valueY.size() && valueY.size() == valueYErr.size())
      {
      size_t n = valueX.size();
      this->Array->SetNumberOfComponents(3);
      this->Array->SetNumberOfTuples(n);
      for (size_t i = 0; i < n; i ++)
        {
        double xy[3];
        xy[0] = valueX[i];
        xy[1] = valueY[i];
        xy[2] = valueYErr[i];
        this->Array->SetTupleValue(i, xy);
        }
      }
    }
  else
    {
    if (valueX.size() == valueY.size())
      {
      size_t n = valueX.size();
      this->Array->SetNumberOfComponents(3);
      this->Array->SetNumberOfTuples(n);
      for (size_t i = 0; i < n; i ++)
        {
        double xy[3];
        xy[0] = valueX[i];
        xy[1] = valueY[i];
        xy[2] = 0.0;
        this->Array->SetTupleValue(i, xy);
        }
      }
    }

  this->EndModify(disabledModify);


}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLDoubleArrayNode::Copy(vtkMRMLNode *anode)
{

  Superclass::Copy(anode);
  //vtkMRMLDoubleArrayNode *node = (vtkMRMLDoubleArrayNode *) anode;
  //int type = node->GetType();
  
}


//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
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
void vtkMRMLDoubleArrayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayNode::SetSize(unsigned int n)
{
  this->Array->SetNumberOfComponents(3);
  this->Array->SetNumberOfTuples(n);
  this->Modified();
}


//----------------------------------------------------------------------------
unsigned int vtkMRMLDoubleArrayNode::GetSize()
{
  return this->Array->GetNumberOfTuples();
}


//----------------------------------------------------------------------------
double vtkMRMLDoubleArrayNode::GetYAxisValue(double vtkNotUsed(x), int vtkNotUsed(interp))
{
  // not implemented yet
  //  double xy[2];
  //  
  //  this->Modified();
  //  return 1;
  return 0.0;
}


//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::GetXYValue(int index, double* x, double* y)
{
  double xy[3];

  if (this->Array->GetNumberOfComponents() >= 2 && index < this->Array->GetNumberOfTuples())
    {
    this->Array->GetTupleValue(index, xy);
    *x = xy[0];
    *y = xy[1];
    return 1;
    }
  else
    {
    return 0;
    }
}


//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::GetXYValue(int index, double* x, double* y, double* yerr)
{
  double xy[3];

  if (this->Array->GetNumberOfComponents() >= 3 && index < this->Array->GetNumberOfTuples())
    {
    this->Array->GetTupleValue(index, xy);
    *x    = xy[0];
    *y    = xy[1];
    *yerr = xy[2];
    return 1;
    }
  else
    {
    return 0;
    }
}


//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::SetXYValue(int index, double x, double y)
{
  double xy[3];
  if (this->Array->GetNumberOfComponents() >= 2 && index < this->Array->GetNumberOfTuples())
    {
    xy[0] = x;
    xy[1] = y;
    xy[2] = 0.0;
    this->Array->SetTupleValue(index, xy);
    this->Modified();
    return 1;
    }
  else
    {
    return 0;
    }

}


//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::SetXYValue(int index, double x, double y, double yerr)
{
  double xy[3];
  if (this->Array->GetNumberOfComponents() >= 3 && index < this->Array->GetNumberOfTuples())
    {
    xy[0] = x;
    xy[1] = y;
    xy[2] = yerr;
    this->Array->SetTupleValue(index, xy);
    this->Modified();
    return 1;
    }
  else
    {
    return 0;
    }

}


//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::AddXYValue(double x, double y)
{
  double xy[3];
  if (this->Array->GetNumberOfComponents() >= 2)
    {
    xy[0] = x;
    xy[1] = y;
    xy[2] = 0.0;
    this->Array->InsertNextTuple(xy);
    this->Modified();
    return 1;
    }
  else
    {
    return 0;
    }


}


//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::AddXYValue(double x, double y, double yerr)
{
  double xy[3];
  if (this->Array->GetNumberOfComponents() >= 3)
    {
    xy[0] = x;
    xy[1] = y;
    xy[2] = yerr;
    this->Array->InsertNextTuple(xy);
    this->Modified();
    return 1;
    }
  else
    {
    return 0;
    }
}


//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayNode::GetRange(double* rangeX, double* rangeY, int fIncludeError)
{
  rangeX[0] = 0.0;
  rangeX[1] = 0.0;
  rangeY[0] = 0.0;
  rangeY[1] = 0.0;

  if (!this->Array)
    {
    return;
    }

  int nTuples = this->Array->GetNumberOfTuples();
  int nComp   = this->Array->GetNumberOfComponents();

  double xy[3];
  double c;

  // Consider error value in the range calculation,
  // if fIncludeError=1 and number of components is larger than 3

  if (fIncludeError && nComp > 2)
    {
    c = 1.0;
    }
  else
    {
    xy[2] = 0.0;
    c     = 0.0;
    }

  if (nTuples > 0)
    {
    // Get the first values as an initial value
    this->Array->GetTupleValue(0, xy);
    rangeX[0] = xy[0];
    rangeX[1] = xy[0];
    rangeY[0] = xy[1] - c * xy[2];
    rangeY[1] = xy[1] + c * xy[2];
    
    // Search the array
    for (int i = 1; i < nTuples; i ++)
      {
      this->Array->GetTupleValue(i, xy);

      // X value
      if (xy[0] < rangeX[0])
        {
        rangeX[0] = xy[0];
        }
      else if (xy[0] > rangeX[1])
        {
        rangeX[1] = xy[0];
        }

      // Y and error
      double low  = xy[1] - c * xy[2];
      double high = xy[1] + c * xy[2];

      if (low < rangeY[0])
        {
        rangeY[0] = low;
        }
      if (high > rangeY[1])
        {
        rangeY[1] = high;
        }
      }

    }

}


//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayNode::GetXRange(double* range)
{
  range[0] = 0.0;
  range[1] = 0.0;

  if (!this->Array)
    {
    return;
    }

  int nTuples = this->Array->GetNumberOfTuples();
  double xy[3];

  if (this->Array->GetNumberOfComponents() >= 2 && nTuples > 0)
    {

    // Get the first values as an initial value
    this->Array->GetTupleValue(0, xy);
    range[0] = xy[0];
    range[1] = xy[0];

    // Search the array
    for (int i = 1; i < nTuples; i ++)
      {
      this->Array->GetTupleValue(i, xy);
      if (xy[0] < range[0])
        {
        range[0] = xy[0];
        }
      else if (xy[0] > range[1])
        {
        range[1] = xy[0];
        }
      }

    }
}


//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayNode::GetYRange(double* range, int fIncludeError)
{
  range[0] = 0.0;
  range[1] = 0.0;

  if (!this->Array)
    {
    return;
    }

  int nTuples = this->Array->GetNumberOfTuples();
  int nComp   = this->Array->GetNumberOfComponents();

  double xy[3];
  double c;

  // Consider error value in the range calculation,
  // if fIncludeError=1 and number of components is larger than 3

  if (fIncludeError && nComp > 2)
    {
    c = 1.0;
    }
  else
    {
    xy[2] = 0.0;
    c     = 0.0;
    }

  if (nTuples > 0)
    {

    // Get the first values as an initial value
    this->Array->GetTupleValue(0, xy);
    range[0] = xy[1] - c * xy[2];
    range[1] = xy[1] + c * xy[2];
    
    // Search the array
    for (int i = 1; i < nTuples; i ++)
      {
      this->Array->GetTupleValue(i, xy);
      double low  = xy[1] - c * xy[2];
      double high = xy[1] + c * xy[2];

      if (low < range[0])
        {
        range[0] = low;
        }
      if (high > range[1])
        {
        range[1] = high;
        }
      }

    }

}

void vtkMRMLDoubleArrayNode::SetLabels(const LabelsVectorType &labels)
{
   this->Labels = labels;
}

const vtkMRMLDoubleArrayNode::LabelsVectorType & vtkMRMLDoubleArrayNode::GetLabels() const
{
    return this->Labels;
}

