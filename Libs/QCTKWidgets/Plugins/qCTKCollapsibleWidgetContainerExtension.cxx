#include "qCTKCollapsibleWidgetContainerExtension.h"
#include "qCTKCollapsibleWidget.h"

qCTKCollapsibleWidgetContainerExtension::qCTKCollapsibleWidgetContainerExtension(
  qCTKCollapsibleWidget *widget,
  QObject *parent)
    :QObject(parent)
{
    myWidget = widget;
}

void qCTKCollapsibleWidgetContainerExtension::addWidget(QWidget *widget)
{
  myWidget->setWidget(widget);//myWidget->addPage(widget);
}

int qCTKCollapsibleWidgetContainerExtension::count() const
{
  return 1;//return myWidget->count();
}

int qCTKCollapsibleWidgetContainerExtension::currentIndex() const
{
  return 0;//return myWidget->currentIndex();
}

void qCTKCollapsibleWidgetContainerExtension::insertWidget(int index, QWidget *widget)
{
  myWidget->setWidget(widget);
}

void qCTKCollapsibleWidgetContainerExtension::remove(int index)
{
//  myWidget->removePage(index);
}

void qCTKCollapsibleWidgetContainerExtension::setCurrentIndex(int index)
{
  //myWidget->setCurrentIndex(index);
}

QWidget* qCTKCollapsibleWidgetContainerExtension::widget(int index) const
{
  return myWidget->widget();//return myWidget->widget(index);
}
