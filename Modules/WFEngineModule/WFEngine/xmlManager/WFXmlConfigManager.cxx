#include "WFXmlConfigManager.h"

#include <iostream>

using namespace WFEngine::nmWFXmlConfigManager;
//need to recognize the XERCESC types
XERCES_CPP_NAMESPACE_USE;


WFXmlConfigManager::WFXmlConfigManager() : WFXmlManager()
{
    this->knownWorkflows = "knownWorkflows";
    this->workflow = "workflow";
    this->lookUpPaths = "lookUpPaths";
    this->path = "path";
}

WFXmlConfigManager::~WFXmlConfigManager()
{
}

int WFXmlConfigManager::loadConfigFile(std::string &xmlFileName)
{
 return this->loadXmlFile(xmlFileName);
}

//int WFXmlConfigManager::loadXmlFile(std::string &xmlFileName)
//{
// int retval = 0;
// retval = initializeXerces(xmlFileName);
// if(retval != 1)
// {
//  //DOMNode *myXMLConfigFile = this->xmlParser->getDocument();
//  DOMElement *myRootElem = this->xmlDoc->getDocumentElement();
//  //  //std::cout<<myRootElem->getTagName()<<std::endl;
// }
// return retval;
//}

WFXmlConfigManager* WFXmlConfigManager::New()
{
 return new WFXmlConfigManager();
}

std::vector<WFXmlConfigManager::myAttrMap> WFXmlConfigManager::getAllKnownWorkflows()
{
  std::vector<myAttrMap> myKnownWFList;
  
  DOMNodeList *childNodeList = this->getAllChildesByName(knownWorkflows, workflow);
  //std::cout<<"childNodeList->GetLength "<<childNodeList->getLength()<<std::endl;
  myKnownWFList = this->getAttributesOfChilds(childNodeList);
  return myKnownWFList;
}

void WFXmlConfigManager::removeAllNodesWithAttribute(std::string &parentTagName, std::string &childTagName, std::string &attribName, std::string &attribValue)
{
  DOMNodeList *myKnownWFNLs = this->getAllChildesByName(parentTagName, childTagName);
  for(int i = 0; i < myKnownWFNLs->getLength(); i++)
  {
      if (myKnownWFNLs->item(i)->getNodeType() == 1)
      {
        DOMElement *curElem = (DOMElement*)(myKnownWFNLs->item(i));
        DOMNamedNodeMap *attrMap = curElem->getAttributes();
        std::string curAttrValue = XMLString::transcode(attrMap->getNamedItem(XMLString::transcode(attribName.c_str()))->getNodeValue());
        //std::cout<<curAttrValue<<"="<<attribValue<<std::endl;
        if(curAttrValue == attribValue)
        {
          //std::cout<<"found"<<std::endl;
          curElem->getParentNode()->removeChild(curElem);
          
        }
      }
  }
}

std::vector<WFXmlConfigManager::myAttrMap> WFXmlConfigManager::getAllLookUpPaths()
{
  std::vector<myAttrMap> myLookUpPaths;
  std::string parentTagName = "lookUpPaths";
  std::string childTagName = "path";
  myLookUpPaths = this->getAttributesOfChilds(this->getAllChildesByName(parentTagName, childTagName));
  return myLookUpPaths;
}

void WFXmlConfigManager::removeKnownWorkflow(std::string &fileName)
{
  std::string attribName = "fileName";
  //std::cout<<">>>>remove "<<fileName<<std::endl;  
  this->removeAllNodesWithAttribute(knownWorkflows,workflow, attribName, fileName);
}

void WFXmlConfigManager::removeLookUpPath(std::string &path)
{
  std::string parentTagName = "lookUpPaths";
  std::string childTagName = "path";
  std::string attribName = "url";
    
  this->removeAllNodesWithAttribute(parentTagName,childTagName, attribName, path);
}

void WFXmlConfigManager::addKnownWorkflow(std::string &fileName, bool visible, bool enabled)
{
  DOMNode *myKnownWFNode = this->getParentElementByName(knownWorkflows);
  
  if(myKnownWFNode != NULL && myKnownWFNode->getNodeType() == 1)
  {
    DOMElement *newKWF = xmlDoc->createElement(XMLString::transcode(workflow.c_str()));
    newKWF->setAttribute(XMLString::transcode("fileName"), XMLString::transcode(fileName.c_str()));
    newKWF->setAttribute(XMLString::transcode("visible"), XMLString::transcode(visible?"true":"false"));
    newKWF->setAttribute(XMLString::transcode("enabled"), XMLString::transcode(enabled?"true":"false"));
    myKnownWFNode->appendChild(newKWF);
  }
}

std::map<std::string, std::string> WFXmlConfigManager::getKnownWorkflows()
{
    this->tempNodeList = this->getAllChildesByName(knownWorkflows, workflow);
    //std::cout<<tempNodeList->getLength()<<std::endl;
}

void WFXmlConfigManager::Destroy()
{
    delete(this);
}

