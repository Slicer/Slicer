/*==========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

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

#include "vtkMRMLTimeSeriesBundleNode.h"

#include "vtkGlobFileNames.h"

#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerVolumesLogic.h"

//#include "vtkMPIController.h"
#include "vtkMultiThreader.h"
#include "vtkMutexLock.h"

#include <cmath>

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
vtkMRMLScalarVolumeNode* vtkFourDAnalysisLogic::AddMapVolumeNode(vtkMRMLTimeSeriesBundleNode* bundleNode,
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
void vtkFourDAnalysisLogic::GenerateParameterMapMT(const char* scriptFile,
                                                   vtkMRMLCurveAnalysisNode* curveNode,
                                                   vtkMRMLTimeSeriesBundleNode* bundleNode,
                                                   const char* outputNodeNamePrefix,
                                                   int start, int end,
                                                   int imin, int imax, int jmin, int jmax, int kmin, int kmax)
{
  if (!scriptFile || !bundleNode || !curveNode)
    {
    return;
    }

  int nFrames = bundleNode->GetNumberOfFrames();  
  vtkStringArray* nameArray = curveNode->GetOutputValueNameArray();

  StatusMessageType statusMessage;
  statusMessage.show = 1;
  statusMessage.progress = 0.0;
  statusMessage.message = "Preparing maps....";
  this->InvokeEvent ( vtkFourDAnalysisLogic::ProgressDialogEvent, &statusMessage);        

  // Create map volumes for each parameter
  int numKeys = nameArray->GetNumberOfTuples();

  ParameterImageMapType      ParameterImages;
  ParameterVolumeNodeMapType ParameterImageNodes;

  for (int i = 0; i < numKeys; i ++)
    {
    char  nodeName[256];
    const char* paramName = nameArray->GetValue(i);
    sprintf(nodeName, "%s_%s", outputNodeNamePrefix, paramName);
    std::cerr << "Creating " << nodeName << std::endl;
    vtkMRMLScalarVolumeNode* node  = AddMapVolumeNode(bundleNode, nodeName);
    ParameterImages[paramName]     = node->GetImageData();
    ParameterImageNodes[paramName] = node;
    }
  
  // Check the index range
  int max = nFrames;
  if (start < 0)   start = 0;
  if (end >= max)  end   = max-1;
  if (start > end) start = end;
  
  // Make an array of vtkImageData
  int nSrcPoints = end - start + 1;
  std::vector<vtkImageData*> imageVector;
  std::vector<double>        imageTimeStampVector;
  imageVector.resize(nSrcPoints);
  imageTimeStampVector.resize(nSrcPoints);

  for (int i = start; i <= end; i ++)
    {
    // image
    vtkMRMLScalarVolumeNode* node = vtkMRMLScalarVolumeNode::SafeDownCast(bundleNode->GetFrameNode(i));
    imageVector[i-start] = node->GetImageData();

    // time stamp
    vtkMRMLTimeSeriesBundleNode::TimeStamp ts;
    bundleNode->GetTimeStamp(i, &ts);
    imageTimeStampVector[i-start] = (double)ts.second + (double)ts.nanosecond / 1000000000.0;

    std::cerr << "Listing image data: " << node->GetName() << std::endl;
    }
  
  int* dim = imageVector[0]->GetDimensions();
  int x = dim[0]; int y = dim[1]; int z = dim[2];

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

  // prepare sets of curve analysis nodes and script interfaces
  //vtkMultiThreader*   curveAnalysisThread;
  //vtkMPIController* controller = vtkMPIController::New();
  vtkMultiThreader* controller = vtkMultiThreader::New();
  //controller->Initialize(0, NULL);
  int nProcs = 1;

  // When using MPI, the number of processes is determined
  // by the external program which launches this application.
  // However, when using threads, we need to set it ourselves.
  //if (controller->IsA("vtkThreadedController"))
  //  {
  //  // Set the number of processes to 2 for this example.
  //  controller->SetNumberOfProcesses(nProcs);
  //  } 
  //nProcs = controller->GetNumberOfProcesses();

  controller->SetNumberOfThreads(nProcs);

  std::vector<vtkMRMLCurveAnalysisNode*>        curveAnalysisNodeVector;
  std::vector<vtkCurveAnalysisPythonInterface*> curveAnalysisScriptInterfaceVector;
  std::vector<ThreadInfo> threadInfo;
  
  curveAnalysisNodeVector.resize(nProcs);
  curveAnalysisScriptInterfaceVector.resize(nProcs);
  threadInfo.resize(nProcs);


  // min / max 
  int range[3][2];
  range[0][0] = imin;  // i
  range[0][1] = imax;  // i
  range[1][0] = jmin;  // j
  range[1][1] = jmax;  // j
  range[2][0] = kmin;  // k
  range[2][1] = kmax;  // k

  // split by index that has the largest range
  int nindexes[3];
  nindexes[0] = imax - imin;
  nindexes[1] = jmax - jmin;
  nindexes[2] = kmax - kmin;

  // splitted index
  int sindex = 0; // 0: i,   1: j,  2: k
  if (nindexes[1] > nindexes[sindex]) sindex = 1;
  if (nindexes[2] > nindexes[sindex]) sindex = 2;
  
  int sstep = nindexes[sindex] / nProcs;
  if (nindexes[sindex] % nProcs)
    {
    sstep += 1;
    }

  int smin = range[sindex][0];
  int smax = range[sindex][1];

  vtkMutexLock* mutex = vtkMutexLock::New();

  for (int i = 0; i < nProcs; i ++)
    {
    // curve analysis node
    curveAnalysisNodeVector[i] = vtkMRMLCurveAnalysisNode::New();
    this->GetMRMLScene()->AddNode(curveAnalysisNodeVector[i]);
    curveAnalysisNodeVector[i]->Copy(curveAnalysisNodeVector[i]);

    // script
    curveAnalysisScriptInterfaceVector[i] = vtkCurveAnalysisPythonInterface::New();
    curveAnalysisScriptInterfaceVector[i]->SetScript(scriptFile);
    curveAnalysisScriptInterfaceVector[i]->SetCurveAnalysisNode(curveAnalysisNodeVector[i]);

    // thread info
    threadInfo[i].id                     = i;
    threadInfo[i].ptr                    = this;
    threadInfo[i].curveAnalysisInterface = curveAnalysisScriptInterfaceVector[i];
    threadInfo[i].imageVector            = imageVector;
    threadInfo[i].imageTimeStampVector   = imageTimeStampVector;
    threadInfo[i].parameterImageMap      = ParameterImages;
    threadInfo[i].mutex                  = mutex;

    range[sindex][0] = smin + sstep * i;
    range[sindex][1] = smin + sstep * (i+1);
    if (range[sindex][1] > smax) range[sindex][1] = smax;

    threadInfo[i].rangei[0] = range[0][0];
    threadInfo[i].rangei[1] = range[0][1];
    threadInfo[i].rangej[0] = range[1][0];
    threadInfo[i].rangej[1] = range[1][1];
    threadInfo[i].rangek[0] = range[2][0];
    threadInfo[i].rangek[1] = range[2][1];
    
    controller->SetMultipleMethod(i, (vtkThreadFunctionType)CurveAnalysisThread, (void*)&threadInfo[i]);

    }

  controller->MultipleMethodExecute();
  //controller->Finalize();

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


//---------------------------------------------------------------------------
void* vtkFourDAnalysisLogic::CurveAnalysisThread(void* ptr)
{
  vtkMultiThreader::ThreadInfo* vinfo = 
    static_cast<vtkMultiThreader::ThreadInfo*>(ptr);
  vtkFourDAnalysisLogic::ThreadInfo* threadInfo = static_cast<vtkFourDAnalysisLogic::ThreadInfo*>(vinfo->UserData);

  int thread_id = threadInfo->id;
  
  int nSrcPoints = threadInfo->imageVector.size();

  vtkDoubleArray* srcCurve = vtkDoubleArray::New();
  vtkDoubleArray* fittedCurve = vtkDoubleArray::New();
  srcCurve->SetNumberOfComponents(2);
  srcCurve->SetNumberOfTuples(nSrcPoints);

  fittedCurve->SetNumberOfComponents(2);
  fittedCurve->SetNumberOfTuples(0);        

  //threadInfo->mutex->Lock();
  
  std::vector<vtkImageData*> imageVector = threadInfo->imageVector;
  std::vector<double> imageTimeStampVector = threadInfo->imageTimeStampVector;

  //threadInfo->mutex->Unlock();

  vtkMRMLCurveAnalysisNode* curveNode = threadInfo->curveAnalysisInterface->GetCurveAnalysisNode();

  int imin = threadInfo->rangei[0];
  int imax = threadInfo->rangei[1];
  int jmin = threadInfo->rangej[0];
  int jmax = threadInfo->rangej[1];
  int kmin = threadInfo->rangek[0];
  int kmax = threadInfo->rangek[1];

  //PyThreadState* state = Py_NewInterpreter();

  for (int k = kmin; k < kmax; k ++)
    {
    for (int j = jmin; j < jmax; j ++)
      {
      for (int i = imin; i < imax; i ++)
        {
        fprintf(stderr, "Thread %d: Fitting curve at (i=%d, j=%d, k=%d)\n", thread_id, i, j, k);

        // Copy intensity data
        for (int t = 0; t < nSrcPoints; t ++)
          {
          double xy[2];
          xy[0] = imageTimeStampVector[t];
          xy[1] = imageVector[t]->GetScalarComponentAsDouble(i, j, k, 0);
          srcCurve->SetTuple(t, xy);
          fittedCurve->InsertNextTuple(xy);
          }
        
        curveNode->SetTargetCurve(srcCurve);
        curveNode->SetFittedCurve(fittedCurve);
        
        threadInfo->curveAnalysisInterface->Run();
        
        // Put results
        ParameterImageMapType::iterator iter;
        for (iter = threadInfo->parameterImageMap.begin();
             iter != threadInfo->parameterImageMap.end();
             iter ++)
          {
          float param = (float)curveNode->GetOutputValue(iter->first.c_str());
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

  //Py_EndInterpreter(state);
  
  std::cerr << "Thread: " << thread_id << " .... end" << std::endl;
  
}



//---------------------------------------------------------------------------
void vtkFourDAnalysisLogic::GenerateParameterMap(vtkCurveAnalysisPythonInterface* script,
                                                 vtkMRMLCurveAnalysisNode* curveNode,
                                                 vtkMRMLTimeSeriesBundleNode* bundleNode,
                                                 const char* outputNodeNamePrefix,
                                                 int start, int end,
                                                 int imin, int imax, int jmin, int jmax, int kmin, int kmax)
{

  if (!script || !bundleNode || !curveNode)
    {
    return;
    }

  int nFrames = bundleNode->GetNumberOfFrames();  
  vtkStringArray* nameArray = curveNode->GetOutputValueNameArray();
  

  StatusMessageType statusMessage;
  statusMessage.show = 1;
  statusMessage.progress = 0.0;
  statusMessage.message = "Preparing maps....";
  this->InvokeEvent ( vtkFourDAnalysisLogic::ProgressDialogEvent, &statusMessage);        

  // Create map volumes for each parameter
  int numKeys = nameArray->GetNumberOfTuples();
  typedef std::map<std::string, vtkImageData*>            ParameterImageMapType;
  typedef std::map<std::string, vtkMRMLScalarVolumeNode*> ParameterVolumeNodeMapType;

  ParameterImageMapType      ParameterImages;
  ParameterVolumeNodeMapType ParameterImageNodes;

  for (int i = 0; i < numKeys; i ++)
    {
    char  nodeName[256];
    const char* paramName = nameArray->GetValue(i);
    sprintf(nodeName, "%s_%s", outputNodeNamePrefix, paramName);
    std::cerr << "Creating " << nodeName << std::endl;
    vtkMRMLScalarVolumeNode* node  = AddMapVolumeNode(bundleNode, nodeName);
    ParameterImages[paramName]     = node->GetImageData();
    ParameterImageNodes[paramName] = node;
    }
  
  // Check the index range
  int max = nFrames;
  if (start < 0)   start = 0;
  if (end >= max)  end   = max-1;
  if (start > end) start = end;
  
  // Make an array of vtkImageData
  int nSrcPoints = end - start + 1;
  std::vector<vtkImageData*> imageVector;
  std::vector<double>        imageTimeStampVector;
  imageVector.resize(nSrcPoints);
  imageTimeStampVector.resize(nSrcPoints);

  for (int i = start; i <= end; i ++)
    {
    vtkMRMLScalarVolumeNode* node = vtkMRMLScalarVolumeNode::SafeDownCast(bundleNode->GetFrameNode(i));
    imageVector[i-start] = node->GetImageData();

    vtkMRMLTimeSeriesBundleNode::TimeStamp ts;
    bundleNode->GetTimeStamp(i, &ts);
    imageTimeStampVector[i-start] = (double)ts.second + (double)ts.nanosecond / 1000000000.0;

    std::cerr << "Listing image data: " << node->GetName() << std::endl;
    }
  
  int* dim = imageVector[0]->GetDimensions();
  int x = dim[0]; int y = dim[1]; int z = dim[2];

  vtkDoubleArray* srcCurve = vtkDoubleArray::New();
  vtkDoubleArray* fittedCurve = vtkDoubleArray::New();
  srcCurve->SetNumberOfComponents(2);
  srcCurve->SetNumberOfTuples(nSrcPoints);

  fittedCurve->SetNumberOfComponents(2);
  fittedCurve->SetNumberOfTuples(0);        

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

  // set curve analysis node to the script interface
  script->SetCurveAnalysisNode(curveNode);
  
  this->InvokeEvent ( vtkFourDAnalysisLogic::ProgressDialogEvent, &statusMessage);
  double numVoxel = (double) (kmax-kmin)*(jmax-jmin)*(imax-imin);
  double counter  = 0;
  char   progressMsg[128];
  
  for (int k = kmin; k < kmax; k ++)
    {
    std::cerr << std::endl;
    for (int j = jmin; j < jmax; j ++)
      {
      for (int i = imin; i < imax; i ++)
        {
        // Update progress message.
        counter += 1.0;
        statusMessage.progress = counter / numVoxel;
        sprintf(progressMsg, "Fitting curve at (i=%d, j=%d, k=%d)", i, j, k);
        statusMessage.message = progressMsg;
        this->InvokeEvent ( vtkFourDAnalysisLogic::ProgressDialogEvent, &statusMessage);        

        // Copy intensity data
        for (int t = 0; t < nSrcPoints; t ++)
          {
          double xy[2];
          xy[0] = imageTimeStampVector[t];//(double) t + start;
          xy[1] = imageVector[t]->GetScalarComponentAsDouble(i, j, k, 0);
          srcCurve->SetTuple(t, xy);
          fittedCurve->InsertNextTuple(xy);
          }

        curveNode->SetTargetCurve(srcCurve);
        curveNode->SetFittedCurve(fittedCurve);

        script->Run();
        
        // Put results
        ParameterImageMapType::iterator iter;
        for (iter = ParameterImages.begin(); iter != ParameterImages.end(); iter ++)
          {
          float param = (float)curveNode->GetOutputValue(iter->first.c_str());
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
  
  statusMessage.show = 0;
  statusMessage.progress = 0.0;
  statusMessage.message = "";
  this->InvokeEvent ( vtkFourDAnalysisLogic::ProgressDialogEvent, &statusMessage);

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


