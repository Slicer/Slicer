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

#include "vtkKWMenuButton.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWCheckButton.h"
#include "vtkKWEntry.h"

#define MIN_RESOLUTION 0.00001

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

  this->WindowLevelAutoManual = vtkKWMenuButtonWithLabel::New() ;
  this->TresholdAutoManual = vtkKWMenuButtonWithLabel::New();
  this->TresholdApply = vtkKWCheckButtonWithLabel::New();

  this->WindowLevelRange = vtkKWRange::New();
  this->LevelEntry = vtkKWEntry::New();
  this->WindowEntry = vtkKWEntry::New();
 
  this->ThresholdRange = vtkKWRange::New();
  this->ColorTransferFunctionEditor = vtkKWColorTransferFunctionEditor::New();   
  this->Histogram = vtkKWHistogram::New();
  this->TransferFunction = vtkColorTransferFunction::New();
  this->ColorTransferFunctionEditor->SetColorTransferFunction(this->TransferFunction);
  this->TransferFunction->Delete();
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

  if ( this->LevelEntry ) {
      this->LevelEntry->Delete();
      this->LevelEntry = NULL;
  }
  if ( this->WindowEntry ) {
      this->WindowEntry->Delete();
      this->WindowEntry = NULL;
  }
  if ( this->WindowLevelAutoManual ) {
      this->WindowLevelAutoManual->Delete();
      this->WindowLevelAutoManual = NULL ;
  }
  if ( this->TresholdAutoManual ) {
      this->TresholdAutoManual->Delete();
      this->TresholdAutoManual = NULL;
  }
  if ( this->TresholdApply ) {
      this->TresholdApply->Delete();
      this->TresholdApply = NULL;
  }
  if ( this->WindowLevelRange ) {
      this->WindowLevelRange->Delete();
      this->WindowLevelRange = NULL;
  }
  if ( this->ThresholdRange ) {
      this->ThresholdRange->Delete();
      this->ThresholdRange = NULL;
  }
  if ( this->Histogram ) {
      this->Histogram->Delete();
      this->Histogram = NULL;
  }
  if ( this->ColorTransferFunctionEditor) {
      this->ColorTransferFunctionEditor->Delete();
      this->ColorTransferFunctionEditor = NULL;
  }

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
      
    this->UpdateFromImage();

    this->UpdateTransferFunction();

    this->Modified();   
    }
}


void vtkKWWindowLevelThresholdEditor::SetWindowLevel(double window, double level)
{
  
  this->WindowLevelRange->SetRange(level - 0.5*window, level + 0.5*window);
  this->WindowEntry->SetValueAsDouble(window);
  this->LevelEntry->SetValueAsDouble(level);
  this->UpdateTransferFunction();
}

double vtkKWWindowLevelThresholdEditor::GetWindow()
{
  //double *range = this->WindowLevelRange->GetRange();
  //return range[1] -  range[0];
  return this->WindowEntry->GetValueAsDouble();
}

double vtkKWWindowLevelThresholdEditor::GetLevel()
{
  //double *range = this->WindowLevelRange->GetRange();
  //return 0.5 * (range[1] +  range[0]);
  return this->LevelEntry->GetValueAsDouble();
}

 
void vtkKWWindowLevelThresholdEditor::SetThreshold(double lower, double upper)
{
  this->ThresholdRange->SetRange(lower, upper);
  this->UpdateTransferFunction();
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
  
  this->UpdateTransferFunction();  

  vtkKWFrame *winLevelFrame = vtkKWFrame::New ( );
  winLevelFrame->SetParent (this);
  winLevelFrame->Create();
  this->Script(
    "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2", 
    winLevelFrame->GetWidgetName());

  this->WindowLevelAutoManual->SetParent(winLevelFrame);
  this->WindowLevelAutoManual->Create();

  this->WindowLevelAutoManual->SetLabelWidth(12);
  this->WindowLevelAutoManual->SetLabelText("Window/Level:");
  this->WindowLevelAutoManual->GetWidget()->GetMenu()->AddRadioButton ( "Auto");
  this->WindowLevelAutoManual->GetWidget()->GetMenu()->AddRadioButton ( "Manual");
  this->WindowLevelAutoManual->GetLabel()->SetJustificationToRight();
  this->WindowLevelAutoManual->GetWidget()->SetValue ( "Manual" );
  this->WindowLevelAutoManual->GetWidget()->GetMenu()->SetItemCommand(0, this, "ProcessButtonsCommand");
  this->WindowLevelAutoManual->GetWidget()->GetMenu()->SetItemCommand(1, this, "ProcessButtonsCommand");
  this->WindowLevelAutoManual->GetWidget()->SetWidth ( 7 );
  this->Script(
    "pack %s -side left -anchor nw -expand n -fill x -padx 2 -pady 2", 
    this->WindowLevelAutoManual->GetWidgetName());

  this->WindowLevelRange->SetParent(winLevelFrame);
  this->WindowLevelRange->Create();
  this->WindowLevelRange->SymmetricalInteractionOn();
  this->WindowLevelRange->EntriesVisibilityOff ();
  this->WindowLevelRange->SetCommand(this, "ProcessWindowLevelCommand");
  this->WindowLevelRange->SetStartCommand(this, "ProcessWindowLevelStartCommand");
  this->Script(
    "pack %s -side left -anchor nw -expand yes -fill x -padx 2 -pady 2", 
    this->WindowLevelRange->GetWidgetName());

  // Window/Level entries
  this->WindowEntry->SetParent(this->WindowLevelRange);
  this->WindowEntry->Create();
  this->WindowEntry->SetWidth(10);
  this->WindowEntry->SetCommand(this, "ProcessWindowEntryCommand");
  this->Script("grid %s -row 0 -column 3 -sticky w",
               this->WindowEntry->GetWidgetName() );

  this->LevelEntry->SetParent(this->WindowLevelRange);
  this->LevelEntry->Create();
  this->LevelEntry->SetWidth(10);
  this->LevelEntry->SetCommand(this, "ProcessLevelEntryCommand");
  this->Script("grid %s -row 0 -column 5 -sticky e",
               this->LevelEntry->GetWidgetName() );

  vtkKWFrame *threshFrame = vtkKWFrame::New ( );
  threshFrame->SetParent (this);
  threshFrame->Create();
  this->Script(
    "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2", 
    threshFrame->GetWidgetName());

  this->TresholdAutoManual->SetParent(threshFrame);
  this->TresholdAutoManual->Create();
  this->TresholdAutoManual->SetLabelWidth(12);
  this->TresholdAutoManual->GetLabel()->SetJustificationToRight();
  this->TresholdAutoManual->GetWidget()->SetWidth ( 7 );
  this->TresholdAutoManual->SetLabelText("Threshold:");
  this->TresholdAutoManual->GetWidget()->GetMenu()->AddRadioButton ( "Auto"); 
  this->TresholdAutoManual->GetWidget()->GetMenu()->AddRadioButton ( "Manual");
  this->TresholdAutoManual->GetWidget()->SetValue ( "Manual" );
  this->TresholdAutoManual->GetWidget()->GetMenu()->SetItemCommand(0, this, "ProcessButtonsCommand");
  this->TresholdAutoManual->GetWidget()->GetMenu()->SetItemCommand(1, this, "ProcessButtonsCommand");
  this->Script(
    "pack %s -side left -anchor nw -expand n -padx 2 -pady 2", 
    this->TresholdAutoManual->GetWidgetName());

  this->ThresholdRange->SetParent(threshFrame);
  this->ThresholdRange->Create();
  this->ThresholdRange->SymmetricalInteractionOff();
  this->ThresholdRange->SetCommand(this, "ProcessThresholdCommand");
  this->ThresholdRange->SetStartCommand(this, "ProcessThresholdStartCommand");
  this->Script(
    "pack %s -side left -anchor w -expand y -fill x -padx 2 -pady 2", 
    this->ThresholdRange->GetWidgetName());

    vtkKWFrame *applyFrame = vtkKWFrame::New ( );
  applyFrame->SetParent(this);
    applyFrame->Create();
    this->Script (
                  "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                  applyFrame->GetWidgetName());
  
    this->TresholdApply->SetParent(applyFrame);
    this->TresholdApply->Create();
    this->TresholdApply->SetLabelText("Apply");
    this->TresholdApply->GetWidget()->SetCommand(this, "ProcessCheckButtonCommand");
    this->Script(
      "pack %s -side right -anchor n -fill x -expand n -padx 2 -pady 2", 
      this->TresholdApply->GetWidgetName());  


  this->ColorTransferFunctionEditor->SetParent(this);
  this->ColorTransferFunctionEditor->Create();
  this->ColorTransferFunctionEditor->ExpandCanvasWidthOn();
  this->ColorTransferFunctionEditor->SetCanvasWidth(300);
  this->ColorTransferFunctionEditor->SetCanvasHeight(126);
  this->ColorTransferFunctionEditor->LabelVisibilityOff ();
  this->ColorTransferFunctionEditor->SetBalloonHelpString(
    "Another color transfer function editor. The point position is now on "
    "top, the point style is an arrow down, guidelines are shown for each "
    "point (useful when combined with an histogram), point indices are "
    "hidden, ticks are displayed in the parameter space, the label "
    "and the parameter range are on top, its width is set explicitly. "
    "The range and histogram are based on a real image data.");
  this->ColorTransferFunctionEditor->SetWholeParameterRangeToFunctionRange();
  this->ColorTransferFunctionEditor->SetVisibleParameterRangeToWholeParameterRange();
  
  this->ColorTransferFunctionEditor->SetPointPositionInValueRangeToTop();
  this->ColorTransferFunctionEditor->SetPointStyleToCursorDown();
  this->ColorTransferFunctionEditor->FunctionLineVisibilityOff();
  this->ColorTransferFunctionEditor->PointGuidelineVisibilityOff();
  // This will disable mobing points on the editor
  this->ColorTransferFunctionEditor->PointVisibilityOff(); 
  this->ColorTransferFunctionEditor->PointIndexVisibilityOff();
  this->ColorTransferFunctionEditor->SelectedPointIndexVisibilityOff();
  this->ColorTransferFunctionEditor->MidPointEntryVisibilityOff();
  this->ColorTransferFunctionEditor->SharpnessEntryVisibilityOff();
  
  this->ColorTransferFunctionEditor->ParameterTicksVisibilityOff();
  this->ColorTransferFunctionEditor->ComputeValueTicksFromHistogramOff();
  this->ColorTransferFunctionEditor->SetDisableAddAndRemove(1);
  this->ColorTransferFunctionEditor->SetColorSpaceOptionMenuVisibility(0);

  this->Script(
    "pack %s -side bottom -anchor nw -expand y -fill x -padx 2 -pady 2", 
    this->ColorTransferFunctionEditor->GetWidgetName());
  
  
  this->ColorTransferFunctionEditor->SetHistogramStyleToPolyLine();
  this->ColorTransferFunctionEditor->SetHistogramColor(1.0, 0., 0.);
  this->ColorTransferFunctionEditor->SetHistogramPolyLineWidth (2);

  this->ColorTransferFunctionEditor->SetColorRampPositionToCanvas();
  
  this->ColorTransferFunctionEditor->SetColorRampOutlineStyleToNone();
  
  this->ColorTransferFunctionEditor->SetColorRampHeight(
    this->ColorTransferFunctionEditor->GetCanvasHeight() - 2);
  this->ColorTransferFunctionEditor->SetPointMarginToCanvasToNone();
  
  this->ThresholdRange->SetWholeRange(0, 255);
  this->WindowLevelRange->SetWholeRange(0, 255);
  this->SetWindowLevel(100, 100);
  this->SetThreshold(0, 255);

  this->UpdateTransferFunction();

  // Override the column sorting behavior by always updating

   // clean up...
  winLevelFrame->Delete ( );
  threshFrame->Delete ( );
  applyFrame->Delete();

}

void vtkKWWindowLevelThresholdEditor::UpdateFromImage()
{
  if (this->ImageData != NULL)
  {   

    this->Histogram->BuildHistogram( this->ImageData->GetPointData()->GetScalars(), 0);
    double *range = this->Histogram->GetRange();

    this->ThresholdRange->SetWholeRange(range[0], range[1]);
    this->WindowLevelRange->SetWholeRange(range[0] - 0.5 * (range[0] + range[1]), range[1] + 0.5 * (range[0] + range[1]));

    this->SetWindowLevel(range[1] - range[0], 0.5 * (range[0] + range[1]) );
    this->SetThreshold(range[0], range[1]);

    // avoid crash when Image not set for histogram
    this->ColorTransferFunctionEditor->SetHistogram(NULL);
    this->ColorTransferFunctionEditor->SetHistogram(this->Histogram);
  }
}
//----------------------------------------------------------------------------
void vtkKWWindowLevelThresholdEditor::UpdateTransferFunction()
{
  this->TransferFunction->RemoveAllPoints();

  double range[2] = {0,255};
  if (this->ImageData)
  {
    this->ImageData->GetScalarRange(range);
    this->TransferFunction->AdjustRange(range);
  }
  
  this->TransferFunction->SetColorSpaceToRGB();
  this->TransferFunction->AddRGBPoint(range[0], 0, 0, 0);
  //this->TransferFunction->AddRGBPoint(this->GetLowerThreshold(), 179.0/255, 179.0/255, 231.0/255);
  this->TransferFunction->AddRGBPoint(this->GetLowerThreshold(), 0, 0, 0);
  //this->TransferFunction->AddRGBPoint((range[0] + range[1]) * 0.5, 0.0, 1.0, 1.0);
  //this->TransferFunction->AddRGBPoint(this->GetUpperThreshold(), 179.0/255, 179.0/255, 231.0/255);
  this->TransferFunction->AddRGBPoint(this->GetUpperThreshold(), 1, 1, 1);
  this->TransferFunction->AddRGBPoint(range[1], 1, 1, 1);
  this->TransferFunction->SetAlpha(1.0);
  this->TransferFunction->Build();
  this->ColorTransferFunctionEditor->Update();
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
  double window = max-min;
  double level = 0.5*(min+max);
  if (fabs(window - this->GetWindow() ) < MIN_RESOLUTION &&
      fabs(level - this->GetLevel() ) < MIN_RESOLUTION)
    {
    return;
    }
  this->WindowEntry->SetValueAsDouble(window);
  this->LevelEntry->SetValueAsDouble(level);
  this->UpdateTransferFunction();
  this->InvokeEvent(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, range);
}

void vtkKWWindowLevelThresholdEditor::ProcessWindowLevelStartCommand(double min, double max)
{
  double range[2];
  range[0] = min;
  range[1] = max;
  this->WindowEntry->SetValueAsDouble(max-min);
  this->LevelEntry->SetValueAsDouble(0.5*(min+max));
  this->UpdateTransferFunction();
  this->InvokeEvent(vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent, range);
}

void vtkKWWindowLevelThresholdEditor::ProcessThresholdCommand(double min, double max)
{
  double range[2];
  range[0] = min;
  range[1] = max;
  this->UpdateTransferFunction();
  this->InvokeEvent(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, range);
}

void vtkKWWindowLevelThresholdEditor::ProcessThresholdStartCommand(double min, double max)
{
  double range[2];
  range[0] = min;
  range[1] = max;
  this->UpdateTransferFunction();
  this->InvokeEvent(vtkKWWindowLevelThresholdEditor::ValueStartChangingEvent, range);
}

void vtkKWWindowLevelThresholdEditor::ProcessWindowEntryCommand(double window)
{
  double *wrange = this->WindowLevelRange->GetRange();

  if (fabs(window - (wrange[1]-wrange[0]) ) < MIN_RESOLUTION)
    {
    return;
    }
  double range[2];
  double level = this->GetLevel();
  range[0] = window - 0.5*level;
  range[1] = window + 0.5*level;
   
  this->WindowLevelRange->SetRange(level - 0.5*window, level + 0.5*window);
  this->UpdateTransferFunction();
  this->InvokeEvent(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, range);
}

void vtkKWWindowLevelThresholdEditor::ProcessLevelEntryCommand(double level)
{
  double *wrange = this->WindowLevelRange->GetRange();
  if (fabs(level - 0.5*(wrange[1]+wrange[0]) ) < MIN_RESOLUTION)
    {
    return;
    }
  double range[2];
  double window = this->GetWindow();
  range[0] = window - 0.5*level;
  range[1] = window + 0.5*level;
   
  this->WindowLevelRange->SetRange(level - 0.5*window, level + 0.5*window);
  this->UpdateTransferFunction();
  this->InvokeEvent(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, range);
}

int vtkKWWindowLevelThresholdEditor::GetAutoWindowLevel()
{
  if (strcmp(this->WindowLevelAutoManual->GetWidget()->GetValue(), "Auto"))
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

void vtkKWWindowLevelThresholdEditor::SetAutoWindowLevel(int value)
{
  if (value == 1)
  {
  this->WindowLevelAutoManual->GetWidget()->SetValue("Auto");
  }
  else if (value == 0)
  {
  this->WindowLevelAutoManual->GetWidget()->SetValue("Manual");
  }
}

int vtkKWWindowLevelThresholdEditor::GetAutoThreshold()
{
  if (strcmp(this->TresholdAutoManual->GetWidget()->GetValue(), "Auto"))
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

void vtkKWWindowLevelThresholdEditor::SetAutoThreshold(int value)
{
  if (value == 1)
  {
  this->TresholdAutoManual->GetWidget()->SetValue("Auto");
  }
  else if (value == 0)
  {
  this->TresholdAutoManual->GetWidget()->SetValue("Manual");
  }
}

int vtkKWWindowLevelThresholdEditor::GetApplyThreshold()
{
  return this->TresholdApply->GetWidget()->GetState();
}

void vtkKWWindowLevelThresholdEditor::SetApplyThreshold(int value)
{
  this->TresholdApply->GetWidget()->SetState(value);
}

void vtkKWWindowLevelThresholdEditor::ProcessButtonsCommand()
{
  this->InvokeEvent(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, NULL);
}

void vtkKWWindowLevelThresholdEditor::ProcessCheckButtonCommand(int state)
{
  this->InvokeEvent(vtkKWWindowLevelThresholdEditor::ValueChangedEvent, NULL);
}
