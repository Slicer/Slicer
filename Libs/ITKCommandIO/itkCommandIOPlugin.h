/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   ITKCommandIO
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/
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
