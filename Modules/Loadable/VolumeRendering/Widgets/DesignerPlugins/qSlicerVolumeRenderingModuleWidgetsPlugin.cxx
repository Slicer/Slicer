#include "qSlicerVolumeRenderingModuleWidgetsPlugin.h"

#include <QtGlobal>

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(customwidgetplugin, qSlicerVolumeRenderingModuleWidgetsPlugin);
#endif
