
#include "itkMGHImageIOFactory.h"
#include "itkCreateObjectFunction.h"
#include "itkMGHImageIO.h"
#include "itkVersion.h"

namespace itk
{

  MGHImageIOFactory::MGHImageIOFactory()
  {
    this->RegisterOverride("itkImageIOBase",
         "itkMGHImageIO",
         "MGH Image IO",
         1,
         CreateObjectFunction<MGHImageIO>::New() );
  }

  MGHImageIOFactory::~MGHImageIOFactory()
  {
  }

  const char*
  MGHImageIOFactory::GetITKSourceVersion(void) const
  {
    return ITK_SOURCE_VERSION;
  }
  
  const char*
  MGHImageIOFactory::GetDescription() const
  {
    return "MGH ImageIO Factory, allows the loading of MGH/MGZ images into Insight";
  }

} // end namespace itk
