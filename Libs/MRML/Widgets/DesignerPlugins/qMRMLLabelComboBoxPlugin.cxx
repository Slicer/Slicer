
// qMRML includes
#include "qMRMLLabelComboBoxPlugin.h"
#include "qMRMLLabelComboBox.h"

//-----------------------------------------------------------------------------
qMRMLLabelComboBoxPlugin::qMRMLLabelComboBoxPlugin(QObject *_parent):QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLLabelComboBoxPlugin::createWidget(QWidget *_parent)
{
  qMRMLLabelComboBox* _widget = new qMRMLLabelComboBox(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qMRMLLabelComboBoxPlugin::domXml() const
{
  return "<widget class=\"qMRMLLabelComboBox\" \
          name=\"MRMLLabelComboBox\">\n"
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

//-----------------------------------------------------------------------------
QIcon qMRMLLabelComboBoxPlugin::icon() const
{
  return QIcon(":/Icons/combox.png");
}

//-----------------------------------------------------------------------------
QString qMRMLLabelComboBoxPlugin::includeFile() const
{
  return "qMRMLLabelComboBox.h";
}

//-----------------------------------------------------------------------------
bool qMRMLLabelComboBoxPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLLabelComboBoxPlugin::name() const
{
  return "qMRMLLabelComboBox";
}
