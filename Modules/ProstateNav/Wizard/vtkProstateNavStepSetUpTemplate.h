/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkProstateNavStepSetUpTemplate_h
#define __vtkProstateNavStepSetUpTemplate_h

#include "vtkProstateNavStep.h"
#include "vtkCommand.h"

class vtkKWLoadSaveButtonWithLabel;
class vtkKWFrame;
class vtkKWEntry;
class vtkKWCheckButton;
class vtkKWPushButton;
class vtkKWLabel;
class vtkSlicerNodeSelectorWidget;

class VTK_PROSTATENAV_EXPORT vtkProstateNavStepSetUpTemplate :
  public vtkProstateNavStep
{
public:
  static vtkProstateNavStepSetUpTemplate *New();
  vtkTypeRevisionMacro(vtkProstateNavStepSetUpTemplate,vtkProstateNavStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);  
  
protected:
  vtkProstateNavStepSetUpTemplate();
  ~vtkProstateNavStepSetUpTemplate();

  // GUI Widgets
 
  vtkSlicerNodeSelectorWidget* ScannerConnectorSelector;

  vtkKWFrame *ConnectorFrame;

private:
  vtkProstateNavStepSetUpTemplate(const vtkProstateNavStepSetUpTemplate&);
  void operator=(const vtkProstateNavStepSetUpTemplate&);
};

#endif
