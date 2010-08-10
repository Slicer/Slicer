#include "qSlicerIconEnginePlugin.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerIconEnginePlugin, qSlicerIconEnginePlugin)

//------------------------------------------------------------------------------
qSlicerIconEnginePlugin::qSlicerIconEnginePlugin(QObject* parentObject)
  :ctkIconEnginePlugin(parentObject)
{
  this->setSizeDirectories(QStringList() << "XLarge" << "Large" << "Medium" << "Small");
}
