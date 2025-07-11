#include "qMRMLSliceVerticalControllerWidgetPlugin.h"
#include "qMRMLSliceVerticalControllerWidget.h"

// --------------------------------------------------------------------------
qMRMLSliceVerticalControllerWidgetPlugin::qMRMLSliceVerticalControllerWidgetPlugin(QObject* _parent)
  : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget* qMRMLSliceVerticalControllerWidgetPlugin::createWidget(QWidget* _parent)
{
  qMRMLSliceVerticalControllerWidget* _widget = new qMRMLSliceVerticalControllerWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLSliceVerticalControllerWidgetPlugin::domXml() const
{
  return "<ui language=\"c++\">\n"
         "<widget class=\"qMRMLSliceVerticalControllerWidget\" name=\"MRMLSliceVerticalControllerWidget\">\n"
         "</widget>\n"
         "</ui>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLSliceVerticalControllerWidgetPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qMRMLSliceVerticalControllerWidgetPlugin::includeFile() const
{
  return "qMRMLSliceVerticalControllerWidget.h";
}

// --------------------------------------------------------------------------
bool qMRMLSliceVerticalControllerWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLSliceVerticalControllerWidgetPlugin::name() const
{
  return "qMRMLSliceVerticalControllerWidget";
}
