#ifndef WFXMLWRITER_H_
#define WFXMLWRITER_H_

#include <xercesc/dom/DOMWriterFilter.hpp>

XERCES_CPP_NAMESPACE_USE

class WFXmlWriter : public DOMWriterFilter
{
public:

    /** @name Constructors */
  WFXmlWriter(unsigned long whatToShow = DOMNodeFilter::SHOW_ALL);
    //@{

    /** @name Destructors */
  ~WFXmlWriter(){};
    //@{

  /** @ interface from WFXmlWriter */
  virtual short acceptNode(const DOMNode*) const;
    //@{

  virtual unsigned long getWhatToShow() const {return fWhatToShow;};
  
  virtual void          setWhatToShow(unsigned long toShow) {fWhatToShow = toShow;};

private:
  // unimplemented copy ctor and assignement operator
  WFXmlWriter(const WFXmlWriter&);
  WFXmlWriter & operator = (const WFXmlWriter&);

  unsigned long fWhatToShow;

};

#endif /*WFXMLWRITER_H_*/
