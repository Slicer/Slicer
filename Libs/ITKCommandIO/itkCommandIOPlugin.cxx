#include "itkCommandIOPlugin.h"
#include "itkCommandIOFactory.h"

/**
 * Routine that is called when the shared library is loaded by
 * itk::ObjectFactoryBase::LoadDynamicFactories().
 *
 * itkLoad() is C (not C++) function.
 */
itk::ObjectFactoryBase* itkLoad()
{
    static itk::CommandIOFactory::Pointer f = itk::CommandIOFactory::New();
    return f;
}

