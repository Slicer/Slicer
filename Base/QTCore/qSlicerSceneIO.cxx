/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// QtCore includes
#include "qSlicerSceneIO.h"

// MRML includes
#include <vtkMRMLScene.h>

//-----------------------------------------------------------------------------
qSlicerSceneIO::qSlicerSceneIO(QObject* _parent)
  :qSlicerIO(_parent)
{
}

//-----------------------------------------------------------------------------
QString qSlicerSceneIO::description()const
{
  return "MRML Scene";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSceneIO::fileType()const
{
  return qSlicerIO::SceneFile;
}

//-----------------------------------------------------------------------------
QString qSlicerSceneIO::extensions()const 
{
  return "*.mrml";
}

//-----------------------------------------------------------------------------
bool qSlicerSceneIO::load(const qSlicerIO::IOProperties& properties)
{
  Q_ASSERT(properties.contains("fileName"));
  QString file = properties["fileName"].toString();
  this->mrmlScene()->SetURL(file.toLatin1().data());
  bool clear = false;
  if (properties.contains("clear"))
    {
    clear = properties["clear"].toBool();
    }
  int res = 0;
  if (clear)
    {
    res = this->mrmlScene()->Connect();
    }
  else
    {
    res = this->mrmlScene()->Import();
    }
  return res;
}
