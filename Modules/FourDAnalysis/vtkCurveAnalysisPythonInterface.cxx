/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

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
#include "vtkMRMLTimeSeriesBundleNode.h"
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
  this->CurveAnalysisNode = NULL;

#ifdef Slicer3_USE_PYTHON
  this->CompiledObject = NULL;
#endif // Slicer3_USE_PYTHON

}


//---------------------------------------------------------------------------
vtkCurveAnalysisPythonInterface::~vtkCurveAnalysisPythonInterface()
{
  if (this->CompiledObject)
    {
    //free(this->CompiledObject);
    Py_DECREF(this->CompiledObject);
    }
}


//---------------------------------------------------------------------------
void vtkCurveAnalysisPythonInterface::PrintSelf(ostream& os, vtkIndent indent)
{
}


//---------------------------------------------------------------------------
int vtkCurveAnalysisPythonInterface::SetScript(const char* script)
{
  this->ScriptName = script;

  return 1;
}


//---------------------------------------------------------------------------
int vtkCurveAnalysisPythonInterface::SetCurveAnalysisNode(vtkMRMLCurveAnalysisNode* curveNode)
{
  
  if (curveNode && curveNode->GetID())
    {
    this->CurveAnalysisNode = curveNode;
    GenerateFittingScript();
    return 1;
    }
  else
    {
    this->CurveAnalysisNode = NULL;
    return 0;
    }

}


//---------------------------------------------------------------------------
int vtkCurveAnalysisPythonInterface::SetCurveAnalysisNode(std::vector<vtkMRMLCurveAnalysisNode*> curveNodeVector)
{
  if (curveNodeVector.size() > 0)
    {
    this->CurveAnalysisNodeVector.clear();
    std::vector<vtkMRMLCurveAnalysisNode*>::iterator iter;
    for (iter = curveNodeVector.begin(); iter != curveNodeVector.end(); iter ++)
      {
      if ((*iter) != NULL && (*iter)->GetID())
        {
        this->CurveAnalysisNodeVector.push_back((*iter));
        }
      }
    GenerateFittingScript();
    return 1;
    }
  else
    {
    this->CurveAnalysisNodeVector.clear();
    return 0;
    }
}


//---------------------------------------------------------------------------
int vtkCurveAnalysisPythonInterface::GetInfo()
{
  //NOTE: this function creates vtkMRMLCurveAnalysisNode and obtain necessary parameters
  // (e.g. list of input curves and initial parameters) for the curve fitting.
  // This fucntion should be called after the node is registered to the MRML scene.

  if (this->CurveAnalysisNode == NULL)
    {
    return 0;
    }

#ifdef Slicer3_USE_PYTHON
  PyObject* v;
  std::string pythonCmd;

  // clear curve analysis node
  this->CurveAnalysisNode->ClearInputArrays();
  this->CurveAnalysisNode->ClearInitialParameters();
  this->CurveAnalysisNode->ClearConstants();
  this->CurveAnalysisNode->ClearOutputValues();

  // Obtain MRML CurveAnalysis Node instance
  pythonCmd += "from Slicer import slicer\n";
  pythonCmd += "scene = slicer.MRMLScene\n";
  pythonCmd += "curveNode  = scene.GetNodeByID('";
  pythonCmd += this->CurveAnalysisNode->GetID();
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
  pythonCmd += "curveNames          = caexec.GetInputCurveNames()\n";
  pythonCmd += "initialOptimParams  = caexec.GetInitialParameters()\n";
  pythonCmd += "inputParameterNames = caexec.GetConstantNames()\n";
  pythonCmd += "outputValueNames    = caexec.GetOutputParameterNames()\n";
  pythonCmd += "methodName          = caexec.GetMethodName()\n";
  pythonCmd += "methodDescription   = caexec.GetMethodDescription()\n";

  // Set lists
  pythonCmd += "for key in curveNames:\n";
  pythonCmd += "    curveNode.AddInputArrayName(key)\n";
  pythonCmd += "for key, value in initialOptimParams.iteritems():\n";
  pythonCmd += "    curveNode.SetInitialParameter(key, value)\n";
  pythonCmd += "for key in inputParameterNames:\n";
  pythonCmd += "    curveNode.SetConstant(key, 0.0)\n";
  pythonCmd += "for key in outputValueNames:\n";
  pythonCmd += "    curveNode.SetOutputValue(key, 0.0)\n";
  pythonCmd += "curveNode.SetMethodName(methodName)\n";
  pythonCmd += "curveNode.SetMethodDescription(methodDescription)\n";

  v = PyRun_String(pythonCmd.c_str(),
                   Py_file_input,
                   (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()),
                   (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()));
  
  if (Py_FlushLine())
    {
      PyErr_Clear();
    }

  return 1;

#else // Slicer3_USE_PYTHON

  // always retuns 0 if Python is not available.
  return 0;

#endif // Slicer3_USE_PYTHON



}


//---------------------------------------------------------------------------
int vtkCurveAnalysisPythonInterface::Run()
{

  if (this->CurveAnalysisNode == NULL)
    {
    return 0;
    }

#ifdef Slicer3_USE_PYTHON

  if (this->CompiledObject) // if compiled object exists
    {
    PyObject* dum;
    dum = PyEval_EvalCode ((PyCodeObject *)this->CompiledObject,
                           (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()),
                           (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()));

    if (PyErr_Occurred ())
      {
      PyErr_Print ();
      }

    if (dum != NULL)
      {
      Py_DECREF(dum);
      }

    return 1;
  
    }
  else if (this->PythonCmd != "")
    {
    std::cerr << "-------- Text script is used." << std::endl;

    PyObject* v;
    v = PyRun_String(this->PythonCmd.c_str(),
                     Py_file_input,
                     (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()),
                     (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()));
    
    if (Py_FlushLine())
      {
      PyErr_Clear();
      }
    
    if (v)
      {
      Py_DECREF(v);
      }
    return 1;
    }
  else
    {
    return 0;
    }
    
#else // Slicer3_USE_PYTHON

  // always retuns 0 if Python is not available.
  return 0;

#endif // Slicer3_USE_PYTHON

}


//---------------------------------------------------------------------------
int vtkCurveAnalysisPythonInterface::GenerateFittingScript()
{

#ifdef Slicer3_USE_PYTHON

  if (this->CompiledObject)
    {
    //free(this->CompiledObject);
    Py_DECREF(this->CompiledObject);
    }

  std::string pythonCmd;

  // Obtain MRML CurveAnalysis Node instance
  pythonCmd += "from Slicer import slicer\n";
  pythonCmd += "scene = slicer.MRMLScene\n";
  pythonCmd += "curveNode  = scene.GetNodeByID('";
  pythonCmd += this->CurveAnalysisNode->GetID();
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
  pythonCmd += "targetCurve = curveNode.GetTargetCurve().ToArray()\n";
  pythonCmd += "outputCurve = curveNode.GetFittedCurve().ToArray()\n";
  pythonCmd += "caexec = fda.CurveAnalysisExecuter('";
  pythonCmd += this->ScriptName.c_str();
  pythonCmd += "')\n";

  // Get lists of input curves, initial parameters etc.
  pythonCmd += "curveNames           = caexec.GetInputCurveNames()\n";
  pythonCmd += "initialOptimParams   = caexec.GetInitialParameters()\n";
  pythonCmd += "inputParameterNames  = caexec.GetConstantNames()\n";
  pythonCmd += "outputParameterNames = caexec.GetOutputParameterNames()\n";
  
  // Set lists
  pythonCmd += "initialParamDict = {}\n";
  pythonCmd += "inputParamDict = {}\n";
  pythonCmd += "inputCurveDict = {}\n";
  pythonCmd += "for key, value in initialOptimParams.iteritems():\n";
  pythonCmd += "    initialParamDict[key] = curveNode.GetInitialParameter(key)\n";
  pythonCmd += "for key in inputParameterNames:\n";
  pythonCmd += "    inputParamDict[key] = curveNode.GetConstant(key)\n";
  pythonCmd += "for key in curveNames:\n";
  pythonCmd += "    inputCurveDict[key]   = curveNode.GetInputArray(key).ToArray()\n";

  // Run curve fitting
  pythonCmd += "result = caexec.Execute(inputCurveDict, initialParamDict, inputParamDict, targetCurve, outputCurve)\n";

  // Get results
  pythonCmd += "for key, value in result.iteritems():\n";
  pythonCmd += "    curveNode.SetOutputValue(key, value)\n";

  this->PythonCmd = pythonCmd;

#ifdef Slicer3_USE_PYTHON
  this->CompiledObject = NULL;
#endif // Slicer3_USE_PYTHON

  this->CompiledObject = Py_CompileString (this->PythonCmd.c_str(), "<stdin>", Py_file_input);
  // NOTE: Py_file_input or Py_single_input???

  if (this->CompiledObject)
    {
    // compiled successfully
    return 1;
    }
  else if (PyErr_ExceptionMatches (PyExc_SyntaxError))           
    {
    char *msg = NULL;
    PyObject *exc, *val, *trb, *obj, *dum;
 
    PyErr_Fetch (&exc, &val, &trb);        /* clears exception! */
    
    if (PyArg_ParseTuple (val, "sO", &msg, &obj) &&
        !strcmp (msg, "unexpected EOF while parsing")) /* E_EOF */
      {
      Py_XDECREF (exc);
      Py_XDECREF (val);
      Py_XDECREF (trb);
      }
    else                                   /* some other syntax error */
      {
      PyErr_Restore (exc, val, trb);
      PyErr_Print ();
      }
    return 0;
    }
  else
    {
    PyErr_Print ();
    }
    
#else
  return 0;
#endif // Slicer3_USE_PYTHON

}

