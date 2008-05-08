#include "WFBaseEngine.h"

#include <iostream>
#include <fstream>
#include <dirent.h>
#include <errno.h>

#ifndef WFDIRECTINTERFACE
#include "WFSocketCollection.h"
#endif

#define WFCLIENT_INTF 1
#define WFOPTIONS_INTF 2

using namespace std;

using namespace WFEngine;
//{
WFBaseEngine* WFBaseEngine::sm_wfeBE = 0;

WFBaseEngine::WFBaseEngine()
{
  this->configExists = false;
}

WFBaseEngine::~WFBaseEngine()
{
    if(this->m_wfeOpts)
    {
        this->m_wfeOpts->Destroy();
        this->m_wfeOpts = NULL;
    }
#ifndef WFDIRECTINTERFACE
    if(this->m_wfeSC)
    {
        delete(this->m_wfeSC);
        this->m_wfeSC = NULL;
    }
#endif
}

WFBaseEngine* WFBaseEngine::New()
{
   return new WFBaseEngine();   
}

int WFBaseEngine::InitializeWFEngine(std::string wfConfigFile)
{
  if(wfConfigFile == "")
  {
    fstream fs_op("../Slicer3/Modules/WFEngineModule/wfConfig.xml",ios::in);
    if(!fs_op)
    {
     cout<<"wfConfig.xml not found!"<<endl;
    }
    else
    {
     cout<<"wfConfig.xml found!"<<endl;
     wfConfigFile = "../Slicer3/Modules/WFEngineModule/wfConfig.xml";
     configExists = true;
    }
    fs_op.close();
  }
  else
  {
    cout<<wfConfigFile<<endl;
    
    fstream fs_op(wfConfigFile.c_str(), ios::in);
    if(!fs_op)
    {
     cout<<wfConfigFile<<" could not be found!"<<endl;
    }
    else configExists = true;
    
    fs_op.close();
  }
  
  if(!configExists)
  {
    const string emptyStr = "";
    this->m_wfeOpts = this->InitializeWFEOpts(emptyStr);
  }
  else
  {
    this->m_wfeOpts = this->InitializeWFEOpts(wfConfigFile);
  }
     
  this->InitializeKnowWorkflows();
  
  return 1;
}

nmWFEngineOptions::WFEngineOptions *WFBaseEngine::InitializeWFEOpts(string wfConfigFile)
{
 using namespace nmWFEngineOptions;
 WFEngineOptions *wfeOpts = WFEngineOptions::New();
 wfeOpts->SetConfigFile(wfConfigFile);
 return wfeOpts;
}

void WFBaseEngine::InitializeKnowWorkflows()
{
  
  //look for existing workflow description files
  //remove those which doesn't exist anymore
  //add those if non existent which are in the lookUpPaths
  
  using namespace WFEngine::nmWFXmlConfigManager;
  std::vector<WFXmlConfigManager::myAttrMap> lookUpPaths;
  std::vector<WFXmlConfigManager::myAttrMap> knowWFs;
  
  lookUpPaths = this->m_wfeOpts->GetLookUpPaths();
  knowWFs = this->m_wfeOpts->GetKnownWorkflows();
  
  //std::cout<<"***Configuration checkup***"<<std::endl;
  
  std::vector<WFXmlConfigManager::myAttrMap>::const_iterator endi = knowWFs.end();
  std::vector<WFXmlConfigManager::myAttrMap>::const_iterator iter;
  for(iter = knowWFs.begin(); iter != endi; iter++)
  {
    WFXmlConfigManager::myAttrMap attrMap = (*iter);
    std::string fn = attrMap["fileName"];
    //std::cout<<fn;
    if(validateXMLFile(fn))
    {
        //std::cout<<" validated! Keep!"<<std::endl;   
    }      
    else
    {
        //std::cout<<" not validated! Erase!"<<std::endl;
        this->m_wfeOpts->RemoveKnownWorkflow(fn);  
    }
      
  }
  
  for(iter = lookUpPaths.begin(); iter != lookUpPaths.end(); iter++)
  {
    std::map<std::string,std::string>::const_iterator endj = iter->end();
    for(std::map<std::string,std::string>::const_iterator map_iter = iter->begin(); map_iter != iter->end(); map_iter++)
    {
      DIR *pdir;
      struct dirent *pent;
      bool success = true;
      pdir=opendir(map_iter->second.c_str()); //"." refers to the current dir
      if (!pdir)
      {
        //std::cout<<"opendir(\""<<map_iter->second<<"\") failure; terminating"<<std::endl;
        std::string path = map_iter->second;
        this->m_wfeOpts->RemoveLookUpPath(path);
        success = false;
      }
      errno=0;
      if(success)
      {
        while ((pent=readdir(pdir)))
        {
          std::string fileName(pent->d_name);
          //std::cout<<fileName<<std::endl;
          std::string pathWithFileName = map_iter->second + "/" + fileName;
          if(fileName.length() > 4)
          {
            if(fileName.substr(fileName.length()-4, fileName.length()-1) == ".xml")
            {
              if(validateXMLFile(pathWithFileName))
                this->m_wfeOpts->AddKnownWorkflow(pathWithFileName);
//              else
//                this->m_wfeOpts->RemoveKnownWorkflow(pathWithFileName);
            }
          }          
        }
        if (errno)
        {
          //std::cout<<"readdir() failure; terminating"<<std::endl;
        }
      }      
      closedir(pdir);  
    }
    //std::cout<<"***Configuration done!***"<<std::endl;
  }
  
//  entryVector::const_iterator endi = osArticle.entries.end();
//  for (entryVector::const_iterator i = osArticle.entries.begin(); i != endi;
//  ++i)
//  {
//      entryMap::const_iterator endj = i->end();
//      for (entryMap::const_iterator j = i->begin(); j != endj; ++j)
//      {
//          os << j->first << ' ' << j->second << endl;
//      }
//
//  } 
}

//void WFBaseEngine::InitializeWFFactoryClasses()
//{
// using namespace nmWFFactory;
// using namespace nmWFSocketHandler;
// using namespace nmWFClientInterface;
// 
// this->m_wfeFactory = WFFactory::GetInstance();
//
// WFSocketHandler wfSH;
// WFClientInterface wfCI;
// m_wfeFactory->registerType(wfSH, wfCI);
// //m_Factory->registerTypes();
//}

#ifndef WFDIRECTINTERFACE
void WFBaseEngine::mainInterfaceLoop()
{
 using namespace nmWFServerConnection;
 
 WFEngine::nmWFSocketCollection::WFSocketCollection sc;
 
 //add Workflow Connection
 WFServerConnection *m_wfeSC = WFServerConnection::New();
 
 m_wfeSC->setPort(6867);
 m_wfeSC->bindAndListen();
 
 sc.addSocket(m_wfeSC);
 
 //add Options Connection
 m_wfeSC = WFServerConnection::New();
 
 m_wfeSC->setPort(6868);
 m_wfeSC->setMaxConnections(2);
 m_wfeSC->bindAndListen();
 
 sc.addSocket(m_wfeSC);
 
 sc.setTimeOut(1);
 while(true)
 {
   sc.selectOnSockets();
 }
// m_wfeCLI = WFClientInterface::New(6867, 5, &WFBaseEngine::recvClientData);
 this->SaveState(); 
}

void WFBaseEngine::recvClientData(int socket, char* buffer)
{
  if(buffer == NULL) return;
   
   //std::cout<<"recvClientData:"<<std::endl;
   //std::cout<<buffer<<std::endl;
   
   //int length = sizeof("acknowledgment")+1;
   //char sendData[length];
   std::string sendData = "acknowledgment";
   sm_wfeBE->sendClientData(socket, sendData.c_str());
}

void WFBaseEngine::recvOptionsData(int socket, char* buffer)
{
  if(buffer == NULL) return;
   
   //std::cout<<"recvClientData:"<<std::endl;
   //std::cout<<buffer<<std::endl;
   
   //int length = sizeof("acknowledgment")+1;
   //char sendData[length];
   std::string sendData = "acknowledgment";
   sm_wfeBE->sendClientData(socket, sendData.c_str());
}

void WFBaseEngine::sendClientData(int socket, std::string buffer)
{
  this->m_wfeSC->sendDataToConnection(socket, buffer);
}

#endif

void WFBaseEngine::SaveState()
{
  this->m_wfeOpts->SaveChanges();
}

bool WFBaseEngine::validateXMLFile(std::string &fileName)
{
  std::ifstream file(fileName.c_str());
  if (!file)
    return false;
  
  std::string line;
  while (std::getline(file, line))
  {
    size_t pos = line.find("yawl");
    if(pos != std::string::npos)
    {
      //found
      return true;
    }
  }
  return false;
}
//}
#ifndef WFDIRECTINTERFACE
void WFBaseEngine::RunNetworkInterface()
{
   this->mainInterfaceLoop();
}
#endif

void WFBaseEngine::Destroy()
{
    delete(this);
}

