#include <iostream>
#include <RegisterImagesBatchMakeModuleCLP.h>
#include "bmScriptParser.h"
//#include "bmProgressManagerGUI.h"
#include "itksys/Glob.hxx"
#include "itksys/SystemTools.hxx"
#include <itksys/Process.h>
#include "RegisterImagesBatchMakeModuleConfig.h"

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
    std::cout << " <filter-name>Executable " << std::endl;
    std::cout << m_CurrentAction << std::endl;
    std::cout << "</filter-name>" << std::endl;
    std::cout << "<filter-comment> " << m_RecentOutput.c_str() << std::endl;
    std::cout << "</filter-comment>" << std::endl;
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
    std::cout << " <filter-name>Executable " << std::endl;
    std::cout << m_CurrentAction << std::endl;
    std::cout << " </filter-name>" << std::endl;
    std::cout << " <filter-time>" << std::endl;
    std::cout << " 0" << std::endl;
    std::cout << " </filter-time>" << std::endl;
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

  // Write the script
  std::string script = "echo('Starting BatchMake Script')\n";
  script += "setapp(imageregistration @ImageRegistrationCommandLine_GUIVersion)\n";

  char* buffer = new char[512];
  sprintf(buffer,"set(patientdir '%s')\n",movingImageDir.c_str());
  script += buffer;

  delete [] buffer;
  buffer = new char[512];
  sprintf(buffer,"set(outputdir '%s')\n",outputDir.c_str());
  script += buffer;

  delete [] buffer;
  buffer = new char[512];
  sprintf(buffer,"set(paramfile '%s')\n",parametersFileName.c_str());
  script += buffer;

  delete [] buffer;
  buffer = new char[512];
  sprintf(buffer,"set(fixedimage '%s')\n",fixedImage.c_str());
  script += buffer;

  delete [] buffer;
  buffer = new char[512];
  sprintf(buffer,"set(globMoving '%s')\n",movingImageMask.c_str());
  script += buffer;

  delete [] buffer;
  buffer = new char[512];
  sprintf(buffer,"listdirindir(patients ${patientdir} '%s')\n",movingImageMask.c_str());
  script += buffer;

  script += "foreach(patient ${patients})\n";
  
  script += "  set(currentdir ${patientdir}/${patient})\n";
  script += "  glob(movingImage ${currentdir}/${globMoving})\n";
  
  script += "  set(outputfile ${outputdir}/${patient}_reg.mha)\n";
  
  script += "  setappoption(imageregistration.loadParameters ${paramfile})\n";
  script += "  setappoption(imageregistration.fixedImage ${fixedImage})\n";
  script += "  setappoption(imageregistration.movingImage ${movingImage})\n";
  script += "  setappoption(imageregistration.resampledImage ${outputfile})\n";
  script += "  Run(output ${emsegment})\n";

  script += "endforeach(patient ${patients})\n";

  // Create a progress manager gui
  bm::ScriptParser batchMakeParser;
  bm::ProgressManagerSlicer progressManager;

  batchMakeParser.SetProgressManager(&progressManager);
  
  batchMakeParser.LoadWrappedApplication(BATCHMAKE_WRAPPED_APPLICATION_DIR);
  batchMakeParser.SetBatchMakeBinaryPath(BATCHMAKE_WRAPPED_APPLICATION_DIR);
  
  std::cout << "<filter-start>" << std::endl;
  std::cout << " <filter-name>CondorSubmit" << std::endl;
  std::cout << "</filter-name>" << std::endl;
  std::cout << "<filter-comment> " << std::endl;
  std::cout << "Submitting jobs to Condor" << std::endl;
  std::cout << "</filter-comment>" << std::endl;
  std::cout << "</filter-start>" << std::endl;

  // If we want to run the script locally
  if(runLocally == "locally")
    {
    batchMakeParser.ParseBuffer(script);
    }
  else
    {
    // Run condor watcher
    std::vector<const char*> args;
    args.push_back(CONDORWATCHER_EXECUTABLE);
    args.push_back(0);

    // Run the application
    itksysProcess* gp = itksysProcess_New();
    itksysProcess_SetCommand(gp, &*args.begin());
    itksysProcess_SetOption(gp,
                            itksysProcess_Option_Detach, 1);
    itksysProcess_Execute(gp);

    // Generate the script
    batchMakeParser.RunCondor(script,outputDir.c_str());
    }

  std::cout << "<filter-end>" << std::endl;
  std::cout << " <filter-name>CondorSubmit" << std::endl;
  std::cout << " </filter-name>" << std::endl;
  std::cout << " <filter-time>" << std::endl;
  std::cout << " 0" << std::endl;
  std::cout << " </filter-time>" << std::endl;
  std::cout << "</filter-end>" << std::endl;

  return EXIT_SUCCESS;
}
