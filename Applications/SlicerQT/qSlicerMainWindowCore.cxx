#include "qSlicerMainWindowCore.h" 

#include "qSlicerMainWindowCore_p.h"

// SlicerQT includes
#include "qSlicerApplication.h"
#include "qSlicerModulePanel.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerModuleManager.h"

// QT includes
#include <QSignalMapper>
#include <QToolBar>
#include <QAction>
#include <QDebug>

//-----------------------------------------------------------------------------
qSlicerMainWindowCore::qSlicerMainWindowCore(qSlicerMainWindow* parent):Superclass(parent)
{
  QCTK_INIT_PRIVATE(qSlicerMainWindowCore);
  QCTK_D(qSlicerMainWindowCore);
  
  d->ParentWidget = parent;

  qSlicerModuleManager * moduleManager = qSlicerApplication::application()->moduleManager();
  Q_ASSERT(moduleManager); 

  this->connect(moduleManager,
                SIGNAL(moduleLoaded(qSlicerAbstractModule*)),
                d, SLOT(onModuleLoaded(qSlicerAbstractModule*)));

  this->connect(moduleManager,
                SIGNAL(moduleAboutToBeUnloaded(qSlicerAbstractModule*)),
                d, SLOT(onModuleAboutToBeUnloaded(qSlicerAbstractModule*)));
                 
  QObject::connect(d->ShowModuleActionMapper,
                SIGNAL(mapped(const QString&)),
                this->widget()->modulePanel(),
                SLOT(setModule(const QString&)));
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerMainWindowCore, qSlicerMainWindow*, widget, ParentWidget);

//---------------------------------------------------------------------------
// qSlicerMainWindowCorePrivate methods

//---------------------------------------------------------------------------
qSlicerMainWindowCorePrivate::qSlicerMainWindowCorePrivate()
  {
  this->ShowModuleActionMapper = new QSignalMapper(this);

  this->ToolBarModuleList << "Measurements" << "Transforms" << "Volumes";
  }
    
//---------------------------------------------------------------------------
void qSlicerMainWindowCorePrivate::onModuleLoaded(qSlicerAbstractModule* module)
{
  Q_ASSERT(module);
  QCTK_P(qSlicerMainWindowCore);

  qSlicerAbstractModuleWidget* moduleWidget = module->widgetRepresentation();
  Q_ASSERT(moduleWidget);

  QAction * action = moduleWidget->showModuleAction();
  if (action)
    {
    // Add action to signal mapper
    this->ShowModuleActionMapper->setMapping(action, module->name());
    QObject::connect(action, SIGNAL(triggered()), this->ShowModuleActionMapper, SLOT(map()));

    // Update action state
    bool visible = this->ToolBarModuleList.contains(module->title());
    action->setVisible(visible);
    action->setEnabled(visible);

    // Add action to ToolBar
    p->widget()->moduleToolBar()->addAction(action);
    }
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCorePrivate::onModuleAboutToBeUnloaded(qSlicerAbstractModule* module)
{
  Q_ASSERT(module);
  QCTK_P(qSlicerMainWindowCore);

  qSlicerAbstractModuleWidget* moduleWidget = module->widgetRepresentation();
  Q_ASSERT(moduleWidget);

  QAction * action = moduleWidget->showModuleAction();
  if (action)
    {
    p->widget()->moduleToolBar()->removeAction(action);
    }
}
