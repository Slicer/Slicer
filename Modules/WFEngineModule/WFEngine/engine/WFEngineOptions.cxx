#include "WFEngineOptions.h"
#include <iostream>
//#include "GUI/WFConfigEditor.h"

using namespace std;

using namespace WFEngine::nmWFEngineOptions;
using namespace WFEngine::nmWFXmlConfigManager;

WFEngineOptions::WFEngineOptions()
{
    this->m_configIsActive = false;
    this->m_isLoaded = false;
    
    this->m_WFCfgXmlManager = NULL;     
}

WFEngineOptions::~WFEngineOptions()
{
    if(this->m_WFCfgXmlManager)
    {
        this->m_WFCfgXmlManager->Destroy();
        this->m_WFCfgXmlManager = NULL;
    }
}

WFEngineOptions* WFEngineOptions::New()
{
 WFEngineOptions *wfeOpts = new WFEngineOptions;

 cout<<"WFConfigFile needs to be load!"<<endl;
 //if(!wfeOpts->loadConfigFile(wfConfigFile)) showEditGUI = true;
 
 //if(showEditGUI)
 //{
  //create Config GUI for WFEngine
//  const WFConfigEditor *wfConfigGUI = WFConfigEditor::New(wfeOpts);
 //}
 return wfeOpts;
}

int WFEngineOptions::LoadConfigFile(std::string wfConfigFile)
{
 int retVal = 1;
 this->m_WFCfgXmlManager = WFXmlConfigManager::New();
 retVal = this->m_WFCfgXmlManager->loadConfigFile(wfConfigFile);
 if(retVal)
 {
   this->m_isLoaded = true;
 }
}

void WFEngineOptions::SetConfigFile(std::string wfConfigFile)
{
 if(wfConfigFile != "" && this->LoadConfigFile(wfConfigFile))
 {
  this->m_wfConfigFile = wfConfigFile;
  this->m_configIsActive = true;
 }
}

string WFEngineOptions::GetConfigFile()
{
 return this->m_wfConfigFile;
}

std::vector<WFXmlConfigManager::myAttrMap> WFEngineOptions::GetLookUpPaths()
{
  std::vector<WFXmlConfigManager::myAttrMap> lookUpPaths;
  if(IsLoaded())
  {
    lookUpPaths = this->m_WFCfgXmlManager->getAllLookUpPaths();
    //std::cout<<lookUpPaths.size()<<std::endl;    
  }
  
  return lookUpPaths;
}

std::vector<WFXmlConfigManager::myAttrMap> WFEngineOptions::GetKnownWorkflows()
{
  std::vector<WFXmlConfigManager::myAttrMap> knownWFs;
  if(IsLoaded())
  {
    knownWFs = this->m_WFCfgXmlManager->getAllKnownWorkflows();
    //std::cout<<knownWFs.size()<<std::endl;
  }

  return knownWFs;
}

bool WFEngineOptions::IsLoaded()
{
  return this->m_isLoaded;
}

void WFEngineOptions::RemoveKnownWorkflow(std::string &fileName)
{
  if(IsLoaded())
  {
    this->m_WFCfgXmlManager->removeKnownWorkflow(fileName);
  }
}

void WFEngineOptions::RemoveLookUpPath(std::string &path)
{
  if(IsLoaded())
  {
    this->m_WFCfgXmlManager->removeLookUpPath(path);
  }
}

void WFEngineOptions::AddKnownWorkflow(std::string &fileName)
{
  this->AddKnownWorkflow(fileName, true, true);
}

void WFEngineOptions::AddKnownWorkflow(std::string &fileName, bool visible, bool enabled)
{
  if(IsLoaded())
  {
    this->m_WFCfgXmlManager->addKnownWorkflow(fileName, visible, enabled);
  }
}

void WFEngineOptions::SaveChanges()
{
  this->m_WFCfgXmlManager->saveXmlFile();
}

void WFEngineOptions::Destroy()
{
    delete(this);
}

