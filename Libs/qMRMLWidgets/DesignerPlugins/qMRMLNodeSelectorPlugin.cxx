#include "qMRMLNodeSelectorPlugin.h"
#include "qMRMLNodeSelector.h"

qMRMLNodeSelectorPlugin::qMRMLNodeSelectorPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLNodeSelectorPlugin::createWidget(QWidget *_parent)
{
  qMRMLNodeSelector* _widget = new qMRMLNodeSelector(_parent);
  return _widget;
}

QString qMRMLNodeSelectorPlugin::domXml() const
{
  return "<widget class=\"qMRMLNodeSelector\" \
          name=\"MRMLNodeSelector\">\n"
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

QIcon qMRMLNodeSelectorPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

QString qMRMLNodeSelectorPlugin::includeFile() const
{
  return "qMRMLNodeSelector.h";
}

bool qMRMLNodeSelectorPlugin::isContainer() const
{
  return false;
}

QString qMRMLNodeSelectorPlugin::name() const
{
  return "qMRMLNodeSelector";
}
