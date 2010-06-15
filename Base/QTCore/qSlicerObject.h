/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerObject_h
#define __qSlicerObject_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

#include "qSlicerBaseQTCoreExport.h"

class vtkMRMLScene;
class qSlicerObjectPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerObject
{
public:
  qSlicerObject();
  virtual ~qSlicerObject();

  ///
  /// Return a pointer on the MRML scene
  vtkMRMLScene* mrmlScene() const;

  /// Set the current MRML scene to the widget
  virtual void setMRMLScene(vtkMRMLScene*);

private:
  CTK_DECLARE_PRIVATE(qSlicerObject);
};

#endif
