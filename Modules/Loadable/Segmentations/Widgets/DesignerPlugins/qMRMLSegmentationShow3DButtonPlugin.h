/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qMRMLSegmentationShow3DButtonPlugin_h
#define __qMRMLSegmentationShow3DButtonPlugin_h

#include "qSlicerSegmentationsModuleWidgetsAbstractPlugin.h"

class Q_SLICER_MODULE_SEGMENTATIONS_WIDGETS_PLUGINS_EXPORT qMRMLSegmentationShow3DButtonPlugin
  : public QObject
  , public qSlicerSegmentationsModuleWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLSegmentationShow3DButtonPlugin(QObject* parent = nullptr);

  QWidget *createWidget(QWidget* parent) override;
  QString  domXml() const override;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif
