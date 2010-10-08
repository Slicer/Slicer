/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkCurveAnalysisPythonInterface.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkCurveAnalysisPythonInterface_h
#define __vtkCurveAnalysisPythonInterface_h


#include "vtkObject.h"
#include "vtkFourDAnalysisWin32Header.h"

#include "vtkMRMLCurveAnalysisNode.h"

#include "vtkSlicerConfigure.h" /* Slicer_USE_* */


#ifdef FourDAnalysis_USE_SCIPY
#include <Python.h>
#endif //FourDAnalysis_USE_SCIPY

class VTK_FourDAnalysis_EXPORT vtkCurveAnalysisPythonInterface : public vtkObject
{
 public:

  static vtkCurveAnalysisPythonInterface *New();
  vtkTypeRevisionMacro(vtkCurveAnalysisPythonInterface, vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  int SetScript(const char* filename);
  const char* GetScript() { return this->ScriptName.c_str(); };

  int SetCurveAnalysisNode(vtkMRMLCurveAnalysisNode* curveNode);
  //BTX
  int SetCurveAnalysisNode(std::vector<vtkMRMLCurveAnalysisNode*> curveNodeVector);
  //ETX
  vtkMRMLCurveAnalysisNode* GetCurveAnalysisNode() { return this->CurveAnalysisNode; };

  int GetInfo();
  int Run();

 protected:
  vtkCurveAnalysisPythonInterface();
  virtual ~vtkCurveAnalysisPythonInterface();

 private:

  int GenerateFittingScript();

  //BTX
  std::string ScriptName;
  std::string PythonCmd;
  //ETX

  vtkMRMLCurveAnalysisNode* CurveAnalysisNode;
  //BTX
  std::vector<vtkMRMLCurveAnalysisNode*> CurveAnalysisNodeVector;
  //ETX

#ifdef FourDAnalysis_USE_SCIPY
  PyObject *CompiledObject;
#endif // Slicer_USE_PYTHON

};


#endif //__vtkCurveAnalysisPythonInterface_h
