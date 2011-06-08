#include <iostream>
#include <vector>
#include <string>

#include "vtkChangeTrackerLogic.h"
#include "vtkMRMLChangeTrackerNode.h"

#include "ChangeTrackerCommandLineCLP.h"
#include "vtkNRRDReader.h"
#include "vtkNRRDWriter.h" 
#include "vtkSlicerApplication.h"
#include "vtkKWTkUtilities.h"
#include <vtksys/stl/string>
#include "vtkImageIslandFilter.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMatrix4x4.h"
#include "vtkITKImageWriter.h"
#include "vtkGeneralTransform.h"
#include "vtkImageResample.h"
#include "vtkImageConstantPad.h"
#include "vtkImageMathematics.h"
#include "vtkImageAccumulate.h"

#include "itkAffineTransform.h"
#include "itkTransformFileReader.h"
#include "itkMatrixOffsetTransformBase.h"

#include "vtkImageEuclideanDistance.h"
#include "vtkImageMask.h"

#include <vtksys/SystemTools.hxx>
#include <string>
#include <fstream>

#define USE_ITK_REGISTRATION 1

// Go to Slicer-build/lib/Slicer3/Plugins/ChangeTrackerCommandLine
// ./ChangeTrackerCommandLine --sensitivity 0.5 --threshold 100,277 --roi_min 73,135,92 --roi_max 95,165,105 --intensity_analysis --deformable_analysis --scan1 /data/local/BrainScienceFoundation/Demo/07-INRIA/data/SILVA/2006-spgr.nhdr --scan2 /data/local/BrainScienceFoundation/Demo/07-INRIA/data/SILVA/2007-spgr-scan1.nhdr
//
//
// This is necessary to load in ChangeTracker package in TCL interp.
extern "C" int Slicerbasegui_Init(Tcl_Interp *interp);
extern "C" int Changetracker_Init(Tcl_Interp *interp);
extern "C" int Vtkteem_Init(Tcl_Interp *interp);
extern "C" int Vtkitk_Init(Tcl_Interp *interp);

#define tgVtkCreateMacro(name,type) \
  name  = type::New(); \
  name##Tcl = vtksys::SystemTools::DuplicateString(vtkKWTkUtilities::GetTclNameFromPointer(interp, name)); 

#define tgVtkDefineMacro(name,type) \
  type *name; \
  std::string name##Tcl;\
  tgVtkCreateMacro(name,type); 

#define tgSetDataMacro(name,matrix)               \
 virtual int Set##name(const char *fileName) { \
   if (strcmp(fileName,"None")) { \
    tgVtkCreateMacro(this->name,vtkImageData); \
    this->matrix = vtkMatrix4x4::New(); \
    return tgReadVolume(fileName,this->name,this->matrix);    \
   } \
   this->name = NULL; \
   this->matrix = NULL; \
   std::cout << "Here" << std::endl; \
   return 0; \
 }

int tgReadVolume(const char *fileName, vtkImageData *outputData,  vtkMatrix4x4 *outputMatrix) {
  // Currently only works with nrrd files bc I do not know how to converd itk::ImageFileReader vtkImageData
  vtkNRRDReader *reader = vtkNRRDReader::New();
  reader->SetFileName(fileName);
  reader->Update();

  outputData->DeepCopy(reader->GetOutput());
  outputMatrix->DeepCopy(reader->GetRasToIjkMatrix());

  if (reader->GetReadStatus()) {
    cout << "ERROR: tgReadVolume: could not read " << fileName << endl;
    return 1;
  }
  reader->Delete();
  return 0;
}



class tgCMDLineStructure {
  public:

  tgCMDLineStructure(Tcl_Interp *init) {
    this->Scan1Data = NULL;
    this->Scan1DataTcl = "";
    this->Scan1Matrix = NULL;
    this->Scan2Data = NULL;
    this->Scan2DataTcl = "";
    this->Scan2Matrix = NULL;

    this->interp = init;
  } 

  virtual ~tgCMDLineStructure() {
    if (this->Scan1Data) {
      Scan1Data->Delete();
      this->Scan1Data = NULL;
    }

    if (this->Scan1Matrix) {
      Scan1Matrix->Delete();
      this->Scan1Matrix = NULL;
    }
    
    if (this->Scan2Data) {
      Scan2Data->Delete();
      this->Scan2Data = NULL;
    }

    if (this->Scan2Matrix) {
      Scan2Matrix->Delete();
      this->Scan2Matrix = NULL;
    }

    if ( this->Scan1SegmentedData ){
      Scan1SegmentedData->Delete();
      this->Scan1SegmentedData = NULL;
    }

    if ( this->Scan1SegmentedMatrix ){
      Scan1SegmentedMatrix->Delete();
      this->Scan1SegmentedMatrix = NULL;
    }

    this->interp = NULL;
  } 

  tgSetDataMacro(Scan1Data,Scan1Matrix);
  tgSetDataMacro(Scan2Data,Scan2Matrix);
  tgSetDataMacro(Scan1SegmentedData,Scan1SegmentedMatrix);


  void SetWorkingDir(vtkKWApplication *app, const char* fileNameScan1) {
    this->WorkingDir = vtksys::SystemTools::GetFilenamePath(vtksys::SystemTools::CollapseFullPath(fileNameScan1)) + "-TGcmd";
    cout << "Setting working directory to " << this->WorkingDir << endl;
    char CMD[1024];
    sprintf(CMD,"file isdirectory %s",this->GetWorkingDir()); 
    if (!atoi(app->Script(CMD))) { 
      sprintf(CMD,"file mkdir %s",this->GetWorkingDir()); 
      app->Script(CMD); 
    } 
  }

  void SetWorkingDir(std::string wd){
    this->WorkingDir = wd;
  }

  const char *GetWorkingDir () {return this->WorkingDir.c_str();}

  vtkImageData *Scan1Data; 
  std::string Scan1DataTcl;
  vtkMatrix4x4 *Scan1Matrix; 

  vtkImageData *Scan2Data; 
  std::string Scan2DataTcl;
  vtkMatrix4x4 *Scan2Matrix; 

  vtkImageData *Scan1SegmentedData; 
  std::string Scan1SegmentedDataTcl;
  vtkMatrix4x4 *Scan1SegmentedMatrix; 

  std::string WorkingDir;

  private:
  // int tgReadVolume(const char *fileName, vtkImageData *outputData,  vtkMatrix4x4 *outputMatrix); 

  Tcl_Interp *interp;

};

void tgWriteVolume(const char *fileName, vtkMatrix4x4 *export_matrix, vtkImageData *data) {
  vtkITKImageWriter *iwriter = vtkITKImageWriter::New(); 
  iwriter->SetInput(data); 
  iwriter->SetFileName(fileName);
  iwriter->SetRasToIJKMatrix(export_matrix);
  iwriter->SetUseCompression(1);
  iwriter->Write();
  iwriter->Delete();
}

vtksys_stl::string tgGetSLICER_HOME(char** argv)  
{ 
  vtksys_stl::string slicerHome = "";
  if ( !vtksys::SystemTools::GetEnv("SLICER_HOME", slicerHome) )
  {
    std::string programPath;
    std::string errorMessage;
    if ( !vtksys::SystemTools::FindProgramPath(argv[0], programPath, errorMessage) ) return slicerHome;

    slicerHome = vtksys::SystemTools::GetFilenamePath(programPath.c_str()) + "/../../../";
  } 
  return slicerHome;
}

int tgSetSLICER_HOME(char** argv)  
{ 
  vtksys_stl::string slicerHome = "";
  if ( !vtksys::SystemTools::GetEnv("SLICER_HOME", slicerHome) )
  {
    std::string programPath;
    std::string errorMessage;

    if ( !vtksys::SystemTools::FindProgramPath(argv[0], programPath, errorMessage) ) return 1;

    std::string homeEnv = "SLICER_HOME=";
    homeEnv += vtksys::SystemTools::GetFilenamePath(programPath.c_str()) + "/../../../";
   
    cout << "Set environment: " << homeEnv.c_str() << endl;
    vtkKWApplication::PutEnv(const_cast <char *> (homeEnv.c_str()));
  } else {
    cout << "SLICER_HOME found: " << slicerHome << endl;
  }
  return 0;
}

void  _Print(vtkImageData *DATA,::ostream& os)  {
  vtkIndent indent;
  // DATA->PrintSelf(os,indent.GetNextIndent());
  os << indent <<  "Origin " << DATA->GetOrigin()[0] << " "  <<  DATA->GetOrigin()[1] << " "  <<  DATA->GetOrigin()[2] << endl;
  os << indent <<  "Extent " << DATA->GetExtent()[0] << " "  <<  DATA->GetExtent()[1] << " "  <<  DATA->GetExtent()[2] << " " 
     << DATA->GetExtent()[3] << " "  <<  DATA->GetExtent()[4] << " "  <<  DATA->GetExtent()[5] << endl; 
  os << indent <<  "Spacing " << DATA->GetSpacing()[0] << " "  <<  DATA->GetSpacing()[1] << " "  <<  DATA->GetSpacing()[2] << endl;
}

void InitializeThresholds(tgCMDLineStructure&,vtkImageData*, vtkImageData*, int&, int&,vtkMatrix4x4*);
void CalculateAlternativeMeasure(tgCMDLineStructure&, vtkImageData*,vtkImageData*,vtkImageData*,double,vtkMatrix4x4*);
void InitializeTransformFromFile(const char*, vtkMatrix4x4*);

int main(int argc, char* argv[])
{
  //
  // parse arguments using the CLP system; this creates variables.
  PARSE_ARGS;

  // needed to tell CTest not to truncate the console output
  cout << endl << "ctest needs: CTEST_FULL_OUTPUT" << endl;  

  cout << "Parameters: " << endl;
  cout << "  scan1: " << tgScan1 << endl;
  cout << "  scan2: " << tgScan2 << endl;
  cout << "  scan1 segmentation: " << tgScan1segmented << endl;
  cout << "  scan1tfm: " << scan1tfm << endl;
  cout << "  scan2tfm: " << scan1tfm << endl;
  cout << "  tmp dir name: " << tmpDirName << endl;
  if(tgROIXYZ.size()==3)
    cout << "  ROI center: " << tgROIXYZ[0] << "," << tgROIXYZ[1] << "," << tgROIXYZ[2] << endl;
  else
    cout << "  ROI center not provided" << endl;
  if(tgROIRadius.size()==3)
    cout << "  ROI radius: " << tgROIRadius[0] << "," << tgROIRadius[1] << "," << tgROIRadius[2] << endl;
  else
    cout << "  ROI radius not provided" << endl;

  try { 
    // -------------------------------------
    // Initialize TCL  Script
    // -------------------------------------
    Tcl_Interp *interp = vtkKWApplication::InitializeTcl(argc, argv, &cout);
    if (!interp)
    {
      cout << "Error: InitializeTcl failed" << endl;
      return EXIT_FAILURE; 
    }
    vtkKWApplication *app   = vtkKWApplication::New();
    
    // This is necessary to load in ChangeTracker package in TCL interp.
    Changetracker_Init(interp);
    Vtkteem_Init(interp);
    Vtkitk_Init(interp);


    // SLICER_HOME
    cout << "Setting SLICER home: " << endl;
    vtksys_stl::string slicerHome = tgGetSLICER_HOME(argv);
    if(!slicerHome.size())
    {
      cout << "Error: Cannot find executable" << endl;
      return EXIT_FAILURE; 
    }
    cout << "Slicer home is " << slicerHome << endl;

    vtkChangeTrackerLogic  *logic = vtkChangeTrackerLogic::New();
    logic->SetModuleName("ChangeTracker");
    std::string logicTcl = vtksys::SystemTools::DuplicateString(vtkKWTkUtilities::GetTclNameFromPointer(interp,logic));
    logic->SourceAnalyzeTclScripts(app);
 
    // -------------------------------------
    // Load Parameters for pipeline 
    // -------------------------------------
    tgCMDLineStructure tg(interp);   
    // Create Working Directory 
    tg.SetWorkingDir(tmpDirName); 
    std:: cout << "Working dir: " << tg.GetWorkingDir() << std::endl;

    tgVtkDefineMacro(Scan2LocalNormalized,vtkImageData);
    tgVtkDefineMacro(Scan2Local,vtkImageData); 
    tgVtkDefineMacro(Scan1PreSegment,vtkImageThreshold); 
    tgVtkDefineMacro(Scan1PreSegmentImage,vtkImageData);
    tgVtkDefineMacro(Scan1Segment,vtkImageIslandFilter); 
    tgVtkDefineMacro(Scan1SegmentOutput,vtkImageData);
    tgVtkDefineMacro(Scan1SuperSample,vtkImageData);  
    tgVtkDefineMacro(Scan1SegmentationSuperSample,vtkImageData); 
    tgVtkDefineMacro(Scan2SuperSample,vtkImageData); 

    tgVtkDefineMacro(Scan2Global,vtkImageData); 

    vtkMatrix4x4 *scan1TfmMatrix = vtkMatrix4x4::New();
    vtkMatrix4x4 *scan2TfmMatrix = vtkMatrix4x4::New();
    vtkMatrix4x4 *supersampleMatrix = vtkMatrix4x4::New(); 


    if (tg.SetScan1Data(tgScan1.c_str())) 
      {
      cerr << "ERROR: Failed to read Scan 1" << endl;
      return EXIT_FAILURE;
      }

    if (tg.SetScan2Data(tgScan2.c_str())) 
      {
      cerr << "ERROR: Failed to read Scan 2" << endl;
      return EXIT_FAILURE;
      }

    if (tg.SetScan1SegmentedData(tgScan1segmented.c_str()))
      {
      cerr << "ERROR: Failed to read Scan 1 segmentation" << endl;
      return EXIT_FAILURE;
      }

    if (TerminationStep && tgOutput=="")
      {
      cerr << "ERROR: non-zero termination step implies non-empty output image" << endl;
      return EXIT_FAILURE;
      }
    
    if (!tg.Scan1Data || !tg.Scan2Data || !tg.Scan1SegmentedData) 
      {
      cerr << "ERROR: --scan1, --scan2 and --scan1segmented have to be defined" << endl;
      return EXIT_FAILURE; 
      }
       
    if ((tgROIXYZ.size() != 3) || (tgROIRadius.size() != 3) ) {
      cerr << "ERROR: --ROIMin or --ROIMax are not corretly defined!" << endl;
      return EXIT_FAILURE; 
    }

    
    std::string Scan1SuperSampleFileName = tg.WorkingDir + "/TG_scan1_SuperSampled.nhdr";
    std::string Scan2LocalNormalizedFileName = tg.WorkingDir + "/TG_scan2_norm.nhdr";
    std::string Scan1SegmentFileName = tg.WorkingDir + "/TG_scan1_Segment.nhdr";
    
    scan1TfmMatrix->Identity();
    scan2TfmMatrix->Identity();

    if(scan1tfm.size())
      InitializeTransformFromFile(scan1tfm.c_str(), scan1TfmMatrix);
    if(scan2tfm.size())
      InitializeTransformFromFile(scan2tfm.c_str(), scan2TfmMatrix);

    // supersample ROIs
    //
    //
    // Necessary for creating matrix with correct origin
    // 
    double *Spacing;
    double SuperSampleSpacing; 
    double SuperSampleVol;     
    double Scan1Vol;     
    double SuperSampleRatio;
    int ROIMin[3], ROIMax[3];
    double ROIXYZ[3], ROIRadius[3];
    ROIXYZ[0] = tgROIXYZ[0];
    ROIRadius[0] = tgROIRadius[0];
    ROIXYZ[1] = tgROIXYZ[1];
    ROIRadius[1] = tgROIRadius[1];
    ROIXYZ[2] = tgROIXYZ[2];
    ROIRadius[2] = tgROIRadius[2];

    {
         Spacing =  tg.Scan1Data->GetSpacing();
         std::cout << "Spacing: " << Spacing[0] << " " << Spacing[1] << " " << Spacing[2] << std::endl;
             
         SuperSampleSpacing = logic->DefineSuperSampleSize(Spacing, ROIMin, ROIMax, 0.5, RESCHOICE_ISO);
         SuperSampleVol     = SuperSampleSpacing*SuperSampleSpacing*SuperSampleSpacing;
         Scan1Vol           = (Spacing[0]*Spacing[1]*Spacing[2]);
         SuperSampleRatio   = SuperSampleVol/Scan1Vol;

    }

    Spacing =  tg.Scan1Data->GetSpacing();
    // TODO: pass resampling const in the cmdline
    cerr << "Super sample size defined to be " << SuperSampleSpacing << endl;
    vtkMatrix4x4 *outputMatrix = vtkMatrix4x4::New();
    if (logic->CreateSuperSampleRASFct(tg.Scan1Data,ROIXYZ, ROIRadius, SuperSampleSpacing,Scan1SuperSample, tg.Scan1Matrix,
                                       scan1TfmMatrix, outputMatrix)) {
      cerr << "ERROR: Could not super sample scan1 " << endl;
      return EXIT_FAILURE; 
    }
    outputMatrix->Delete();
    if (logic->CreateSuperSampleRASFct(tg.Scan2Data,ROIXYZ, ROIRadius, SuperSampleSpacing,Scan2SuperSample, tg.Scan2Matrix,
                                       scan2TfmMatrix, supersampleMatrix)) {
      cerr << "ERROR: Could not super sample scan1 " << endl;
      return EXIT_FAILURE; 
    }
    cerr << "Input threshold: " << tgThreshold[0] << ", " << tgThreshold[1] << endl;
    // this one with nn interpolator

    if(!tgThreshold[0] && !tgThreshold[1]){
      if (logic->CreateSuperSampleRASFct(tg.Scan1SegmentedData, ROIXYZ, ROIRadius, SuperSampleSpacing,Scan1PreSegmentImage, tg.Scan1SegmentedMatrix,
                                      scan1TfmMatrix, supersampleMatrix, false)) {
        cerr << "ERROR: Could not super sample scan1 segmentation" << endl;
        return EXIT_FAILURE; 
      }
      // run island removal on the input segmentation
      int range[2] = {1,255}; // assume label value is under 255    
      vtkChangeTrackerLogic::DefinePreSegment(Scan1PreSegmentImage,range,Scan1PreSegment);
    } else {
      // threshold input data, use that as segmentation
      cerr << "Defining segmentation by thresholding" << endl;
      int range[2] = {tgThreshold[0],tgThreshold[1]};
      vtkChangeTrackerLogic::DefinePreSegment(Scan1SuperSample,range,Scan1PreSegment);
    }

    vtkChangeTrackerLogic::DefineSegment(Scan1PreSegment->GetOutput(),Scan1Segment);
    Scan1SegmentOutput->DeepCopy(Scan1Segment->GetOutput());

    if(!tgThreshold[0] && !tgThreshold[1])
      InitializeThresholds(tg, Scan1SuperSample, Scan1SegmentOutput, tgThreshold[0], tgThreshold[1],supersampleMatrix);

    // normalize intensities to scan1
    std::string CMD = "::ChangeTrackerTcl::HistogramNormalization_FCT " + Scan1SuperSampleTcl + " " + Scan1SegmentOutputTcl + " " 
      + Scan2SuperSampleTcl + " " + Scan2LocalNormalizedTcl;
    app->Script(CMD.c_str()); 
    
    // determine the thresholds as the intensity range of the first scan
    // masked by the tumor label
    cerr << "Thresholds for intensity analysis: " << tgThreshold[0] << ", " << tgThreshold[1] << endl;

    // run the analysis
    char parameters[100];
    sprintf(parameters, " %f %d %d" ,tgSensitivity, tgThreshold[0], tgThreshold[1]);
    CMD = "::ChangeTrackerTcl::Analysis_Intensity_CMD " + logicTcl + " " + Scan1SuperSampleTcl + " " + Scan1SegmentOutputTcl + " " +  Scan2LocalNormalizedTcl + " " + parameters ;
    
    // ------------- INTENSITY ANALYSIS  --------------------
    double Analysis_Intensity_Growth = -1;
    double Analysis_Intensity_Shrink = -1;
    double Analysis_Intensity_Total = -1;   
    app->Script(CMD.c_str());

    cout << "=========================" << endl;    
    logic->MeassureGrowth(tgThreshold[0], tgThreshold[1], Analysis_Intensity_Shrink, Analysis_Intensity_Growth,Scan1SegmentOutput);
    
    Analysis_Intensity_Total = Analysis_Intensity_Growth + Analysis_Intensity_Shrink; 
    CMD = tg.WorkingDir + "/TG_Analysis_Intensity.nhdr";
    tgWriteVolume(CMD.c_str(),supersampleMatrix,logic->GetAnalysis_Intensity_ROIBinDisplay());

    cout << "Intensity analysis sensitivity: " << tgSensitivity << endl;
    cout << "Analysis Intensity: Shrinkage " << -Analysis_Intensity_Shrink << " Growth " << Analysis_Intensity_Growth << " Total " <<  Analysis_Intensity_Total << "Super sample " << SuperSampleVol << endl;

    cout << "Intensity Metric:\n" << endl;
    cout << "  Shrinkage: " << - Analysis_Intensity_Shrink *SuperSampleVol << " mm^3 (" << int(- Analysis_Intensity_Shrink *SuperSampleRatio) << " Voxels)" << endl;
    cout << "  Growth: " << Analysis_Intensity_Growth *SuperSampleVol << " mm^3 (" << int( Analysis_Intensity_Growth *SuperSampleRatio) << " Voxels)" << endl;
    cout << "  Total change: " << Analysis_Intensity_Total *SuperSampleVol << " mm^3 (" << int( Analysis_Intensity_Total *SuperSampleRatio) << "Voxels)" << endl;

    // 
    // ------------- ANALYZE TYPE: DEFORMABLE  --------------------
    //
    double Analysis_SEGM_Growth = -1; 
    double Analysis_JACO_Growth = -1; 

    tgWriteVolume(Scan1SuperSampleFileName.c_str(), supersampleMatrix, Scan1SuperSample);
    tgWriteVolume(Scan2LocalNormalizedFileName.c_str(), supersampleMatrix, Scan2LocalNormalized);
    tgWriteVolume(Scan1SegmentFileName.c_str(), supersampleMatrix, Scan1SegmentOutput);

//    CMD = "/projects/birn/fedorov/projects/BSF-Meningioma/Code/ChangeTrackerExtra/ChangeDetection/bin2/LocalThresholding "+
//      Scan1SegmentFileName + " " + Scan1SuperSampleFileName + " " + Scan2LocalNormalizedFileName +
//      " " + tg.WorkingDir;
//    std::cout << "New analysis: " << CMD << std::endl;
//    system(CMD.c_str());

    if (tgDeformableAnalysisFlag) { 

      std::string SCAN1_TO_SCAN2_SEGM_NAME           = tg.WorkingDir + "/TG_Deformable_Scan1SegmentationAlignedToScan2.nrrd";
      std::string SCAN1_TO_SCAN2_DEFORM_NAME         = tg.WorkingDir + "/TG_Deformable_Deformation_1-2.nrrd";
      std::string SCAN2_TO_SCAN1_DEFORM_NAME = tg.WorkingDir + "/TG_Deformable_Deformation_2-1.nrrd";
      std::string SCAN1_TO_SCAN2_RESAMPLED_NAME      = tg.WorkingDir + "/TG_Deformable_Scan1AlignedToScan2.nrrd";
      std::string SCAN2_TO_SCAN1_RESAMPLED_NAME      = tg.WorkingDir + "/TG_Deformable_Scan2AlignedToScan1.nrrd";
      std::string ANALYSIS_SEGM_FILE                 = tg.WorkingDir + "/Analysis_Deformable_Sementation_Result.txt";    
      std::string ANALYSIS_JACOBIAN_FILE             = tg.WorkingDir + "/Analysis_Deformable_Jacobian_Result.txt";  
      std::string ANALYSIS_JACOBIAN_IMAGE            = tg.WorkingDir + "/Analysis_Deformable_Jacobian.nrrd";  


      std::string CMD =  "::ChangeTrackerTcl::Analysis_Deformable_Fct " + Scan1SuperSampleFileName + " " + Scan1SegmentFileName + " " + Scan2LocalNormalizedFileName + " "
                                                                  + SCAN1_TO_SCAN2_SEGM_NAME + " " + SCAN1_TO_SCAN2_DEFORM_NAME + " " 
                                                                      + SCAN2_TO_SCAN1_DEFORM_NAME + " " + SCAN1_TO_SCAN2_RESAMPLED_NAME + " "  
                                                                  + ANALYSIS_SEGM_FILE + " " + ANALYSIS_JACOBIAN_FILE + " " + ANALYSIS_JACOBIAN_IMAGE + " " 
                                                                  + SCAN2_TO_SCAN1_RESAMPLED_NAME;
      //cout << CMD.c_str() << endl;
      cout << "=======" << endl;
   
      app->Script(CMD.c_str());

      CMD =  "lindex [::ChangeTrackerTcl::ReadASCIIFile " + ANALYSIS_SEGM_FILE +"] 0";
      Analysis_SEGM_Growth = atof(app->Script(CMD.c_str()));
      cout << "Segmentation Result " << Analysis_SEGM_Growth <<endl;; 

      CMD =  "lindex [::ChangeTrackerTcl::ReadASCIIFile " + ANALYSIS_JACOBIAN_FILE +"] 0";
      Analysis_JACO_Growth = atof(app->Script(CMD.c_str()));
      cout << "Jacobian Result: " << Analysis_JACO_Growth << endl;
      
    } 

    // 
    // ------------- Print Out Results --------------------
    // 
    if (saveResultsLog) {
      std::string fileName = tg.WorkingDir + "/AnalysisOutcome.log";
      std::ofstream outFile(fileName.c_str());
      if (outFile.fail()) {
        cout << "Error: Cannot write to file " << fileName.c_str() << endl;
        return EXIT_FAILURE;
      }
 
      outFile  << "This file was generated by vtkMrmChangeTrackerNode " << "\n";
      outFile << "Date:    " << app->Script("puts [clock format [clock seconds]]") << "\n";
      outFile  << "Scan1_Ref: " << tgScan1.c_str()     << "\n";
      outFile  << "Scan2_Ref: " << tgScan2.c_str()     << "\n";
      outFile  << "ROI:" << endl;
      outFile  << "  ROI Center: " << tgROIXYZ[0] << " " << tgROIXYZ[1] << " " << tgROIXYZ[2] << "\n";
      outFile  << "  ROI Radius: " << tgROIRadius[0] << " " << tgROIRadius[1] << " " << tgROIRadius[2] << "\n";
      outFile  << "Threshold: [" << tgThreshold[0] <<", " << tgThreshold[1] << "]\n";
      if (tgIntensityAnalysisFlag) {

        outFile  << "Analysis based on Intensity Pattern" << "\n";
        outFile  << "  Sensitivity:      " << tgSensitivity << "\n";
        outFile  << "  Shrinkage:        " << floor(-Analysis_Intensity_Shrink *1000 *SuperSampleVol)/1000.0 << "mm^3 (" 
         << int(-Analysis_Intensity_Shrink*SuperSampleRatio) << " Voxels)" << "\n";
        outFile  << "  Growth:           " << floor(Analysis_Intensity_Growth *1000 *SuperSampleVol)/1000.0 << "mm^3 (" 
         << int(Analysis_Intensity_Growth*SuperSampleRatio) << " Voxels)" << "\n";
        outFile  << "  Total Change:     " << floor(Analysis_Intensity_Total *1000 *SuperSampleVol)/1000.0 << "mm^3 (" 
         << int(Analysis_Intensity_Total*SuperSampleRatio) << " Voxels)" << "\n";
      }
      if (tgDeformableAnalysisFlag) { 
        outFile  << "Analysis based on Deformable Map" << "\n";
        outFile  << "  Segmentation Metric: "<<  floor(Analysis_SEGM_Growth*1000)/1000.0 << "mm^3 (" 
                 << int(Analysis_SEGM_Growth/Scan1Vol) << " Voxels)\n";
        outFile  << "  Jacobian Metric:     "<<  floor(Analysis_JACO_Growth*1000)/1000.0 << "mm^3 (" << int(Analysis_JACO_Growth/Scan1Vol ) << " Voxels)\n";
      }
      outFile  << "\n" << endl;
      std::string CMD;
      for (int i = 0 ; i < argc ; i ++) {
        CMD.append(argv[i]);
        CMD.append(" ");
      }
      outFile  << "PWD:      " <<  getenv("PWD") << "\n";
      outFile  << "CMD:      " << CMD.c_str() << "\n";

      outFile.close();
    }

    // 
    // ------------- CLEAN UP --------------------
    // 

    // Delete all instances
    if (Scan2Local)       Scan2Local->Delete();
    if (Scan2LocalNormalized)  Scan2LocalNormalized->Delete();
    if (Scan1PreSegment)  Scan1PreSegment->Delete();
    if (Scan1Segment)     Scan1Segment->Delete();
    if (Scan1SuperSample) Scan1SuperSample->Delete();
    if (Scan2SuperSample) Scan2SuperSample->Delete();
    if (Scan2Global)      Scan2Global->Delete();
    if (Scan1SegmentOutput) Scan1SegmentOutput->Delete();
    if (Scan1SegmentationSuperSample) Scan1SegmentationSuperSample->Delete();
    if (Scan1PreSegmentImage) Scan1PreSegmentImage->Delete();

    supersampleMatrix->Delete();
    scan1TfmMatrix->Delete();
    scan2TfmMatrix->Delete();
    logic->Delete();
    app->Delete();
    
    logic = NULL;
    app = NULL;

    cerr << "Success so far!" << endl;
  } catch (...){
    cerr << "There was some error!" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

void InitializeThresholds(tgCMDLineStructure &tg, vtkImageData* image, vtkImageData* segm, int &thrMin, int &thrMax, vtkMatrix4x4* m){
    vtkImageEuclideanDistance* dist = vtkImageEuclideanDistance::New();
    vtkImageMathematics* mult = vtkImageMathematics::New();
    vtkImageThreshold* thresh = vtkImageThreshold::New();
    vtkImageThreshold* thresh1 = vtkImageThreshold::New();
    vtkImageCast *cast1 = vtkImageCast::New();
    vtkImageCast *cast = vtkImageCast::New();
    vtkImageAccumulate* hist = vtkImageAccumulate::New();
    
    thresh->SetInput(segm);
    thresh->ThresholdBetween(1,100);
    thresh->SetInValue(1);
    thresh->SetOutValue(0);
    thresh->Update();
    
    dist->SetInput(thresh->GetOutput());
    dist->SetAlgorithmToSaito();
    dist->SetMaximumDistance(15);
    dist->ConsiderAnisotropyOff();
    dist->Update();

    thresh1->SetInput(dist->GetOutput());
    thresh1->ThresholdBetween(1,2);
    thresh1->SetInValue(1);
    thresh1->SetOutValue(0);
    thresh1->Update();

    cast1->SetInput(thresh1->GetOutput());
    cast1->SetOutputScalarTypeToShort();
    cast1->Update();

    mult->SetInput(0, image);
    mult->SetInput(1, cast1->GetOutput());
    mult->SetOperationToMultiply();

    cast->SetInput(mult->GetOutput());
    cast->SetOutputScalarTypeToShort();
  
    hist->SetInput(cast->GetOutput());
    hist->IgnoreZeroOn();
    hist->Update();

    hist->SetComponentOrigin(0.,0.,0.);
    hist->SetComponentExtent((int)hist->GetMin()[0],(int)hist->GetMax()[0],0,0,0,0);
    hist->SetComponentSpacing(1.,0.,0.);
    hist->IgnoreZeroOn();
    hist->Update();

    thrMin = (int)(hist->GetMin()[0]);
    thrMax = (int)(hist->GetMax()[0]);

    int idx = (int)(hist->GetMin()[0]);
    float mean = 0, stdev = 0, cnt = 0;
    for(idx=(int)(hist->GetMin()[0]); idx<hist->GetMax()[0]; idx++)
      {
      mean += hist->GetOutput()->GetScalarComponentAsFloat(idx,0,0,0)*(float)idx;
      cnt += hist->GetOutput()->GetScalarComponentAsFloat(idx,0,0,0);
      }
    mean = mean/cnt;

    for(idx=(int)(hist->GetMin()[0]);
        idx<(int)(hist->GetMax()[0]);
        idx++){
      float d = mean - idx;
      stdev += d*d*hist->GetOutput()->GetScalarComponentAsFloat(idx,0,0,0);
    }

    stdev = sqrt(stdev/cnt);
    thrMin = (int)(floor(mean));

    dist->Delete();
    mult->Delete();
    thresh->Delete();
    thresh1->Delete();
    cast->Delete();
    cast1->Delete();
    hist->Delete();
 
//    std::cerr << "Histogram min: " << thrMin << std::endl;
//    std::cerr << "Histogram max: " << thrMax << std::endl;
//    std::cerr << "Histogram mean: " << mean << std::endl;
//    std::cerr << "Histogram STD: " << stdev << std::endl;

}

void InitializeTransformFromFile(const char* fname, vtkMatrix4x4* m){
  int i, j;
  typedef itk::MatrixOffsetTransformBase<double,3,3> DoubleMatrixOffsetType;
  typedef itk::MatrixOffsetTransformBase<float,3,3> FloatMatrixOffsetType;
  typedef itk::TransformFileReader TransformReader;
  typedef itk::AffineTransform<double> TransformType;
  // from MRMLTransformStorageNode
  vtkMatrix4x4 *lps2ras = vtkMatrix4x4::New();
  lps2ras->Identity();
  (*lps2ras)[0][0] = (*lps2ras)[1][1] = -1.0;

  vtkMatrix4x4 *ras2lps = vtkMatrix4x4::New();
  ras2lps->Identity();
  (*ras2lps)[0][0] = (*ras2lps)[1][1] = -1.0;

  TransformType::Pointer transform = TransformType::New();

  TransformReader::Pointer tfmReader = TransformReader::New();
  tfmReader->SetFileName(fname);
  tfmReader->Update();

  TransformReader::TransformType::Pointer tfm = *(tfmReader->GetTransformList()->begin());
  DoubleMatrixOffsetType::Pointer da = dynamic_cast<DoubleMatrixOffsetType*>(tfm.GetPointer());
  FloatMatrixOffsetType::Pointer fa = dynamic_cast<FloatMatrixOffsetType*>(tfm.GetPointer());
  if (da)
    {
    for(i=0;i<3;i++){
      for(j=0;j<3;j++){
        (*m)[i][j] = da->GetMatrix()[i][j];
      }
      (*m)[i][3] = da->GetOffset()[i];
    }
    }
  else if (fa)
    {
    for(i=0;i<3;i++){
      for(j=0;j<3;j++){
        (*m)[i][j] = da->GetMatrix()[i][j];
      }
      (*m)[i][3] = da->GetOffset()[i];
    }
    }
  else
    {
    std::cout << "Initial transform is of an unsupported type" << std::endl;
    }

  vtkMatrix4x4::Multiply4x4(lps2ras, m, m);
  vtkMatrix4x4::Multiply4x4(m, ras2lps, m);
  m->Invert();

  std::cout << "Initial transform: ";
  vtkIndent indent;
  m->PrintSelf(std::cerr, indent);
  std::cout << "==================" << std::endl;
  lps2ras->Delete();
  ras2lps->Delete();
}
