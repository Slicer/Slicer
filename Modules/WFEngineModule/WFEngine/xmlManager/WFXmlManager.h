#ifndef WFXMLMANAGER_H_
#define WFXMLMANAGER_H_

#define XMLVAL_AUTO 0
#define XMLVAL_NEVER 1
#define XMLVAL_ALWAYS 2

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include <string>
#include <map>
#include <vector>
//#include <list>

//need to recognize the XERCESC types
XERCES_CPP_NAMESPACE_USE;

namespace WFEngine
{
 namespace nmWFXmlManager
 {
  
  //---------------------------------------------------------------------------
  //Local data
  //
  //gXmlFile
  //  The path to the file to parser. Set via command line.
  //
  //gDoNamespaces
  //  Indicates whether namespace processing should be done.
  //
  //gDoSchema
  //  Indicates whether schema processing should be done.
  //
  //gSchemaFullChecking
  //  Indicates whether full schema constraint checking should be done.
  //
  //gDoCreate
  //  Indicates whether entity reference nodes needs to be created or not
  //  Defaults to false
  //
  //gOutputEncoding
  //  The encoding we are to output in. If not set on the command line,
  //  then it is defaults to the encoding of the input XML file.
  //
  //gSplitCdataSections
  //  Indicates whether split-cdata-sections is to be enabled or not.
  //
  //gDiscardDefaultContent
  //  Indicates whether default content is discarded or not.
  //
  //gUseFilter
  //  Indicates if user wants to plug in the DOMPrintFilter.
  //
  //gValScheme
  //  Indicates what validation scheme to use. It defaults to 'auto', but
  //  can be set via the -v= command.
  //
  //---------------------------------------------------------------------------
   
  class WFXmlManager
  {
  public:
//   virtual WFXmlManager* New() const = 0;
      int saveXmlFile();
      typedef std::map<std::string,std::string> myAttrMap;
      void Destroy();
  protected:
      DOMDocument* xmlDoc;
      int loadXmlFile(std::string &xmlFileName);
      WFXmlManager();
      virtual ~WFXmlManager();
      
      //set the parameters before loading the xmlFile
      void setXercesParsingOptions(bool doNamespaces,
         bool doSchema,
         bool schemaFullChecking,
         bool doCreate,
         bool splitCdataSections,
         bool discardDefaultContent,
         bool useFilter,
         bool formatPrettyPrint,
         bool writeBOM,
         int validateSchema);
      
      DOMElement *getParentElementByName(std::string &parentTagName);
      myAttrMap getAllAttributesFromElement(DOMElement* curElem);
      DOMNodeList *getAllChildesByName(std::string &parentTagName, std::string &childTagName);
      DOMNodeList *getAllChildesByName(DOMElement *parentTagName, std::string &childTagName);
      std::vector<myAttrMap> getAttributesOfChilds(DOMNodeList *nodeList);
      
      DOMElement *getElementByPath(std::string &xmlPath);
      DOMElement *getElementByPath(DOMElement *parentElement, std::string &xmlPath);
      DOMElement *getFirstChildByName(DOMElement *parentElement, std::string &tagName);
      
  private:
      int initializeXerces(std::string xmlFileName);
      
      WFXmlManager* m_wfXmlMgr;
      
      std::string gXmlFile;// = 0;
      bool  gDoNamespaces;//          = false;
      bool  gDoSchema;//              = false;
      bool  gSchemaFullChecking;//    = false;
      bool  gDoCreate;//              = false;
      
      char* goutputfile;
      //options for DOMLSSerializer's features
      
      bool gSplitCdataSections;//    = true;
      bool gDiscardDefaultContent;// = true;
      bool gUseFilter;//             = false;
      bool gFormatPrettyPrint;//     = false;
      bool gWriteBOM;//              = false;
      bool gWhitespaceInElementContent;
      XMLCh* gOutputEncoding;
      
      XercesDOMParser::ValSchemes gValScheme;// = XercesDOMParser::Val_Auto;
  };
 }
}

#endif /*WFXMLMANAGER_H_*/
