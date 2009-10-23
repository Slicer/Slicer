#ifndef __qCTKExtensionFactory_h
#define __qCTKExtensionFactory_h

#include <QtDesigner/QExtensionFactory>

class QExtensionManager;

class qCTKExtensionFactory: public QExtensionFactory
{
    Q_OBJECT

public:
    qCTKExtensionFactory(QExtensionManager *parent = 0);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif
