#include "qCTKTreeComboBoxPlugin.h"
#include "qCTKTreeComboBox.h"

qCTKTreeComboBoxPlugin::qCTKTreeComboBoxPlugin(QObject *_parent)
        : QObject(_parent)
{

}

QWidget *qCTKTreeComboBoxPlugin::createWidget(QWidget *_parent)
{
  qCTKTreeComboBox* _widget = new qCTKTreeComboBox(_parent);
  return _widget;
}

QString qCTKTreeComboBoxPlugin::domXml() const
{
  return "<widget class=\"qCTKTreeComboBox\" \
          name=\"CTKTreeComboBox\">\n"
          "</widget>\n";
}
// --------------------------------------------------------------------------
QIcon qCTKTreeComboBoxPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

QString qCTKTreeComboBoxPlugin::includeFile() const
{
  return "qCTKTreeComboBox.h";
}

bool qCTKTreeComboBoxPlugin::isContainer() const
{
  return false;
}

QString qCTKTreeComboBoxPlugin::name() const
{
  return "qCTKTreeComboBox";
}
