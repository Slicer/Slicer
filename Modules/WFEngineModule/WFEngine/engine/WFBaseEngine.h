#ifndef WFENGINE_H_
#define WFENGINE_H_

#include "WFEngineOptions.h"
#include "WFWorkflowManager.h"

#ifndef WFDIRECTINTERFACE
#include "WFServerConnection.h"
#endif

#include <string>

namespace WFEngine
{
 class WFBaseEngine
 {
 public:
  static WFBaseEngine* New();
  int InitializeWFEngine(std::string wfConfigFile);

  void Destroy();
#ifndef WFDIRECTINTERFACE
  void RunNetworkInterface();
#endif
 protected:
 
  WFBaseEngine();
  virtual ~WFBaseEngine();
 
  nmWFEngineOptions::WFEngineOptions * InitializeWFEOpts(std::string wfConfigFile);
  void InitializeWFFactoryClasses();
  void InitializeWFInterfaces();
  void InitializeKnowWorkflows();
   
  void SaveState();
  
  bool validateXMLFile(std::string &fileName);
  bool configExists;
  
  WFBaseEngine *GetWFBEInstance();
 
  //static string wfConfigFile;
  nmWFEngineOptions::WFEngineOptions *m_wfeOpts;
 private:
 
  static void recvClientData(int socket, char* buffer);
  static void recvOptionsData(int socket, char* buffer);
  void sendClientData(int socket, std::string sendData);
  void sendOptionsData(int socket, std::string sendData);
  
  static WFBaseEngine* sm_wfeBE;
//  nmWFFactory::WFFactory *m_wfeFactory;
  
//  nmWFClientInterface::WFClientInterface *m_wfeCLI;
#ifndef WFDIRECTINTERFACE
  void mainInterfaceLoop();
  
  //ConnectionHandler is the main Handler for all server connections
  nmWFServerConnection::WFServerConnection *m_wfeSC;
#endif
 };
}

#endif /*WFENGINE_H_*/
