/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile:$
  Date:      $Date:$
  Version:   $Revision:$

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCurveAnalysisPythonInterface.h"

#include "vtkDoubleArray.h"
#include "vtkMRML4DBundleNode.h"
#include "vtkMRMLScalarVolumeNode.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerConfigure.h" /* Slicer3_USE_* */

#ifdef Slicer3_USE_PYTHON
#include <Python.h>
#endif

vtkStandardNewMacro(vtkCurveAnalysisPythonInterface);
vtkCxxRevisionMacro(vtkCurveAnalysisPythonInterface, "$Revision: $");

//---------------------------------------------------------------------------
vtkCurveAnalysisPythonInterface::vtkCurveAnalysisPythonInterface()
{
  this->ScriptName = "";
}


//---------------------------------------------------------------------------
vtkCurveAnalysisPythonInterface::~vtkCurveAnalysisPythonInterface()
{
}


//---------------------------------------------------------------------------
void vtkCurveAnalysisPythonInterface::PrintSelf(ostream& os, vtkIndent indent)
{
}


//---------------------------------------------------------------------------
int vtkCurveAnalysisPythonInterface::SetScript(const char* script)
{
  this->ScriptName = script;
}


//---------------------------------------------------------------------------
int vtkCurveAnalysisPythonInterface::GetInfo(vtkMRMLCurveAnalysisNode* curveNode)
{
  //NOTE: this function creates vtkMRMLCurveAnalysisNode and obtain necessary parameters
  // (e.g. list of input curves and initial parameters) for the curve fitting.

  std::cerr << "int vtkCurveAnalysisPythonInterface::GetInfo(vtkMRMLCurveAnalysisNode* curveNode)" << std::endl;

#ifdef Slicer3_USE_PYTHON
  std::cerr << "int vtkCurveAnalysisPythonInterface::GetInfo(vtkMRMLCurveAnalysisNode* curveNode) -- 2" << std::endl;
  PyObject* v;
  std::string pythonCmd;

  // clear curve analysis node
  curveNode->ClearInputData();
  curveNode->ClearInitialOptimParameters();
  curveNode->ClearParameters();

  // Obtain MRML CurveAnalysis Node instance
  pythonCmd += "from Slicer import slicer\n";
  pythonCmd += "scene = slicer.MRMLScene\n";
  pythonCmd += "curveNode  = scene.GetNodeByID('";
  pythonCmd += curveNode->GetID();
  pythonCmd += "')\n";

  // Load 4D Analysis Python Module
  pythonCmd += "import imp\n";
  pythonCmd += "fp, pathname, description = imp.find_module('FourDAnalysis')\n";
  pythonCmd += "try:\n";
  pythonCmd += "    fda = imp.load_module('FourDAnalysis', fp, pathname, description)\n";
  pythonCmd += "finally:\n";
  pythonCmd += "    if fp:\n";
  pythonCmd += "        fp.close()\n";

  // Get input and output curves from MRML node
  pythonCmd += "caexec      = fda.CurveAnalysisExecuter('";
  pythonCmd += this->ScriptName.c_str();
  pythonCmd += "')\n";

  // Get lists of input curves, initial parameters etc.
  pythonCmd += "curveNames           = caexec.GetInputCurveNames()\n";
  pythonCmd += "initialOptimParams   = caexec.GetInitialOptimParams()\n";
  pythonCmd += "inputParameterNames  = caexec.GetInputParameterNames()\n";
  pythonCmd += "outputParameterNames = caexec.GetOutputParameterNames()\n";

  // Set lists
  pythonCmd += "for key in curveNames:\n";
  pythonCmd += "    curveNode.AddInputDataName(key)\n";
  pythonCmd += "for key, value in initialOptimParams.iteritems():\n";
  pythonCmd += "    curveNode.SetInitialOptimParameter(key, value)\n";
  pythonCmd += "for key in inputParameterNames:\n";
  pythonCmd += "    curveNode.SetInputParameter(key, 0.0)\n";
  pythonCmd += "for key in outputParameterNames:\n";
  pythonCmd += "    curveNode.SetParameter(key, 0.0)\n";

  v = PyRun_String(pythonCmd.c_str(),
                   Py_file_input,
                   (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()),
                   (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()));
  
  if (Py_FlushLine())
    {
      PyErr_Clear();
    }

#endif // Slicer3_USE_PYTHON
}


//---------------------------------------------------------------------------
int vtkCurveAnalysisPythonInterface::Run(vtkMRMLCurveAnalysisNode* curveNode)
{

#ifdef Slicer3_USE_PYTHON

  PyObject* v;
  std::string pythonCmd;

  // Obtain MRML CurveAnalysis Node instance
  pythonCmd += "from Slicer import slicer\n";
  pythonCmd += "scene = slicer.MRMLScene\n";
  pythonCmd += "curveNode  = scene.GetNodeByID('";
  pythonCmd += curveNode->GetID();
  pythonCmd += "')\n";

  // Load 4D Analysis Python Module
  pythonCmd += "import imp\n";
  pythonCmd += "fp, pathname, description = imp.find_module('FourDAnalysis')\n";
  pythonCmd += "try:\n";
  pythonCmd += "    fda = imp.load_module('FourDAnalysis', fp, pathname, description)\n";
  pythonCmd += "finally:\n";
  pythonCmd += "    if fp:\n";
  pythonCmd += "        fp.close()\n";

  // Get input and output curves from MRML node
  pythonCmd += "targetCurve  = curveNode.GetSourceData().ToArray()\n";
  pythonCmd += "outputCurve = curveNode.GetFittedData().ToArray()\n";
  pythonCmd += "caexec = fda.CurveAnalysisExecuter('";
  pythonCmd += this->ScriptName.c_str();
  pythonCmd += "')\n";

  // Get lists of input curves, initial parameters etc.
  pythonCmd += "curveNames           = caexec.GetInputCurveNames()\n";
  pythonCmd += "initialOptimParams   = caexec.GetInitialOptimParams()\n";
  pythonCmd += "inputParameterNames  = caexec.GetInputParameterNames()\n";
  pythonCmd += "outputParameterNames = caexec.GetOutputParameterNames()\n";
  
  // Set lists
  pythonCmd += "initialParamDict = {}\n";
  pythonCmd += "inputParamDict = {}\n";
  pythonCmd += "inputCurveDict = {}\n";
  pythonCmd += "for key, value in initialOptimParams.iteritems():\n";
  pythonCmd += "    initialParamDict[key] = curveNode.GetInitialOptimParameter(key)\n";
  pythonCmd += "for key in inputParameterNames:\n";
  pythonCmd += "    inputParamDict[key] = curveNode.GetInputParameter(key)\n";
  pythonCmd += "for key in curveNames:\n";
  pythonCmd += "    inputCurveDict[key]   = curveNode.GetInputData(key).ToArray()\n";

  // Run curve fitting
  pythonCmd += "result = caexec.Execute(inputCurveDict, initialParamDict, inputParamDict, targetCurve, outputCurve)\n";

  // Get results
  pythonCmd += "for key, value in result.iteritems():\n";
  pythonCmd += "    curveNode.SetParameter(key, value)\n";

  v = PyRun_String(pythonCmd.c_str(),
                   Py_file_input,
                   (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()),
                   (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()));
  
  if (Py_FlushLine())
    {
      PyErr_Clear();
    }

#endif // Slicer3_USE_PYTHON
}
