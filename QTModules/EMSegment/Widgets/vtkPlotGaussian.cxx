/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPlotGaussian.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

  =========================================================================*/

#include "vtkPlotGaussian.h"

// VTK includes
#include "vtkContext2D.h"
#include "vtkContextDevice2D.h"
#include "vtkContextMapper2D.h"
#include "vtkContextScene.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"
#include "vtkPen.h"
#include "vtkPoints2D.h"
#include "vtkTable.h"
#include "vtkTransform2D.h"

// STD includes
#include <cmath>

// Copied from vtkImageGeneral.h
// Some constants having to do with the way single
// floats are represented on alphas and sparcs
#define MANTSIZE (23)
#define SIGNBIT (1 << 31)
#define EXPMASK (255 << MANTSIZE)
#define MENTMASK ((~EXPMASK)&(~SIGNBIT))
#define PHANTOM_BIT (1 << MANTSIZE)
#define EXPBIAS 127
#define SHIFTED_BIAS (EXPBIAS << MANTSIZE)
#define SHIFTED_BIAS_COMP ((~ SHIFTED_BIAS) + 1)
#define ONE_OVER_2_PI 0.5/3.14159265358979
#define ONE_OVER_ROOT_2_PI sqrt(ONE_OVER_2_PI)
#define MINUS_ONE_OVER_2_LOG_2 -.72134752



// ---------------------------------
// Normal Gauss Function
// --------------------------------
inline float GeneralGauss(float inverse_sigma, float x) {
  x *= inverse_sigma;
  return (ONE_OVER_ROOT_2_PI* inverse_sigma * exp(-0.5 * x * x));
}


//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPlotGaussian);

//-----------------------------------------------------------------------------
vtkPlotGaussian::vtkPlotGaussian()
{
  this->Bounds[0] = 0.;
  this->Bounds[1] = 1.;
  this->Bounds[2] = 0.;
  this->Bounds[3] = 1.;
  this->Extent[0] = 0.;
  this->Extent[1] = -1.;
  this->Extent[2] = 0.;
  this->Extent[3] = -1.;

  this->Mean = 0.f;
  this->Covariance = 1.f;
  this->Probability = 1.f;
  this->Log = false;

  vtkTable* table = vtkTable::New();
  vtkFloatArray* xArray = vtkFloatArray::New();
  xArray->SetNumberOfComponents(1);
  xArray->SetName("x");
  vtkFloatArray* yArray = vtkFloatArray::New();
  yArray->SetNumberOfComponents(1);
  yArray->SetName("y");
  table->AddColumn(xArray);
  table->AddColumn(yArray);
  xArray->Delete();
  yArray->Delete();
  this->SetInput(table, "x", "y");
  table->Delete();
}

//-----------------------------------------------------------------------------
vtkPlotGaussian::~vtkPlotGaussian()
{
}

//-----------------------------------------------------------------------------
void vtkPlotGaussian::Update()
{
  if (this->GetMTime() > this->BoundsTime)
    {
    // FWHM = 2*sqrt(2*ln(2))*c ~= 2.35482 * c
    float fullWidthAtHalfMaximum = 2.35482 * sqrt(this->Covariance);
    // let's make sure we cover the whole bump
    float bumpFullWidth = fullWidthAtHalfMaximum * 3.f;
    this->Bounds[0] = this->Log ? exp(this->Mean - bumpFullWidth / 2.f) : this->Mean - bumpFullWidth / 2.f;
    this->Bounds[1] = this->Log ? exp(this->Mean + bumpFullWidth / 2.f) : this->Mean + bumpFullWidth / 2.f;
    this->Bounds[2] = 0.;
    this->Bounds[3] = this->Probability * GeneralGauss(sqrt(1.f / this->Covariance), 0.);
    this->Extent[0] = 0.;
    this->Extent[1] = -1.;
    this->Extent[2] = 0.;
    this->Extent[3] = -1.;
    this->BoundsTime.Modified();
    }
  this->Superclass::Update();
}

//-----------------------------------------------------------------------------
void vtkPlotGaussian::GetBounds(double* bounds)
{
  bounds[0] = this->Bounds[0];
  bounds[1] = this->Bounds[1];
  bounds[2] = this->Bounds[2];
  bounds[3] = this->Bounds[3];
}

//-----------------------------------------------------------------------------
bool vtkPlotGaussian::Paint(vtkContext2D *painter)
{
  // This is where everything should be drawn, or dispatched to other methods.
  vtkDebugMacro(<< "Paint event called in vtkPlotGaussian.");

  if (!this->Visible)
    {
    return false;
    }

  // find the region to loop over
  double extent[4] = {VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, VTK_DOUBLE_MIN};
  double p[2], pTrans[2];
  p[0] = 0.;
  p[1] = 0.;
  painter->GetTransform()->InverseTransformPoints(p, pTrans, 1);
  extent[0] = (pTrans[0] < extent[0] ? pTrans[0] : extent[0]);
  extent[1] = (pTrans[0] > extent[1] ? pTrans[0] : extent[1]);
  extent[2] = (pTrans[1] < extent[2] ? pTrans[1] : extent[2]);
  extent[3] = (pTrans[1] > extent[3] ? pTrans[1] : extent[3]);
  p[0] = this->GetScene()->GetSceneWidth();
  p[1] = this->GetScene()->GetSceneHeight();
  painter->GetTransform()->InverseTransformPoints(p, pTrans, 1);
  extent[0] = (pTrans[0] < extent[0] ? pTrans[0] : extent[0]);
  extent[1] = (pTrans[0] > extent[1] ? pTrans[0] : extent[1]);
  extent[2] = (pTrans[1] < extent[2] ? pTrans[1] : extent[2]);
  extent[3] = (pTrans[1] > extent[3] ? pTrans[1] : extent[3]);
  if (extent[0] != this->Extent[0] ||
      extent[1] != this->Extent[1] ||
      extent[2] != this->Extent[2] ||
      extent[3] != this->Extent[3])
    {
    this->Extent[0] = extent[0];
    this->Extent[1] = extent[1];
    this->Extent[2] = extent[2];
    this->Extent[3] = extent[3];
    this->ComputeGaussian();
    }

  return this->vtkPlotLine::Paint(painter);
}

//-----------------------------------------------------------------------------
void vtkPlotGaussian::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkPlotGaussian::ComputeGaussian()
{
  if (this->Covariance <= 0.f)
    {
    vtkErrorMacro(<< "Covariance is  not positive");
    return;
    }
  float invSqrtDetCovariance = sqrt(1.f / this->Covariance);
  const int dim = this->GetScene()->GetSceneWidth();

  vtkTable* table = this->Data->GetInput();
  vtkFloatArray* xArray = vtkFloatArray::SafeDownCast(
    this->Data->GetInputArrayToProcess(0, table));
  xArray->SetNumberOfTuples(dim);

  vtkFloatArray* yArray = vtkFloatArray::SafeDownCast(
    this->Data->GetInputArrayToProcess(1, table));
  yArray->SetNumberOfTuples(dim);

  float x = this->Extent[0];
  float step = (this->Extent[1] - this->Extent[0]) / dim;
  for (int i = 0; i < dim; ++i)
    {
    xArray->SetValue(i, x);
    float y;
    if (!this->Log)
      {
      y = this->Probability * GeneralGauss(invSqrtDetCovariance, x - this->Mean);
      }
    else
      {
      y = this->Probability * GeneralGauss(invSqrtDetCovariance, log(x + 1.f) - this->Mean);
      }
    yArray->SetValue(i, y);
    x += step;
    }
  this->Data->Modified();
  this->Superclass::Update();
}
