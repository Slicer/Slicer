#include "qCTKCollapsibleButtonPlugin.h"
#include "qCTKCollapsibleButton.h"

qCTKCollapsibleButtonPlugin::qCTKCollapsibleButtonPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qCTKCollapsibleButtonPlugin::createWidget(QWidget *_parent)
{
  qCTKCollapsibleButton* _widget = new qCTKCollapsibleButton(_parent);
  return _widget;
}


QString qCTKCollapsibleButtonPlugin::domXml() const
{
  return "<widget class=\"qCTKCollapsibleButton\" \
          name=\"CTKCollapsibleButton\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>300</width>\n"
          "   <height>100</height>\n"
          "  </rect>\n"
          " </property>\n"
          " <property name=\"text\">"
          "  <string>CollapsibleButton</string>"
          " </property>"
          " <property name=\"contentsFrameShape\">"
          "  <enum>QFrame::StyledPanel</enum>"
          " </property>"
          "</widget>\n";
}

QString qCTKCollapsibleButtonPlugin::includeFile() const
{
  return "qCTKCollapsibleButton.h";
}

bool qCTKCollapsibleButtonPlugin::isContainer() const
{
  return true;
}

QString qCTKCollapsibleButtonPlugin::name() const
{
  return "qCTKCollapsibleButton";
}
