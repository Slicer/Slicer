
// qMRML includes
#include "qMRMLWidgetPlugin.h"
#include "qMRMLWidget.h"

// --------------------------------------------------------------------------
qMRMLWidgetPlugin::qMRMLWidgetPlugin(QObject *_parent):QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLWidget* _widget = new qMRMLWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLWidget\" \
          name=\"MRMLWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLWidgetPlugin::icon() const
{
  return QIcon(":/Icons/widget.png");
}

// --------------------------------------------------------------------------
QString qMRMLWidgetPlugin::includeFile() const
{
  return "qMRMLWidget.h";
}

// --------------------------------------------------------------------------
bool qMRMLWidgetPlugin::isContainer() const
{
  return true;
}

// --------------------------------------------------------------------------
QString qMRMLWidgetPlugin::name() const
{
  return "qMRMLWidget";
}
