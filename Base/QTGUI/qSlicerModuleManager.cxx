#include "qSlicerModuleManager.h"

// SlicerQT includes
#include "qSlicerApplication.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerModulePanel.h"

// QT includes
#include <QToolBar>
#include <QAction>
#include <QSignalMapper>

//-----------------------------------------------------------------------------
struct qSlicerModuleManagerPrivate: public qCTKPrivate<qSlicerModuleManager>
{
  QCTK_DECLARE_PUBLIC(qSlicerModuleManager);
  
  qSlicerModuleManagerPrivate()
    {
    this->ModulePanel = 0;
    }

  // Description:
  // Instantiate a module panel
  void instantiateModulePanel();

  qSlicerModulePanel*                    ModulePanel;
};

//-----------------------------------------------------------------------------
qSlicerModuleManager::qSlicerModuleManager():Superclass()
{
  QCTK_INIT_PRIVATE(qSlicerModuleManager);
}

//-----------------------------------------------------------------------------
void qSlicerModuleManager::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}

//---------------------------------------------------------------------------
void qSlicerModuleManager::showModule(const QString& moduleTitle)
{
  QCTK_D(qSlicerModuleManager);
  d->instantiateModulePanel();
  Q_ASSERT(d->ModulePanel);

  qDebug() << "Show module by title:" << moduleTitle;
  qSlicerAbstractModule * module = this->getModule(moduleTitle);
  Q_ASSERT(module);
  d->ModulePanel->setModule(module);
}

//---------------------------------------------------------------------------
void qSlicerModuleManager::showModuleByName(const QString& moduleName)
{
  QCTK_D(qSlicerModuleManager);
  d->instantiateModulePanel();
  Q_ASSERT(d->ModulePanel);

  qDebug() << "Show module by name:" << moduleName;
  qSlicerAbstractModule * module = this->getModuleByName(moduleName);
  Q_ASSERT(module);
  d->ModulePanel->setModule(module);
}

//---------------------------------------------------------------------------
void qSlicerModuleManager::setModulePanelVisible(bool visible)
{
  QCTK_D(qSlicerModuleManager);
  
  d->instantiateModulePanel();
  Q_ASSERT(d->ModulePanel);

  d->ModulePanel->setVisible(visible);
}

//---------------------------------------------------------------------------
void qSlicerModuleManager::setModulePanelGeometry(int ax, int ay, int aw, int ah)
{
  QCTK_D(qSlicerModuleManager);
  
  d->instantiateModulePanel();
  Q_ASSERT(d->ModulePanel);

  d->ModulePanel->setGeometry(QRect(ax, ay, aw, ah));
}

//---------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerModuleManager, qSlicerAbstractModulePanel*, modulePanel, ModulePanel);

//---------------------------------------------------------------------------
// qSlicerModuleManagerPrivate methods

//---------------------------------------------------------------------------
void qSlicerModuleManagerPrivate::instantiateModulePanel()
{
  if (!this->ModulePanel)
    {
    this->ModulePanel =
      new qSlicerModulePanel(0, qSlicerApplication::application()->defaultWindowFlags());
    }
}
