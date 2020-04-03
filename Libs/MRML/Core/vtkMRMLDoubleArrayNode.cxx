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
#include "vtkMRMLDoubleArrayStorageNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkDoubleArray.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>
#include <vector>

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
    this->Array = nullptr;
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
      this->Array->GetTypedTuple(i, xy);
      ssX    << xy[0] << ", ";
      ssY    << xy[1] << ", ";
      ssYerr << xy[2] << ", ";
      }
    this->Array->GetTypedTuple(n, xy);
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

  while (*atts != nullptr)
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
        this->Array->SetTypedTuple(i, xy);
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
        this->Array->SetTypedTuple(i, xy);
        }
      }
    }

  this->EndModify(disabledModify);


}

//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLDoubleArrayNode* sourceDoubleArrayNode = vtkMRMLDoubleArrayNode::SafeDownCast(anode);
  if (!sourceDoubleArrayNode)
    {
    return;
    }
  if (deepCopy)
    {
    if (sourceDoubleArrayNode->GetArray())
      {
      if (this->GetArray())
        {
        this->GetArray()->DeepCopy(sourceDoubleArrayNode->GetArray());
        }
      else
        {
        vtkSmartPointer<vtkDoubleArray> newArray
          = vtkSmartPointer<vtkDoubleArray>::Take(sourceDoubleArrayNode->GetArray()->NewInstance());
        newArray->DeepCopy(sourceDoubleArrayNode->GetArray());
        this->SetArray(newArray);
        }
      }
    else
      {
      // input was nullptr
      this->SetArray(nullptr);
      }
    }
  else
    {
    // shallow-copy
    this->SetArray(sourceDoubleArrayNode->GetArray());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLDoubleArrayNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

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
int vtkMRMLDoubleArrayNode::GetValues(int index, double* values)
{
  if (index < 0 ||
    index >= this->Array->GetNumberOfTuples())
    {
    return 0;
    }
  this->Array->GetTypedTuple(index, values);
  return 1;
}


//----------------------------------------------------------------------------
double vtkMRMLDoubleArrayNode::GetValue(int index, int component, int& success)
{
  if (component < 0 ||
    component >= this->Array->GetNumberOfComponents() ||
    index < 0 ||
    index >= this->Array->GetNumberOfTuples())
    {
    success = 0;
    return -1;
    }
  std::vector<double> tuple(this->Array->GetNumberOfComponents());
  success = this->GetValues(index, &tuple[0]);

  return tuple[component];
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
  if ( this->Array->GetNumberOfComponents() < 2 ||
    index < 0 ||
    index >= this->Array->GetNumberOfTuples())
    {
    return 0;
    }

  std::vector<double> tuple(this->Array->GetNumberOfComponents());
  int success = this->GetValues(index, &tuple[0]);

  *x = tuple[0];
  *y = tuple[1];

  return success;
}


//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::GetXYValue(int index, double* x, double* y, double* yerr)
{
  if ( this->Array->GetNumberOfComponents() < 3 ||
    index < 0 ||
    index >= this->Array->GetNumberOfTuples())
    {
    return 0;
    }

  std::vector<double> tuple(this->Array->GetNumberOfComponents());
  int success = this->GetValues(index, &tuple[0]);

  *x = tuple[0];
  *y = tuple[1];
  *yerr = tuple[2];

  return success;
}


//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::SetValues(int index, double* values)
{
  if (index < 0 ||
    index >= this->Array->GetNumberOfTuples())
    {
    return 0;
    }
  this->Array->SetTypedTuple(index, values);
  this->Modified();
  return 1;
}


//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::SetValue(int index, int component, double value)
{
  if (component < 0 ||
    component >= this->Array->GetNumberOfComponents() ||
    index < 0 ||
    index >= this->Array->GetNumberOfTuples())
    {
    return 0;
    }
  std::vector<double> tuple(this->Array->GetNumberOfComponents());
  int successGet = this->GetValues(index, &tuple[0]);

  tuple[component] = value;
  int successSet = this->SetValues(index, &tuple[0]);

  return (successGet && successSet);
}


//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::SetXYValue(int index, double x, double y)
{
  if (this->Array->GetNumberOfComponents() < 2 ||
    index < 0 ||
    index >= this->Array->GetNumberOfTuples())
    {
    return 0;
    }
  std::vector<double> tuple(this->Array->GetNumberOfComponents(), 0.0);

  tuple[0] = x;
  tuple[1] = y;
  return this->SetValues(index, &tuple[0]);
}

//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::SetXYValue(int index, double x, double y, double yerr)
{
  if (this->Array->GetNumberOfComponents() < 3 ||
    index < 0 ||
    index >= this->Array->GetNumberOfTuples())
    {
    return 0;
    }
  std::vector<double> tuple(this->Array->GetNumberOfComponents(), 0.0);

  tuple[0] = x;
  tuple[1] = y;
  tuple[2] = yerr;
  return this->SetValues(index, &tuple[0]);
}


//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::AddValues(double* values)
{
  this->Array->InsertNextTuple(values);
  this->Modified();
  return 1;
}


//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::AddValue(int component, double value)
{
  if (component < 0 ||
    component >= this->Array->GetNumberOfComponents())
    {
    return 0;
    }
  std::vector<double> tuple(this->Array->GetNumberOfComponents(), 0.0);

  tuple[component] = value;
  return this->AddValues(&tuple[0]);
}


//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::AddXYValue(double x, double y)
{
  if (this->Array->GetNumberOfComponents() < 2)
    {
    return 0;
    }
  std::vector<double> tuple(this->Array->GetNumberOfComponents(), 0.0);

  tuple[0] = x;
  tuple[1] = y;
  return this->AddValues(&tuple[0]);
}


//----------------------------------------------------------------------------
int vtkMRMLDoubleArrayNode::AddXYValue(double x, double y, double yerr)
{
  if (this->Array->GetNumberOfComponents() < 3)
    {
    return 0;
    }
  std::vector<double> tuple(this->Array->GetNumberOfComponents(), 0.0);

  tuple[0] = x;
  tuple[1] = y;
  tuple[2] = yerr;
  return this->AddValues(&tuple[0]);
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
    this->Array->GetTypedTuple(0, xy);
    rangeX[0] = xy[0];
    rangeX[1] = xy[0];
    rangeY[0] = xy[1] - c * xy[2];
    rangeY[1] = xy[1] + c * xy[2];

    // Search the array
    for (int i = 1; i < nTuples; i ++)
      {
      this->Array->GetTypedTuple(i, xy);

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
    this->Array->GetTypedTuple(0, xy);
    range[0] = xy[0];
    range[1] = xy[0];

    // Search the array
    for (int i = 1; i < nTuples; i ++)
      {
      this->Array->GetTypedTuple(i, xy);
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
    this->Array->GetTypedTuple(0, xy);
    range[0] = xy[1] - c * xy[2];
    range[1] = xy[1] + c * xy[2];

    // Search the array
    for (int i = 1; i < nTuples; i ++)
      {
      this->Array->GetTypedTuple(i, xy);
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

//---------------------------------------------------------------------------
void vtkMRMLDoubleArrayNode::SetLabels(const LabelsVectorType &labels)
{
   this->Labels = labels;
}

//---------------------------------------------------------------------------
const vtkMRMLDoubleArrayNode::LabelsVectorType & vtkMRMLDoubleArrayNode::GetLabels() const
{
    return this->Labels;
}


//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLDoubleArrayNode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLDoubleArrayStorageNode"));
}
