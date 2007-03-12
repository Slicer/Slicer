#include "WFXmlWriter.h"

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLString.hpp>

static const XMLCh  element_person[]=
{
  chLatin_p, chLatin_e, chLatin_r, chLatin_s, chLatin_o, chLatin_n, chNull 
};

static const XMLCh  element_link[]=
{
  chLatin_l, chLatin_i, chLatin_n, chLatin_k, chNull 
};

WFXmlWriter::WFXmlWriter(unsigned long whatToShow)
:fWhatToShow(whatToShow)
{}

short WFXmlWriter::acceptNode(const DOMNode* node) const
{
  //
  // The DOMLSSerializer shall call getWhatToShow() before calling 
  // acceptNode(), to show nodes which are supposed to be 
  // shown to this filter.
  // 
  // REVISIT: In case the DOMLSSerializer does not follow the protocol, 
  //          Shall the filter honour, or NOT, what it claims
  //          it is interested in ?
  // 
  // The DOMLS specs does not specify that acceptNode() shall do
  // this way, or not, so it is up the implementation,
  // to skip the code below for the sake of performance ...
  //
  if ((getWhatToShow() & (1 << (node->getNodeType() - 1))) == 0)
    return DOMNodeFilter::FILTER_ACCEPT;

  switch (node->getNodeType())
  {
  case DOMNode::ELEMENT_NODE:
    {
      // for element whose name is "person", skip it
      if (XMLString::compareString(node->getNodeName(), element_person)==0)
        return DOMNodeFilter::FILTER_SKIP;
      // for element whose name is "line", reject it
      if (XMLString::compareString(node->getNodeName(), element_link)==0)
        return DOMNodeFilter::FILTER_REJECT;
      // for rest, accept it
      return DOMNodeFilter::FILTER_ACCEPT;

      break;
    }
  case DOMNode::COMMENT_NODE:
    {
      // the WhatToShow will make this no effect
      return DOMNodeFilter::FILTER_REJECT;
      break;
    }
  case DOMNode::TEXT_NODE:
    {
      // the WhatToShow will make this no effect
      return DOMNodeFilter::FILTER_REJECT;
      break;
    }
  case DOMNode::DOCUMENT_TYPE_NODE:
    {
      // even we say we are going to process document type,
      // we are not able be to see this node since
      // DOMLSSerializerImpl (a XercesC's default implementation
      // of DOMLSSerializer) will not pass DocumentType node to
      // this filter.
      //
      return DOMNodeFilter::FILTER_REJECT;  // no effect
      break;
    }
  case DOMNode::DOCUMENT_NODE:
    {
      // same as DOCUMENT_NODE
      return DOMNodeFilter::FILTER_REJECT;  // no effect
      break;
    }
  default :
    {
      return DOMNodeFilter::FILTER_ACCEPT;
      break;
    }
  }

  return DOMNodeFilter::FILTER_ACCEPT;
}
