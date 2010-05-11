
// Qt includes
#include <QSignalMapper>
#include <QToolBar>
#include <QAction>
#include <QDebug>

#include "vtkSlicerConfigure.h" // For Slicer3_USE_PYTHONQT

// CTK includes
#ifdef Slicer3_USE_PYTHONQT
#include <ctkPythonShell.h>
#endif

// SlicerQt includes
#include "qSlicerMainWindowCore.h" 
#include "qSlicerMainWindowCore_p.h"
#include "qSlicerApplication.h"
#include "qSlicerModulePanel.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerModuleManager.h"
#include "qSlicerIOManager.h"
#ifdef Slicer3_USE_PYTHONQT
#include "qSlicerPythonManager.h"
#endif

// MRML includes
#include <vtkMRMLScene.h>

//---------------------------------------------------------------------------
// qSlicerMainWindowCorePrivate methods

//---------------------------------------------------------------------------
qSlicerMainWindowCorePrivate::qSlicerMainWindowCorePrivate()
  {
  this->PythonShell = 0; 
  this->ShowModuleActionMapper = new QSignalMapper(this);

  this->ToolBarModuleList << "Measurements" << "MRMLTree" << "Transforms" << "Volumes";
  }
    
//---------------------------------------------------------------------------
void qSlicerMainWindowCorePrivate::onModuleLoaded(qSlicerAbstractModule* module)
{
  Q_ASSERT(module);
  CTK_P(qSlicerMainWindowCore);

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
  CTK_P(qSlicerMainWindowCore);

  qSlicerAbstractModuleWidget* moduleWidget = module->widgetRepresentation();
  Q_ASSERT(moduleWidget);

  QAction * action = moduleWidget->showModuleAction();
  if (action)
    {
    p->widget()->moduleToolBar()->removeAction(action);
    }
}

//-----------------------------------------------------------------------------
// qSlicerMainWindowCore methods

//-----------------------------------------------------------------------------
qSlicerMainWindowCore::qSlicerMainWindowCore(qSlicerMainWindow* _parent):Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerMainWindowCore);
  CTK_D(qSlicerMainWindowCore);
  
  d->ParentWidget = _parent;

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
CTK_GET_CXX(qSlicerMainWindowCore, qSlicerMainWindow*, widget, ParentWidget);


//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onFileAddDataActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openLoadDataDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onFileImportSceneActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openImportSceneDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onFileLoadSceneActionTriggered()
{
  qSlicerApplication::application()->ioManager()->openLoadSceneDialog();
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onEditUndoActionTriggered()
{
  qSlicerApplication::application()->mrmlScene()->Undo();
}

//---------------------------------------------------------------------------
void qSlicerMainWindowCore::onEditRedoActionTriggered()
{
  qSlicerApplication::application()->mrmlScene()->Redo();
}

//-----------------------------------------------------------------------------
void qSlicerMainWindowCore::onWindowPythonInteractorActionTriggered()
{
#ifdef Slicer3_USE_PYTHONQT
  CTK_D(qSlicerMainWindowCore);
  if (!d->PythonShell)
    {
    Q_ASSERT(qSlicerApplication::application()->pythonManager());
    d->PythonShell = new ctkPythonShell(qSlicerApplication::application()->pythonManager()/*, d->ParentWidget*/);
    d->PythonShell->setAttribute(Qt::WA_QuitOnClose, false);
    d->PythonShell->resize(600, 280);
    }
  Q_ASSERT(d->PythonShell);
  d->PythonShell->show();
  d->PythonShell->raise();
#endif
}

