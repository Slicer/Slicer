#include "qMRMLColorTableComboBoxPlugin.h"
#include "qMRMLColorTableComboBox.h"

qMRMLColorTableComboBoxPlugin
::qMRMLColorTableComboBoxPlugin(QObject *parentObject)
  : QObject(parentObject)
{
}

QWidget *qMRMLColorTableComboBoxPlugin::createWidget(QWidget *parentWidget)
{
  qMRMLColorTableComboBox* widget = new qMRMLColorTableComboBox(parentWidget);
  return widget;
}

QString qMRMLColorTableComboBoxPlugin::domXml() const
{
  return "<widget class=\"qMRMLColorTableComboBox\" \
          name=\"ColorTableComboBox\">\n"
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

QIcon qMRMLColorTableComboBoxPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

QString qMRMLColorTableComboBoxPlugin::includeFile() const
{
  return "qMRMLColorTableComboBox.h";
}

bool qMRMLColorTableComboBoxPlugin::isContainer() const
{
  return false;
}

QString qMRMLColorTableComboBoxPlugin::name() const
{
  return "qMRMLColorTableComboBox";
}
