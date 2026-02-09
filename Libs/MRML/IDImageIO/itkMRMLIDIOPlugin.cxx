#include "itkMRMLIDIOPlugin.h"
#include "itkMRMLIDImageIOFactory.h"

/**
 * Routine that is called when the shared library is loaded by
 * itk::ObjectFactoryBase::LoadDynamicFactories().
 *
 * slicer_itkLoad() is C (not C++) function.
 */
itk::ObjectFactoryBase* slicer_itkLoad()
{
  static itk::MRMLIDImageIOFactory::Pointer f = itk::MRMLIDImageIOFactory::New();
  return f;
}
