
#ifndef H_ITK_MGH_IMAGE_IO_FACTORY_H
#define H_ITK_MGH_IMAGE_IO_FACTORY_H

#include "itkObjectFactoryBase.h"
#include "itkImageIOBase.h"

#include "itkMGHIOWin32Header.h"


namespace itk
{

  class MGHImageIO_EXPORT MGHImageIOFactory : public ObjectFactoryBase
  {
  public:
    /** Standard class typedefs **/
    typedef MGHImageIOFactory         Self;
    typedef ObjectFactoryBase         Superclass;
    typedef SmartPointer<Self>        Pointer;
    typedef SmartPointer<const Self>  ConstPointer;

    /** Class methods used to interface with the registered factories **/
    virtual const char* GetITKSourceVersion(void) const;
    virtual const char* GetDescription(void)  const;
    
    /** Method for class instantiation **/
    itkFactorylessNewMacro(Self);
    
    /** RTTI (and related methods) **/
    itkTypeMacro(MGHImageIOFactory, ObjectFactoryBase);

    /** Register one factory of this type **/
    static void RegisterOneFactory(void)
      {
  MGHImageIOFactory::Pointer MGHFactory = MGHImageIOFactory::New();
  ObjectFactoryBase::RegisterFactory(MGHFactory);
      }

  protected:
    MGHImageIOFactory();
    ~MGHImageIOFactory();

  private:
    MGHImageIOFactory(const Self&); // purposely not implemented
    void operator=(const Self&); // purposely not implemented
    
  }; // end class MGHImageIOFactory


} // end namespace itk

#endif // H_ITK_MGH_IMAGE_IO_FACTORY_H
