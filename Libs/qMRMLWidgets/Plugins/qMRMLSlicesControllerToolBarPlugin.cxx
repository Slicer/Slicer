#include "qMRMLSlicesControllerToolBarPlugin.h"
#include "qMRMLSlicesControllerToolBar.h"

// --------------------------------------------------------------------------
qMRMLSlicesControllerToolBarPlugin::qMRMLSlicesControllerToolBarPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLSlicesControllerToolBarPlugin::createWidget(QWidget *_parent)
{
  qMRMLSlicesControllerToolBar* _widget = new qMRMLSlicesControllerToolBar(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLSlicesControllerToolBarPlugin::domXml() const
{
  return "<widget class=\"qMRMLSlicesControllerToolBar\" \
          name=\"MRMLSlicesControllerToolBar\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLSlicesControllerToolBarPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qMRMLSlicesControllerToolBarPlugin::includeFile() const
{
  return "qMRMLSlicesControllerToolBar.h";
}

// --------------------------------------------------------------------------
bool qMRMLSlicesControllerToolBarPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLSlicesControllerToolBarPlugin::name() const
{
  return "qMRMLSlicesControllerToolBar";
}

