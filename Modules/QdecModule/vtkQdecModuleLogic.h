/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkQdecModuleLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkQdecModuleLogic_h
#define __vtkQdecModuleLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerModelsLogic.h"

#include "vtkQdecModule.h"

#include "QdecProject.h"
#include "vtkKWApplication.h"

class VTK_QDECMODULE_EXPORT vtkQdecModuleLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkQdecModuleLogic *New();
  vtkTypeMacro(vtkQdecModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};

  // Description:
  // Qdec project, encapsulates useful methods
  QdecProject* QDECProject;
  
  // Description:
  // Load in the data table, uses QDECProject method
  int LoadDataTable(const char* fileName);

  // Description:
  // set the subjects directory in the qdec project, using None if null or empty string passed
  void SetSubjectsDirectory(const char *fileName);

  // Description:
  // Set up the GLM design on the Qdec project
  // Returns 0 on success
  int CreateGlmDesign(const char *name, const char *discreteFactor1, const char *discreteFactor2, const char *continuousFactor1, const char *continuousFactor2, const char* measure, const char* hemisphere, int smoothness);

  // Description:
  // Run the GLM fit, pass through it's return, 0 is success
  int RunGlmFit();

  // Description:
  // Try to load the results of the GLM fit run
  // return 0 on success
  int LoadResults(vtkSlicerModelsLogic *modelsLogic, vtkKWApplication *app);

  // Description:
  // Load the plottable results data
  int LoadPlotData(const char *fileName);

  // Description:
  // get/set the name of the tcl script that holds the plotting commands
  vtkGetStringMacro(PlotTclScript);
  vtkSetStringMacro(PlotTclScript);

  // Description:
  // have we loaded the plot tcl script?
  vtkGetMacro(TclScriptLoaded, int);
  vtkSetMacro(TclScriptLoaded, int);

protected:
  vtkQdecModuleLogic();
  ~vtkQdecModuleLogic();
  vtkQdecModuleLogic(const vtkQdecModuleLogic&);
  void operator=(const vtkQdecModuleLogic&);

  // Description:
  // the name of the tcl script with the plotting commands
  char * PlotTclScript;

  int TclScriptLoaded;
};

#endif
