#include "qCTKDynamicSpacerPlugin.h"
#include "qCTKDynamicSpacer.h"

// --------------------------------------------------------------------------
qCTKDynamicSpacerPlugin::qCTKDynamicSpacerPlugin(QObject *_parent)
  : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qCTKDynamicSpacerPlugin::createWidget(QWidget *_parent)
{
  qCTKDynamicSpacer* _widget = new qCTKDynamicSpacer(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qCTKDynamicSpacerPlugin::domXml() const
{
  return "<widget class=\"qCTKDynamicSpacer\" name=\"CTKSpacer\">\n"
    "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qCTKDynamicSpacerPlugin::icon() const
{
  return QIcon(":/Icons/vspacer.png");
}

// --------------------------------------------------------------------------
QString qCTKDynamicSpacerPlugin::includeFile() const
{
  return "qCTKDynamicSpacer.h";
}

// --------------------------------------------------------------------------
bool qCTKDynamicSpacerPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qCTKDynamicSpacerPlugin::name() const
{
  return "qCTKDynamicSpacer";
}
