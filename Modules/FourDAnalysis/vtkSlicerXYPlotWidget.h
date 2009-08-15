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
// .NAME vtkSlicerXYPlotWidget
// .SECTION Description
// vtkSlicerXYPlotWidget is a widget containing widgets that help view and edit plot
//

#ifndef __vtkSlicerXYPlotWidget_h
#define __vtkSlicerXYPlotWidget_h

#include "vtkFourDAnalysisWin32Header.h"
#include "vtkKWRenderWidget.h"
#include "vtkMRMLXYPlotNode.h"

#include <string>
#include <vector>

class vtkDoubleArray;
class vtkXYPlotActor;
class vtkDataObject;

//BTX 

/**** copied from vtkSlicerComponentGUI.h  ****/

#ifndef vtkSetMRMLNodeMacro
#define vtkSetMRMLNodeMacro(node,value)  { \
  vtkObject *oldNode = (node); \
  this->MRMLObserverManager->SetObject ( vtkObjectPointer( &(node)), (value) ); \
  if ( oldNode != (node) ) \
    { \
    this->InvokeEvent (vtkCommand::ModifiedEvent); \
    } \
};
#endif

#ifndef vtkSetAndObserveMRMLNodeMacro
#define vtkSetAndObserveMRMLNodeMacro(node,value)  { \
  vtkObject *oldNode = (node); \
  this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &(node) ), (value) ); \
  if ( oldNode != (node) ) \
    { \
    this->InvokeEvent (vtkCommand::ModifiedEvent); \
    } \
};
#endif

#ifndef vtkSetAndObserveMRMLNodeEventsMacro
#define vtkSetAndObserveMRMLNodeEventsMacro(node,value,events)  { \
  vtkObject *oldNode = (node); \
  this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &(node)), (value), (events)); \
  if ( oldNode != (node) ) \
    { \
    this->InvokeEvent (vtkCommand::ModifiedEvent); \
    } \
};
#endif

//ETX

class VTK_FourDAnalysis_EXPORT vtkSlicerXYPlotWidget : public vtkKWRenderWidget
{

  //----------------------------------------------------------------
  // Data types
  //----------------------------------------------------------------
  
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
  

  //----------------------------------------------------------------
  // New() / PrintSelf
  //----------------------------------------------------------------

 public:

  static vtkSlicerXYPlotWidget* New();
  vtkTypeRevisionMacro(vtkSlicerXYPlotWidget,vtkKWRenderWidget);
  void PrintSelf(ostream& os, vtkIndent indent);


  //----------------------------------------------------------------
  // MRML Event callbacks
  //----------------------------------------------------------------
 public:

  void SetInMRMLCallbackFlag (int flag) {
    this->InMRMLCallbackFlag = flag;
  }
  vtkGetMacro(InMRMLCallbackFlag, int);

  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, void * /*callData*/ );

 protected:
  //BTX
  // a shared function that call the virtual ProcessMRMLEvents subclasses,
  // if they are defined.
  static void MRMLCallback( vtkObject *__caller,
                            unsigned long eid, void *__clientData, void *callData );


  //----------------------------------------------------------------
  // Set and Get Methods
  //----------------------------------------------------------------

 public:

  // Description:
  // Get and set the scene to observe.
  vtkGetObjectMacro(MRMLScene, vtkMRMLScene);
  void SetMRMLScene(vtkMRMLScene *MRMLScene);

  // Description:
  // Set and observe XYPlotNode
  void SetAndObserveXYPlotNode(vtkMRMLXYPlotNode* node);

  // Description:
  // Get XY plot node 
  vtkGetObjectMacro(XYPlotNode, vtkMRMLXYPlotNode);


  //----------------------------------------------------------------
  // Graph operations
  //----------------------------------------------------------------
 protected:
  void UpdateGraph();
  //void ClearPlot();
  //int  AddPlot(vtkDoubleArray* data, const char* label); // returns plot id
  //void SetColor(int id, double r, double g, double b);

  void AddVerticalLine(double x);
  void AddHorizontalLine(double y);
  void SetAxisLineColor(double r, double g, double b);
  void RemoveLines();

  //void AutoRangeOn();
  //void AutoRangeOff();
  //void SetXrange(double min, double max);
  //void SetYrange(double min, double max);
  //
  //void ErrorBarOn();
  //void ErrorBarOff();


  //----------------------------------------------------------------
  // Subroutines for plotting
  //----------------------------------------------------------------
 protected:
  // Description:
  // Create a vtkDataObject to draw a line on the graph
  vtkDataObject* CreateDataObjectForLine(double p1[2], double p2[2]);

  // Description:
  // Create a vtkDoubleArray to draw a line with error bars
  vtkDoubleArray* CreatePlotDataWithErrorBar(vtkDoubleArray* srcData);


  //----------------------------------------------------------------
  // Constructor / destructor
  //----------------------------------------------------------------
 protected:

  vtkSlicerXYPlotWidget();
  virtual ~vtkSlicerXYPlotWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();


 private:
  vtkSlicerXYPlotWidget(const vtkSlicerXYPlotWidget&); // Not implemented
  void operator=(const vtkSlicerXYPlotWidget&); // Not implemented


 protected:
  //----------------------------------------------------------------
  // MRML scene and event callbacks
  //----------------------------------------------------------------

  vtkMRMLScene* MRMLScene;
  vtkMRMLXYPlotNode* XYPlotNode;

  // Description:
  // MRML observer manager
  vtkObserverManager *MRMLObserverManager;

  // Description::
  // Holders for MRML callbacks
  vtkCallbackCommand *MRMLCallbackCommand;

  // Description:
  // Flag to avoid event loops
  int InMRMLCallbackFlag;

  //----------------------------------------------------------------
  // Widgets
  //----------------------------------------------------------------

  vtkXYPlotActor* PlotActor;


  //----------------------------------------------------------------
  // Flags
  //----------------------------------------------------------------

  int Updating;

  //BTX
  typedef std::vector<PlotDataType> PlotDataVectorType;
  typedef std::vector<AxisLineType> AxisLineVectorType;
  //ETX

  //PlotDataVectorType PlotDataVector;
  AxisLineVectorType VerticalLines;
  AxisLineVectorType HorizontalLines;

  double AxisLineColor[3];
  //int    AutoRangeX;
  //int    AutoRangeY;
  double RangeX[2];
  double RangeY[2];

  //int    ErrorBar;

};

#endif

