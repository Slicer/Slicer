#include "qMRMLScalarsDisplayWidgetPlugin.h"
#include "qMRMLScalarsDisplayWidget.h"

qMRMLScalarsDisplayWidgetPlugin::qMRMLScalarsDisplayWidgetPlugin(QObject* _parent)
  : QObject(_parent)
{
}

QWidget* qMRMLScalarsDisplayWidgetPlugin::createWidget(QWidget* _parent)
{
  qMRMLScalarsDisplayWidget* _widget = new qMRMLScalarsDisplayWidget(_parent);
  return _widget;
}

QString qMRMLScalarsDisplayWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLScalarsDisplayWidget\" \
          name=\"MRMLScalarsDisplayWidget\">\n"
         " <property name=\"geometry\">\n"
         "  <rect>\n"
         "   <x>0</x>\n"
         "   <y>0</y>\n"
         "   <width>200</width>\n"
         "   <height>20</height>\n"
         "  </rect>\n"
         " </property>\n"
         "</widget>\n";
}

QIcon qMRMLScalarsDisplayWidgetPlugin::icon() const
{
  return QIcon(":/Icons/table.png");
}

QString qMRMLScalarsDisplayWidgetPlugin::includeFile() const
{
  return "qMRMLScalarsDisplayWidget.h";
}

bool qMRMLScalarsDisplayWidgetPlugin::isContainer() const
{
  return false;
}

QString qMRMLScalarsDisplayWidgetPlugin::name() const
{
  return "qMRMLScalarsDisplayWidget";
}
