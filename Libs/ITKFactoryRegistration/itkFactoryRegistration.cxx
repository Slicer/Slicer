
#include "itkFactoryRegistration.h"

// ITK includes
#include <itkImageFileReader.h>
#include <itkTransformFileReader.h>

// The following code is required to ensure that the
// mechanism allowing the ITK factory to be registered is not
// optimized out by the compiler.
void itk::itkFactoryRegistration()
{
  return;
}
