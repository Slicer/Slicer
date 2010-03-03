#include "qCTKCollapsibleGroupBoxPlugin.h"
#include "qCTKCollapsibleGroupBox.h"
#include <QDebug>

// --------------------------------------------------------------------------
qCTKCollapsibleGroupBoxPlugin::qCTKCollapsibleGroupBoxPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qCTKCollapsibleGroupBoxPlugin::createWidget(QWidget *_parent)
{
  qCTKCollapsibleGroupBox* _widget = new qCTKCollapsibleGroupBox(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qCTKCollapsibleGroupBoxPlugin::domXml() const
{
  return "<widget class=\"qCTKCollapsibleGroupBox\" \
          name=\"CTKCollapsibleGroupBox\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>300</width>\n"
          "   <height>100</height>\n"
          "  </rect>\n"
          " </property>\n"
          " <property name=\"title\">"
          "  <string>GroupBox</string>"
          " </property>"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qCTKCollapsibleGroupBoxPlugin::icon() const
{
  return QIcon(":/Icons/groupboxcollapsible.png");
}

// --------------------------------------------------------------------------
QString qCTKCollapsibleGroupBoxPlugin::includeFile() const
{
  return "qCTKCollapsibleGroupBox.h";
}

// --------------------------------------------------------------------------
bool qCTKCollapsibleGroupBoxPlugin::isContainer() const
{
  return true;
}

// --------------------------------------------------------------------------
QString qCTKCollapsibleGroupBoxPlugin::name() const
{
  return "qCTKCollapsibleGroupBox";
}
