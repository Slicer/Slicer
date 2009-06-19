/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRML4DBundleNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkCurveAnalysisPythonInterface_h
#define __vtkCurveAnalysisPythonInterface_h


#include "vtkObject.h"
#include "vtkFourDAnalysisWin32Header.h"

#include "vtkMRMLCurveAnalysisNode.h"

class VTK_FourDAnalysis_EXPORT vtkCurveAnalysisPythonInterface : public vtkObject
{
 public:

  static vtkCurveAnalysisPythonInterface *New();
  vtkTypeRevisionMacro(vtkCurveAnalysisPythonInterface, vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  int SetScript(const char* filename);
  int GetInfo(vtkMRMLCurveAnalysisNode* curveNode);
  int Run(vtkMRMLCurveAnalysisNode* curveNode);

 protected:
  vtkCurveAnalysisPythonInterface();
  virtual ~vtkCurveAnalysisPythonInterface();

 private:

  //BTX
  std::string ScriptName;
  //ETX

};


#endif //__vtkCurveAnalysisPythonInterface_h
