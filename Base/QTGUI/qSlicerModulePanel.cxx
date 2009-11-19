#include "qSlicerModulePanel.h"

// SlicerQT includes
#include "qSlicerAbstractModule.h"

// CTK includes
#include "qCTKCollapsibleWidget2.h"

// QT includes
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QStackedWidget>
#include <QResizeEvent>
#include <QTextBrowser>

//---------------------------------------------------------------------------
struct qSlicerModulePanel::qInternal
{
  QTextBrowser*          HelpLabel;
  QBoxLayout*            Layout;
  QScrollArea*           ScrollArea;
};

//---------------------------------------------------------------------------
qSlicerModulePanel::qSlicerModulePanel(QWidget* parent, Qt::WindowFlags f)
  :qSlicerAbstractModulePanel(parent, f)
{
  this->Internal = new qInternal;
  QWidget* panel = new QWidget;
  qCTKCollapsibleWidget2* help = new qCTKCollapsibleWidget2("Help");
  help->setCollapsed(true);
  help->setSizePolicy(
    QSizePolicy::Ignored, help->sizePolicy().verticalPolicy());
  // QTextBrowser instead of QLabel because QLabel don't word wrap links
  // correctly
  this->Internal->HelpLabel = new QTextBrowser;
  this->Internal->HelpLabel->setOpenExternalLinks(true);
  this->Internal->HelpLabel->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->Internal->HelpLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->Internal->HelpLabel->setFrameShape(QFrame::NoFrame);
  QPalette p = this->Internal->HelpLabel->palette();
  p.setBrush(QPalette::Window, QBrush ());
  this->Internal->HelpLabel->setPalette(p);

  QGridLayout* helpLayout = new QGridLayout(help);
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
void qSlicerModulePanel::setModule(qSlicerAbstractModule* module)
{
  // Retrieve current module associated with the module panel
  QLayoutItem* item = this->Internal->Layout->itemAt(1);
  qSlicerAbstractModule* currentModule =
    item ? qobject_cast<qSlicerAbstractModule*>(item->widget()) : 0;

  // If module is already set, return.
  if (module == currentModule)
    {
    return;
    }

  if (currentModule)
    {
    // Remove the current module
    this->removeModule(currentModule);
    }

  if (module)
    {
    // Add the new module
    this->addModule(module);
    }
  else
    {
    this->Internal->HelpLabel->setHtml("");
    }
}

//---------------------------------------------------------------------------
void qSlicerModulePanel::clear()
{
  this->setModule(0);
}

//---------------------------------------------------------------------------
void qSlicerModulePanel::addModule(qSlicerAbstractModule* module)
{
  Q_ASSERT(module);

  // Update module layout
  module->layout()->setContentsMargins(0, 0, 0, 0);

  // Insert module in the panel
  this->Internal->Layout->insertWidget(1, module);

  module->setSizePolicy(QSizePolicy::Ignored, module->sizePolicy().verticalPolicy());
  module->setVisible(true);

  this->Internal->HelpLabel->setHtml(module->helpText());

  emit moduleAdded(module);
}

//---------------------------------------------------------------------------
void qSlicerModulePanel::removeModule(qSlicerAbstractModule* module)
{
  Q_ASSERT(module);

  int index = this->Internal->Layout->indexOf(module);
  if (index == -1)
    {
    return;
    }

  //emit moduleAboutToBeRemoved(module);

  // Remove widget from layout
  //this->Internal->Layout->removeWidget(module);
  this->Internal->Layout->takeAt(index);

  module->setVisible(false);
  module->setParent(0);

//   // if nobody took ownership of the module, make sure it both lost its parent and is hidden
//   if (module->parent() == this->Internal->Layout->parentWidget())
//     {
//     module->setVisible(false);
//     module->setParent(0);
//     }

  emit moduleRemoved(module);
}

//---------------------------------------------------------------------------
void qSlicerModulePanel::removeAllModule()
{
  this->clear();
}
