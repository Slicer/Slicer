#include "itkMRMLIDIOPlugin.h"
#include "itkMRMLIDImageIOFactory.h"

/**
 * Routine that is called when the shared library is loaded by
 * itk::ObjectFactoryBase::LoadDynamicFactories().
 *
 * This entry point is a C (not C++) function.
 */
itk::ObjectFactoryBase* ITK_LOAD_FUNCTION_NAME()
{
  static itk::MRMLIDImageIOFactory::Pointer f = itk::MRMLIDImageIOFactory::New();
  return f;
}
