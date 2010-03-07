#include "qMRMLWindowLevelWidgetPlugin.h"
#include "qMRMLWindowLevelWidget.h"

qMRMLWindowLevelWidgetPlugin::qMRMLWindowLevelWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLWindowLevelWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLWindowLevelWidget* _widget = new qMRMLWindowLevelWidget(_parent);
  return _widget;
}

QString qMRMLWindowLevelWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLWindowLevelWidget\" \
          name=\"MRMLWindowLevelWidget\">\n"
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

QIcon qMRMLWindowLevelWidgetPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

QString qMRMLWindowLevelWidgetPlugin::includeFile() const
{
  return "qMRMLWindowLevelWidget.h";
}

bool qMRMLWindowLevelWidgetPlugin::isContainer() const
{
  return false;
}

QString qMRMLWindowLevelWidgetPlugin::name() const
{
  return "qMRMLWindowLevelWidget";
}
