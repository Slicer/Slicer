/*==========================================================================

Portions (c) Copyright 2008-2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $HeadURL$
Date:      $Date$
Version:   $Revision$

==========================================================================*/

#ifndef __vtkOpenIGTLinkIFWin32Header_h
#define __vtkOpenIGTLinkIFWin32Header_h

#include <vtkOpenIGTLinkIFConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(OpenIGTLinkIF_EXPORTS)
#define VTK_OPENIGTLINKIF_EXPORT __declspec( dllexport ) 
#else
#define VTK_OPENIGTLINKIF_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_OPENIGTLINKIF_EXPORT 
#endif
#endif
