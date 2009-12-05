#include <iostream>

#include <bmScriptParser.h>

#include <itksys/Glob.hxx>
#include <itksys/SystemTools.hxx>
#include <itksys/Process.h>

#include "SlicerBatchMakeConfig.h"
#include "DiffusionTensorEstimationBatchMakeModuleCLP.h"
#include <fstream>
namespace bm
{

// Create a progress manager for slicer
class ProgressManagerSlicer : public ProgressManager
{
public:
  ProgressManagerSlicer() 
    {
    m_RecentOutput = "";
    m_Progress = 0;
    m_NumberOfActions = 1;
    m_CurrentAction = 0;
    };

  ~ProgressManagerSlicer() {};
  virtual void SetStatus(MString status)
    {
    }

  virtual void IsRunning()
    {
    }
  virtual void AddAction(MString name)
    {
    std::cout << "<filter-start>" << std::endl;
    std::cout << " <filter-name>Executable" << m_CurrentAction 
                                            << "</filter-name>" << std::endl;
    std::cout << " <filter-comment>" << m_RecentOutput.c_str() 
                                     << "</filter-comment>" << std::endl;
    std::cout << "</filter-start>" << std::endl;

    m_CurrentAction++;
    m_Progress++;
    std::cout << "<filter-progress>" << std::endl;
    std::cout << m_Progress/m_NumberOfActions << std::endl;
    std::cout << "</filter-progress>" << std::endl;
    }

  virtual void FinishAction(MString output)
    {
    std::cout << "<filter-end>" << std::endl;
    std::cout << " <filter-name>Executable" << m_CurrentAction 
                                            << " </filter-name>" << std::endl;
    std::cout << " <filter-time>0</filter-time>" << std::endl;
    std::cout << "</filter-end>" << std::endl;
    }
  virtual void AddOutput(MString output)
    {
    }
  virtual void AddError(MString output)
    {
    }
  virtual void SetFinished(MString message)
    {
    }
  virtual void DisplayOutput(MString message)
    {
    m_RecentOutput = message.toChar();
    }
  virtual void DisplayError(MString message)
    {
    }

  void SetNumberOfActions(unsigned long actions)
    {
    m_NumberOfActions = actions;
    }

protected:

  std::string m_RecentOutput;
  int m_CurrentAction;
  float m_Progress;
  unsigned long m_NumberOfActions;
};

} // end namespace

int main(int argc, char* argv[])
{
  PARSE_ARGS;

  // ----------------------------------------------------------
  // Write the script. This file will be saved to disk to control
  // the batchmake run.
  // ----------------------------------------------------------
  std::string script = "echo('Starting BatchMake Script')\n";
  
  // Input data files to loop over
  char *buffer = new char[512];
  sprintf(buffer,"glob(inputFiles %s/%s)\n",
          dataDir.c_str(), dataMask.c_str());
  script += buffer;
  delete [] buffer;

  script += "setapp(dtapp @DiffusionTensorEstimation)\n";

  // this is from gaussian blur not sure if needed:
  // julien's add
  script += "InputDirectory('" + dataDir + "')\n";
  script += "OutputDirectory( '" + outputDir + "')\n";
  script += "WorkingDirectory( '" + outputDir + "')\n";
  script += "ExecutableDirectory('/home/condor/applications/')\n";
  script += "GridTransferFile( NONE )\n";
  // end julien's add


  // Scripted loop over input files
  // -------------------------
  script += "foreach(file ${inputFiles})\n";
  script += "    GetFilename(outputName ${file} NAME_WITHOUT_EXTENSION)\n";
  script += "    GetFilename(outputExtension ${file} EXTENSION)\n";

  script += "    Set(outputFilename "+outputDir+"/${outputName}-DTI.nhdr)\n";
  script += "    Set(outputBaseline "+outputDir+"/${outputName}-B0.nhdr)\n";
  script += "    Set(outputMask "+outputDir+"/${outputName}-mask.nhdr)\n";

  script += "    echo('Processing file: '${file})\n";

  std::stringstream out1;
  std::stringstream out2;
  std::stringstream out3;
  std::stringstream out4;
  
  // Parameters to the tensor estimation method that are set at run time
  script += "    setappoption(dtapp.e " + estimationMethod + ")\n";
  out1 << ShiftNegativeEigenvalues;
  script += "    setappoption(dtapp.shiftNeg " + out1.str() + ")\n";
  out2 << otsuOmegaThreshold;
  script += "    setappoption(dtapp.o " + out2.str() + "})\n";
  out3 << removeIslands;
  script += "    setappoption(dtapp.removeislands " + out3.str() + "})\n";
  out4 << applyMask;
  script += "    setappoption(dtapp.applymask " + out4.str() + "})\n";

  // IO information
  script += "    setappoption(dtapp.inputVolume ${file})\n";
  script += "    setappoption(dtapp.outputTensor ${outputFilename})\n";
  script += "    setappoption(dtapp.outputBaseline ${outputBaseline})\n";
  script += "    setappoption(dtapp.thresholdMask ${outputMask})\n";

  // Info to run module on this input set
  script += "    run(output ${dtapp})\n";
  
  script += "endforeach(file inputFiles)\n";
  // End of script's loop over input files (above)
  // -------------------------


  // Write script to disk to execute the batch
  // -------------------------
  std::string scriptFile = outputDir
                           + "/SlicerDiffusionTensorEstimationBatchMakeModule.bms";
  std::ofstream file( scriptFile.c_str() );
  file << script; 
  file.close();


  // Create a progress manager gui
  // -------------------------
  bm::ScriptParser batchMakeParser;
  bm::ProgressManagerSlicer progressManager;
  
  // Come up with an approximation of the number of Run()
  // by counting number of input files
  itksys::Glob glob;
  if(dataMaskRecursive)
    {
    glob.RecurseOn();
    }
  std::string convertedGlobExpression = dataDir;
  convertedGlobExpression += "/";
  convertedGlobExpression += dataMask;
  itksys::SystemTools::ConvertToUnixSlashes(convertedGlobExpression);
  glob.FindFiles(convertedGlobExpression);
  unsigned int actions = glob.GetFiles().size();

  progressManager.SetNumberOfActions((actions*2)+10);
  batchMakeParser.SetProgressManager(&progressManager);

  batchMakeParser.LoadWrappedApplication(BatchMake_WRAPPED_APPLICATION_DIR);
  batchMakeParser.SetBatchMakeBinaryPath(BatchMake_WRAPPED_APPLICATION_DIR);
  
  if(!runUsingCondor)
    {
    // If we want to run the script locally
    std::cout << "<filter-start>" << std::endl;
    std::cout << " <filter-name>LocalSubmit</filter-name>" << std::endl;
    std::cout << " <filter-comment>Procsssing data on local machine</filter-comment>" << std::endl;
    std::cout << "</filter-start>" << std::endl;

    batchMakeParser.ParseBuffer(script);

    std::cout << "<filter-end>" << std::endl;
    std::cout << " <filter-name>LocalSubmit</filter-name>" << std::endl;
    std::cout << " <filter-time>0</filter-time>" << std::endl;
    std::cout << "</filter-end>" << std::endl;
    }
  else
    {
    // If we want to run the script on condor

    std::cout << "<filter-start>" << std::endl;
    std::cout << " <filter-name>CondorSubmit</filter-name>" << std::endl;
    std::cout << " <filter-comment>Submitting jobs to Condor</filter-comment>" << std::endl;
    std::cout << "</filter-start>" << std::endl;

    // Run condor watcher -- requires FLTK
    //std::vector<const char*> args;
    //args.push_back(condorwatcher.c_str());
    //args.push_back(0);
    //itksysProcess* gp = itksysProcess_New();
    //itksysProcess_SetCommand(gp, &*args.begin());
    //itksysProcess_SetOption(gp,
                            //itksysProcess_Option_Detach, 1);
    //itksysProcess_Execute(gp);

    // Generate the script
    batchMakeParser.RunCondor( script );
    //                         , outputDir.c_str());
    
    std::cout << "<filter-end>" << std::endl;
    std::cout << " <filter-name>CondorSubmit</filter-name>" << std::endl;
    std::cout << " <filter-time>0</filter-time>" << std::endl;
    std::cout << "</filter-end>" << std::endl;
    }


  return EXIT_SUCCESS;
}
