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

#include "vtkMRMLModelNode.h"

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
  // get the subjects dir from the qdec project
//BTX
  std::string GetSubjectsDirectory();

  // Description:
  // get the average subject file path from the qdec project
  std::string GetAverageSubject();
//ETX

  // Description:
  // Set up the GLM design on the Qdec project
  // Returns 0 on success
  int CreateGlmDesign(const char *name, const char *discreteFactor1, const char *discreteFactor2, const char *continuousFactor1, const char *continuousFactor2, const char* measure, const char* hemisphere, int smoothness);

  // Description:
  // Load up the already computed GLM output from a Qdec project file, a
  // .tar.gz that's been renamed .qdec
  // Returns 0 on success
  int LoadProjectFile(const char *fileName, const char *tempDir);
  
  // Description:
  // Run the GLM fit, pass through it's return, 0 is success
  int RunGlmFit();

  // Description:
  // Try to load the results of the GLM fit run
  // return 0 on success
  int LoadResults(vtkSlicerModelsLogic *modelsLogic, vtkKWApplication *app);

  // Description:
  // get/set the name of the tcl script that holds the plotting commands
  vtkGetStringMacro(PlotTclScript);
  vtkSetStringMacro(PlotTclScript);

  // Description:
  // have we loaded the plot tcl script?
  vtkGetMacro(TclScriptLoaded, int);
  vtkSetMacro(TclScriptLoaded, int);

  // Description:
  // number of questions that the loaded results answer
  vtkGetMacro(NumberOfQuestions, int);
  vtkSetMacro(NumberOfQuestions, int);

  // Description:
  // return one of the question strings, returns an empty string if not set
  // num is used as the index into the ContrastQuestions vector, is checked
  // against that size
  //BTX
  std::string GetQuestion(int num);
  // Description:
  // return the scalar array name associated with this question
  std::string GetQuestionScalarName(const char * question);
  //ETX

  vtkGetObjectMacro ( ModelNode, vtkMRMLModelNode );
  void SetMRMLNode ( vtkMRMLModelNode *node )
  { vtkSetMRMLNodeMacro ( this->ModelNode, node ); }
  void SetAndObserveMRMLNode ( vtkMRMLModelNode *node )
        { vtkSetAndObserveMRMLNodeMacro (this->ModelNode, node ); }


protected:
  vtkQdecModuleLogic();
  ~vtkQdecModuleLogic();
  vtkQdecModuleLogic(const vtkQdecModuleLogic&);
  void operator=(const vtkQdecModuleLogic&);

  // Description:
  // the name of the tcl script with the plotting commands
  char * PlotTclScript;

  int TclScriptLoaded;

  int NumberOfQuestions;

  //BTX
  std::map < std::string, std::string > QuestionScalars;
  //ETX

  // Description:
  // save the model node that last loaded the scalars onto
  vtkMRMLModelNode *ModelNode;
  
};

#endif
