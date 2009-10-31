#include "qMRMLWidgetsBasePlugin.h"

qMRMLWidgetsBasePlugin::qMRMLWidgetsBasePlugin()
{
}

QString qMRMLWidgetsBasePlugin::group() const 
{ 
  return "Slicer [MRML Widgets]"; 
}

QIcon qMRMLWidgetsBasePlugin::icon() const
{
  return QIcon(); 
}

QString qMRMLWidgetsBasePlugin::toolTip() const 
{ 
  return QString(); 
}

QString qMRMLWidgetsBasePlugin::whatsThis() const 
{
  return QString(); 
}


