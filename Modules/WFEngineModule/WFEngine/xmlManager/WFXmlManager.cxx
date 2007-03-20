#include "WFXmlManager.h"

#include "DOMTreeErrorReporter.h"
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include "WFXmlWriter.h"

#include <string>

using namespace WFEngine::nmWFXmlManager;

XERCES_CPP_NAMESPACE_USE;

WFXmlManager::WFXmlManager()
{
 gDoNamespaces          = false;
 gDoSchema              = false;
 gSchemaFullChecking    = false;
 gDoCreate              = false;
 
 char* goutputfile      = 0;
 //options for DOMLSSerializer's features
 XMLCh* gOutputEncoding = 0;
 
 gSplitCdataSections    = true;
 gDiscardDefaultContent = true;
 gUseFilter;//             = false;
 gFormatPrettyPrint     = true;
 gWhitespaceInElementContent = false;
 gWriteBOM              = false;
 gValScheme = XercesDOMParser::Val_Auto;
}

WFXmlManager::~WFXmlManager()
{
}

//template<class Child>
//WFXmlManager* WFXmlManager::New()
//{
// WFXmlManager* myWFXmlMgr = new WFXmlManager;
// myWFXmlMgr->m_wfXmlMgr = myWFXmlMgr;
// return myWFXmlMgr;
//}

int WFXmlManager::initializeXerces(std::string xmlFileName)
{
    int retVal = 1;
    //Initialize the XML4C2 system
    try
    {
        XMLPlatformUtils::Initialize();
    }

    catch(const XMLException &toCatch)
    {
        XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
             << "  Exception message:"
             << StrX(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
        retVal = 0;
        return retVal;
    }
    
    XercesDOMParser *parser = new XercesDOMParser;
    parser->setValidationScheme(gValScheme);
    parser->setDoNamespaces(gDoNamespaces);
    parser->setDoSchema(gDoSchema);
    parser->setValidationSchemaFullChecking(gSchemaFullChecking);
    parser->setCreateEntityReferenceNodes(gDoCreate);

    DOMTreeErrorReporter *errReporter = new DOMTreeErrorReporter();
    parser->setErrorHandler(errReporter);

    //
    //  Parse the XML file, catching any XML exceptions that might propogate
    //  out of it.
    //
    try
    {
        parser->parse(xmlFileName.c_str());
    }
    catch (const OutOfMemoryException&)
    {
        XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
        retVal = 0;
    }
    catch (const XMLException& e)
    {
        XERCES_STD_QUALIFIER cerr << "An error occurred during parsing\n   Message: "
             << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
        retVal = 0;
    }

    catch (const DOMException& e)
    {
        const unsigned int maxChars = 2047;
        XMLCh errText[maxChars + 1];

        XERCES_STD_QUALIFIER cerr << "\nDOM Error during parsing: '" << xmlFileName << "'\n"
             << "DOMException code is:  " << e.code << XERCES_STD_QUALIFIER endl;

        if (DOMImplementation::loadDOMExceptionMsg(e.code, errText, maxChars))
             XERCES_STD_QUALIFIER cerr << "Message is: " << StrX(errText) << XERCES_STD_QUALIFIER endl;

        retVal = 0;
    }

    catch (...)
    {
        XERCES_STD_QUALIFIER cerr << "An error occurred during parsing\n " << XERCES_STD_QUALIFIER endl;
        retVal = 0;
    }
    
    if(retVal)
    {
     this->xmlDoc = parser->getDocument();
     //std::cout<<XMLString::transcode(this->xmlDoc->getDocumentElement()->getTagName())<<std::endl;
     this->goutputfile = (char*)xmlFileName.c_str();
    }
    //std::cout<<"retVal="<<retVal<<std::endl;
    //std::cout<<"ErrorCount: "<<parser->getErrorCount()<<std::endl;
    ////std::cout<<this->xmlParser->getErrorCount()<<std::endl;
    return retVal;
}

void WFXmlManager::setXercesParsingOptions(bool doNamespaces,
  bool doSchema,
  bool schemaFullChecking,
  bool doCreate,
  bool splitCdataSections,
  bool discardDefaultContent,
  bool useFilter,
  bool formatPrettyPrint,
  bool writeBOM,
  int validateScheme)
{
 this->gDoNamespaces = doNamespaces;
 this->gDoSchema = doSchema;
 this->gSchemaFullChecking = schemaFullChecking;
 this->gDoCreate = doCreate;
 this->gSplitCdataSections = splitCdataSections;
 this->gDiscardDefaultContent = discardDefaultContent;
 this->gUseFilter = useFilter;
 this->gFormatPrettyPrint = formatPrettyPrint;
 this->gWriteBOM = writeBOM;
 
 switch (validateScheme)
 {
 case 0:
  this->gValScheme = XercesDOMParser::Val_Auto;
  break;
 case 1:
  this->gValScheme = XercesDOMParser::Val_Never;
  break;
 case 2:
  this->gValScheme = XercesDOMParser::Val_Always;
  break;
 }
}

int WFXmlManager::saveXmlFile()
{
  int retval;
  WFXmlWriter *myFilter = 0;

    try
    {
        // get a serializer, an instance of DOMLSSerializer
        XMLCh tempStr[3] = {chLatin_L, chLatin_S, chNull};
        DOMImplementation *impl          = DOMImplementationRegistry::getDOMImplementation(tempStr);
        DOMWriter   *theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();
//        DOMLSOutput       *theOutputDesc = ((DOMImplementationLS*)impl)->createLSOutput();

        // set user specified output encoding
        theSerializer->setEncoding(gOutputEncoding);

        // plug in user's own filter
        if (gUseFilter)
        {
            // even we say to show attribute, but the DOMLSSerializer
            // will not show attribute nodes to the filter as
            // the specs explicitly says that DOMLSSerializer shall
            // NOT show attributes to DOMLSSerializerFilter.
            //
            // so DOMNodeFilter::SHOW_ATTRIBUTE has no effect.
            // same DOMNodeFilter::SHOW_DOCUMENT_TYPE, no effect.
            //
            myFilter = new WFXmlWriter(DOMNodeFilter::SHOW_ELEMENT   |
                                          DOMNodeFilter::SHOW_ATTRIBUTE |
                                          DOMNodeFilter::SHOW_DOCUMENT_TYPE);
            theSerializer->setFilter(myFilter);
        }

        // plug in user's own error handler
//        DOMConfiguration* serializerConfig=theSerializer->getDomConfig();
        
        // set feature if the serializer supports the feature/mode
        if (theSerializer->canSetFeature(XMLUni::fgDOMWRTSplitCdataSections, gSplitCdataSections))
            theSerializer->setFeature(XMLUni::fgDOMWRTSplitCdataSections, gSplitCdataSections);

        if (theSerializer->canSetFeature(XMLUni::fgDOMWRTDiscardDefaultContent, gDiscardDefaultContent))
            theSerializer->setFeature(XMLUni::fgDOMWRTDiscardDefaultContent, gDiscardDefaultContent);

        if (theSerializer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, gFormatPrettyPrint))
            theSerializer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, gFormatPrettyPrint);

        if (theSerializer->canSetFeature(XMLUni::fgDOMWRTBOM, gWriteBOM))
            theSerializer->setFeature(XMLUni::fgDOMWRTBOM, gWriteBOM);

        if (theSerializer->canSetFeature(XMLUni::fgDOMWRTWhitespaceInElementContent , gWhitespaceInElementContent))
            theSerializer->setFeature(XMLUni::fgDOMWRTWhitespaceInElementContent , gWhitespaceInElementContent);
        //
        // Plug in a format target to receive the resultant
        // XML stream from the serializer.
        //
        // StdOutFormatTarget prints the resultant XML stream
        // to stdout once it receives any thing from the serializer.
        //
        XMLFormatTarget *myFormTarget;
        myFormTarget=new LocalFileFormatTarget(goutputfile);
//        theOutputDesc->setByteStream(myFormTarget);
        
        // get the DOM representation
        DOMNode *doc = this->xmlDoc;
        
        //
        // do the serialization through DOMLSSerializer::write();
        //
        theSerializer->writeNode(myFormTarget, *doc);

//        theOutputDesc->release();
        theSerializer->release();

        //
        // Filter, formatTarget and error handler
        // are NOT owned by the serializer.
        //
        delete myFormTarget;

        if (gUseFilter)
            delete myFilter;

    }
    catch (const OutOfMemoryException&)
    {
        XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
        retval = 5;
    }
    catch (XMLException& e)
    {
        XERCES_STD_QUALIFIER cerr << "An error occurred during creation of output transcoder. Msg is:"
            << XERCES_STD_QUALIFIER endl
            << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
        retval = 4;
    }
}

int WFXmlManager::loadXmlFile(std::string &xmlFileName)
{
    if(this->xmlDoc)
    {
//        delete(this->xmlDoc);
        this->xmlDoc = NULL;    
    }
    
    return initializeXerces(xmlFileName);
}

DOMElement *WFXmlManager::getParentElementByName(std::string &parentTagName)
{
    return this->getFirstChildByName(this->xmlDoc->getDocumentElement(), parentTagName);
}

DOMNodeList *WFXmlManager::getAllChildesByName(std::string &parentTagName, std::string &childTagName)
{
  DOMElement *myKnownWFNode = this->getParentElementByName(parentTagName);
  //check if the node type is an ElementNode
  if (myKnownWFNode != NULL)
  {
   DOMElement *curElem = (DOMElement*)myKnownWFNode;
   //std::cout<<XMLString::transcode(curElem->getTagName())<<" "<<childTagName.c_str()<<std::endl;
   DOMNodeList *parentChilds = curElem->getElementsByTagName(XMLString::transcode(childTagName.c_str()));
   //std::cout<<parentChilds->getLength()<<std::endl;
   return parentChilds;
  }
  else return NULL;
}

DOMNodeList *WFXmlManager::getAllChildesByName(DOMElement *parentElement, std::string &childTagName)
{
//  DOMElement *myKnownWFNode = this->getElementByPath(parentElement, childTagName);
  //check if the node type is an ElementNode
  if (parentElement != NULL)
  {
   //std::cout<<XMLString::transcode(parentElement->getTagName())<<" "<<childTagName.c_str()<<std::endl;
   DOMNodeList *parentChilds = parentElement->getElementsByTagName(XMLString::transcode(childTagName.c_str()));
   //std::cout<<parentChilds->getLength()<<std::endl;
   return parentChilds;
  }
  else return NULL;
}

std::vector<WFXmlManager::myAttrMap> WFXmlManager::getAttributesOfChilds(DOMNodeList *nodeList)
{
  std::vector<WFXmlManager::myAttrMap> myChildList;
  for(int i = 0; i < nodeList->getLength(); i++)
  {
      if (nodeList->item(i)->getNodeType() == 1)
      {
       myChildList.push_back(this->getAllAttributesFromElement((DOMElement*)(nodeList->item(i))));
      }
  }
  
  return myChildList;
}

WFXmlManager::myAttrMap WFXmlManager::getAllAttributesFromElement(DOMElement *curElem)
{
    WFXmlManager::myAttrMap curAttrMap;
 DOMNamedNodeMap *attrMap = curElem->getAttributes();
 for(int i = 0; i < attrMap->getLength(); i++)
 {
  //std::cout<<XMLString::transcode(attrMap->item(i)->getNodeName())<<"='"<<XMLString::transcode(attrMap->item(i)->getNodeValue())<<"'"<<std::endl;
  curAttrMap.insert(std::make_pair(XMLString::transcode(attrMap->item(i)->getNodeName()), XMLString::transcode(attrMap->item(i)->getNodeValue())));
 }
 return curAttrMap;
}

DOMElement *WFXmlManager::getElementByPath(std::string &xmlPath)
{
    return this->getElementByPath(this->xmlDoc->getDocumentElement(), xmlPath);
}

DOMElement *WFXmlManager::getElementByPath(DOMElement *parentElement, std::string &xmlPath)
{
    DOMElement *retDOMElement;
    DOMElement *curParentDOMElement = parentElement;
    unsigned int offset = 0;
    unsigned int delimIndex = 0;
    const std::string delim = "/";
    
    delimIndex = xmlPath.find(delim, offset);

    if(delimIndex >= xmlPath.length())
    {
        return this->getFirstChildByName(parentElement, xmlPath);
    }        
    else
    {
        while (delimIndex < xmlPath.length())
        {
            std::string subStr = xmlPath.substr(offset, delimIndex - offset);
            retDOMElement = this->getFirstChildByName(curParentDOMElement,subStr);
            if(retDOMElement)
            {
                offset += delimIndex - offset + delim.length();
                delimIndex = xmlPath.find(delim, offset);    
            }
            else
                return curParentDOMElement;
            
        }   
    }
    //std::cout<<xmlPath<<" not found!"<<std::endl;
    return NULL;
}

DOMElement *WFXmlManager::getFirstChildByName(DOMElement *parentElement, std::string &tagName)
{
    DOMNodeList *myKnownWFNL = parentElement->getElementsByTagName(XMLString::transcode(tagName.c_str()));
    //std::cout<<myKnownWFNL->getLength()<<" <"<<tagName<<"> Length"<<std::endl;
     
    if(myKnownWFNL->getLength() > 0)
    {
        for(int i = 0; i < myKnownWFNL->getLength(); i++)
        {
            if(myKnownWFNL->item(i)->getNodeType() == 1)
                return (DOMElement*)(myKnownWFNL->item(i));
        }        
    }    
    return NULL;
}

void WFXmlManager::Destroy()
{
    delete(this);
}
