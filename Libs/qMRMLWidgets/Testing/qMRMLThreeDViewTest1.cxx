/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// QT includes
#include <QApplication>

// qMRML includes
#include "qMRMLThreeDView.h"

// STD includes
#include <cstdlib>
#include <iostream>

int qMRMLThreeDViewTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);
  qMRMLThreeDView view;
  view.show();
  return EXIT_SUCCESS;
}

