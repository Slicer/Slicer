/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerSegmentationsEditorEffectsPythonQtDecorators_h
#define __qSlicerSegmentationsEditorEffectsPythonQtDecorators_h

// PythonQt includes
#include <PythonQt.h>

// Slicer includes
#include "qSlicerSegmentEditorEffectFactory.h"

#include "qSlicerSegmentationsEditorEffectsExport.h"

// NOTE:
//
// For decorators it is assumed that the methods will never be called
// with the self argument as nullptr.  The self argument is the first argument
// for non-static methods.
//

class qSlicerSegmentationsEditorEffectsPythonQtDecorators : public QObject
{
  Q_OBJECT
public:

  qSlicerSegmentationsEditorEffectsPythonQtDecorators()
    {
    //PythonQt::self()->registerClass(&qSlicerSegmentEditorEffectFactory::staticMetaObject);
    // Note: Use registerCPPClassForPythonQt to register pure Cpp classes
    }

public slots:

  //----------------------------------------------------------------------------
  // qSlicerSegmentEditorEffectFactory

  //----------------------------------------------------------------------------
  // static methods

  //----------------------------------------------------------------------------
  qSlicerSegmentEditorEffectFactory* static_qSlicerSegmentEditorEffectFactory_instance()
    {
    return qSlicerSegmentEditorEffectFactory::instance();
    }

  //----------------------------------------------------------------------------
  // instance methods

  //----------------------------------------------------------------------------
  bool registerEffect(qSlicerSegmentEditorEffectFactory* factory,
                      PythonQtPassOwnershipToCPP<qSlicerSegmentEditorAbstractEffect*> plugin)
    {
    return factory->registerEffect(plugin);
    }
};

//-----------------------------------------------------------------------------
void initqSlicerSegmentationsEditorEffectsPythonQtDecorators()
{
  PythonQt::self()->addDecorators(new qSlicerSegmentationsEditorEffectsPythonQtDecorators);
}

#endif
