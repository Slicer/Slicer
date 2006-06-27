#ifndef __itkMRMLIDIOPlugin_h
#define __itkMRMLIDIOPlugin_h

#include "itkObjectFactoryBase.h"

#ifdef WIN32
#ifdef MRMLIDIOPlugin_EXPORTS
#define MRMLIDIOPlugin_EXPORT __declspec(dllexport)
#else
#define MRMLIDIOPlugin_EXPORT __declspec(dllimport)
#endif
#else
#define MRMLIDIOPlugin_EXPORT 
#endif

/**
 * Routine that is called when the shared library is loaded by
 * itk::ObjectFactoryBase::LoadDynamicFactories().
 *
 * itkLoad() is C (not C++) function.
 */
extern "C" {
    MRMLIDIOPlugin_EXPORT itk::ObjectFactoryBase* itkLoad();
} 
#endif  
