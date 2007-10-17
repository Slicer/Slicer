/*=========================================================================

  Module:    $RCSfile: vtkKWWindowLevelThresholdEditor.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkKWWindowLevelThresholdEditor -  multi column list box to display/edit matrix4x4
// .SECTION Description
// vtkKWWindowLevelThresholdEditor is a widget containing widgets that help view and
// edit a matrix
//

#ifndef __vtkKWWindowLevelThresholdEditor_h
#define __vtkKWWindowLevelThresholdEditor_h

#include "vtkKWCompositeWidget.h"

#include "vtkImageData.h"
#include "vtkColorTransferFunction.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkImageAccumulateDiscrete.h"
#include "vtkImageBimodalAnalysis.h"

#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWWindow.h"
#include "vtkKWRange.h"
#include "vtkKWEntry.h"
#include "vtkKWColorTransferFunctionEditor.h"
#include "vtkKWHistogram.h"
#include "vtkImageExtractComponents.h"

#include "vtkSlicerBaseGUI.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkKWWindowLevelThresholdEditor : public vtkKWCompositeWidget
{
public:
  static vtkKWWindowLevelThresholdEditor* New();
  vtkTypeRevisionMacro(vtkKWWindowLevelThresholdEditor,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Associated ImageData
  vtkGetObjectMacro(ImageData, vtkImageData);
  void SetImageData(vtkImageData* ImageData);

  // Description:
  // Get/Set Window
  void SetWindowLevel(double window, double level);
  double GetWindow();
  double GetLevel();


  // Description:
  // Get/Set LowerThreshold
  void SetThreshold(double lower, double upper);
  double GetUpperThreshold();
  double GetLowerThreshold();

  // Description:
  // Get/Set enum Off/Auto/Manual Threshold
  int GetThresholdType();
  void SetThresholdType(int value);

  // Description:
  // Get GetAutoWindowLevel 0/1
  int GetAutoWindowLevel();
  void SetAutoWindowLevel(int value);
  
  // Description:
  // Command to call when the User manipulates the widget
  virtual void SetCommand(vtkObject *object, const char *method);

  // TODO: have special commands for start/end events
  //virtual void SetStartCommand(vtkObject *object, const char *method);
  //virtual void SetEndCommand(vtkObject *object, const char *method);

  // Description:
  // TODO: access internal widgets
  //vtkKWRange* GetXRange() { return this->Range[0]; };
  void ProcessButtonsCommand();

  void ProcessCheckButtonCommand(int state);

  void ProcessWindowEntryCommand(double win);
  void ProcessLevelEntryCommand(double level);

  void ProcessWindowLevelCommand(double min, double max);
  void ProcessThresholdCommand(double min, double max);

  void ProcessWindowLevelStartCommand(double min, double max);
  void ProcessThresholdStartCommand(double min, double max);

//BTX
   enum
    {
      ThresholdOff = 0,
      ThresholdManual,
      ThresholdAuto
    };

    enum
    {
      ValueChangedEvent = 10000,
      ValueStartChangingEvent
    };

//ETX

protected:
  vtkKWWindowLevelThresholdEditor();
  ~vtkKWWindowLevelThresholdEditor();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Update the transfer widget with the current state
  void UpdateTransferFunction();

  // Update the widgets with the current ImageData
  void UpdateFromImage();

  // update threshold and windo/level based on automatic method
  void UpdateAutoLevels();

  char *Command;
  //char *StartCommand;
  //char *EndCommand;


private:
  vtkKWWindowLevelThresholdEditor(const vtkKWWindowLevelThresholdEditor&); // Not implemented
  void operator=(const vtkKWWindowLevelThresholdEditor&); // Not implemented
  
  vtkKWMenuButtonWithLabel *WindowLevelAutoManual;
  vtkKWMenuButtonWithLabel *TresholdAutoManual;
  vtkImageData *ImageData;
  vtkKWHistogram *Histogram;
  vtkColorTransferFunction *TransferFunction;
  vtkKWColorTransferFunctionEditor *ColorTransferFunctionEditor;

  vtkImageAccumulateDiscrete *Accumulate;
  vtkImageBimodalAnalysis *Bimodal;
  vtkImageExtractComponents *ExtractComponents;

  vtkKWRange *WindowLevelRange;
  vtkKWEntry *LevelEntry;
  vtkKWEntry *WindowEntry;
  vtkKWRange *ThresholdRange;
};

#endif

