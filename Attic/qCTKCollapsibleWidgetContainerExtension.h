#ifndef __qCTKCollapsibleWidgetContainerExtension_h
#define __qCTKCollapsibleWidgetContainerExtension_h

#include <QtDesigner/QDesignerContainerExtension>
#include "qCTKWidgetsPluginExport.h"

class QExtensionManager;
class qCTKCollapsibleWidget;

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKCollapsibleWidgetContainerExtension: public QObject,
                                               public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension);

public:
    qCTKCollapsibleWidgetContainerExtension(qCTKCollapsibleWidget *widget, QObject *parent);

    void addWidget(QWidget *widget);
    int count() const;
    int currentIndex() const;
    void insertWidget(int index, QWidget *widget);
    void remove(int index);
    void setCurrentIndex(int index);
    QWidget *widget(int index) const;

private:
    qCTKCollapsibleWidget *myWidget;
};

#endif
