#include "qCTKWidgetsBasePlugin.h"

qCTKWidgetsBasePlugin::qCTKWidgetsBasePlugin()
{
  this->Initialized = false;
}

QString qCTKWidgetsBasePlugin::group() const 
{ 
  return "Slicer [CTK Widgets]"; 
}

QIcon qCTKWidgetsBasePlugin::icon() const
{
  return QIcon(); 
}

QString qCTKWidgetsBasePlugin::toolTip() const 
{ 
  return QString(); 
}

QString qCTKWidgetsBasePlugin::whatsThis() const 
{
  return QString(); 
}

void qCTKWidgetsBasePlugin::initialize(QDesignerFormEditorInterface *formEditor)
{
  if (this->Initialized)
    {
    return;
    }
  this->Initialized = true;
}
