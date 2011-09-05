/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>

// CTK includes
#include <ctkWidgetsUtils.h>

// SlicerQt includes
#include "qSlicerModulePanel.h"
#include "ui_qSlicerModulePanel.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerAbstractModuleWidget.h"

//---------------------------------------------------------------------------
class qSlicerModulePanelPrivate: public Ui_qSlicerModulePanel
{
public:
  void setupUi(qSlicerWidget* widget);
  /*
  QTextBrowser*          HelpLabel;
  //QWebView*              HelpLabel;
  QBoxLayout*            Layout;
  QScrollArea*           ScrollArea;
  ctkCollapsibleButton* HelpCollapsibleButton;
  */
};

//---------------------------------------------------------------------------
qSlicerModulePanel::qSlicerModulePanel(QWidget* _parent, Qt::WindowFlags f)
  :qSlicerAbstractModulePanel(_parent, f)
  , d_ptr(new qSlicerModulePanelPrivate)
{
  Q_D(qSlicerModulePanel);
  d->setupUi(this);
}

//---------------------------------------------------------------------------
qSlicerModulePanel::~qSlicerModulePanel()
{
}

//---------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerModulePanel::currentModule()const
{
  Q_D(const qSlicerModulePanel);
  QBoxLayout* scrollAreaLayout =
    qobject_cast<QBoxLayout*>(d->ScrollArea->widget()->layout());

  QLayoutItem* item = scrollAreaLayout->itemAt(1);

  qSlicerAbstractModuleWidget* currentModuleWidget =
    item ? qobject_cast<qSlicerAbstractModuleWidget*>(item->widget()) : 0;

  return currentModuleWidget ? currentModuleWidget->module() : 0;
}

//---------------------------------------------------------------------------
QString qSlicerModulePanel::currentModuleName()const
{
  qSlicerAbstractCoreModule* module = this->currentModule();
  return module ? module->name() : QString();
}

//---------------------------------------------------------------------------
void qSlicerModulePanel::setModule(const QString& moduleName)
{
  if (!this->moduleManager())
    {
    return;
    }

  qSlicerAbstractCoreModule * module = 0;
  if (!moduleName.isEmpty())
    {
    module = this->moduleManager()->module(moduleName);
    Q_ASSERT(module);
    }
  this->setModule(module);
}

//---------------------------------------------------------------------------
void qSlicerModulePanel::setModule(qSlicerAbstractCoreModule* module)
{
  // Retrieve current module associated with the module panel
  qSlicerAbstractCoreModule* oldModule = this->currentModule();
  if (module == oldModule)
    {
    return;
    }

  if (oldModule)
    {
    // Remove the current module
    this->removeModule(oldModule);
    }

  if (module)
    {
    qDebug() << "Show module (name):" << module->name();
    // Add the new module
    this->addModule(module);
    }
  else
    {
    //d->HelpLabel->setHtml("");
    }
}

//---------------------------------------------------------------------------
void qSlicerModulePanel::addModule(qSlicerAbstractCoreModule* module)
{
  Q_ASSERT(module);

  qSlicerAbstractModuleWidget* moduleWidget =
    dynamic_cast<qSlicerAbstractModuleWidget*>(module->widgetRepresentation());
  if (moduleWidget == 0)
    {
    qDebug() << "Warning, there is no UI for the module"<< module->name();
    emit moduleAdded(module->name());
    return;
    }

  Q_ASSERT(!moduleWidget->moduleName().isEmpty());

  Q_D(qSlicerModulePanel);

  // Update module layout
  if (moduleWidget->layout())
    {
    moduleWidget->layout()->setContentsMargins(0, 0, 0, 0);
    }
  else
    {
    qWarning() << moduleWidget->moduleName() << "widget doesn't have a top-level layout.";
    }

  // Insert module in the panel
  QBoxLayout* scrollAreaLayout =
    qobject_cast<QBoxLayout*>(d->ScrollArea->widget()->layout());
  Q_ASSERT(scrollAreaLayout);
  scrollAreaLayout->insertWidget(1, moduleWidget,1);

  moduleWidget->setSizePolicy(QSizePolicy::Ignored, moduleWidget->sizePolicy().verticalPolicy());
  moduleWidget->setVisible(true);

  QString help = module->helpText();
  d->HelpCollapsibleButton->setVisible(!help.isEmpty());
  d->HelpLabel->setHtml(help);
  //d->HelpLabel->load(QString("http://www.slicer.org/slicerWiki/index.php?title=Modules:Transforms-Documentation-3.4&useskin=chick"));
  d->AcknowledgementLabel->clear();
  qSlicerAbstractModule* guiModule = qobject_cast<qSlicerAbstractModule*>(module);
  if (guiModule && !guiModule->logo().isNull())
    {
    d->AcknowledgementLabel->document()->addResource(QTextDocument::ImageResource,
      QUrl("module://logo.png"), QVariant(guiModule->logo()));
    d->AcknowledgementLabel->append(
      QString("<center><img src=\"module://logo.png\"/></center><br>"));
    }
  QString acknowledgement = module->acknowledgementText();
  d->AcknowledgementLabel->insertHtml(acknowledgement);
  if (!module->contributor().isEmpty())
    {
    QString contributors = QString("<br><u>Contributors:</u> <i>") + module->contributor() + "</i>";
    d->AcknowledgementLabel->append(contributors);
    }

  moduleWidget->enter();

  emit moduleAdded(module->name());
}

//---------------------------------------------------------------------------
void qSlicerModulePanel::removeModule(qSlicerAbstractCoreModule* module)
{
  Q_ASSERT(module);

  qSlicerAbstractModuleWidget * moduleWidget =
    dynamic_cast<qSlicerAbstractModuleWidget*>(module->widgetRepresentation());
  Q_ASSERT(moduleWidget);

  Q_D(qSlicerModulePanel);

  QBoxLayout* scrollAreaLayout =
    qobject_cast<QBoxLayout*>(d->ScrollArea->widget()->layout());
  Q_ASSERT(scrollAreaLayout);
  int index = scrollAreaLayout->indexOf(moduleWidget);
  if (index == -1)
    {
    return;
    }

  moduleWidget->exit();

  //emit moduleAboutToBeRemoved(moduleWidget->module());

  // Remove widget from layout
  //d->Layout->removeWidget(module);
  scrollAreaLayout->takeAt(index);

  moduleWidget->setVisible(false);
  moduleWidget->setParent(0);

  // if nobody took ownership of the module, make sure it both lost its parent and is hidden
//   if (moduleWidget->parent() == d->Layout->parentWidget())
//     {
//     moduleWidget->setVisible(false);
//     moduleWidget->setParent(0);
//     }

  emit moduleRemoved(module->name());
}

//---------------------------------------------------------------------------
void qSlicerModulePanel::removeAllModules()
{
  this->setModule("");
}

//---------------------------------------------------------------------------
// qSlicerModulePanelPrivate methods

//---------------------------------------------------------------------------
void qSlicerModulePanelPrivate::setupUi(qSlicerWidget * widget)
{
  this->Ui_qSlicerModulePanel::setupUi(widget);
  this->HelpLabel->setOpenExternalLinks(true);
  this->AcknowledgementLabel->setOpenExternalLinks(true);
}

/*
  QWidget* panel = new QWidget;
  this->HelpCollapsibleButton = new ctkCollapsibleButton("Help");
  this->HelpCollapsibleButton->setCollapsed(true);
  this->HelpCollapsibleButton->setSizePolicy(
    QSizePolicy::Ignored, QSizePolicy::Minimum);

  // Note: QWebView could be used as an alternative to ctkFittedTextBrowser ?
  //this->HelpLabel = new QWebView;
  this->HelpLabel = static_cast<QTextBrowser*>(new ctkFittedTextBrowser);
  this->HelpLabel->setOpenExternalLinks(true);
  this->HelpLabel->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->HelpLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->HelpLabel->setFrameShape(QFrame::NoFrame);

  QPalette p = this->HelpLabel->palette();
  p.setBrush(QPalette::Window, QBrush ());
  this->HelpLabel->setPalette(p);

  QGridLayout* helpLayout = new QGridLayout(this->HelpCollapsibleButton);
  helpLayout->addWidget(this->HelpLabel);

  this->Layout = new QVBoxLayout(panel);
  this->Layout->addWidget(this->HelpCollapsibleButton);
  this->Layout->addItem(
    new QSpacerItem(0, 0, QSizePolicy::Minimum,
                    QSizePolicy::MinimumExpanding));

  this->ScrollArea = new QScrollArea;
  this->ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->ScrollArea->setWidget(panel);
  this->ScrollArea->setWidgetResizable(true);

  QGridLayout* gridLayout = new QGridLayout;
  gridLayout->addWidget(this->ScrollArea);
  gridLayout->setContentsMargins(0,0,0,0);
  widget->setLayout(gridLayout);
}
*/
