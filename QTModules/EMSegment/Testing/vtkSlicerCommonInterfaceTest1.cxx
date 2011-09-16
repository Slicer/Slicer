#include "vtkSlicerCommonInterface.h"
#include "vtkEMSegmentLogic.h"
#include <vtksys/SystemTools.hxx>

// test the tcl adapter provided by the Slicer Common Interface
int main(int argc, char** argv)
{

  vtkSlicerCommonInterface* common = vtkSlicerCommonInterface::New();

  common->Startup(argc,argv,&cout);

  std::string appTcl = std::string(common->GetApplicationTclName());

  if (appTcl.size() != 8)
    {
    std::cerr << "The Tcl Name for the Application has an invalid size!";
    return EXIT_FAILURE;
    }

  std::ostringstream os;
  os << "namespace eval slicer3 set Application ";
  os << appTcl;
  common->EvaluateTcl(os.str().c_str());

  // now create the EMLogic
  vtkEMSegmentLogic* emLogic = vtkEMSegmentLogic::New();

  // get the tcl name of EMLogic
  std::string EMSLogicTcl = vtksys::SystemTools::DuplicateString(common->GetTclNameFromPointer(emLogic));

  if (EMSLogicTcl.size() != 8)
    {
    std::cerr << "The Tcl Name for EMSLogic has an invalid size!";
    return EXIT_FAILURE;
    }

  std::cout << "===========================================" << std::endl;
  // build a get command for the EMLogic
  std::string tclCommand = "";
  tclCommand += std::string(EMSLogicTcl);
  tclCommand += " GetDebug";
  std::cout << "Tcl: " << tclCommand << std::endl;


  // check if we can access the EMLogic from Tcl
  common->EvaluateTcl(tclCommand.c_str());

  // cleanup
  emLogic->Delete();

  common->DestroySlicerApplication();

  common->Delete();

  return EXIT_SUCCESS;

}
