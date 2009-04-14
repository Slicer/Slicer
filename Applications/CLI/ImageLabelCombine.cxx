/*=========================================================================

  Program:   Slicer3
  Language:  C++
  Module:    $HeadURL: http://svn.slicer.org/Slicer3/trunk/Applications/CLI/ConfidenceConnected.cxx $
  Date:      $Date: 2008-11-24 14:36:19 -0500 (Mon, 24 Nov 2008) $
  Version:   $Revision: 7965 $

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include <iostream>
#include <algorithm>
#include <string>

#include "itkPluginFilterWatcher.h"
#include "itkPluginUtilities.h"

#include "vtkSmartPointer.h"
#include "vtkTimerLog.h"

#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkITKImageWriter.h"
#include "vtkImageData.h"
#include "ImageLabelCombineCLP.h"

#include "vtkImageLabelCombine.h"

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace {

} // end of anonymous namespace


int main( int argc, char * argv[] ){

  PARSE_ARGS;

  // Read in label volume inputs
  vtkITKArchetypeImageSeriesReader* readerA = vtkITKArchetypeImageSeriesScalarReader::New();
  readerA->SetArchetype(InputLabelMap_A.c_str());
  readerA->SetOutputScalarTypeToNative();
  readerA->SetDesiredCoordinateOrientationToNative();
  readerA->SetUseNativeOriginOn();
  readerA->Update();
   
  vtkITKArchetypeImageSeriesReader* readerB = vtkITKArchetypeImageSeriesScalarReader::New();
  readerB->SetArchetype(InputLabelMap_B.c_str());
  readerB->SetOutputScalarTypeToNative();
  readerB->SetDesiredCoordinateOrientationToNative();
  readerB->SetUseNativeOriginOn();
  readerB->Update();

  
  // combine labels
  vtkImageLabelCombine *labelCombine = vtkImageLabelCombine::New();
  labelCombine->SetOverwriteInput(!FirstOverwrites);
  labelCombine->SetInput1( readerA->GetOutput() );
  labelCombine->SetInput2( readerB->GetOutput() );
  labelCombine->Update();

  // Output
  vtkITKImageWriter *writer = vtkITKImageWriter::New();
  writer->SetFileName(OutputLabelMap.c_str());
  writer->SetInput( labelCombine->GetOutput() );
  
  if (readerA->GetRasToIjkMatrix())
    {
    writer->SetRasToIJKMatrix( readerA->GetRasToIjkMatrix() );
    }
  writer->Write();

  //Delete everything
  readerA->Delete();
  readerB->Delete();
  labelCombine->Delete();
  writer->Delete();

  return EXIT_SUCCESS;
}
