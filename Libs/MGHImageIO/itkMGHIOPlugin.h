#ifndef __itkMGHIOPlugin_h
#define __itkMGHIOPlugin_h

#include "itkObjectFactoryBase.h"

#ifdef WIN32
#ifdef MGHIOPlugin_EXPORTS
#define MGHIOPlugin_EXPORT __declspec(dllexport)
#else
#define MGHIOPlugin_EXPORT __declspec(dllimport)
#endif
#else
#define MGHIOPlugin_EXPORT 
#endif

/**
 * Routine that is called when the shared library is loaded by
 * itk::ObjectFactoryBase::LoadDynamicFactories().
 *
 * itkLoad() is C (not C++) function.
 */
extern "C" {
    MGHIOPlugin_EXPORT itk::ObjectFactoryBase* itkLoad();
} 
#endif  
