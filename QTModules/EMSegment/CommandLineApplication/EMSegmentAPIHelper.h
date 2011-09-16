#include "vtkSlicerCommonInterface.h"
#include <sstream>
#include <vtksys/SystemTools.hxx>
#include <vtksys/stl/string>

#ifdef Slicer3_USE_KWWIDGETS
extern "C" int Emsegment_Init(Tcl_Interp *interp);
extern "C" int Vtkteem_Init(Tcl_Interp *interp);
extern "C" int Vtkitk_Init(Tcl_Interp *interp);
extern "C" int Slicerbaselogic_Init(Tcl_Interp *interp);
extern "C" int Mrml_Init(Tcl_Interp *interp);
extern "C" int Mrmlcli_Init(Tcl_Interp *interp); 
extern "C" int Commandlinemodule_Init(Tcl_Interp *interp);
#endif

#define tgSetDataMacro(name,matrix)                             \
  virtual int Set##name(const char *fileName) {                 \
    if (strcmp(fileName,"None")) {                              \
      tgVtkCreateMacro(this->name,vtkImageData);                \
      this->matrix = vtkMatrix4x4::New();                       \
      return tgReadVolume(fileName,this->name,this->matrix);    \
    }                                                           \
    this->name = NULL;                                          \
    this->matrix = NULL;                                        \
    std::cout << "Here" << std::endl;                           \
    return 0;                                                   \
  }


vtksys_stl::string tgGetSLICER_HOME(char** argv)  
{ 
  vtksys_stl::string slicerHome = "";
  if ( !vtksys::SystemTools::GetEnv("Slicer3_HOME", slicerHome) )
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
  if ( !vtksys::SystemTools::GetEnv("Slicer3_HOME", slicerHome) )
    {
      std::string programPath;
      std::string errorMessage;

      if ( !vtksys::SystemTools::FindProgramPath(argv[0], programPath, errorMessage) ) return 1;

      std::string homeEnv = "Slicer3_HOME=";
      homeEnv += vtksys::SystemTools::GetFilenamePath(programPath.c_str()) + "/../../../";
   
      cout << "Set environment: " << homeEnv.c_str() << endl;
      vtksys::SystemTools::PutEnv(const_cast <char *> (homeEnv.c_str()));
    } else {
    cout << "Slicer3_HOME found: " << slicerHome << endl;
  }
  return 0;
}

Tcl_Interp* CreateTclInterp(int argc, char** argv, vtkSlicerCommonInterface *slicerCommon)
{

  if (!slicerCommon)
    {
    cout << "Error: Could not get Slicer common interface." << endl;
    return NULL;
    }

  Tcl_Interp *interp = slicerCommon->Startup(argc,argv,&cout);

#ifdef Slicer3_USE_KWWIDGETS
  // This is necessary to load in EMSEgmenter package in TCL interp.
  Emsegment_Init(interp);
  Slicerbaselogic_Init(interp);
  Mrml_Init(interp);
  Mrmlcli_Init(interp); 
  Vtkteem_Init(interp);
  Vtkitk_Init(interp);
  Commandlinemodule_Init(interp);

#endif
  return interp;
}

#ifdef _WIN32
std::string StripBackslashes(const std::string& s)
{
  std::string outString;
  for (unsigned int i = 0; i < s.size(); ++i)
    {
      if (s[i] != '\\')
        {
          outString.push_back(s[i]);
        }
      else if (i > 0 && s[i-1] == '\\')
        {
          outString.push_back(s[i]);
        }
    }
  return outString;
}
#endif

vtkSlicerApplicationLogic* InitializeApplication(vtkSlicerCommonInterface *slicerCommon, int argc, char** argv)
{
  // SLICER_HOME
  cout << "Setting SLICER home.. " << endl;
  vtkstd::string slicerHome = tgGetSLICER_HOME(argv);
  if(!slicerHome.size())
    {
      cout << "Error: Cannot find executable" << endl;
      return NULL;
    }
  cout << "Slicer home is " << slicerHome << endl;

  slicerCommon->PromptBeforeExitOff();

  std::string appTcl = std::string(slicerCommon->GetApplicationTclName());
  std::ostringstream os;
  os << "namespace eval slicer3 set Application ";
  os << appTcl;
  slicerCommon->EvaluateTcl(os.str().c_str());


  vtkSlicerApplicationLogic* appLogic = vtkSlicerApplicationLogic::New();
  std::string appLogicTcl = std::string(slicerCommon->GetTclNameFromPointer(appLogic));
  std::ostringstream os2;
  os2 << "namespace eval slicer3 set ApplicationLogic ";
  os2 << appLogicTcl;
  slicerCommon->EvaluateTcl(os2.str().c_str());

  // set BinDir to make functionality like GetSvnRevision available
  std::string slicerBinDir = slicerHome + "/bin";
  slicerBinDir = vtksys::SystemTools::CollapseFullPath(slicerBinDir.c_str());
  slicerCommon->SetApplicationBinDir(slicerBinDir.c_str());

  // Make generic later 
  slicerCommon->EvaluateTcl("set ::env(KILIS_MODULE) KilisSandbox");
  //std::string CMD = std::string("set ::env(SLICER_HOME) ") + slicerHome + "/..";
  //slicerCommon->EvaluateTcl(CMD.c_str());
  std::string CMD = "";
  CMD = "set argv { "; 
  for (int i = 2 ; i < argc ; i++) CMD += std::string(argv[i]) + " ";
  CMD += " }  "; 
  slicerCommon->EvaluateTcl(CMD.c_str());
  return  appLogic;
}


void CleanUp(vtkSlicerApplicationLogic* appLogic, vtkSlicerCommonInterface *slicerCommon)
{
  if (appLogic)
    {
      appLogic->Delete();
      appLogic = 0;
    }

  if (slicerCommon)
    {
    slicerCommon->DestroySlicerApplication();
    slicerCommon->Delete();
    slicerCommon = 0;
    }

}
