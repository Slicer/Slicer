/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPVScalarBarActor.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

// .NAME vtkPVScalarBarActor - A scalar bar with labels of fixed font.
//
// .SECTION Description
//
// vtkPVScalarBarActor has basically the same functionality as vtkScalarBarActor
// except that the fonts are set to a fixed size and tick labels vary in precision
// before their size is adjusted to meet geometric constraints.
// These changes make it easier to control in ParaView.
//

#ifndef __vtkPVScalarBarActor_h
#define __vtkPVScalarBarActor_h

#include "vtkScalarBarActor.h"
#include "vtkSlicerDataProbeModuleLogicExport.h"

#include "vtkSmartPointer.h" // For ivars
#include <vector> // For ivars

class VTK_SLICER_DATAPROBELIB_MODULE_LOGIC_EXPORT vtkPVScalarBarActor : public vtkScalarBarActor
{
public:
  vtkTypeMacro(vtkPVScalarBarActor, vtkScalarBarActor);
  void PrintSelf(ostream &os, vtkIndent indent) override;
  static vtkPVScalarBarActor *New();

  // Description:
  // The bar aspect ratio (length/width).  Defaults to 20.  Note that this
  // the aspect ratio of the color bar only, not including labels.
  vtkGetMacro(AspectRatio, double);
  vtkSetMacro(AspectRatio, double);

  // Description:
  // If true (the default), the printf format used for the labels will be
  // automatically generated to make the numbers best fit within the widget.  If
  // false, the LabelFormat ivar will be used.
  vtkGetMacro(AutomaticLabelFormat, int);
  vtkSetMacro(AutomaticLabelFormat, int);
  vtkBooleanMacro(AutomaticLabelFormat, int);

  // Description:
  // If true (the default), tick marks will be drawn.
  vtkGetMacro(DrawTickMarks, int);
  vtkSetMacro(DrawTickMarks, int);
  vtkBooleanMacro(DrawTickMarks, int);

  // Description:
  // Set whether the range endpoints (minimum and maximum) are added
  // as labels alongside other value labels.
  vtkGetMacro(AddRangeLabels, int);
  vtkSetMacro(AddRangeLabels, int);
  vtkBooleanMacro(AddRangeLabels, int);

  // Description:
  // Set the C-style format string for the range labels.
  vtkGetStringMacro(RangeLabelFormat);
  vtkSetStringMacro(RangeLabelFormat);

  // Description:
  // Set the title justification. Valid values are VTK_TEXT_LEFT,
  // VTK_TEXT_CENTERED, and VTK_TEXT_RIGHT.
  vtkGetMacro(TitleJustification, int);
  vtkSetClampMacro(TitleJustification, int, VTK_TEXT_LEFT, VTK_TEXT_RIGHT);

  // Description:
  // Set whether the scalar data range endpoints (minimum and maximum)
  // are added as annotations.
  vtkGetMacro(AddRangeAnnotations, int);
  vtkSetMacro(AddRangeAnnotations, int);
  vtkBooleanMacro(AddRangeAnnotations, int);

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow*) override;

  // Description:
  // Draw the scalar bar and annotation text to the screen.
  int RenderOverlay(vtkViewport* viewport) override;

protected:
  vtkPVScalarBarActor();
  ~vtkPVScalarBarActor() override;

  // Description:
  // These methods override the subclass implementation.
  void PrepareTitleText() override;
  void ComputeScalarBarThickness() override;
  void LayoutTitle() override;
  void ComputeScalarBarLength() override;
  void LayoutTicks() override;
  void ConfigureAnnotations() override;
  void ConfigureTitle() override;
  void ConfigureTicks() override;

  // Description:
  // Annotate the min/max values on the scalar bar (in interval/ratio mode).
  //
  // This overrides the subclass implementation.
  void EditAnnotations() override;

  // Description:
  // Set up the ScalarBar, ScalarBarMapper, and ScalarBarActor based on the
  // current position and orientation of this actor.
  //virtual void PositionScalarBar(const int propSize[2], vtkViewport *viewport);

  // Description:
  // Set up the texture used to render the scalar bar.
  virtual void BuildScalarBarTexture();

  // Description:
  // A convenience function for creating one of the labels.  A text mapper
  // and associated actor are added to LabelMappers and LabelActors
  // respectively.  The index to the newly created entries is returned.
  virtual int CreateLabel(
    double value, int minDigits,
    int targetWidth, int targetHeight, vtkViewport* viewport);

//BTX
  // Description:
  // Given a data range, finds locations for tick marks that will have
  // "friendly" labels (that is, can be represented with less units of
  // precision).
  virtual std::vector<double> LinearTickMarks(
    const double range[2], int maxTicks, int& minDigits, bool intOnly=false);
  virtual std::vector<double> LogTickMarks(
    const double range[2], int maxTicks, int& minDigits);
//ETX

  double AspectRatio;
  int AutomaticLabelFormat;
  int DrawTickMarks;
  int AddRangeLabels;

  char* RangeLabelFormat;

  vtkTexture* ScalarBarTexture;
  vtkPolyData* TickMarks;
  vtkPolyDataMapper2D* TickMarksMapper;
  vtkActor2D* TickMarksActor;

  // Description:
  // Space, in pixels, between the labels and the bar itself.  Currently set in
  // PositionTitle.
  int LabelSpace;

  // Description:
  // The justification/alignment of the title.
  int TitleJustification;

  // Description:
  // Flag to add minimum and maximum as annotations
  int AddRangeAnnotations;

private:
  vtkPVScalarBarActor(const vtkPVScalarBarActor &) = delete;
  void operator=(const vtkPVScalarBarActor &) = delete;
};

#endif //__vtkPVScalarBarActor_h
