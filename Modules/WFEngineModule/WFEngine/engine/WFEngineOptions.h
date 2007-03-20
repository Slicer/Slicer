#ifndef WFENGINEOPTIONS_H_
#define WFENGINEOPTIONS_H_

#include <WFXmlConfigManager.h>

#include <string>
#include <vector>
#include <map>

namespace WFEngine
{
 namespace nmWFEngineOptions
 {
  class WFEngineOptions
  {
  public:
   typedef struct wfDescription
   {
    std::string fileName;
    bool visible;
    bool embedded;
   } wfDesc;
   
   //static WFEngineOptions *New(const string wfConfigFile, bool showEditGUI);
   static WFEngineOptions *New();
 
   void SetConfigFile(std::string wfConfigFile);
   std::string GetConfigFile();
 
   
   std::vector<WFEngine::nmWFXmlConfigManager::WFXmlConfigManager::myAttrMap> GetLookUpPaths();  
   std::vector<WFEngine::nmWFXmlConfigManager::WFXmlConfigManager::myAttrMap> GetKnownWorkflows();
   
   void AddKnownWorkflow(std::string &fileName);
   void AddKnownWorkflow(std::string &fileName, bool visible, bool enabled);
   
   bool IsLoaded();
   
   void RemoveKnownWorkflow(std::string &fileName);
   void RemoveLookUpPath(std::string &path);
   void SaveChanges();
  
   void Destroy();
  protected:
   WFEngineOptions();
   virtual ~WFEngineOptions();
 
  private:
   WFEngine::nmWFXmlConfigManager::WFXmlConfigManager *m_WFCfgXmlManager;
 
   wfDescription m_wfDesc;
 
   std::map<int, struct wfDescription> m_knownWFs;
 
   int LoadConfigFile(std::string wfConfigFile);
 
   bool m_configIsActive;
   bool m_isLoaded;
   std::string m_wfConfigFile;
   
   void recvOptionsData(char* buffer);   
  };
 }

}
#endif /*WFENGINEOPTIONS_H_*/
