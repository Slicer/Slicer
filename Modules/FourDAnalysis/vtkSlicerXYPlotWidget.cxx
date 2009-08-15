/*=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH)
  All rights reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $

=========================================================================*/
#include "vtkCommand.h"

#include "vtkSlicerXYPlotWidget.h"

#include "vtkKWMultiColumnList.h"
#include "vtkObjectFactory.h"

#include "vtkXYPlotActor.h"
#include "vtkProperty2D.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkDoubleArray.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPiecewiseFunction.h"
#include "vtkFieldData.h"
#include "vtkDataArray.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkTextProperty.h"
#include "vtkGlyphSource2D.h"

#include <math.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerXYPlotWidget );
vtkCxxRevisionMacro(vtkSlicerXYPlotWidget, "$Revision: 1.49 $");

//----------------------------------------------------------------------------
vtkSlicerXYPlotWidget::vtkSlicerXYPlotWidget()
{

  // Set up callbacks
  this->MRMLObserverManager = vtkObserverManager::New();
  this->MRMLObserverManager->AssignOwner( this );
  this->MRMLObserverManager->GetCallbackCommand()->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLObserverManager->GetCallbackCommand()->SetCallback(vtkSlicerXYPlotWidget::MRMLCallback);
  this->MRMLCallbackCommand = this->MRMLObserverManager->GetCallbackCommand();

  this->MRMLScene  = NULL;
  this->XYPlotNode = NULL;

  this->Updating  = 0;
  this->PlotActor = NULL;
  //this->PlotDataVector.clear();
  this->VerticalLines.clear();
  this->HorizontalLines.clear();

  this->RangeX[0] = 0.0;
  this->RangeX[1] = 1.0;
  this->RangeY[0] = 0.0;
  this->RangeY[1] = 1.0;

  //this->AutoRangeX = 1;
  //this->AutoRangeY = 1;

  this->AxisLineColor[0] = 0.0;
  this->AxisLineColor[1] = 0.0;
  this->AxisLineColor[2] = 0.0;

  //this->ErrorBar = 0;
}


//----------------------------------------------------------------------------
vtkSlicerXYPlotWidget::~vtkSlicerXYPlotWidget()
{

  //this->PlotDataVector.clear();
  this->VerticalLines.clear();
  this->HorizontalLines.clear();

  if (this->PlotActor)
    {
    this->PlotActor->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  this->Superclass::CreateWidget();

  this->PlotActor = vtkXYPlotActor::New();

  this->PlotActor->SetDataObjectPlotModeToColumns();
  this->PlotActor->SetXValuesToValue();

  this->PlotActor->GetPositionCoordinate()->SetValue(0.0, 0.0, 0);
  this->PlotActor->GetPosition2Coordinate()->SetValue(1.0, 1.0, 0); //#relative to Position

  //this->PlotActor->SetXValuesToArcLength();
  //this->PlotActor->SetNumberOfXLabels(6);

  this->PlotActor->SetTitle("");
  this->PlotActor->SetXTitle("Time (s)");
  this->PlotActor->SetYTitle("");
  this->PlotActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
  this->PlotActor->GetProperty()->SetLineWidth(1);
  //this->PlotActor->GetProperty()->SetPointSize(3);

  //# Set text prop color (same color for backward compat with test)
  //# Assign same object to all text props
  vtkTextProperty* tprop = this->PlotActor->GetTitleTextProperty();
  tprop->SetColor(0.0, 0.0, 0.0);
  tprop->ItalicOff();
  tprop->BoldOff();
  tprop->ShadowOff();
  tprop->SetFontSize(10);

  this->PlotActor->SetAxisTitleTextProperty(tprop);
  this->PlotActor->SetAxisLabelTextProperty(tprop);
  
  //this->PlotActor->PlotPointsOn();
  //this->PlotActor->SetGlyphSize(10.0);
  //this->PlotActor->GetGlyphSource()->SetGlyphTypeToSquare();
  //this->PlotActor->PlotCurvePointsOn();

  //# Create the Renderers, RenderWindow, and RenderWindowInteractor.
  vtkRenderer* ren = this->GetRenderer();
  //ren->SetBackground(1.0, 1.0, 1.0);
  ren->SetBackground(1.0, 1.0, 1.0);
  ren->SetViewport(0.0, 0.0, 1.0, 1.0);
  ren->AddActor2D(this->PlotActor);
  //this->GetRenderWindow()->SetSize(500, 250);

}


//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  //os << indent << "Matrix4x4: " << this->GetMatrix4x4() << endl;
}


//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::ProcessMRMLEvents ( vtkObject * caller, 
                                                unsigned long event, void * callData )
{

  if (event == vtkCommand::ModifiedEvent)
    {
    vtkMRMLXYPlotNode* pnode = vtkMRMLXYPlotNode::SafeDownCast(caller);
    if (pnode)
      {
      this->UpdateGraph();
      }
    }
}


//----------------------------------------------------------------------------
// Description:
// the MRMLCallback is a static function to relay modified events from the 
// observed mrml node back into the gui layer for further processing
//
void vtkSlicerXYPlotWidget::MRMLCallback(vtkObject *caller, 
                                         unsigned long eid, void *clientData, void *callData)
{
  vtkSlicerXYPlotWidget *self = reinterpret_cast<vtkSlicerXYPlotWidget *>(clientData);
  
  if (self->GetInMRMLCallbackFlag())
    {
#ifdef _DEBUG
    vtkDebugWithObjectMacro(self, "In vtkSlicerXYPlotWidget *********MRMLCallback called recursively?");
#endif
    return;
    }
  
  vtkDebugWithObjectMacro(self, "In vtkSlicerXYPlotWidget MRMLCallback");
  
  self->SetInMRMLCallbackFlag(1);
  self->ProcessMRMLEvents(caller, eid, callData);
  self->SetInMRMLCallbackFlag(0);
}


//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::SetMRMLScene( vtkMRMLScene *aMRMLScene)
{

  if ( this->MRMLScene )
    {
    this->MRMLScene->RemoveObserver( this->MRMLCallbackCommand );
    this->MRMLScene->Delete ( );
    this->MRMLScene = NULL;
    //    this->MRMLScene->Delete();
    }

  this->MRMLScene = aMRMLScene;

  if ( this->MRMLScene )
    {
    this->MRMLScene->Register(this);
    this->MRMLScene->AddObserver( vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver( vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver( vtkMRMLScene::NewSceneEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver( vtkMRMLScene::SceneCloseEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver( vtkMRMLScene::SceneEditedEvent, this->MRMLCallbackCommand );
    }

}


//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::SetAndObserveXYPlotNode(vtkMRMLXYPlotNode* node)
{

  if (this->XYPlotNode)
    {
    vtkSetAndObserveMRMLObjectMacro(this->XYPlotNode, NULL);
    }
    
  this->XYPlotNode = node;
  
  if (node)
    {
    vtkMRMLXYPlotNode* pnode = this->GetXYPlotNode();
    
    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkSetAndObserveMRMLObjectEventsMacro(this->XYPlotNode, pnode, events);
    events->Delete();
    }

}


//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::UpdateGraph()
{

  if ( this->Updating ) 
    {
    return;
    }
  this->Updating = 1;

  if (!this->XYPlotNode)
    {
    return;
    }

  unsigned int numArray = this->XYPlotNode->GetNumberOfArrays();

  if (numArray <= 0)
    {
    return;
    }

  //--------------------------------------------------
  // Check automatic range adjustment flag
  if (this->XYPlotNode->GetAutoXRange() == 0)  // off
    {
    this->XYPlotNode->GetXRange(this->RangeX);
    }

  if (this->XYPlotNode->GetAutoYRange() == 0)  // off
    {
    this->XYPlotNode->GetYRange(this->RangeY);
    }
  
  //--------------------------------------------------
  // If auto-range options active, the ranges have to be
  // determined by finding minimum and maximum values in the data.
  int autoX = this->XYPlotNode->GetAutoXRange();
  int autoY = this->XYPlotNode->GetAutoYRange();
  
  if (autoX && autoY)
    {
    vtkMRMLDoubleArrayNode* node;
    double rangeX[2];
    double rangeY[3];
    int errorBar;
    
    // Substitute the first values
    node  = this->XYPlotNode->GetArrayNode(0);
    errorBar = this->XYPlotNode->GetErrorBarPlot(0);
    node->GetRange(rangeX, rangeY, errorBar);

    if (autoX)
      {
      this->RangeX[0] = rangeX[0];
      this->RangeX[1] = rangeX[1];
      }
    if (autoY)
      {
      this->RangeY[0] = rangeY[0];
      this->RangeY[1] = rangeY[1];
      }

    // Search the list
    for (unsigned int i = 1; i < numArray; i ++)
      {
      node = this->XYPlotNode->GetArrayNode(i);
      errorBar = this->XYPlotNode->GetErrorBarPlot(i);
      node->GetRange(rangeX, rangeY, errorBar);
      if (autoX)
        {
        if (rangeX[0] < this->RangeX[0])
          {
          rangeX[0] = this->RangeX[0];
          }
        if (rangeX[1] < this->RangeX[1])
          {
          rangeX[1] = this->RangeX[1];
          }
        }
      if (autoY)
        {
        if (rangeY[0] < this->RangeY[0])
          {
          rangeY[0] = this->RangeY[0];
          }
        if (rangeY[1] < this->RangeY[1])
          {
          rangeY[1] = this->RangeY[1];
          }
        }
      }
    }


  //// -----------------------------------------
  //// Set color for vertical and horizontal lines
  //
  //AxisLineVectorType::iterator aiter;
  //for (aiter = this->VerticalLines.begin(); aiter != this->VerticalLines.end(); aiter ++)
  //  {
  //  aiter->color[0] = this->AxisLineColor[0];
  //  aiter->color[1] = this->AxisLineColor[1];
  //  aiter->color[2] = this->AxisLineColor[2];
  //  }
  //for (aiter = this->HorizontalLines.begin(); aiter != this->HorizontalLines.end(); aiter ++)
  //  {
  //  aiter->color[0] = this->AxisLineColor[0];
  //  aiter->color[1] = this->AxisLineColor[1];
  //  aiter->color[2] = this->AxisLineColor[2];
  //  }


  
  // -----------------------------------------
  // Drawing

  if (this->PlotActor)
    {
    this->PlotActor->RemoveAllInputs();

    // -----------------------------------------
    // Draw curves

    int obj = 0;
    //PlotDataVectorType::iterator iter;

    // Search the list
    for (unsigned int i = 1; i < numArray; i ++)
      {
      if (this->XYPlotNode->GetVisibility(i))
        {
        double r;
        double g;
        double b;
        this->XYPlotNode->GetColor(i, &r, &g, &b);
        vtkMRMLDoubleArrayNode* node = this->XYPlotNode->GetArrayNode(i);
        int errorBar = this->XYPlotNode->GetErrorBarPlot(i);
        vtkDoubleArray* array = node->GetArray();

        vtkFieldData* fieldData = vtkFieldData::New();
        
        if (errorBar)
          {
          // if error bar plotting is enabled, generate plot data with error bars.
          vtkDoubleArray* data = CreatePlotDataWithErrorBar(array);
          fieldData->AddArray(data);
          data->Delete();
          }
        else
          {
          fieldData->AddArray(array);
          }

        vtkDataObject* dataObject = vtkDataObject::New();
        dataObject->SetFieldData( fieldData );

        this->PlotActor->AddDataObjectInput(dataObject);

        this->PlotActor->SetDataObjectXComponent(obj, 0);
        this->PlotActor->SetDataObjectYComponent(obj, 1);
        this->PlotActor->SetPlotColor(obj, r, g, b);

        fieldData->Delete();
        dataObject->Delete();

        obj ++;
        
        }
      }

    //// -----------------------------------------
    //// Draw vertical lines
    //
    //AxisLineVectorType::iterator aiter;
    //for (aiter = this->VerticalLines.begin(); aiter != this->VerticalLines.end(); aiter ++)
    //  {
    //  if (aiter->visible)
    //    {
    //    vtkFloatArray* value = vtkFloatArray::New();
    //    value->SetNumberOfComponents( static_cast<vtkIdType>(2) );
    //    float xy[2];
    //    xy[0] = aiter->pos;  xy[1] = this->RangeY[0]; 
    //    value->InsertNextTuple( xy );
    //    xy[0] = aiter->pos;  xy[1] = this->RangeY[1]; 
    //    value->InsertNextTuple( xy );
    //
    //    vtkFieldData* fieldData = vtkFieldData::New();
    //    fieldData->AddArray(value);
    //    value->Delete();
    //    
    //    vtkDataObject* dataObject = vtkDataObject::New();
    //    dataObject->SetFieldData( fieldData );
    //    fieldData->Delete();
    //
    //    this->PlotActor->AddDataObjectInput(dataObject);
    //    dataObject->Delete();
    //
    //    this->PlotActor->SetDataObjectXComponent(obj, 0);
    //    this->PlotActor->SetDataObjectYComponent(obj, 1);
    //    this->PlotActor->SetPlotColor(obj, aiter->color[0], aiter->color[1], aiter->color[2]);
    //
    //    obj ++;
    //    }
    //  }
    //
    //
    //// -----------------------------------------
    //// Draw horizontal lines
    //
    //for (aiter = this->HorizontalLines.begin(); aiter != this->HorizontalLines.end(); aiter ++)
    //  {
    //  if (aiter->visible)
    //    {
    //    vtkFloatArray* value = vtkFloatArray::New();
    //    value->SetNumberOfComponents( static_cast<vtkIdType>(2) );
    //    float xy[2];
    //    xy[0] = this->RangeX[0];  xy[1] = aiter->pos; 
    //    value->InsertNextTuple( xy );
    //    xy[0] = this->RangeX[1];  xy[1] = aiter->pos; 
    //    value->InsertNextTuple( xy );
    //
    //    vtkFieldData* fieldData = vtkFieldData::New();
    //    fieldData->AddArray(value);
    //    value->Delete();
    //
    //    vtkDataObject* dataObject = vtkDataObject::New();
    //    dataObject->SetFieldData( fieldData );
    //    fieldData->Delete();
    //
    //    this->PlotActor->AddDataObjectInput(dataObject);
    //    dataObject->Delete();
    //
    //    this->PlotActor->SetDataObjectXComponent(obj, 0);
    //    this->PlotActor->SetDataObjectYComponent(obj, 1);
    //    this->PlotActor->SetPlotColor(obj, aiter->color[0], aiter->color[1], aiter->color[2]);
    //
    //    obj ++;
    //    }
    //  }

    this->PlotActor->SetXRange(this->RangeX[0], this->RangeX[1]);
    this->PlotActor->SetYRange(this->RangeY[0], this->RangeY[1]);
    
    this->GetRenderWindowInteractor()->Initialize();
    this->GetRenderWindow()->Render();
    this->GetRenderWindowInteractor()->Start();

    }

    
  this->Updating = 0;
}



////----------------------------------------------------------------------------
//void vtkSlicerXYPlotWidget::ClearPlot()
//{
//  //this->PlotDataVector.clear();
//}


////----------------------------------------------------------------------------
//int vtkSlicerXYPlotWidget::AddPlot(vtkDoubleArray* data, const char* label)
//{
//  PlotDataType plotData;
//  plotData.data    = data;
//  plotData.label   = label;
//  plotData.visible = 1;
//
//  this->PlotDataVector.push_back(plotData);
//  //return this->PlotDataVector.size()-1;
//}


//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::AddVerticalLine(double x)
{
  AxisLineType lineData;
  lineData.pos     = x;
  lineData.visible = 1;
  this->VerticalLines.push_back(lineData);
}


//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::AddHorizontalLine(double y)
{
  AxisLineType lineData;
  lineData.pos     = y;
  lineData.visible = 1;
  this->HorizontalLines.push_back(lineData);
}


//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::SetAxisLineColor(double r, double g, double b)
{
  this->AxisLineColor[0] = r;
  this->AxisLineColor[1] = g;
  this->AxisLineColor[2] = b;
}


//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::RemoveLines()
{
  this->VerticalLines.clear();
  this->HorizontalLines.clear();
}


////----------------------------------------------------------------------------
//void vtkSlicerXYPlotWidget::SetColor(int id, double r, double g, double b)
//{
//  if (id >= this->PlotDataVector.size())
//    {
//    return;
//    }
//
//  this->PlotDataVector[id].color[0] = r;
//  this->PlotDataVector[id].color[1] = g;
//  this->PlotDataVector[id].color[2] = b;
//}


////----------------------------------------------------------------------------
//void vtkSlicerXYPlotWidget::AutoRangeOn()
//{
//  this->AutoRangeX = 1;
//  this->AutoRangeY = 1;
//}
//
//
////----------------------------------------------------------------------------
//void vtkSlicerXYPlotWidget::AutoRangeOff()
//{
//  this->AutoRangeX = 0;
//  this->AutoRangeY = 0;
//}
//
//
////----------------------------------------------------------------------------
//void vtkSlicerXYPlotWidget::SetXrange(double min, double max)
//{
//  if (min < max)
//    {
//    this->AutoRangeX = 0;
//    this->RangeX[0] = min;
//    this->RangeX[1] = max;
//    }
//}
//
//
////----------------------------------------------------------------------------
//void vtkSlicerXYPlotWidget::SetYrange(double min, double max)
//{
//  if (min < max)
//    {
//    this->AutoRangeY = 0;
//    this->RangeY[0] = min;
//    this->RangeY[1] = max;
//    }
//}
//
//
////----------------------------------------------------------------------------
//void vtkSlicerXYPlotWidget::ErrorBarOn()
//{
//  this->ErrorBar = 1;
//}
//
//
////----------------------------------------------------------------------------
//void vtkSlicerXYPlotWidget::ErrorBarOff()
//{
//  this->ErrorBar = 0;
//}



//----------------------------------------------------------------------------
vtkDataObject* vtkSlicerXYPlotWidget::CreateDataObjectForLine(double p1[2], double p2[2])
{

  vtkDoubleArray* value = vtkDoubleArray::New();
  value->SetNumberOfComponents( static_cast<vtkIdType>(2) );
  value->InsertNextTuple( p1 );
  value->InsertNextTuple( p2 );
  
  vtkFieldData* fieldData = vtkFieldData::New();
  fieldData->AddArray(value);
  value->Delete();
  
  vtkDataObject* dataObject = vtkDataObject::New();
  dataObject->SetFieldData( fieldData );
  fieldData->Delete();
  
  return dataObject;
  
}


//----------------------------------------------------------------------------
vtkDoubleArray* vtkSlicerXYPlotWidget::CreatePlotDataWithErrorBar(vtkDoubleArray* srcData)
{
  vtkDoubleArray* plotData;
  plotData = vtkDoubleArray::New();
  plotData->SetNumberOfComponents( static_cast<vtkIdType>(2) );
  
  // Check if the data exists and it has 3 components (x, y, and error)
  if (!srcData || srcData->GetNumberOfComponents() != static_cast<vtkIdType>(3))
    {
    int nData = srcData->GetNumberOfTuples();
    for (int i = 0; i < nData; i ++)
      {
      double* values = srcData->GetTuple(i);
      plotData->InsertNextTuple( values );
      }
    return plotData;
    }

  // Note: Error bar
  //
  //        p2
  //   p0 --+-- p1    ---
  //        |          ^
  //        *          | 2 * error
  //        |          v
  //   p4 --+-- p5    ---   
  //        p3
  //
  //      |<->| error bar width * 2
  //
  // 'error bar width' = ((this->RangeX[1] - this->RangeX[0]) / nData) / 8
  //
  
  int nData = srcData->GetNumberOfTuples();
  double errorBarWidth;

  if (nData > 10)
    {
    errorBarWidth = ((this->RangeX[1] - this->RangeX[0]) / (double)nData) / 8.0;
    }
  else
    {
    errorBarWidth = ((this->RangeX[1] - this->RangeX[0]) / 10.0) / 8.0;
    }
  
  for (int j = 0; j < nData; j ++)
    {
    double* values = srcData->GetTuple(j);
    double p[6][2];
    
    // set x coordinates
    p[0][0] = p[4][0] = values[0] - errorBarWidth;
    p[2][0] = p[3][0] = values[0];
    p[1][0] = p[5][0] = values[0] + errorBarWidth;
    
    // set y coordinates
    p[0][1] = p[1][1] = p[2][1] = values[1] + values[2];
    p[3][1] = p[4][1] = p[5][1] = values[1] - values[2];
    
    // Add data point
    plotData->InsertNextTuple( values );

    // Add points for error bars
    plotData->InsertNextTuple( p[2] );
    plotData->InsertNextTuple( p[0] );
    plotData->InsertNextTuple( p[1] );
    plotData->InsertNextTuple( p[2] );
    plotData->InsertNextTuple( p[3] );
    plotData->InsertNextTuple( p[4] );
    plotData->InsertNextTuple( p[5] );
    plotData->InsertNextTuple( p[3] );
    plotData->InsertNextTuple( values );
    }

  return plotData;
}


