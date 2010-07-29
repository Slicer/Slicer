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
#include "qSlicerMRMLTreeModuleWidget.h"
#include "ui_qSlicerMRMLTreeModule.h"

//-----------------------------------------------------------------------------
class qSlicerMRMLTreeModuleWidgetPrivate: public ctkPrivate<qSlicerMRMLTreeModuleWidget>,
                                          public Ui_qSlicerMRMLTreeModule
{
public:
  qSlicerMRMLTreeModuleWidgetPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerMRMLTreeModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerMRMLTreeModuleWidget::setup()
{
  CTK_D(qSlicerMRMLTreeModuleWidget);
  d->setupUi(this);

}
