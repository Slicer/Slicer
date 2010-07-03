#include "qMRMLROIWidgetPlugin.h"
#include "qMRMLROIWidget.h"

qMRMLROIWidgetPlugin::qMRMLROIWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLROIWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLROIWidget* _widget = new qMRMLROIWidget(_parent);
  return _widget;
}

QString qMRMLROIWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLROIWidget\" \
          name=\"MRMLROIWidget\">\n"
          "</widget>\n";
}

QIcon qMRMLROIWidgetPlugin::icon() const
{
  return QIcon(":/Icons/sliderspinbox.png");
}

QString qMRMLROIWidgetPlugin::includeFile() const
{
  return "qMRMLROIWidget.h";
}

bool qMRMLROIWidgetPlugin::isContainer() const
{
  return false;
}

QString qMRMLROIWidgetPlugin::name() const
{
  return "qMRMLROIWidget";
}
