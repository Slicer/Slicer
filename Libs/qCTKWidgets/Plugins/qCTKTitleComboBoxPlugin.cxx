#include "qCTKTitleComboBoxPlugin.h"
#include "qCTKTitleComboBox.h"

qCTKTitleComboBoxPlugin::qCTKTitleComboBoxPlugin(QObject *_parent)
        : QObject(_parent)
{

}

QWidget *qCTKTitleComboBoxPlugin::createWidget(QWidget *_parent)
{
  qCTKTitleComboBox* _widget = new qCTKTitleComboBox(_parent);
  return _widget;
}

QString qCTKTitleComboBoxPlugin::domXml() const
{
  return "<widget class=\"qCTKTitleComboBox\" \
          name=\"CTKTitleComboBox\">\n"
          "</widget>\n";
}
// --------------------------------------------------------------------------
QIcon qCTKTitleComboBoxPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

QString qCTKTitleComboBoxPlugin::includeFile() const
{
  return "qCTKTitleComboBox.h";
}

bool qCTKTitleComboBoxPlugin::isContainer() const
{
  return false;
}

QString qCTKTitleComboBoxPlugin::name() const
{
  return "qCTKTitleComboBox";
}
