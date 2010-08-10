#include <ctkIconEnginePlugin.h>

/// Customized ctkIconEnginePlugin to look for icons in specific directories.
class qSlicerIconEnginePlugin: public ctkIconEnginePlugin
{
  Q_OBJECT
public:
  qSlicerIconEnginePlugin(QObject* parent = 0);
};
