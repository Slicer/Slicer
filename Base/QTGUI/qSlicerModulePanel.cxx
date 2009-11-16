#include "qSlicerModulePanel.h"

#include "qSlicerAbstractModule.h"
#include "qCTKCollapsibleWidget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QStackedWidget>
#include <QResizeEvent>
//---------------------------------------------------------------------------
struct qSlicerModulePanel::qInternal
{
  QLabel*                HelpLabel;
  //QStackedWidget*        ModuleContainer;
  QBoxLayout*            Layout;
  QScrollArea*           ScrollArea;
};

//---------------------------------------------------------------------------
qSlicerModulePanel::qSlicerModulePanel(QWidget* parent, Qt::WindowFlags f)
  :qSlicerAbstractModulePanel(parent, f)
{
  this->Internal = new qInternal;
  QWidget* panel = new QWidget;
  qCTKCollapsibleWidget* help = new qCTKCollapsibleWidget;
  help->setTitle("Help");
  help->setCollapsed(true);
  help->setSizePolicy(
    QSizePolicy::Ignored, help->sizePolicy().verticalPolicy());
  this->Internal->HelpLabel = new QLabel;
  this->Internal->HelpLabel->setWordWrap(true);
  this->Internal->HelpLabel->setTextFormat(Qt::RichText);
  QGridLayout* helpLayout = new QGridLayout(help->widget());
  helpLayout->addWidget(this->Internal->HelpLabel);

  this->Internal->Layout = new QVBoxLayout(panel);
  this->Internal->Layout->addWidget(help);
  this->Internal->Layout->addStretch(1);

  this->Internal->ScrollArea = new QScrollArea;
  this->Internal->ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->Internal->ScrollArea->setWidget(panel);
  this->Internal->ScrollArea->setWidgetResizable(true);
  
  QGridLayout* gridLayout = new QGridLayout(this);
  gridLayout->addWidget(this->Internal->ScrollArea);
  gridLayout->setContentsMargins(0,0,0,0);
  this->setLayout(gridLayout);
}

//---------------------------------------------------------------------------
qSlicerModulePanel::~qSlicerModulePanel()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void qSlicerModulePanel::addModule(qSlicerAbstractModule* module)
{
  qDebug() << "qSlicerModulePanel::" << __FUNCTION__ << ": " << (module ? module->title() : "0");
  QLayoutItem* item = this->Internal->Layout->itemAt(1);
  QWidget* widget = item ? item->widget() : 0;
  if (widget)
    {
    qSlicerAbstractModule* oldModule = qobject_cast<qSlicerAbstractModule *>(widget);
    if (oldModule == module)
      {
      return;
      }
    emit moduleAboutToBeHidden(oldModule);
    this->Internal->Layout->removeWidget(widget);
    emit moduleHidden(oldModule);
    // if nobody took ownership of the module, make sure it lost its parent.
    if (widget->parent() == this->Internal->Layout->parentWidget())
      {
      widget->setParent(0);
      }
    }
  if (module)
    {
    module->layout()->setContentsMargins(0, 0, 0, 0);
    this->Internal->Layout->insertWidget(1, module);
    // If the module was invisible, make it visible in the panel.
    // (the panel might be invisible though. but this is another story).
    module->setVisible(true);
    this->Internal->HelpLabel->setText(module->helpText());
    }
  else
    {
    this->Internal->HelpLabel->setText("");
    }
  emit moduleAdded(module);
}
