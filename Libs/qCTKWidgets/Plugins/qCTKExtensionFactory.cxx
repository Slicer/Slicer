#include "qCTKExtensionFactory.h"

// #include "qCTKCollapsibleWidgetContainerExtension.h"
// #include "qCTKCollapsibleWidget.h"

qCTKExtensionFactory::qCTKExtensionFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{

}

QObject *qCTKExtensionFactory::createExtension(QObject *object,
                                               const QString &iid,
                                               QObject *parent) const
{
  // qCTKCollapsibleWidget *widget = qobject_cast<qCTKCollapsibleWidget*>(object);
  // 
  // if (widget && (iid == Q_TYPEID(QDesignerContainerExtension))) {
  //     return new qCTKCollapsibleWidgetContainerExtension(widget, parent);
  // } else {
  //     return 0;
  // }
  return 0; 
}
