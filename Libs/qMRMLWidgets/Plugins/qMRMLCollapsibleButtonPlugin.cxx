
// qMRML includes
#include "qMRMLCollapsibleButtonPlugin.h"
#include "qMRMLCollapsibleButton.h"

//-----------------------------------------------------------------------------
qMRMLCollapsibleButtonPlugin::qMRMLCollapsibleButtonPlugin(QObject *_parent):QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLCollapsibleButtonPlugin::createWidget(QWidget *_parent)
{
  qMRMLCollapsibleButton* _widget = new qMRMLCollapsibleButton(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qMRMLCollapsibleButtonPlugin::domXml() const
{
  return "<widget class=\"qMRMLCollapsibleButton\" \
          name=\"MRMLCollapsibleButton\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qMRMLCollapsibleButtonPlugin::includeFile() const
{
  return "qMRMLCollapsibleButton.h";
}

//-----------------------------------------------------------------------------
bool qMRMLCollapsibleButtonPlugin::isContainer() const
{
  return true;
}

//-----------------------------------------------------------------------------
QString qMRMLCollapsibleButtonPlugin::name() const
{
  return "qMRMLCollapsibleButton";
}
