#include "qCTKWidgetsPlugin.h"

qCTKWidgetsPlugin::qCTKWidgetsPlugin()
{
  this->Initialized = false;
}

QString qCTKWidgetsPlugin::group() const 
{ 
  return "Slicer [CTK Widgets]"; 
}

QIcon qCTKWidgetsPlugin::icon() const
{
  return QIcon(); 
}

QString qCTKWidgetsPlugin::toolTip() const 
{ 
  return QString(); 
}

QString qCTKWidgetsPlugin::whatsThis() const 
{
  return QString(); 
}

void qCTKWidgetsPlugin::initialize(QDesignerFormEditorInterface *formEditor)
{
  if (this->Initialized)
    {
    return;
    }
  this->Initialized = true;
}
