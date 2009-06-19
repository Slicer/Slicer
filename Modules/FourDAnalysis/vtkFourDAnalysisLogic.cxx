/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.na-mic.org/NAMICSandBox/trunk/IGTLoadableModules/FourDAnalysis/vtkFourDAnalysisLogic.cxx $
  Date:      $Date: 2009-02-03 12:05:00 -0500 (Tue, 03 Feb 2009) $
  Version:   $Revision: 3633 $

==========================================================================*/


#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"

#include "vtkSlicerColorLogic.h"
#include "vtkFourDAnalysisLogic.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkCommandLineModuleLogic.h"
#include "vtkCommandLineModuleGUI.h"

#include "itkMetaDataDictionary.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkNumericSeriesFileNames.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkImageFileWriter.h"
#include "itkTimeProbesCollectorBase.h"

#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLBSplineTransformNode.h"
#include "itksys/DynamicLoader.hxx"

#include "vtkMRML4DBundleNode.h"

#include "vtkGlobFileNames.h"

#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerVolumesLogic.h"

#include <cmath>

#ifdef Slicer3_USE_PYTHON
//// If debug, Python wants pythonxx_d.lib, so fake it out
//#ifdef _DEBUG
//#undef _DEBUG
//#include <Python.h>
//#define _DEBUG
//#else
#include <Python.h>
//#endif

//extern "C" {
//  void init_mytkinter( Tcl_Interp* );
//  void init_slicer(void );
//}
//#include "vtkTclUtil.h"
//
#endif

vtkCxxRevisionMacro(vtkFourDAnalysisLogic, "$Revision: 3633 $");
vtkStandardNewMacro(vtkFourDAnalysisLogic);

//---------------------------------------------------------------------------
vtkFourDAnalysisLogic::vtkFourDAnalysisLogic()
{

  // Timer Handling

  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkFourDAnalysisLogic::DataCallback);

  this->FrameNodeVector.clear();

  this->CurveCache.clear();

  this->VolumeBundleID = "";
  this->RegisteredVolumeBundleID = "";

}


//---------------------------------------------------------------------------
vtkFourDAnalysisLogic::~vtkFourDAnalysisLogic()
{

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

}


//---------------------------------------------------------------------------
void vtkFourDAnalysisLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkFourDAnalysisLogic:             " << this->GetClassName() << "\n";

}

//----------------------------------------------------------------------------
void vtkFourDAnalysisLogic::ProcessLogicEvents(vtkObject *caller, 
                                            unsigned long event, 
                                            void *callData)
{
  if (event ==  vtkCommand::ProgressEvent) 
    {
    this->InvokeEvent ( vtkCommand::ProgressEvent,callData );
    }
}

//---------------------------------------------------------------------------
void vtkFourDAnalysisLogic::DataCallback(vtkObject *caller, 
                                       unsigned long eid, void *clientData, void *callData)
{
  vtkFourDAnalysisLogic *self = reinterpret_cast<vtkFourDAnalysisLogic *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkFourDAnalysisLogic DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkFourDAnalysisLogic::UpdateAll()
{
}


//---------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkFourDAnalysisLogic::AddMapVolumeNode(vtkMRML4DBundleNode* bundleNode,
                                                                 const char* nodeName)
{
  vtkMRMLScene* scene = this->GetMRMLScene();

  vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::New();
  vtkMRMLScalarVolumeDisplayNode* displayNode = vtkMRMLScalarVolumeDisplayNode::New();

  volumeNode->SetScene(scene);

  vtkImageData* imageData = vtkImageData::New();
  vtkMRMLScalarVolumeNode *firstFrameNode 
    = vtkMRMLScalarVolumeNode::SafeDownCast(bundleNode->GetFrameNode(0));

  if (firstFrameNode && firstFrameNode->GetImageData())
    {
    vtkImageData* firstImageData = firstFrameNode->GetImageData();

    imageData->SetDimensions(firstImageData->GetDimensions());
    imageData->SetExtent(firstImageData->GetExtent());
    imageData->SetSpacing(firstImageData->GetSpacing());
    imageData->SetOrigin(firstImageData->GetOrigin());
    imageData->SetNumberOfScalarComponents(1);
    imageData->SetScalarTypeToFloat();  // Set to float to store parameters
    imageData->AllocateScalars();
    }

  volumeNode->SetAndObserveImageData(imageData);
  volumeNode->SetName(nodeName);
  vtkMatrix4x4* mat = vtkMatrix4x4::New();
  firstFrameNode->GetIJKToRASMatrix(mat);
  volumeNode->SetIJKToRASMatrix(mat);
  mat->Delete();

  scene->SaveStateForUndo();
  volumeNode->SetScene(scene);
  volumeNode->SetDescription("Created by 4D Analysis Module");
  
  double range[2];
  vtkDebugMacro("Set basic display info");
  //volumeNode->GetImageData()->GetScalarRange(range);
  range[0] = 0.0;
  range[1] = 256.0;
  displayNode->SetAutoWindowLevel(0);
  displayNode->SetAutoThreshold(0);
  displayNode->SetLowerThreshold(range[0]);
  displayNode->SetUpperThreshold(range[1]);
  displayNode->SetWindow(range[1] - range[0]);
  displayNode->SetLevel(0.5 * (range[1] + range[0]) );
  
  vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New();
  displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
  displayNode->SetScene(scene);
  scene->AddNode(displayNode);  
  scene->AddNode(volumeNode);  

  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  //volumeNode->Delete();
  //storageNode->Delete();
  //displayNode->Delete();
  //colorLogic->Delete();

  volumeNode->Modified();
  volumeNode->GetImageData()->Modified();

  return volumeNode;
}


//---------------------------------------------------------------------------
int vtkFourDAnalysisLogic::GetNumberOfFrames()
{
  return this->FrameNodeVector.size();
}


//---------------------------------------------------------------------------
const char* vtkFourDAnalysisLogic::GetFrameNodeID(int index)
{
  if (index >= 0 && index < (int)this->FrameNodeVector.size())
    {
    return this->FrameNodeVector[index].c_str();
    }
  else
    {
    return NULL;
    }
}


//---------------------------------------------------------------------------
int vtkFourDAnalysisLogic::SaveIntensityCurves(vtkIntensityCurves* curves, const char* fileNamePrefix)
{
  if (!curves)
    {
    return 0;
    }

  vtkIntArray* labels = curves->GetLabelList();
  int nLabels = labels->GetNumberOfTuples();
  for (int i = 0; i < nLabels; i ++)
    {
    int label = labels->GetValue(i);
    char filename[256];
    sprintf (filename, "%s_%03d.csv", fileNamePrefix, label);
    
    std::ofstream fout;
    fout.open(filename, std::ios::out);
    if (fout.fail())
      {
      vtkErrorMacro ("vtkFourDAnalysisLogic: could not open file " << filename );
      return 0;
      }
    curves->OutputDataInCSV(fout, label);
    fout.close();
    }
  
  return 1;
}


//---------------------------------------------------------------------------
int vtkFourDAnalysisLogic::SaveCurve(vtkDoubleArray* curve, const char* fileNamePrefix)
{
  char filename[256];
  sprintf (filename, "%s.csv", fileNamePrefix);
  std::ofstream fout;
  fout.open(filename, std::ios::out);
  if (fout.fail())
    {
    vtkErrorMacro ("vtkFourDAnalysisLogic: could not open file " << filename );
    return 0;
    }
  
  int n = curve->GetNumberOfTuples();
  for (int i = 0; i < n; i ++)
    {
    double* xy = curve->GetTuple(i);
    // Write the data
    //      t        ,      mean
    //   ---------------------------
    fout << xy[0] << ", " << xy[1] << std::endl;
    }

  fout.close();
  
  return 1;
}


//---------------------------------------------------------------------------
void vtkFourDAnalysisLogic::GenerateParameterMap(const char* script,
                                                 vtkMRML4DBundleNode* bundleNode,
                                                 const char* outputNodeNamePrefix,
                                                 int start, int end,
                                                 int imin, int imax, int jmin, int jmax, int kmin, int kmax)
{
  // Add a new vtkMRMLCurveAnalysisNode to the MRML scene
  vtkMRMLCurveAnalysisNode* curveNode = vtkMRMLCurveAnalysisNode::New();
  this->GetMRMLScene()->AddNode(curveNode);

  // Prepare vtkDoubleArray to pass the source cueve data
  vtkDoubleArray* srcCurve = vtkDoubleArray::New();
  srcCurve->SetNumberOfComponents(2);

  vtkDoubleArray* fittedCurve = vtkDoubleArray::New();
  fittedCurve->SetNumberOfComponents(2);
  
  curveNode->SetSourceData(srcCurve);
  curveNode->SetFittedData(fittedCurve);

  if (!bundleNode)
    {
    return;
    }

  int nFrames = bundleNode->GetNumberOfFrames();  
  
  // ----------------------------------------
  // Setup the executer
  std::cerr << "Setting up the executer" << std::endl;
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
  pythonCmd += "caexec = fda.CurveAnalysisExecuter('";
  pythonCmd += script;
  pythonCmd += "')\n";
  
  // Get output parameter list
  std::cerr << "Get output parameter list" << std::endl;
  pythonCmd += "parameters = caexec.GetOutputParameterNames()\n";
  pythonCmd += "for key in parameters:\n";
  pythonCmd += "    curveNode.SetParameter(key, 0.0)\n";

  v = PyRun_String(pythonCmd.c_str(),
                   Py_file_input,
                   (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()),
                   (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()));
  if (Py_FlushLine())
    {
    PyErr_Clear();
    }
  
  vtkStringArray* nameArray = curveNode->GetParameterNameArray();
  
  // Create map volumes for each parameter
  int numKeys = nameArray->GetNumberOfTuples();
  typedef std::map<std::string, vtkImageData*> ParameterImageMapType;
  typedef std::map<std::string, vtkMRMLScalarVolumeNode*> ParameterVolumeNodeMapType;
  ParameterImageMapType ParameterImages;
  ParameterVolumeNodeMapType ParameterImageNodes;
  for (int i = 0; i < numKeys; i ++)
    {
    char  nodeName[256];
    const char* paramName = nameArray->GetValue(i);
    sprintf(nodeName, "%s_%s", outputNodeNamePrefix, paramName);
    std::cerr << "Creating " << nodeName << std::endl;
    vtkMRMLScalarVolumeNode* node = AddMapVolumeNode(bundleNode, nodeName);
    ParameterImages[paramName] = node->GetImageData();
    ParameterImageNodes[paramName] = node;
    }
  
  // Check the index range
  int max = nFrames;
  if (start < 0)   start = 0;
  if (end >= max)  end   = max-1;
  if (start > end) start = end;
  
  // Make an array of vtkImageData
  int nSrcPoints = end - start + 1;
  vtkImageData* imageArray[nSrcPoints];
  for (int i = start; i <= end; i ++)
    {
    vtkMRMLScalarVolumeNode* node = vtkMRMLScalarVolumeNode::SafeDownCast(bundleNode->GetFrameNode(i));
    imageArray[i-start] = node->GetImageData();
    std::cerr << "Listing image data: " << node->GetName() << std::endl;
    }
  
  int* dim = imageArray[0]->GetDimensions();
  int x = dim[0]; int y = dim[1]; int z = dim[2];

  srcCurve->SetNumberOfTuples(nSrcPoints);
  fittedCurve->SetNumberOfTuples(0);

  // Run Map generation
  //  -- prepare python commands 
  pythonCmd  = "";
  pythonCmd += "inputCurve  = curveNode.GetSourceData().ToArray()\n";
  pythonCmd += "outputCurve = curveNode.GetFittedData().ToArray()\n";

  v = PyRun_String(pythonCmd.c_str(),
                   Py_file_input,
                   (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()),
                   (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()));
  if (Py_FlushLine())
    {
    PyErr_Clear();
    }
  
  pythonCmd  = "";
  pythonCmd += "result      = caexec.Execute(inputCurve, outputCurve)\n";
  pythonCmd += "for key, value in result.iteritems():\n";
  pythonCmd += "    curveNode.SetParameter(key, value)\n";
  pythonCmd += "\n";

  // Check index
  if (imin < 0 || imin >= imax || imax >= x ||
      jmin < 0 || jmin >= jmax || jmax >= y ||
      kmin < 0 || kmin >= kmax || kmax >= z)
    {
    imin = 0;
    jmin = 0;
    kmin = 0;
    imax = x;
    jmax = y;
    kmax = z;
    }
  
  for (int k = kmin; k < kmax; k ++)
    {
    std::cerr << std::endl;
    std::cerr << "Processing Slice k = " << k << std::endl;
    for (int j = jmin; j < jmax; j ++)
      {
      std::cerr << "    Processing line j = " << j << std::endl;
      for (int i = imin; i < imax; i ++)
        {
        // Copy intensity data
        for (int t = 0; t < nSrcPoints; t ++)
          {
          double xy[2];
          xy[0] = (double) t + start;
          xy[1] = imageArray[t]->GetScalarComponentAsDouble(i, j, k, 0);
          srcCurve->SetTuple(t, xy);
          }

        // Run curve fitting
        v = PyRun_String(pythonCmd.c_str(),
                         Py_file_input,
                         (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()),
                         (PyObject*)(vtkSlicerApplication::GetInstance()->GetPythonDictionary()));
        if (Py_FlushLine())
          {
          PyErr_Clear();
          }

        // Put results
        ParameterImageMapType::iterator iter;
        for (iter = ParameterImages.begin(); iter != ParameterImages.end(); iter ++)
          {
          float param = (float)curveNode->GetParameter(iter->first.c_str());
          if (!std::isnormal(param))
            {
            param = 0.0;
            }
          iter->second->SetScalarComponentFromFloat(i, j, k, 0, param);
          }
        }
      }
    //std::cerr << std::endl;
    }
  
  // Put results
  ParameterVolumeNodeMapType::iterator iter;
  for (iter = ParameterImageNodes.begin(); iter != ParameterImageNodes.end(); iter ++)
    {
    double range[2];
    vtkImageData* imageData = iter->second->GetImageData();
    vtkMRMLScalarVolumeDisplayNode* displayNode 
      = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(iter->second->GetDisplayNode());
    imageData->Update();
    imageData->GetScalarRange(range);
    std::cerr << "range = (" << range[0] << ", " << range[1] << ")" << std::endl;
    displayNode->SetAutoWindowLevel(0);
    displayNode->SetAutoThreshold(0);
    displayNode->SetLowerThreshold(range[0]);
    displayNode->SetUpperThreshold(range[1]);
    displayNode->SetWindow(range[1] - range[0]);
    displayNode->SetLevel(0.5 * (range[1] + range[0]));
    }
  
  std::cerr << "END " << std::endl;
}


