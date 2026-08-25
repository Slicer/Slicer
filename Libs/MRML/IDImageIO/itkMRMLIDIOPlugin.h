#ifndef itkMRMLIDIOPlugin_h
#define itkMRMLIDIOPlugin_h

#include "itkObjectFactoryBase.h"

// TRANSIENT ITK-VERSION SHIM: delete once Slicer's minimum ITK always defines
// ITK_LOAD_FUNCTION_NAME -- ITK main once #6787 merges, and release-5.4 from
// whichever patch release carries its backport. Guarded on the macro rather
// than on ITK_VERSION_MAJOR/ITK_VERSION_MINOR because ITK main is 6.0.0 both
// before and after the merge, so no ITK_VERSION comparison distinguishes them;
// this comment carries the ITK_VERSION marker so the shim greps out with the
// rest of the version-conditional code. ITK releases without the macro
// hardcode "itkLoad" in LoadLibrariesInPath, so that is its only correct value.
#ifndef ITK_LOAD_FUNCTION_NAME
# define ITK_LOAD_FUNCTION_NAME itkLoad
#endif

#ifdef _WIN32
# ifdef MRMLIDIOPlugin_EXPORTS
#  define MRMLIDIOPlugin_EXPORT __declspec(dllexport)
# else
#  define MRMLIDIOPlugin_EXPORT __declspec(dllimport)
# endif
#else
# define MRMLIDIOPlugin_EXPORT
#endif

/**
 * Routine that is called when the shared library is loaded by
 * itk::ObjectFactoryBase::LoadDynamicFactories().
 *
 * This entry point is a C (not C++) function.
 */
extern "C"
{
  MRMLIDIOPlugin_EXPORT itk::ObjectFactoryBase* ITK_LOAD_FUNCTION_NAME();
}
#endif
