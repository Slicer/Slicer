#include "qMRMLThreeDViewInformationWidgetPlugin.h"
#include "qMRMLThreeDViewInformationWidget.h"

// --------------------------------------------------------------------------
qMRMLThreeDViewInformationWidgetPlugin::qMRMLThreeDViewInformationWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLThreeDViewInformationWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLThreeDViewInformationWidget* _widget = new qMRMLThreeDViewInformationWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLThreeDViewInformationWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLThreeDViewInformationWidget\" \
          name=\"MRMLThreeDViewInformationWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLThreeDViewInformationWidgetPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qMRMLThreeDViewInformationWidgetPlugin::includeFile() const
{
  return "qMRMLThreeDViewInformationWidget.h";
}

// --------------------------------------------------------------------------
bool qMRMLThreeDViewInformationWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLThreeDViewInformationWidgetPlugin::name() const
{
  return "qMRMLThreeDViewInformationWidget";
}
