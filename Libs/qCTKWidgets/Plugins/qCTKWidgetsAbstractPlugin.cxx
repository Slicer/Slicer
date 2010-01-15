#include "qCTKWidgetsAbstractPlugin.h"

qCTKWidgetsAbstractPlugin::qCTKWidgetsAbstractPlugin()
{
  this->Initialized = false;
}

QString qCTKWidgetsAbstractPlugin::group() const 
{ 
  return "Slicer [CTK Widgets]"; 
}

QIcon qCTKWidgetsAbstractPlugin::icon() const
{
  return QIcon(); 
}

QString qCTKWidgetsAbstractPlugin::toolTip() const 
{ 
  return QString(); 
}

QString qCTKWidgetsAbstractPlugin::whatsThis() const 
{
  return QString(); 
}

void qCTKWidgetsAbstractPlugin::initialize(QDesignerFormEditorInterface *formEditor)
{
  Q_UNUSED(formEditor);
  if (this->Initialized)
    {
    return;
    }
  this->Initialized = true;
}
