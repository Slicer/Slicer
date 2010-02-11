#include "qCTKAddRemoveComboBoxPlugin.h"
#include "qCTKAddRemoveComboBox.h"

qCTKAddRemoveComboBoxPlugin::qCTKAddRemoveComboBoxPlugin(QObject *_parent)
        : QObject(_parent)
{

}

QWidget *qCTKAddRemoveComboBoxPlugin::createWidget(QWidget *_parent)
{
  qCTKAddRemoveComboBox* _widget = new qCTKAddRemoveComboBox(_parent);
  return _widget;
}

QString qCTKAddRemoveComboBoxPlugin::domXml() const
{
  return "<widget class=\"qCTKAddRemoveComboBox\" \
          name=\"CTKNumericInput\">\n"
          "</widget>\n";
}

QString qCTKAddRemoveComboBoxPlugin::includeFile() const
{
  return "qCTKAddRemoveComboBox.h";
}

bool qCTKAddRemoveComboBoxPlugin::isContainer() const
{
  return false;
}

QString qCTKAddRemoveComboBoxPlugin::name() const
{
  return "qCTKAddRemoveComboBox";
}
