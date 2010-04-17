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

#include "vtkVolumes.h"
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
#include "vtkKWCheckButton.h"

#include "vtkKWColorTransferFunctionEditor.h"
#include "vtkKWHistogram.h"
#include "vtkImageExtractComponents.h"

#include "vtkSlicerBaseGUI.h"

class vtkKWWindowLevelThresholdEditorInternals;
class vtkKWPushButtonSetWithLabel;
class vtkSlicerVolumesIcons;
class VTK_VOLUMES_EXPORT vtkKWWindowLevelThresholdEditor : public vtkKWCompositeWidget
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
  // icons
  vtkGetObjectMacro(WindowLevelPresetIcons, vtkSlicerVolumesIcons);

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

  vtkGetMacro(ProcessCallbacks, int);
  vtkSetMacro(ProcessCallbacks, int);
  vtkBooleanMacro(ProcessCallbacks, int);

  // Description: 
  // The number of changes between the transfer function updates
  vtkGetMacro(UpdateTransferFunctionPeriod, int);
  vtkSetMacro(UpdateTransferFunctionPeriod, int);

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
  void ProcessWindowLevelEndCommand(double min, double max);

  void ProcessThresholdStartCommand(double min, double max);
  void ProcessThresholdEndCommand(double min, double max);

  // Description:
  // deal with changes from the window level preset menu
  void ProcessWindowLevelPresetsMenuCommand(char *winLevel);

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

  vtkGetMacro(PresetSize, int);
  virtual void SetPresetSize(int);

  // Description:
  // call back from the preset buttons
  void PresetWindowLevelCallback(int rank);

  // Description:
  // clear out the preset menu
  void ClearWindowLevelPresetsMenu();

  // Description:
  // Add a preset to the menu
  void AddDisplayVolumePreset(double win, double level);
  
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

  int ProcessCallbacks;

  int UpdateTransferFunctionPeriod;
  int UpdateTransferFunctionCount;

  unsigned long ImageModifiedTime;

  // Presets for Window/Level
  //BTX
  class Preset
  {
  public:
    double Window;
    double Level;
    char *HelpString;
    char *ColorTableNodeID;
    vtkKWIcon *Icon;
    // use the Icon String if no icon yet
    char *IconString;
    Preset() { this->HelpString = 0; this->ColorTableNodeID = 0; this->Icon = NULL; this->IconString = 0;};
  };
  // PIMPL Encapsulation for STL containers

  vtkKWWindowLevelThresholdEditorInternals *Internals;
  friend class vtkKWWindowLevelThresholdEditorInternals;

  //ETX

  // Description:
  // Add default presets
  virtual void AddDefaultPresets();
  // Description:
  // Create the presets
  virtual void CreatePresets();
  // Description:
  // Update widgets from the preset
  int UpdateWindowLevelFromPreset(const Preset *preset);
  
private:
  vtkKWWindowLevelThresholdEditor(const vtkKWWindowLevelThresholdEditor&); // Not implemented
  void operator=(const vtkKWWindowLevelThresholdEditor&); // Not implemented
  
  vtkKWMenuButtonWithLabel *WindowLevelAutoManual;
  vtkKWMenuButtonWithLabel *ThresholdAutoManual;
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
  // this is a set of defaults
  vtkKWPushButtonSetWithLabel *WindowLevelPresetsButtonSet;
  int PresetSize;
  // this is set from the display node
  vtkKWMenuButtonWithLabel *WindowLevelPresetsMenu;
  vtkKWRange *ThresholdRange;
  vtkKWCheckButton *UpdateTransferFunctionButton;

  vtkSlicerVolumesIcons *WindowLevelPresetIcons;
};

#endif

