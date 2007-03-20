#ifndef WFXMLCONFIGMANAGER_H_
#define WFXMLCONFIGMANAGER_H_

#include "WFXmlManager.h"

#include <list>
#include <vector>
#include <string>
#include <map>

using namespace WFEngine::nmWFXmlManager;

namespace WFEngine
{
 namespace nmWFXmlConfigManager
 {
  class WFXmlConfigManager : public WFXmlManager
  {
  public:
   static WFXmlConfigManager* New();
   int loadConfigFile(std::string &xmlFileName);
   int parseConfig(std::string &parentTag);
   
//   typedef std::list< std::vector<std::string,std::string> > tagAttrList;
      
   std::vector<myAttrMap> getAllKnownWorkflows();
   
   std::vector<myAttrMap> getAllLookUpPaths();
   
   void removeKnownWorkflow(std::string &fileName);
   void removeLookUpPath(std::string &path);
   void addKnownWorkflow(std::string &fileName, bool visible, bool enabled);
   std::map<std::string, std::string> getKnownWorkflows();
   
   void Destroy();
  protected:
    WFXmlConfigManager();
    virtual ~WFXmlConfigManager();
    void removeAllNodesWithAttribute(std::string &parentTagName, std::string &childTagName, std::string &attribName, std::string &attribValue);
//    int loadXmlFile(std::string &xmlFileName);
    
  private:
   DOMNodeList *tempNodeList;
   typedef WFXmlManager super;
   
   std::string knownWorkflows;
   std::string workflow;
   std::string lookUpPaths;
   std::string path;
  };
 }
}

#endif /*WFXMLCONFIGMANAGER_H_*/
