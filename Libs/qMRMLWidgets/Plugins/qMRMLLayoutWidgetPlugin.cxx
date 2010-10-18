#include "qMRMLLayoutWidgetPlugin.h"
#include "qMRMLLayoutWidget.h"

qMRMLLayoutWidgetPlugin::qMRMLLayoutWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

QWidget *qMRMLLayoutWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLLayoutWidget* _widget = new qMRMLLayoutWidget(_parent);
  return _widget;
}

QString qMRMLLayoutWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLLayoutWidget\" \
          name=\"MRMLLayoutWidget\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>200</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

QIcon qMRMLLayoutWidgetPlugin::icon() const
{
  return QIcon(":/Icons/table.png");
}

QString qMRMLLayoutWidgetPlugin::includeFile() const
{
  return "qMRMLLayoutWidget.h";
}

bool qMRMLLayoutWidgetPlugin::isContainer() const
{
  return false;
}

QString qMRMLLayoutWidgetPlugin::name() const
{
  return "qMRMLLayoutWidget";
}
