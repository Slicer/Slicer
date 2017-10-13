/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

/// itkMRMLIDIOExport
///
/// The itkMRMLIDIOExport captures some system differences between Unix
/// and Windows operating systems.

#ifndef itkMRMLIDIOExport_h
#define itkMRMLIDIOExport_h

#include <itkMRMLIDImageIOConfigure.h>

#if defined(WIN32) && !defined(MRMLIDIO_STATIC)
#if defined(MRMLIDIO_EXPORTS)
#define MRMLIDImageIO_EXPORT __declspec( dllexport )
#else
#define MRMLIDImageIO_EXPORT __declspec( dllimport )
#endif
#else
#define MRMLIDImageIO_EXPORT
#endif

#endif
