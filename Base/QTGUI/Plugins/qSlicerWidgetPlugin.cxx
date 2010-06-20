#include "qSlicerWidgetPlugin.h"
#include "qSlicerWidget.h"
#include "qSlicerApplication.h" // For qSlicerApplyPalette function

// --------------------------------------------------------------------------
qSlicerWidgetPlugin::qSlicerWidgetPlugin()
{
}

// --------------------------------------------------------------------------
QWidget *qSlicerWidgetPlugin::createWidget(QWidget *_parent)
{
  qSlicerWidget* _widget = new qSlicerWidget(_parent);
  QPalette slicerPalette = _widget->palette();

  // Apply Slicer Palette using the non-member function defined in qSlicerApplication
  qSlicerApplyPalette(slicerPalette);

  _widget->setPalette(slicerPalette);
  _widget->setAutoFillBackground(true);
  return _widget;
}

// --------------------------------------------------------------------------
QString qSlicerWidgetPlugin::domXml() const
{
  return "<widget class=\"qSlicerWidget\" \
          name=\"SlicerWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qSlicerWidgetPlugin::icon() const
{
  return QIcon(":/Icons/widget.png");
}

// --------------------------------------------------------------------------
QString qSlicerWidgetPlugin::includeFile() const
{
  return "qSlicerWidget.h";
}

// --------------------------------------------------------------------------
bool qSlicerWidgetPlugin::isContainer() const
{
  return true;
}

// --------------------------------------------------------------------------
QString qSlicerWidgetPlugin::name() const
{
  return "qSlicerWidget";
}
