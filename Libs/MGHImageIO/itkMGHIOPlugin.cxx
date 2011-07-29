#include "itkMGHIOPlugin.h"
#include "itkMGHImageIOFactory.h"

/**
 * Routine that is called when the shared library is loaded by
 * itk::ObjectFactoryBase::LoadDynamicFactories().
 *
 * itkLoad() is C (not C++) function.
 */
itk::ObjectFactoryBase* itkLoad()
{
  static itk::MGHImageIOFactory::Pointer f = itk::MGHImageIOFactory::New();
  return f;
}
