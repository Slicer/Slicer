#ifndef __SimpleXMLParserBase_h
#define __SimpleXMLParserBase_h
//
// standard includes
#include <iostream>
#include <ostream>
#include <string>
#include <list>
//
// xerces_c includes
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
XERCES_CPP_NAMESPACE_USE

/**
  * \class SimpleXMLParserBase
  * contains the common code
  * needed to Read and Write an XML file.
  * Any actual interpretation is delegated to subclass
  */
class SimpleXMLParserBase
{
public:
  typedef XercesDOMParser ParserType;
  SimpleXMLParserBase():m_Parser(0)
  {}

  void Read(const std::string & filename)
  {
    try
      {
      XMLPlatformUtils::Initialize();
      }
    catch ( const XMLException & toCatch )
      {
      std::cerr << "Can't Initialize XMLPlatformUtils" << std::endl;
      throw;
      }
    //  Create our parser, then attach an error handler to the parser.
    //  The parser will call back to methods of the ErrorHandler if it
    //  discovers errors during the course of parsing the XML document.
    //
    this->m_Parser = new ParserType;
    this->m_Parser->setValidationScheme(ParserType::Val_Auto);
    this->m_Parser->setDoNamespaces(false);
    this->m_Parser->setDoSchema(false);
    this->m_Parser->setValidationSchemaFullChecking(false);
    this->m_Parser->setCreateEntityReferenceNodes(false);
    //
    //  Parse the XML file, catching any XML exceptions that might propogate
    //  out of it.
    //
    bool errorsOccurred = false;
    try
      {
      this->m_Parser->parse( filename.c_str() );
      }
    catch ( const OutOfMemoryException & )
      {
      std::cerr << "OutOfMemoryException" << std::endl;
      errorsOccurred = true;
      }
    catch ( const XMLException & e )
      {
      std::cerr << "An error occurred during parsing\n   Message: "
                << e.getMessage() << std::endl;
      errorsOccurred = true;
      }

    catch ( const DOMException & e )
      {
      const unsigned int maxChars = 2047;
      XMLCh              errText[maxChars + 1];

      std::cerr << "\nDOM Error during parsing: '" << filename << "'\n"
                << "DOMException code is:  " << e.code << std::endl;

      if ( DOMImplementation::loadDOMExceptionMsg(e.code, errText, maxChars) )
        {
        std::cerr << "Message is: " << errText << std::endl;
        }

      errorsOccurred = true;
      }
    catch ( ... )
      {
      std::cerr << "An error occurred during parsing\n " << std::endl;
      errorsOccurred = true;
      }
    if ( errorsOccurred )
      {
      throw;
      }
  }

  void Write(const std::string & filename)
  {
    if ( m_Parser == 0 )
      {
      throw;
      }
    // get a serializer, an instance of DOMWriter
    XMLCh tempStr[100];
    XMLString::transcode("LS", tempStr, 99);
    DOMImplementation *impl =
      DOMImplementationRegistry::getDOMImplementation(tempStr);
    DOMWriter *theSerializer =
      ( (DOMImplementationLS *)impl )->createDOMWriter();

    XMLFormatTarget *formatTarget = new LocalFileFormatTarget( filename.c_str() );
    theSerializer->writeNode
      ( formatTarget,
      *static_cast< DOMNode * >( this->m_Parser->getDocument() ) );
    delete theSerializer;
  }

  ParserType * Parser()
  {
    return m_Parser;
  }

protected:
  ParserType *m_Parser;
};

#endif // SimpleXMLParserBase_h
