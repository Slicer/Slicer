#include <QExtensionManager>
#include <QExtensionFactory>
#include <QDesignerFormEditorInterface>
#include <QDesignerContainerExtension>

#include "qCTKCollapsibleWidgetPlugin.h"
#include "qCTKCollapsibleWidget.h"
#include "qCTKExtensionFactory.h"

qCTKCollapsibleWidgetPlugin::qCTKCollapsibleWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qCTKCollapsibleWidgetPlugin::createWidget(QWidget *_parent)
{
  qCTKCollapsibleWidget* _widget = new qCTKCollapsibleWidget(_parent);
  return _widget;
}


QString qCTKCollapsibleWidgetPlugin::domXml() const
{
  return QLatin1String("\
 <ui language=\"c++\">                                                  \
     <widget class=\"qCTKCollapsibleWidget\" name=\"CTKCollapsibleWidget\"> \
       <property name=\"geometry\">                                     \
        <rect>                                                          \
         <x>0</x>                                                       \
         <y>0</y>                                                       \
         <width>300</width>                                             \
         <height>100</height>                                           \
        </rect>                                                         \
       </property>                                                      \
       <property name=\"contentsFrameShape\">                           \
         <enum>QFrame::StyledPanel</enum>                               \
       </property>                                                      \
       <property name=\"contentsFrameShadow\">                          \
         <enum>QFrame::Raised</enum>                                    \
       </property>                                                      \
       <widget class=\"QWidget\" name=\"page\" />                       \
     </widget>                                                          \
     <customwidgets>                                                    \
         <customwidget>                                                 \
             <class>qCTKCollapsibleWidget</class>                       \
             <extends>QWidget</extends>                                 \
             <addpagemethod>setWidget</addpagemethod>                   \
         </customwidget>                                                \
     </customwidgets>                                                   \
 </ui>");
}

QString qCTKCollapsibleWidgetPlugin::includeFile() const
{
  return "qCTKCollapsibleWidget.h";
}

bool qCTKCollapsibleWidgetPlugin::isContainer() const
{
  return true;
}

QString qCTKCollapsibleWidgetPlugin::name() const
{
  return "qCTKCollapsibleWidget";
}

void qCTKCollapsibleWidgetPlugin::initialize(QDesignerFormEditorInterface *formEditor)
{
  if (this->Initialized)
    {
    return;
    }
  // set Initialized to true
  this->qCTKWidgetsAbstractPlugin::initialize(formEditor);

  QExtensionManager *manager = formEditor->extensionManager();
  QExtensionFactory *factory = new qCTKExtensionFactory(manager);
  
  Q_ASSERT(manager != 0);
  manager->registerExtensions(factory, Q_TYPEID(QDesignerContainerExtension));
}
