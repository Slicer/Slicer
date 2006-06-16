#ifndef __itkCommandIOPlugin_h
#define __itkCommandIOPlugin_h

#include "itkObjectFactoryBase.h"

#ifdef WIN32
#ifdef CommandIOPlugin_EXPORTS
#define CommandIOPlugin_EXPORT __declspec(dllexport)
#else
#define CommandIOPlugin_EXPORT __declspec(dllimport)
#endif
#else
#define CommandIOPlugin_EXPORT 
#endif

/**
 * Routine that is called when the shared library is loaded by
 * itk::ObjectFactoryBase::LoadDynamicFactories().
 *
 * itkLoad() is C (not C++) function.
 */
extern "C" {
    CommandIOPlugin_EXPORT itk::ObjectFactoryBase* itkLoad();
} 
#endif  
