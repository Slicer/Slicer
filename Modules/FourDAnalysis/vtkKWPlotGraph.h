/*=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH)
  All rights reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================*/
// .NAME vtkKWPlotGraph -  multi column list box to display/edit matrix4x4
// .SECTION Description
// vtkKWPlotGraph is a widget containing widgets that help view and
// edit a matrix
//

#ifndef __vtkKWPlotGraph_h
#define __vtkKWPlotGraph_h

#include "vtkFourDAnalysisWin32Header.h"
#include "vtkKWRenderWidget.h"

#include <string>
#include <vector>

class vtkDoubleArray;
class vtkXYPlotActor;
class vtkDataObject;

class VTK_FourDAnalysis_EXPORT vtkKWPlotGraph : public vtkKWRenderWidget
{
 public:
  //BTX
  typedef struct {
    int             visible;  // 0: invisible   1: visible
    vtkDoubleArray* data;
    std::string     label;
    double          color[3];
  } PlotDataType;
  typedef struct {
    int             visible;
    double          pos;
    double          color[3];
  } AxisLineType;
  //ETX
  
 public:
  static vtkKWPlotGraph* New();
  vtkTypeRevisionMacro(vtkKWPlotGraph,vtkKWRenderWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  void UpdateGraph();
  void ClearPlot();
  int  AddPlot(vtkDoubleArray* data, const char* label); // returns plot id
  void SetColor(int id, double r, double g, double b);

  void AddVerticalLine(double x);
  void AddHorizontalLine(double y);
  void SetAxisLineColor(double r, double g, double b);
  void RemoveLines();

  void AutoRangeOn();
  void AutoRangeOff();
  void SetXrange(double min, double max);
  void SetYrange(double min, double max);

  void ErrorBarOn();
  void ErrorBarOff();

  // Description:
  // Command to call when the User manipulates the widget
  virtual void SetCommand(vtkObject *object, const char *method) {};

  // TODO: have special commands for start/end events
  //virtual void SetStartCommand(vtkObject *object, const char *method);
  //virtual void SetEndCommand(vtkObject *object, const char *method);

  // Description:
  // TODO: access internal widgets
  //vtkKWRange* GetXRange() { return this->Range[0]; };

 protected:
  vtkKWPlotGraph();
  virtual ~vtkKWPlotGraph();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Description:
  // Create a vtkDataObject to draw a line on the graph
  vtkDataObject* CreateDataObjectForLine(double p1[2], double p2[2]);


  // Description:
  // Create a vtkDoubleArray to draw a line with error bars
  vtkDoubleArray* CreatePlotDataWithErrorBar(vtkDoubleArray* srcData);

  char *Command;
  //char *StartCommand;
  //char *EndCommand;

 private:
  vtkKWPlotGraph(const vtkKWPlotGraph&); // Not implemented
  void operator=(const vtkKWPlotGraph&); // Not implemented

  vtkXYPlotActor* PlotActor;

  int Updating;

  //BTX
  typedef std::vector<PlotDataType> PlotDataVectorType;
  typedef std::vector<AxisLineType> AxisLineVectorType;
  //ETX

  PlotDataVectorType PlotDataVector;
  AxisLineVectorType VerticalLines;
  AxisLineVectorType HorizontalLines;

  double AxisLineColor[3];
  int    AutoRangeX;
  int    AutoRangeY;
  double RangeX[2];
  double RangeY[2];

  int    ErrorBar;

};

#endif

