/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qMRMLSegmentationShow3DButtonPlugin.h"
#include "qMRMLSegmentationShow3DButton.h"

//-----------------------------------------------------------------------------
qMRMLSegmentationShow3DButtonPlugin::qMRMLSegmentationShow3DButtonPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLSegmentationShow3DButtonPlugin::createWidget(QWidget* parentWidget)
{
  qMRMLSegmentationShow3DButton* pluginWidget =
    new qMRMLSegmentationShow3DButton(parentWidget);
  return pluginWidget;
}

//-----------------------------------------------------------------------------
QString qMRMLSegmentationShow3DButtonPlugin::domXml() const
{
  return "<widget class=\"qMRMLSegmentationShow3DButton\" \
          name=\"SegmentationShow3DButton\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qMRMLSegmentationShow3DButtonPlugin::includeFile() const
{
  return "qMRMLSegmentationShow3DButton.h";
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentationShow3DButtonPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLSegmentationShow3DButtonPlugin::name() const
{
  return "qMRMLSegmentationShow3DButton";
}
