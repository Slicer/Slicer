/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkProstateNavManualControlStep_h
#define __vtkProstateNavManualControlStep_h

#include "vtkProstateNavStep.h"

class vtkKWFrame;
class vtkKWScaleWithEntry;

class VTK_PROSTATENAV_EXPORT vtkProstateNavManualControlStep : public vtkProstateNavStep
{
public:
  static vtkProstateNavManualControlStep *New();
  vtkTypeRevisionMacro(vtkProstateNavManualControlStep,vtkProstateNavStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);

protected:
  vtkProstateNavManualControlStep();
  ~vtkProstateNavManualControlStep();

  // GUI Widgets
  vtkKWFrame *MainFrame;
  vtkKWFrame *ControlFrame;

  vtkKWScaleWithEntry* PRScale;
  vtkKWScaleWithEntry* PAScale;
  vtkKWScaleWithEntry* PSScale;
  vtkKWScaleWithEntry* NRScale;
  vtkKWScaleWithEntry* NAScale;
  vtkKWScaleWithEntry* NSScale;

private:
  vtkProstateNavManualControlStep(const vtkProstateNavManualControlStep&);
  void operator=(const vtkProstateNavManualControlStep&);
};

#endif
