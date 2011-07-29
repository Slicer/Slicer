#include "qMRMLSliceInformationWidgetPlugin.h"
#include "qMRMLSliceInformationWidget.h"

// --------------------------------------------------------------------------
qMRMLSliceInformationWidgetPlugin::qMRMLSliceInformationWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLSliceInformationWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLSliceInformationWidget* _widget = new qMRMLSliceInformationWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLSliceInformationWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLSliceInformationWidget\" \
          name=\"MRMLSliceInformationWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLSliceInformationWidgetPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qMRMLSliceInformationWidgetPlugin::includeFile() const
{
  return "qMRMLSliceInformationWidget.h";
}

// --------------------------------------------------------------------------
bool qMRMLSliceInformationWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLSliceInformationWidgetPlugin::name() const
{
  return "qMRMLSliceInformationWidget";
}

