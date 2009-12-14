#ifndef __qCTKExtensionFactory_h
#define __qCTKExtensionFactory_h

#include <QtDesigner/QExtensionFactory>
#include "qCTKWidgetsPluginExport.h"

class QExtensionManager;

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKExtensionFactory: public QExtensionFactory
{
    Q_OBJECT

public:
    qCTKExtensionFactory(QExtensionManager *parent = 0);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif
