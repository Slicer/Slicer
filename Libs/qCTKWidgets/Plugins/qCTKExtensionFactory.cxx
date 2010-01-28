#include "qCTKExtensionFactory.h"

// #include "qCTKCollapsibleWidgetContainerExtension.h"
// #include "qCTKCollapsibleWidget.h"

qCTKExtensionFactory::qCTKExtensionFactory(QExtensionManager *_parent)
    : QExtensionFactory(_parent)
{

}

QObject *qCTKExtensionFactory::createExtension(QObject *object,
                                               const QString &iid,
                                               QObject *_parent) const
{
  Q_UNUSED(object);
  Q_UNUSED(iid);
  Q_UNUSED(_parent);
  // qCTKCollapsibleWidget *widget = qobject_cast<qCTKCollapsibleWidget*>(object);
  // 
  // if (widget && (iid == Q_TYPEID(QDesignerContainerExtension))) {
  //     return new qCTKCollapsibleWidgetContainerExtension(widget, parent);
  // } else {
  //     return 0;
  // }
  return 0; 
}
