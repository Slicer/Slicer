/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerSceneIO_h
#define __qSlicerSceneIO_h

// QtCore includes
#include "qSlicerIO.h"

///
/// qSlicerSceneIO is the IO class that handle MRML scene
/// It internally call vtkMRMLScene::Connect() or vtkMRMLScene::Import() 
/// depending on the clear flag.
class qSlicerSceneIO: public qSlicerIO
{
  Q_OBJECT
public: 
  qSlicerSceneIO(QObject* _parent = 0);
  
  virtual QString description()const;
  ///
  /// Support qSlicerIO::SceneFile
  virtual qSlicerIO::IOFileType fileType()const;

  ///
  /// Support only .mrml files
  virtual QString extensions()const;

  ///
  /// the supported properties are:
  /// QString fileName: the path of the mrml scene to load
  /// bool clear: wether the current should be cleared or not
  virtual bool load(const qSlicerIO::IOProperties& properties);
};


#endif
