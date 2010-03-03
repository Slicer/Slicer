#include "qCTKCoordinatesWidgetPlugin.h"
#include "qCTKCoordinatesWidget.h"

qCTKCoordinatesWidgetPlugin::qCTKCoordinatesWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qCTKCoordinatesWidgetPlugin::createWidget(QWidget *_parent)
{
  qCTKCoordinatesWidget* _widget = new qCTKCoordinatesWidget(_parent);
  return _widget;
}

QString qCTKCoordinatesWidgetPlugin::domXml() const
{
  return "<widget class=\"qCTKCoordinatesWidget\" \
          name=\"CTKCoordinatesWidget\">\n"
          "</widget>\n";
}

QIcon qCTKCoordinatesWidgetPlugin::icon() const
{
  return QIcon(":/Icons/doublespinbox.png");
}

QString qCTKCoordinatesWidgetPlugin::includeFile() const
{
  return "qCTKCoordinatesWidget.h";
}

bool qCTKCoordinatesWidgetPlugin::isContainer() const
{
  return false;
}

QString qCTKCoordinatesWidgetPlugin::name() const
{
  return "qCTKCoordinatesWidget";
}
