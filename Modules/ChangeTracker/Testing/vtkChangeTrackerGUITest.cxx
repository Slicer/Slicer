/*=auto=========================================================================

  Portions (c) Copyright 2010 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// GUI includes
#include "vtkChangeTrackerGUI.h"
#include "vtkChangeTrackerAnalysisStep.h"
#include "vtkChangeTrackerFirstScanStep.h"
#include "vtkChangeTrackerROIStep.h"
#include "vtkChangeTrackerSegmentationStep.h"
#include "vtkChangeTrackerTypeStep.h"

// STL includes
#include <stdlib.h>

#include "TestingMacros.h"

int vtkChangeTrackerGUITest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [] )
{
  vtkChangeTrackerGUI* gui = vtkChangeTrackerGUI::New();

  vtkChangeTrackerAnalysisStep* step1 = vtkChangeTrackerAnalysisStep::New();
  vtkChangeTrackerFirstScanStep* step2 = vtkChangeTrackerFirstScanStep::New();
  vtkChangeTrackerROIStep* step3 = vtkChangeTrackerROIStep::New();
  vtkChangeTrackerSegmentationStep* step4 = vtkChangeTrackerSegmentationStep::New();
  vtkChangeTrackerTypeStep* step5 = vtkChangeTrackerTypeStep::New();

    {
    typedef vtkChangeTrackerGUI Superclass, MySuperclass;
    EXERCISE_BASIC_OBJECT_METHODS( gui );
    }

    {
    typedef vtkChangeTrackerAnalysisStep Superclass, MySuperclass;
    EXERCISE_BASIC_OBJECT_METHODS( step1 );
    }

    {
    typedef vtkChangeTrackerFirstScanStep Superclass, MySuperclass;
    EXERCISE_BASIC_OBJECT_METHODS( step2 );
    }

    {
    typedef vtkChangeTrackerROIStep Superclass, MySuperclass;
    EXERCISE_BASIC_OBJECT_METHODS( step3 );
    }

    {
    typedef vtkChangeTrackerSegmentationStep Superclass, MySuperclass;
    EXERCISE_BASIC_OBJECT_METHODS( step4 );
    }

    {
    typedef vtkChangeTrackerTypeStep Superclass, MySuperclass;
    EXERCISE_BASIC_OBJECT_METHODS( step5 );
    }

  gui->Delete();
  step1->Delete();
  step2->Delete();
  step3->Delete();
  step4->Delete();
  step5->Delete();

  return EXIT_SUCCESS;
}

