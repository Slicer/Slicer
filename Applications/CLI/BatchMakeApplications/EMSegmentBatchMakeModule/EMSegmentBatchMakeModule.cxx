#include <iostream>

#include <bmScriptParser.h>

#include <itksys/Glob.hxx>
#include <itksys/SystemTools.hxx>
#include <itksys/Process.h>

#include "SlicerBatchMakeConfig.h"
#include "EMSegmentBatchMakeModuleCLP.h"

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
    std::cout << " <filter-comment> " << m_RecentOutput.c_str()  
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
                                            << "</filter-name>" << std::endl;
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

  // Write the script
  std::string script = "echo('Starting BatchMake Script')\n";
  script += "setapp(emsegment @EMSegmentCommandLine_GUIVersion)\n";

  char* buffer = new char[512];
  sprintf(buffer,"set(patientdir '%s')\n",dataDir.c_str());
  script += buffer;

  delete [] buffer;
  buffer = new char[512];
  sprintf(buffer,"set(outputdir '%s')\n",outputDir.c_str());
  script += buffer;

  delete [] buffer;
  buffer = new char[512];
  sprintf(buffer,"set(mrmlfile '%s')\n",mrmlSceneFileName.c_str());
  script += buffer;

  delete [] buffer;
  buffer = new char[512];
  sprintf(buffer,"set(globt1 '%s')\n",Target1Mask.c_str());
  script += buffer;

  delete [] buffer;
  buffer = new char[512];
  sprintf(buffer,"set(globt2 '%s')\n",Target2Mask.c_str());
  script += buffer;

  delete [] buffer;
  buffer = new char[512];
  sprintf(buffer,"listdirindir(patients ${patientdir} '%s')\n",DataMask.c_str());
  script += buffer;

  script += "foreach(patient ${patients})\n";
  
  script += "  set(currentdir ${patientdir}/${patient})\n";
  script += "  glob(t1image ${currentdir}/${globt1})\n";
  script += "  glob(t2image ${currentdir}/${globt2})\n";
  
  script += "  set(outputfile ${outputdir}/${patient}_labelmap.mha)\n";
  
  script += "  setappoption(emsegment.mrmlSceneFileName ${mrmlfile})\n";
  script += "  setappoption(emsegment.resultVolumeFileName ${outputfile})\n";
  script += "  setappoption(emsegment.targetVolumeFileName1 ${t1image})\n";
  script += "  setappoption(emsegment.targetVolumeFileName2 ${t2image})\n";
  script += "  Run(output ${emsegment})\n";

  script += "endforeach(patient ${patients})\n";

  // Create a progress manager gui
  bm::ScriptParser batchMakeParser;
  bm::ProgressManagerSlicer progressManager;

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
    // If we want to run the script on condor/grid

    std::cout << "<filter-start>" << std::endl;
    std::cout << " <filter-name>CondorSubmit</filter-name>" << std::endl;
    std::cout << " <filter-comment>Submitting jobs to Condor</filter-comment>" << std::endl;
    std::cout << "</filter-start>" << std::endl;
 
    // Run condor watcher
    // REQUIRES FLTK :(
    //
    // std::vector<const char*> args;
    // args.push_back(CONDORWATCHER_EXECUTABLE);
    // args.push_back(0);

    // Run the application
    // itksysProcess* gp = itksysProcess_New();
    // itksysProcess_SetCommand(gp, &*args.begin());
    // itksysProcess_SetOption(gp,
                            // itksysProcess_Option_Detach, 1);
    // itksysProcess_Execute(gp);

    // Generate the script
    batchMakeParser.RunCondor(script, outputDir.c_str());

    std::cout << "<filter-end>" << std::endl;
    std::cout << " <filter-name>CondorSubmit</filter-name>" << std::endl;
    std::cout << " <filter-time>0</filter-time>" << std::endl;
    std::cout << "</filter-end>" << std::endl;
    }

  return EXIT_SUCCESS;
}
