/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile$
 *  Language:  C++
 *  Date:      $Date: 2007-08-31 11:20:20 -0500 (Fri, 31 Aug 2007) $
 *  Version:   $Revision: 10358 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the above copyright notices for more information.
 *  =========================================================================*/
#ifdef WIN32
#define MODULE_IMPORT __declspec(dllimport)
#else
#define MODULE_IMPORT
#endif

extern "C" MODULE_IMPORT int BRAINSFitPrimary(int, char* []);

// main function built in BRAINSFitPrimary.cxx so that testing only builds
// templates once.
int main(int argc, char *argv[])
{
  return BRAINSFitPrimary(argc, argv);
}
