#include "qMRMLNodeComboBoxPlugin.h"
#include "qMRMLNodeComboBox.h"

qMRMLNodeComboBoxPlugin::qMRMLNodeComboBoxPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLNodeComboBoxPlugin::createWidget(QWidget *_parent)
{
  qMRMLNodeComboBox* _widget = new qMRMLNodeComboBox(_parent);
  return _widget;
}

QString qMRMLNodeComboBoxPlugin::domXml() const
{
  return "<widget class=\"qMRMLNodeComboBox\" \
          name=\"MRMLNodeComboBox\">\n"
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

QString qMRMLNodeComboBoxPlugin::includeFile() const
{
  return "qMRMLNodeComboBox.h";
}

bool qMRMLNodeComboBoxPlugin::isContainer() const
{
  return false;
}

QString qMRMLNodeComboBoxPlugin::name() const
{
  return "qMRMLNodeComboBox";
}
