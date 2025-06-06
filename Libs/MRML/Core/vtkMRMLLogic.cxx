
// MRMLLogic includes
#include "vtkMRMLLogic.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <set>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLLogic);

//------------------------------------------------------------------------------
vtkMRMLLogic::vtkMRMLLogic()
{
}

//------------------------------------------------------------------------------
vtkMRMLLogic::~vtkMRMLLogic() = default;

//----------------------------------------------------------------------------
std::string vtkMRMLLogic::GetApplicationHomeDirectory()
{
  std::string applicationHome;
  if (vtksys::SystemTools::GetEnv(MRML_APPLICATION_HOME_DIR_ENV) != nullptr)
  {
    applicationHome = std::string(vtksys::SystemTools::GetEnv(MRML_APPLICATION_HOME_DIR_ENV));
  }
  else
  {
    if (vtksys::SystemTools::GetEnv("PWD") != nullptr)
    {
      applicationHome =  std::string(vtksys::SystemTools::GetEnv("PWD"));
    }
    else
    {
      applicationHome =  std::string("");
    }
  }
  return applicationHome;
}

//----------------------------------------------------------------------------
std::string vtkMRMLLogic::GetApplicationShareDirectory()
{
  std::string applicationHome = vtkMRMLLogic::GetApplicationHomeDirectory();
  std::vector<std::string> filesVector;
  filesVector.emplace_back(""); // for relative path
  filesVector.push_back(applicationHome);
  filesVector.emplace_back(MRML_APPLICATION_SHARE_SUBDIR);
  std::string applicationShare = vtksys::SystemTools::JoinPath(filesVector);

  return applicationShare;
}
