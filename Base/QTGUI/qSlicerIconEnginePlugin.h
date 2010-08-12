#ifndef __qSlicerIconEnginePlugin_h
#define __qSlicerIconEnginePlugin_h

// Qt includes
#include <ctkIconEnginePlugin.h>

// SlicerQt includes
#include "qSlicerIconEnginePluginExport.h"

/// Customized ctkIconEnginePlugin to look for icons in specific directories.
/// By default it will look for icons in the Small/Medium/Large and XLarge
/// directories
class Q_SLICER_BASE_QTGUI_ICON_ENGINE_EXPORT qSlicerIconEnginePlugin: public ctkIconEnginePlugin
{
  Q_OBJECT
public:
  qSlicerIconEnginePlugin(QObject* parent = 0);
};

#endif
