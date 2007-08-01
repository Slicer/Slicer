/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkQdecModuleLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include <vtksys/SystemTools.hxx>
#include <vtksys/Directory.hxx>

#include "vtkObjectFactory.h"

#include "vtkQdecModuleLogic.h"
#include "vtkQdecModule.h"

#include "vtkSlicerApplication.h"

// for loading the outputs of the GLM processing
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerModelsLogic.h"
#include "vtkGDFReader.h"

vtkQdecModuleLogic* vtkQdecModuleLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkQdecModuleLogic");
  if(ret)
    {
      return (vtkQdecModuleLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkQdecModuleLogic;
}


//----------------------------------------------------------------------------
vtkQdecModuleLogic::vtkQdecModuleLogic()
{
    this->QDECProject = new QdecProject();

    this->PlotTclScript = NULL;
    vtksys_stl::string slicerHome;
    if (!vtksys::SystemTools::GetEnv("SLICER_HOME", slicerHome))
      {
      this->PlotTclScript = "../Libs/Qdec/vtkFreeSurferReaders.tcl";
      vtkDebugMacro("Can't find SLICER_HOME env var, using plot tcl script path = " << this->GetPlotTclScript());
      }
    else
      {
      std::string scriptPath = slicerHome + "/Libs/Qdec/vtkFreeSurferReaders.tcl";
      this->SetPlotTclScript(const_cast <char *> (scriptPath.c_str()));
      vtkDebugMacro("Found SLICER_HOME env var, using plot tcl script path = " << this->GetPlotTclScript());
      }
    this->TclScriptLoaded = 0;
}


//----------------------------------------------------------------------------
vtkQdecModuleLogic::~vtkQdecModuleLogic()
{
  if (QDECProject)
    {
    delete this->QDECProject;
    this->QDECProject = NULL;
    }
  if (this->PlotTclScript)
    {
    delete [] this->PlotTclScript;
    this->PlotTclScript = NULL;
    }

}

//----------------------------------------------------------------------------
void vtkQdecModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  
}


//----------------------------------------------------------------------------
int vtkQdecModuleLogic::LoadDataTable(const char* fileName)
{
  vtkDebugMacro("LoadDataTable: trying to load " << fileName);

  if (this->QDECProject)
    {
    // returns -1 on error, 0 on success
    int err = this->QDECProject->LoadDataTable(fileName);
    vtkDebugMacro("Return from LoadDataTable call on QDECProject = " << err);
    if (err == 0)
      {
      return 1;
      }
    else
      {
      vtkErrorMacro("LoadDataTable: Failed to load data file " << fileName);
      }
    }
  else
    {
    vtkErrorMacro("LoadDataTable: QDEC Project is unintialised, cannot use it to load data file " << fileName);
    }
  return 0;
}

//----------------------------------------------------------------------------
//void vtkQdecModuleLogic::CreateGLMDesign(const char* name)

//----------------------------------------------------------------------------
void vtkQdecModuleLogic::SetSubjectsDirectory(const char *fileName)
{
  if (QDECProject)
    {
    if (!fileName || strcmp(fileName,"") == 0)
      {
      vtkDebugMacro("SetSubjectsDirectory: Empty filename, using 'None'");
      this->QDECProject->SetSubjectsDir("None");
      }
    else
      {
      vtkDebugMacro("Setting the qdec projects subjects dir to " << fileName);
      this->QDECProject->SetSubjectsDir(fileName);
      }
    }
  else
    {
    vtkErrorMacro("SetSubjectsDirectory: QDEC project null, can't set subjects dir");
    }
}

//----------------------------------------------------------------------------
int vtkQdecModuleLogic::CreateGlmDesign(const char *name, const char *discreteFactor1, const char *discreteFactor2, const char *continuousFactor1, const char *continuousFactor2, const char* measure, const char* hemisphere, int smoothness)
{
  if (this->QDECProject)
    {
    // pass NULL for the progress update gui
    int err = this->QDECProject->CreateGlmDesign(name, discreteFactor1, discreteFactor2, 
                                                 continuousFactor1, continuousFactor2, 
                                                 measure, hemisphere, smoothness, NULL);
    if (err == 0)
      {
      // success
      return 0;
      }
    else
      {
      vtkErrorMacro("CreateGlmDesign: error creating the qdec project glm design");
      return -1;
      }
    }
  else
    {
    return -1;
    }
}

//----------------------------------------------------------------------------
int vtkQdecModuleLogic::RunGlmFit()
{
  if (this->QDECProject)
    {
    return this->QDECProject->RunGlmFit();
    }
  else
    {
    vtkErrorMacro("RunGlmFit: null QDEC project...");
    return -1;
    }
}

//----------------------------------------------------------------------------
int vtkQdecModuleLogic::LoadResults(vtkSlicerModelsLogic *modelsLogic, vtkKWApplication *app)
{
  if (!QDECProject)
    {
    return -1;
    }

  // Make sure we got the results.
  QdecGlmFitResults* results =
    this->QDECProject->GetGlmFitResults();
  if (results == NULL)
    {
    vtkErrorMacro("LoadPlotData: results are null.");
    return -1;
    }
  vtkDebugMacro("Got the GLM Fit results from the QDEC project");
  
  // fsaverage surface to load. This isn't returned in the results,
  // but we know where it is because it's just a normal subject in
  // the subjects dir.
  string fnSubjects = this->QDECProject->GetSubjectsDir();
  string sHemi =  this->QDECProject->GetHemi();
  string fnSurface = fnSubjects + "/fsaverage/surf/" + sHemi + ".inflated";
  vtkDebugMacro( "Surface: " << fnSurface.c_str() );
  
  // get the Models Logic and load the average surface file
//  vtkSlicerModelsLogic *modelsLogic = vtkSlicerModelsGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Models"))->GetLogic();
  vtkMRMLModelNode *modelNode = NULL;
  if (modelsLogic)
    {
    modelNode = modelsLogic->AddModel( fnSurface.c_str() );
    if (modelNode == NULL)
      {
      vtkErrorMacro("Unable to load average surface file " << fnSurface.c_str());
      }
    else
      {
      vtkDebugMacro("Loaded average surface file " << fnSurface.c_str());
      }
    }
  else
    {
    vtkErrorMacro("Unable to get at Models module to load average surface file.");
    }
  
  // load in the curvature overlay
  string curvFileName = fnSubjects + "/fsaverage/surf/" + sHemi + ".curv";
  vtkDebugMacro( "Surface: " << curvFileName.c_str() << ", adding curv file to " << modelNode->GetName() );
  string curvArrayName = "";
  if (modelsLogic && modelNode)
    {
    if (!modelsLogic->AddScalar(curvFileName.c_str(), modelNode))
      {
      vtkErrorMacro("Unable to add curvature to average model surface: " << curvFileName.c_str());
      }
    else
      {
      // grab the curvature array name
      curvArrayName = modelNode->GetActivePointScalarName("scalars");
      if (strcmp(curvArrayName.c_str(), "") == 0)
        {
        // hack it together
        curvArrayName = string("surf/") + sHemi + string(".curv");
        vtkDebugMacro("Failed to get the active point scalars name, so using curv array name = '" << curvArrayName.c_str() << "'");
        }
      vtkDebugMacro("Added the curvature file " << curvFileName.c_str() << ", got the curvature array name: '" << curvArrayName.c_str() << "'");
      }
    }

  // We should have the same number of questions as sig file. Each
  // sig file has a correpsponding question, and they are in the same
  // order in the vector.
  vector<string> lContrastQuestions = results->GetContrastQuestions();
  vector<string> lfnContrastSigs = results->GetContrastSigFiles();
  assert( lContrastQuestions.size() == lfnContrastSigs.size() );
  
  // Go through and get our sig files and questions.
  vector<string>::iterator fn;
  for( unsigned int nContrast = 0; 
       nContrast < results->GetContrastQuestions().size(); 
       nContrast++ ) 
    {
  
    vtkDebugMacro( "Contrast " << nContrast << ": \""
                   << lContrastQuestions[nContrast].c_str() << "\" in file " 
                   << lfnContrastSigs[nContrast].c_str() );
    // load the sig file
    if (modelsLogic && modelNode)
      {
      if (!modelsLogic->AddScalar(lfnContrastSigs[nContrast].c_str(), modelNode))
        {
        vtkErrorMacro("Unable to add contrast to average model surface: " << lfnContrastSigs[nContrast].c_str());
        }
      else
        {
        if (strcmp(curvArrayName.c_str(), "") != 0)
          {
          // composite with the curv
          string sigArrayName = modelNode->GetActivePointScalarName("scalars");
          if (strcmp(sigArrayName.c_str(), "") == 0)
            {
            // hack it together
            std::string::size_type ptr = lfnContrastSigs[nContrast].find_last_of(std::string("/"));
            
            
            if (ptr != std::string::npos)
              {
              // find the dir name above
              std::string::size_type dirptr = lfnContrastSigs[nContrast].find_last_of(std::string("/"), ptr);
              if (dirptr != std::string::npos)
                {
                sigArrayName = lfnContrastSigs[nContrast].substr(++dirptr);
                vtkDebugMacro("created sig array name = '" << sigArrayName .c_str() << "'");
                }
              else
                {
                sigArrayName = lfnContrastSigs[nContrast].substr(++ptr);
                }
              }
            else
              {
              sigArrayName = lfnContrastSigs[nContrast];
              }
            }
          vtkDebugMacro("Compositing curv '" << curvArrayName.c_str() << "' with sig array '" << sigArrayName.c_str() << "'");
          modelNode->CompositeScalars(curvArrayName.c_str(), sigArrayName.c_str(), 2, 5, 1, 1, 0);
          }
        }
      }
    }

  // The regression coefficient and std dev files to load.
  string fnRegressionCoefficients = results->GetRegressionCoefficientsFile();
  string fnStdDev = results->GetResidualErrorStdDevFile();
  vtkDebugMacro( "Regressions coefficients: "
                 << fnRegressionCoefficients.c_str() );
  
  vtkDebugMacro( "Std dev: " << fnStdDev.c_str() );
  
  // load the std dev file
  if (modelsLogic && modelNode)
    {
    if (!modelsLogic->AddScalar(fnStdDev.c_str(), modelNode))
      {
      vtkErrorMacro("Unable to add the residual errors std dev file " << fnStdDev.c_str() << " to the average surface model");
      }
    }

  // read the plot file
  string fnFSGD =  results->GetFsgdFile();
  vtkDebugMacro( "FSGD plot file: " << fnFSGD.c_str() );

  // read the file
  /*
  vtkGDFReader *gdfReader = vtkGDFReader::New();
  gdfReader->ReadHeader(fnFSGD.c_str(), 1);
  string dataFileName = gdfReader->GetDataFileName();
  vtkDebugMacro("FSGD file read in, y.mgh data file name = " << dataFileName.c_str());
  
  // don't load the data file, it's loaded in the tcl code and associated with the model there
  gdfReader->Delete();
  */

  // set the default to be the curv file for now
  if (modelNode)
    {
    modelNode->SetActiveScalars(curvArrayName.c_str(), "scalars");
    // set the colour table
    if (modelNode->GetDisplayNode())
      {
      modelNode->GetDisplayNode()->SetAndObserveColorNodeID("vtkMRMLFreeSurferProceduralColorNodeGreenRed");
      }
    }

  if (!this->GetTclScriptLoaded())
    {
      vtkWarningMacro("Loading tcl script: " << this->GetPlotTclScript());
      app->LoadScript(this->GetPlotTclScript());
      this->SetTclScriptLoaded(1);
    }
  // set the plot file name
  std::string scriptReturn;
  scriptReturn = app->Script("set ::vtkFreeSurferReaders(PlotFileName) %s", fnFSGD.c_str());
  vtkDebugMacro("Set the plot file name to " << fnFSGD.c_str() << ", return value from tcl script call = " << scriptReturn.c_str());
  scriptReturn = app->Script("vtkFreeSurferReadersPlotApply %s", modelNode->GetID());
  vtkDebugMacro("Called vtkFreeSurferReadersPlotApply with model id " << modelNode->GetID() << ", return value from tcl script call = " << scriptReturn.c_str());

  if (this->GetDebug())
    {
    this->DebugOff();
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkQdecModuleLogic::LoadPlotData(const char *fileName)
{
  if (!QDECProject)
    {
    return -1;
    }
  
  // The fsgd file to plot.
  string fnFSGD;
  if (fileName == NULL)
    {
    // Make sure we got the results.
    QdecGlmFitResults* results =
      this->QDECProject->GetGlmFitResults();
    if (results == NULL)
      {
      vtkErrorMacro("LoadPlotData: results are null.");
      return -1;
      }
    vtkDebugMacro("Got the GLM Fit results from the QDEC project");
    fnFSGD = results->GetFsgdFile();
    }
  else
    {
    fnFSGD = string(fileName);
    }
  vtkDebugMacro( "FSGD plot file: " << fnFSGD.c_str() );
    

  // read the file
  vtkGDFReader *gdfReader = vtkGDFReader::New();
  gdfReader->ReadHeader(fnFSGD.c_str(), 1);
  string dataFileName = gdfReader->GetDataFileName();
  vtkDebugMacro("FSGD file read in, y.mgh data file name = " << dataFileName.c_str());
  
  // load the data file, associating it with the model
  // - done in LoadResults

  gdfReader->Delete();
  return 0;
}
