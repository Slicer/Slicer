#include "qMRMLNodeTreeSelectorPlugin.h"
#include "qMRMLNodeTreeSelector.h"

qMRMLNodeTreeSelectorPlugin::qMRMLNodeTreeSelectorPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLNodeTreeSelectorPlugin::createWidget(QWidget *_parent)
{
  qMRMLNodeTreeSelector* _widget = new qMRMLNodeTreeSelector(_parent);
  return _widget;
}

QString qMRMLNodeTreeSelectorPlugin::domXml() const
{
  return "<widget class=\"qMRMLNodeTreeSelector\" \
          name=\"MRMLNodeTreeSelector\">\n"
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

QIcon qMRMLNodeTreeSelectorPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

QString qMRMLNodeTreeSelectorPlugin::includeFile() const
{
  return "qMRMLNodeTreeSelector.h";
}

bool qMRMLNodeTreeSelectorPlugin::isContainer() const
{
  return false;
}

QString qMRMLNodeTreeSelectorPlugin::name() const
{
  return "qMRMLNodeTreeSelector";
}
