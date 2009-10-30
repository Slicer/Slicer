#include "qSlicerWidget.h" 

#include <QScrollArea>
#include <QVBoxLayout>
#include <QPointer>
#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerWidget::qInternal
{
public:
  qInternal()
    {
    }
  QPointer<QWidget>       ParentContainer; 
};

//-----------------------------------------------------------------------------
qSlicerWidget::qSlicerWidget(QWidget *parent)
 :Superclass(parent)
{
  this->Internal = new qInternal;
}

//-----------------------------------------------------------------------------
qSlicerWidget::~qSlicerWidget()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
void qSlicerWidget::printAdditionalInfo()
{
  this->Superclass::dumpObjectInfo(); 
}

//-----------------------------------------------------------------------------
QWidget* qSlicerWidget::parentWidget()
{
  if (this->isParentContainerScrollArea())
    {
    return this->getScrollAreaParentContainer();
    }
  return qobject_cast<QWidget*>(this->parent());
}

//-----------------------------------------------------------------------------
bool qSlicerWidget::isParentContainerScrollArea()
{
  return (qobject_cast<QScrollArea*>(this->Internal->ParentContainer)!=0); 
}

//-----------------------------------------------------------------------------
QScrollArea* qSlicerWidget::getScrollAreaParentContainer()
{
  return qobject_cast<QScrollArea*>(this->Internal->ParentContainer);
}

//-----------------------------------------------------------------------------
void qSlicerWidget::setScrollAreaAsParentContainer(bool enable)
{
  if (enable)
    {
    if (this->getScrollAreaParentContainer())
      {
      return;
      }
      
    // Instanciate a scrollArea
    QScrollArea * scrollArea = new QScrollArea(); 
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);
    
    // Set window title
    scrollArea->setWindowTitle(this->windowTitle());
    
    // Add an content widget responsible for the layout
    QWidget * scrollAreaWidgetContents = new QWidget();
    this->setParent(scrollAreaWidgetContents);
    
    // Layout vertically and add a spacer/stretcher
    QVBoxLayout * verticalLayout = new QVBoxLayout(scrollAreaWidgetContents);
    verticalLayout->addWidget(this);
    verticalLayout->addStretch();
  
    // Add scrollAreaWidgetContents to the scrollArea
    scrollArea->setWidget(scrollAreaWidgetContents);
    
    this->Internal->ParentContainer = scrollArea;
    }
  else
    {
    if (!this->getScrollAreaParentContainer())
      {
      return;
      }
    this->getScrollAreaParentContainer()->takeWidget();
    this->setParent(0);
    this->Internal->ParentContainer->deleteLater();
    }
}

//-----------------------------------------------------------------------------
void qSlicerWidget::setWindowFlags(Qt::WindowFlags type)
{
  if (this->Internal->ParentContainer)
    {
    this->Internal->ParentContainer->setWindowFlags(type);
    return;
    }
  this->setWindowFlags(type);
}

//---------------------------------------------------------------------------
void qSlicerWidget::setParentGeometry(int ax, int ay, int aw, int ah)
{ 
  if (this->parentWidget()) 
    {
    this->parentWidget()->setGeometry(QRect(ax, ay, aw, ah));
    } 
  else
    {
    this->setGeometry(QRect(ax, ay, aw, ah));
    }
}

//-----------------------------------------------------------------------------
void qSlicerWidget::setParentVisible(bool visible)
{
   if (this->parentWidget())
     {
     this->parentWidget()->setVisible(visible);
     }
   else
     {
     this->Superclass::setVisible(visible);
     }
}
