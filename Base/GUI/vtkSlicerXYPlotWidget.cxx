/*=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH)
  All rights reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL$
  Date:      $Date$

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
#include "vtkLegendBoxActor.h"

#include <math.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerXYPlotWidget );
vtkCxxRevisionMacro(vtkSlicerXYPlotWidget, "$Revision$");

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
  this->PlotManagerNode = NULL;

  this->Updating  = 0;
  this->PlotActor = NULL;
  //this->VerticalLines.clear();
  //this->HorizontalLines.clear();

  this->RangeX[0] = 0.0;
  this->RangeX[1] = 1.0;
  this->RangeY[0] = 0.0;
  this->RangeY[1] = 1.0;

  this->AxisLineColor[0] = 0.0;
  this->AxisLineColor[1] = 0.0;
  this->AxisLineColor[2] = 0.0;

  this->AutoUpdate = 0;
  this->InMRMLCallbackFlag = 0;

}


//----------------------------------------------------------------------------
vtkSlicerXYPlotWidget::~vtkSlicerXYPlotWidget()
{

  //this->VerticalLines.clear();
  //this->HorizontalLines.clear();

  if (this->PlotActor)
    {
    this->PlotActor->Delete();
    }

  if ( this->MRMLScene )
    {
    this->MRMLScene->RemoveObserver( this->MRMLCallbackCommand );
    this->MRMLScene->Delete ( );
    this->MRMLScene = NULL;
    //    this->MRMLScene->Delete();
    }

  if (this->MRMLObserverManager)
    {
    this->MRMLObserverManager->Delete();
    this->MRMLObserverManager = NULL;
    }

  //if (this->PlotManagerNode)
  //  {
  //  this->PlotManagerNode->Delete();
  //  this->PlotManagerNode = NULL;
  //  }
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

  this->PlotActor->SetTitle("title");
  this->PlotActor->SetXTitle("xlabel");
  this->PlotActor->SetYTitle("ylabel");

  this->PlotActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
  this->PlotActor->GetProperty()->SetLineWidth(1);
  this->PlotActor->GetProperty()->SetPointSize(3);

  //# Create the Renderers, RenderWindow, and RenderWindowInteractor.
  vtkRenderer* ren = this->GetRenderer();
  ren->SetBackground(1.0, 1.0, 1.0);
  ren->SetViewport(0.0, 0.0, 1.0, 1.0);
  ren->AddActor2D(this->PlotActor);

}


//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::ProcessMRMLEvents(vtkObject * caller,
                                              unsigned long event,
                                              void * vtkNotUsed(callData))
{

  if ((this->AutoUpdate && event == vtkCommand::ModifiedEvent) ||
      event == vtkMRMLXYPlotManagerNode::UpdateGraphEvent)
    {
    vtkMRMLXYPlotManagerNode* pnode = vtkMRMLXYPlotManagerNode::SafeDownCast(caller);
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
    this->MRMLScene->AddObserver( vtkMRMLScene::SceneClosedEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver( vtkMRMLScene::SceneEditedEvent, this->MRMLCallbackCommand );
    }

}


//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::SetAndObservePlotManagerNode(vtkMRMLXYPlotManagerNode* node)
{

  if (this->PlotManagerNode)
    {
    vtkSetAndObserveMRMLObjectMacro(this->PlotManagerNode, NULL);
    }
    
  this->PlotManagerNode = node;
  
  if (node)
    {
    vtkMRMLXYPlotManagerNode* pnode = this->GetPlotManagerNode();
    
    vtkIntArray *events = vtkIntArray::New();
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    events->InsertNextValue(vtkMRMLXYPlotManagerNode::UpdateGraphEvent);
    vtkSetAndObserveMRMLObjectEventsMacro(this->PlotManagerNode, pnode, events);
    events->Delete();
    }

}


//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::UpdateGraph()
{

  if (!this->PlotManagerNode)
    {
    return;
    }

  if ( this->Updating ) 
    {
    return;
    }
  this->Updating = 1;

  unsigned int numPlots = this->PlotManagerNode->GetNumberOfPlotNodes();

  if (numPlots <= 0)
    {
    this->Updating = 0;
    return;
    }

  vtkIntArray* idList = this->PlotManagerNode->GetPlotNodeIDList();

  //--------------------------------------------------
  // Check automatic range adjustment flag
  if (this->PlotManagerNode->GetAutoXRange() == 0)  // off
    {
    this->PlotManagerNode->GetXRange(this->RangeX);
    }

  if (this->PlotManagerNode->GetAutoYRange() == 0)  // off
    {
    this->PlotManagerNode->GetYRange(this->RangeY);
    }
  
  //--------------------------------------------------
  // If auto-range options active, the ranges have to be
  // determined by finding minimum and maximum values in the data.
  int autoX = this->PlotManagerNode->GetAutoXRange();
  int autoY = this->PlotManagerNode->GetAutoYRange();
  
  if (autoX && autoY)
    {
    vtkMRMLPlotNode* node;

    double rangeX[2];
    double rangeY[2];
    
    // Substitute the first values
    int id = idList->GetValue(0);
    unsigned int p = 0;
    
    // Find the first visible plot node
    // and set initial range
    for (; p < numPlots; p ++)
      {
      node  = this->PlotManagerNode->GetPlotNode(id);
      if (node && node->GetVisible() > 0)
        {
        node->GetXRange(rangeX);
        node->GetYRange(rangeY);
        
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
        break;
        }
      }

    // Search the list
    for (unsigned int i = p+1; i < numPlots; i ++)
      {
      id = idList->GetValue(i);
      node = this->PlotManagerNode->GetPlotNode(id);
          
      if (autoX && node->GetXRange(rangeX))
        {
        if (rangeX[0] < this->RangeX[0])
          {
          this->RangeX[0] = rangeX[0];
          }
        if (rangeX[1] > this->RangeX[1])
          {
          this->RangeX[1] = rangeX[1];
          }
        }
      if (autoY && node->GetYRange(rangeY))
        {
        if (rangeY[0] < this->RangeY[0])
          {
          this->RangeY[0] = rangeY[0];
          }
        if (rangeY[1] > this->RangeY[1])
          {
          this->RangeY[1] = rangeY[1];
          }
        }
      }
    }

  // -----------------------------------------
  // Drawing

  if (this->PlotActor)
    {
    this->PlotActor->RemoveAllInputs();

    // -----------------------------------------
    // Plot title and labels

    // Set text prop color (same color for backward compat with test)
    // Assign same object to all text props
    vtkTextProperty* tprop = vtkTextProperty::New();
    tprop->SetColor(0.0, 0.0, 0.0);
    tprop->ItalicOff();
    tprop->BoldOff();
    tprop->ShadowOff();
    tprop->SetFontSize(10);
    this->PlotActor->SetTitleTextProperty(tprop);
    tprop->Delete();

    vtkTextProperty* attprop = vtkTextProperty::New();
    attprop->SetColor(0.0, 0.0, 0.0);
    attprop->SetOpacity(1.0);
    attprop->ItalicOff();
    attprop->BoldOff();
    attprop->ShadowOff();
    attprop->SetFontSize(10);
    this->PlotActor->SetAxisTitleTextProperty(attprop);
    attprop->Delete();

    vtkTextProperty* altprop = vtkTextProperty::New();
    altprop->SetColor(0.0, 0.0, 0.0);
    altprop->ItalicOff();
    altprop->BoldOff();
    altprop->ShadowOff();
    altprop->SetFontSize(10);
    this->PlotActor->SetAxisLabelTextProperty(altprop);
    altprop->Delete();
    
    // Set plot title
    this->PlotActor->SetTitle(this->PlotManagerNode->GetTitle());


    // Set axis titles
    if (strlen(this->PlotManagerNode->GetXLabel()) == 0)
      {
      this->PlotActor->SetXTitle(" ");
      }
    else
      {
      this->PlotActor->SetXTitle(this->PlotManagerNode->GetXLabel());
      }
    if (strlen(this->PlotManagerNode->GetYLabel()) == 0)
      {
      this->PlotActor->SetYTitle(" ");
      }
    else
      {
      this->PlotActor->SetYTitle(this->PlotManagerNode->GetYLabel());
      }

    //this->PlotActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
    //this->PlotActor->GetProperty()->SetLineWidth(1);
    //this->PlotActor->GetProperty()->SetPointSize(3);
    
    // -----------------------------------------
    // Draw curves

    int obj = 0;

    // Search the list
    for (unsigned int i = 0; i < numPlots; i ++)
      {
      int id = idList->GetValue(i);
      vtkMRMLPlotNode* node  = this->PlotManagerNode->GetPlotNode(id);

      if (node && node->GetVisible())
        {
        double r;
        double g;
        double b;

        node->GetColor(r, g, b);

        vtkDataObject* dataObject = node->GetDrawObject(this->RangeX, this->RangeY);
        if (dataObject)
          {
          this->PlotActor->AddDataObjectInput(dataObject);
          this->PlotActor->SetDataObjectXComponent(obj, 0);
          this->PlotActor->SetDataObjectYComponent(obj, 1);
          this->PlotActor->SetPlotColor(obj, r, g, b);
          dataObject->Delete();

          obj ++;
          }
        
        }
      }
    
    //// -----------------------------------------
    //// Set legend
    ////this->PlotActor->GetLegendActor()->SetNumberOfEntries(obj);
    //obj = 0;
    //
    //for (unsigned int i = 0; i < numPlots; i ++)
    //  {
    //  int id = idList->GetValue(i);
    //  vtkMRMLPlotNode* node;
    //  node  = this->PlotManagerNode->GetPlotNode(id);
    //  if (node->GetVisible())
    //    {
    //    //this->PlotActor->GetLegendActor()->SetEntryString(obj, node->GetName());
    //    this->PlotActor->GetLegendActor()->SetEntryString(obj, "abcd");
    //    obj ++;
    //    }
    //  }
    
    //this->PlotActor->GetLegendActor()->BoxOn();

    this->PlotActor->SetXRange(this->RangeX[0], this->RangeX[1]);
    this->PlotActor->SetYRange(this->RangeY[0], this->RangeY[1]);
    
    this->GetRenderWindowInteractor()->Initialize();
    this->GetRenderWindow()->Render();
    this->GetRenderWindowInteractor()->Start();

    }

  idList->Delete();
    
  this->Updating = 0;
}


//----------------------------------------------------------------------------
void vtkSlicerXYPlotWidget::SetAxisLineColor(double r, double g, double b)
{
  this->AxisLineColor[0] = r;
  this->AxisLineColor[1] = g;
  this->AxisLineColor[2] = b;
}


