/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// qCTK includes
#include "qCTKAbstractFactory.h"
#include "qCTKDoubleSlider.h"

// QT includes
#include <QApplication>
#include <QList>
#include <QTreeWidget>
#include <QString>
#include <QStringList>

// std includes
#include <stdlib.h>
#include <iostream>

int qCTKAbstractFactoryTest1(int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qCTKAbstractFactory<qCTKDoubleSlider>  qctkObject;


  return EXIT_SUCCESS;
}

