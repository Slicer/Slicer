/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPlotGaussian.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// EMSegment includes
#include "qSlicerEMSegmentModuleWidgetsExport.h"

// VTK includes
#include <vtkPlotLine.h>

/// \ingroup Slicer_QtModules_EMSegment
class Q_SLICER_MODULE_EMSEGMENT_WIDGET_EXPORT vtkPlotGaussian : public vtkPlotLine
{
public:
  vtkTypeMacro(vtkPlotGaussian, vtkPlotLine);
  virtual void PrintSelf(ostream &os, vtkIndent indent);

  // Description:
  // Creates a 2D Chart object.
  static vtkPlotGaussian *New();

  // Description:
  // Perform any updates to the item that may be necessary before rendering.
  // The scene should take care of calling this on all items before their
  // Paint function is invoked.
  virtual void Update();

  // Description:
  void GetBounds(double* bounds);

  // Description:
  // Paint event for the XY plot, called whenever the chart needs to be drawn
  virtual bool Paint(vtkContext2D *painter);

  vtkSetMacro(Mean, float);
  vtkGetMacro(Mean, float);

  vtkSetMacro(Covariance, float);
  vtkGetMacro(Covariance, float);

  vtkSetMacro(Probability, float);
  vtkGetMacro(Probability, float);

  vtkSetMacro(Log, bool);
  vtkGetMacro(Log, bool);

//BTX
protected:
  vtkPlotGaussian();
  ~vtkPlotGaussian();

  void ComputeGaussian();

  vtkTimeStamp BoundsTime;
  double Bounds[4];
  double Extent[4];

  float  Mean;
  float  Covariance;
  float  Probability;
  bool   Log;

private:
  vtkPlotGaussian(const vtkPlotGaussian &); // Not implemented.
  void operator=(const vtkPlotGaussian &); // Not implemented.

//ETX
};
