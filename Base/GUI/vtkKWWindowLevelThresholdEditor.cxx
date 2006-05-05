/*=========================================================================

  Module:    $RCSfile: vtkKWWindowLevelThresholdEditor.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWWindowLevelThresholdEditor.h"

#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWWindowLevelThresholdEditor );
vtkCxxRevisionMacro(vtkKWWindowLevelThresholdEditor, "$Revision: 1.49 $");

//----------------------------------------------------------------------------
vtkKWWindowLevelThresholdEditor::vtkKWWindowLevelThresholdEditor()
{
  this->Command = NULL;
  //this->StartCommand = NULL;
  //this->EndCommand   = NULL;

  this->ImageData = NULL;
  
  this->WindoLevelRange = vtkKWRange::New();
  this->ThresholdRange = vtkKWRange::New();
  this->ColorTransferFunctionEditor = vtkKWColorTransferFunctionEditor::New();   
  this->Histogram = vtkKWHistogram::New();
  this->TransferFunction = vtkColorTransferFunction::New();
}

//----------------------------------------------------------------------------
vtkKWWindowLevelThresholdEditor::~vtkKWWindowLevelThresholdEditor()
{
  if (this->Command)
    {
    delete [] this->Command;
    }
  if (this->ImageData)
    {
    this->SetImageData(NULL);
    }
  
  this->WindoLevelRange->Delete();
  this->ThresholdRange->Delete();
  this->Histogram->Delete();
  this->TransferFunction->Delete();
  this->ColorTransferFunctionEditor->Delete();

}

void vtkKWWindowLevelThresholdEditor::SetImageData(vtkImageData* imageData)
{
  if (this->ImageData != imageData) 
    {
    vtkImageData* tempImageData = this->ImageData;
    this->ImageData = imageData;
    if (this->ImageData != NULL)
      {
      this->ImageData->Register(this);
      }
    if (tempImageData != NULL)
      {
      tempImageData->UnRegister(this);
      }
    this->Modified();   
      
    this->UpdateRangesFromImage();

    this->UpdateHistogram();
    }
}


void vtkKWWindowLevelThresholdEditor::SetWindowLevel(double window, double level)
{
  
  this->WindoLevelRange->SetRange(level - 0.5*window, level + 0.5*window);
  
  this->UpdateHistogram();
}
double vtkKWWindowLevelThresholdEditor::GetWindow()
{
  double *range = this->WindoLevelRange->GetRange();
  return range[1] -  range[0];
}

double vtkKWWindowLevelThresholdEditor::GetLevel()
{
  double *range = this->WindoLevelRange->GetRange();
  return 0.5 * (range[1] +  range[0]);
}

 
void vtkKWWindowLevelThresholdEditor::SetThreshold(double lower, double upper)
{
  this->ThresholdRange->SetRange(lower, upper);
  this->UpdateHistogram();
}

double vtkKWWindowLevelThresholdEditor::GetLowerThreshold()
{
  double *range = this->ThresholdRange->GetRange();
  return range[0];
}


double vtkKWWindowLevelThresholdEditor::GetUpperThreshold()
{
  double *range = this->ThresholdRange->GetRange();
  return range[1];
}



//----------------------------------------------------------------------------
void vtkKWWindowLevelThresholdEditor::CreateWidget()
{
  // Check if already created
  
  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  this->Superclass::CreateWidget();
  
  this->UpdateHistogram();  
  
  this->WindoLevelRange->SetParent(this);
  this->WindoLevelRange->Create();
  this->WindoLevelRange->SymmetricalInteractionOn();
  this->WindoLevelRange->SetCommand(this, "ProcessWindowLevelCommand");
  this->WindoLevelRange->SetStartCommand(this, "ProcessWindowLevelStartCommand");
  this->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 20", 
    this->WindoLevelRange->GetWidgetName());
  
  this->ThresholdRange->SetParent(this);
  this->ThresholdRange->Create();
  this->ThresholdRange->SymmetricalInteractionOn();
  this->ThresholdRange->SetCommand(this, "ProcessThresholdCommand");
  this->ThresholdRange->SetStartCommand(this, "ProcessThresholdStartCommand");
  this->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 20", 
    this->ThresholdRange->GetWidgetName());
    
  this->ColorTransferFunctionEditor->SetParent(this);
  this->ColorTransferFunctionEditor->Create();
  this->ColorTransferFunctionEditor->ExpandCanvasWidthOff();
  this->ColorTransferFunctionEditor->SetCanvasWidth(450);
  this->ColorTransferFunctionEditor->SetCanvasHeight(150);
  this->ColorTransferFunctionEditor->SetLabelText("Window/Level/Threshold Editor");
  this->ColorTransferFunctionEditor->SetRangeLabelPositionToTop();
  this->ColorTransferFunctionEditor->SetBalloonHelpString(
    "Another color transfer function editor. The point position is now on "
    "top, the point style is an arrow down, guidelines are shown for each "
    "point (useful when combined with an histogram), point indices are "
    "hidden, ticks are displayed in the parameter space, the label "
    "and the parameter range are on top, its width is set explicitly. "
    "The range and histogram are based on a real image data.");
  
  this->ColorTransferFunctionEditor->SetColorTransferFunction(this->TransferFunction);
  this->ColorTransferFunctionEditor->SetWholeParameterRangeToFunctionRange();
  this->ColorTransferFunctionEditor->SetVisibleParameterRangeToWholeParameterRange();
  
  this->ColorTransferFunctionEditor->SetPointPositionInValueRangeToTop();
  this->ColorTransferFunctionEditor->SetPointStyleToCursorDown();
  this->ColorTransferFunctionEditor->FunctionLineVisibilityOff();
  this->ColorTransferFunctionEditor->PointGuidelineVisibilityOff();
  this->ColorTransferFunctionEditor->PointIndexVisibilityOff();
  this->ColorTransferFunctionEditor->SelectedPointIndexVisibilityOff();
  this->ColorTransferFunctionEditor->MidPointEntryVisibilityOff();
  this->ColorTransferFunctionEditor->SharpnessEntryVisibilityOff();
  this->ColorTransferFunctionEditor->SetLabelPositionToTop();
  
  //this->ColorTransferFunctionEditor->SetHistogram(this->Histogram);
  
  this->ColorTransferFunctionEditor->ParameterTicksVisibilityOn();
  this->ColorTransferFunctionEditor->ComputeValueTicksFromHistogramOn();
  this->ColorTransferFunctionEditor->SetParameterTicksFormat("%-#6.0f");
  
  this->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 20", 
    ColorTransferFunctionEditor->GetWidgetName());
  
  this->TransferFunction->Delete();
  
  this->ColorTransferFunctionEditor->SetHistogramStyleToPolyLine();
  //this->ColorTransferFunctionEditor->SetHistogramColor();
  this->ColorTransferFunctionEditor->SetColorRampPositionToCanvas();
  
  this->ColorTransferFunctionEditor->SetColorRampOutlineStyleToNone();
  
  this->ColorTransferFunctionEditor->SetColorRampHeight(100);
    

  // Override the column sorting behavior by always updating 
}

void vtkKWWindowLevelThresholdEditor::UpdateRangesFromImage()
{
  if (this->ImageData != NULL)
  {   
    this->Histogram->BuildHistogram( this->ImageData->GetPointData()->GetScalars(), 0);
    double *range = this->Histogram->GetRange();
    this->SetWindowLevel(range[0], range[1]);
    this->SetThreshold(range[0], range[1]);

    // avoid crash when Image not set for histogram
    this->ColorTransferFunctionEditor->SetHistogram(this->Histogram);
  }
}
//----------------------------------------------------------------------------
void vtkKWWindowLevelThresholdEditor::UpdateHistogram()
{
  this->TransferFunction->RemoveAllPoints();

  
  this->TransferFunction->SetColorSpaceToRGB();
  this->TransferFunction->AddRGBPoint(this->GetLowerThreshold(), 179, 179, 231);
  //this->TransferFunction->AddRGBPoint((range[0] + range[1]) * 0.5, 0.0, 1.0, 1.0);
  this->TransferFunction->AddRGBPoint(this->GetUpperThreshold(), 179, 179, 231);
}



//----------------------------------------------------------------------------
void vtkKWWindowLevelThresholdEditor::SetCommand(vtkObject *object, const char *method)
{
  this->SetObjectMethodCommand(&this->Command, object, method);
}


//----------------------------------------------------------------------------
void vtkKWWindowLevelThresholdEditor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "ColorTransferFunctionEditor: " << this->ColorTransferFunctionEditor << endl;
}


void vtkKWWindowLevelThresholdEditor::ProcessWindowLevelCommand(double min, double max)
{
  double range[2];
  range[0] = min;
  range[1] = max;
  this->UpdateHistogram();
  this->InvokeEvent(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, range);
}

void vtkKWWindowLevelThresholdEditor::ProcessWindowLevelStartCommand(double min, double max)
{
  double range[2];
  range[0] = min;
  range[1] = max;
  this->UpdateHistogram();
  this->InvokeEvent(vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent, range);
}

void vtkKWWindowLevelThresholdEditor::ProcessThresholdCommand(double min, double max)
{
  double range[2];
  range[0] = min;
  range[1] = max;
  this->UpdateHistogram();
  this->InvokeEvent(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, range);
}

void vtkKWWindowLevelThresholdEditor::ProcessThresholdStartCommand(double min, double max)
{
  double range[2];
  range[0] = min;
  range[1] = max;
  this->UpdateHistogram();
  this->InvokeEvent(vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent, range);
}
