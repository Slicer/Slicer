/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerCoreModule.h"

//-----------------------------------------------------------------------------
class qSlicerCoreModulePrivate: public ctkPrivate<qSlicerCoreModule>
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerCoreModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerCoreModule::setup()
{
}

