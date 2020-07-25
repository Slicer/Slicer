/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

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
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTextDocument>

// QtCore includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerAbstractModuleFactoryManager.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerUtils.h"

// QtGUI includes
#include "qSlicerModuleFactoryFilterModel.h"
#include "qSlicerModulesListView.h"


// --------------------------------------------------------------------------
// qSlicerModulesListViewPrivate

//-----------------------------------------------------------------------------
class qSlicerModulesListViewPrivate
{
  Q_DECLARE_PUBLIC(qSlicerModulesListView);
protected:
  qSlicerModulesListView* const q_ptr;

public:
  enum CustomRole
    {
    ModuleNameRole = Qt::UserRole,
    IsBuiltInRole = Qt::UserRole + 1,
    IsTestingRole = Qt::UserRole + 2,
    IsHiddenRole = Qt::UserRole + 3,
    FullTextSearchRole = Qt::UserRole + 4
    };

  qSlicerModulesListViewPrivate(qSlicerModulesListView& object);
  void init();

  void updateItem(QStandardItem* item);
  void addModules();
  void removeModules();

  int sortedInsertionIndex(const QString& moduleName)const;
  QStandardItem* moduleItem(const QString& moduleName)const;
  QStringList indexListToModules(const QModelIndexList& indexes)const;
  void setModulesCheckState(const QStringList& moduleNames, Qt::CheckState check);

  QStandardItemModel* ModulesListModel;
  qSlicerModuleFactoryFilterModel* FilterModel;
  qSlicerAbstractModuleFactoryManager* FactoryManager;
  bool CheckBoxVisible;
};

// --------------------------------------------------------------------------
// qSlicerModulesListViewPrivate methods

// --------------------------------------------------------------------------
qSlicerModulesListViewPrivate::qSlicerModulesListViewPrivate(qSlicerModulesListView& object)
  :q_ptr(&object)
{
  this->ModulesListModel = nullptr;
  this->FilterModel = nullptr;
  this->FactoryManager = nullptr;
  this->CheckBoxVisible = false;
}

// --------------------------------------------------------------------------
void qSlicerModulesListViewPrivate::init()
{
  Q_Q(qSlicerModulesListView);

  this->ModulesListModel = new QStandardItemModel(q);
  q->connect(this->ModulesListModel, SIGNAL(itemChanged(QStandardItem*)),
             q, SLOT(onItemChanged(QStandardItem*)));
  this->FilterModel = new qSlicerModuleFactoryFilterModel(q);
  this->FilterModel->setSourceModel(this->ModulesListModel);
  q->setModel(this->FilterModel);
}

// --------------------------------------------------------------------------
void qSlicerModulesListViewPrivate::updateItem(QStandardItem* item)
{
  Q_Q(qSlicerModulesListView);
  QString moduleName = item->data(qSlicerModulesListViewPrivate::ModuleNameRole).toString();
  item->setCheckable(true);
  // The module is ignored, therefore it hasn't been loaded
  if (this->FactoryManager != nullptr && this->FactoryManager->ignoredModuleNames().contains(moduleName))
    {
    item->setForeground(q->palette().color(QPalette::Disabled, QPalette::Text));
    }
  // The module was registered, not ignored, initialized, but failed to be loaded
  else if (qobject_cast<qSlicerModuleFactoryManager*>(this->FactoryManager) &&
           !qobject_cast<qSlicerModuleFactoryManager*>(this->FactoryManager)
           ->loadedModuleNames().contains(moduleName))
    {
    item->setForeground(Qt::red);
    }
  // Loaded module
  else
    {
    item->setForeground(q->palette().color(QPalette::Normal, QPalette::Text));
    }
  if (this->CheckBoxVisible)
    {
    if (this->FactoryManager == nullptr ||
        this->FactoryManager->modulesToIgnore().contains(moduleName) )
      {
      item->setCheckState(Qt::Unchecked);
      }
    else
      {
      item->setCheckState(Qt::Checked);
      }
    }
  else
    {
    item->setData(QVariant(), Qt::CheckStateRole);
    }
  qSlicerAbstractCoreModule* coreModule = (this->FactoryManager ? this->FactoryManager->moduleInstance(moduleName) : nullptr);
  if (coreModule)
    {
    item->setText(coreModule->title());

    // Create text for tooltip: title (name), help text, dependency
    QString tooltip = QString("%1 (%2)").arg(coreModule->title()).arg(moduleName);
    QString helpText = coreModule->helpText();
    helpText.replace("\\n", "<br>");
    if (!helpText.isEmpty())
      {
      tooltip += QString("<br><br>%1").arg(helpText);
      }
    if (coreModule->dependencies().count() > 0)
      {
      tooltip += QString("<br><br>Requires: %1").arg(coreModule->dependencies().join(", "));
      }
    item->setToolTip(tooltip);

    // Create text that can be used as full-text search for a module.
    // It includes module title, name, and help text. Help text is converted to plain text
    // to only search in displayed text and not in the content of html tags.
    QTextDocument helpTextDoc;
    helpTextDoc.setHtml(coreModule->helpText());
    QTextDocument acknowledgementTextDoc;
    acknowledgementTextDoc.setHtml(coreModule->acknowledgementText());
    QString contributors = coreModule->contributors().join(",");
    QString searchText = QString("%1 %2 %3 %4 %5")
      .arg(coreModule->title())
      .arg(moduleName)
      .arg(helpTextDoc.toPlainText())
      .arg(acknowledgementTextDoc.toPlainText())
      .arg(contributors);
    item->setData(searchText, qSlicerModulesListViewPrivate::FullTextSearchRole);
    }
  else
    {
    item->setText(moduleName);
    item->setToolTip("");
    item->setData(moduleName, qSlicerModulesListViewPrivate::FullTextSearchRole);
    }

  qSlicerAbstractModule* module = qobject_cast<qSlicerAbstractModule*>(coreModule);
  if (module)
    {
    item->setData(module->isBuiltIn(), qSlicerModulesListViewPrivate::IsBuiltInRole);
    item->setData(qSlicerUtils::isTestingModule(module), qSlicerModulesListViewPrivate::IsTestingRole);
    item->setData(module->isHidden(), qSlicerModulesListViewPrivate::IsHiddenRole);

    // See QTBUG-20248
    bool block = this->ModulesListModel->blockSignals(true);
    item->setIcon(module->icon());
    this->ModulesListModel->blockSignals(block);
    }
}

// --------------------------------------------------------------------------
void qSlicerModulesListViewPrivate::addModules()
{
  Q_Q(qSlicerModulesListView);
  q->addModules(q->modules());
}

// --------------------------------------------------------------------------
void qSlicerModulesListViewPrivate::removeModules()
{
  this->ModulesListModel->clear();
}

// --------------------------------------------------------------------------
int qSlicerModulesListViewPrivate
::sortedInsertionIndex(const QString& moduleName)const
{
  int index = 0;
  for (; index < this->ModulesListModel->rowCount(); ++index)
    {
    QStandardItem* item = this->ModulesListModel->item(index);
    Q_ASSERT(item);
    if (QString::compare(moduleName, item->text(), Qt::CaseInsensitive) < 0)
      {
      break;
      }
    }
  return index;
}

// --------------------------------------------------------------------------
QStandardItem* qSlicerModulesListViewPrivate
::moduleItem(const QString& moduleName)const
{
  QModelIndex start = this->ModulesListModel->index(0, 0);
  QModelIndexList moduleIndexes = this->ModulesListModel->match(
    start, qSlicerModulesListViewPrivate::ModuleNameRole, moduleName,
    /* hits= */ 1, Qt::MatchExactly);
  if (moduleIndexes.count() == 0)
    {
    return nullptr;
    }
  return this->ModulesListModel->itemFromIndex(moduleIndexes.at(0));
}

// --------------------------------------------------------------------------
QStringList qSlicerModulesListViewPrivate
::indexListToModules(const QModelIndexList& indexes)const
{
  QStringList modules;
  foreach(const QModelIndex& index, indexes)
    {
    modules << index.data(qSlicerModulesListViewPrivate::ModuleNameRole).toString();
    }
  return modules;
}

// --------------------------------------------------------------------------
void qSlicerModulesListViewPrivate
::setModulesCheckState(const QStringList& moduleNames,
                       Qt::CheckState checkState)
{
  Q_Q(qSlicerModulesListView);
  foreach(const QString& moduleName, q->modules())
    {
    QStandardItem* moduleItem = this->moduleItem(moduleName);
    if (moduleItem == nullptr)
      {
      continue;
      }
    if (moduleNames.contains(moduleName))
      {
      moduleItem->setCheckState(checkState);
      }
    else
      {
      moduleItem->setCheckState(
        checkState == Qt::Checked ? Qt::Unchecked : Qt::Checked);
      }
    }
}


// --------------------------------------------------------------------------
// qSlicerModulesListView methods

// --------------------------------------------------------------------------
qSlicerModulesListView::qSlicerModulesListView(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerModulesListViewPrivate(*this))
{
  Q_D(qSlicerModulesListView);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerModulesListView::~qSlicerModulesListView() = default;

// --------------------------------------------------------------------------
void qSlicerModulesListView::setFactoryManager(qSlicerAbstractModuleFactoryManager* factoryManager)
{
  Q_D(qSlicerModulesListView);
  if (d->FactoryManager != nullptr)
    {
    disconnect(d->FactoryManager, SIGNAL(modulesRegistered(QStringList)),
               this, SLOT(updateModules(QStringList)));
    disconnect(d->FactoryManager, SIGNAL(moduleInstantiated(QString)),
               this, SLOT(updateModule(QString)));
    disconnect(d->FactoryManager, SIGNAL(modulesToIgnoreChanged(QStringList)),
               this, SLOT(updateModules()));
    disconnect(d->FactoryManager, SIGNAL(moduleIgnored(QString)),
               this, SLOT(updateModule(QString)));
    disconnect(d->FactoryManager, SIGNAL(moduleLoaded(QString)),
               this, SLOT(updateModule(QString)));
    d->removeModules();
    }
  d->FactoryManager = factoryManager;
  if (d->FactoryManager != nullptr)
    {
    connect(d->FactoryManager, SIGNAL(modulesRegistered(QStringList)),
            this, SLOT(updateModules(QStringList)));
    connect(d->FactoryManager, SIGNAL(moduleInstantiated(QString)),
            this, SLOT(updateModule(QString)));
    connect(d->FactoryManager, SIGNAL(modulesToIgnoreChanged(QStringList)),
            this, SLOT(updateModules()));
    connect(d->FactoryManager, SIGNAL(moduleIgnored(QString)),
            this, SLOT(updateModule(QString)));
    connect(d->FactoryManager, SIGNAL(moduleLoaded(QString)),
            this, SLOT(updateModule(QString)));
    }

  this->updateModules();
}

// --------------------------------------------------------------------------
qSlicerAbstractModuleFactoryManager* qSlicerModulesListView::factoryManager()const
{
  Q_D(const qSlicerModulesListView);
  return d->FactoryManager;
}

// --------------------------------------------------------------------------
qSlicerModuleFactoryFilterModel* qSlicerModulesListView::filterModel()const
{
  Q_D(const qSlicerModulesListView);
  return d->FilterModel;
}

// --------------------------------------------------------------------------
bool qSlicerModulesListView::isCheckBoxVisible()const
{
  Q_D(const qSlicerModulesListView);
  return d->CheckBoxVisible;
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::setCheckBoxVisible(bool show)
{
  Q_D(qSlicerModulesListView);
  d->CheckBoxVisible = show;
}

// --------------------------------------------------------------------------
QStringList qSlicerModulesListView::modules()const
{
  Q_D(const qSlicerModulesListView);
  QStringList modules;
  if (d->FactoryManager != nullptr)
    {
    modules << d->FactoryManager->registeredModuleNames();
    modules << d->FactoryManager->modulesToIgnore();
    modules << d->FactoryManager->ignoredModuleNames();
    }
  modules.removeDuplicates();
  modules.sort();
  return modules;
}

// --------------------------------------------------------------------------
QStringList qSlicerModulesListView::checkedModules()const
{
  Q_D(const qSlicerModulesListView);
  QModelIndex start = d->ModulesListModel->index(0,0);
  QModelIndexList checkedModuleList =
    d->ModulesListModel->match(start, Qt::CheckStateRole, Qt::Checked, -1);
  return d->indexListToModules(checkedModuleList);
}

// --------------------------------------------------------------------------
QStringList qSlicerModulesListView::uncheckedModules()const
{
  Q_D(const qSlicerModulesListView);
  QModelIndex start = d->ModulesListModel->index(0,0);
  QModelIndexList checkedModuleList =
    d->ModulesListModel->match(start, Qt::CheckStateRole, Qt::Unchecked, -1);
  return d->indexListToModules(checkedModuleList);
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::setCheckedModules(const QStringList& moduleNames)
{
  Q_D(qSlicerModulesListView);
  d->setModulesCheckState(moduleNames, Qt::Checked);
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::setUncheckedModules(const QStringList& moduleNames)
{
  Q_D(qSlicerModulesListView);
  d->setModulesCheckState(moduleNames, Qt::Unchecked);
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::hideSelectedModules()
{
  Q_D(qSlicerModulesListView);
  QStringList newShowModules = d->FilterModel->showModules();
  QStringList modulesToHide = d->indexListToModules(
    this->selectionModel()->selectedIndexes());
  foreach(const QString& moduleToHide, modulesToHide)
    {
    newShowModules.removeAll(moduleToHide);
    }
  d->FilterModel->setShowModules(newShowModules);
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::moveLeftSelectedModules()
{
  this->moveSelectedModules(-1);
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::moveRightSelectedModules()
{
  this->moveSelectedModules(1);
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::moveSelectedModules(int offset)
{
  Q_D(qSlicerModulesListView);
  QStringList newShowModules = d->FilterModel->showModules();
  QStringList modulesToMove = d->indexListToModules(
    this->selectionModel()->selectedIndexes());
  foreach(const QString& moduleToMove, modulesToMove)
    {
    int moduleIndex = newShowModules.indexOf(moduleToMove);
    if (moduleIndex != -1)
      {
      newShowModules.move(moduleIndex, qBound(0, moduleIndex + offset, newShowModules.count() -1));
      }
    }
  d->FilterModel->setShowModules(newShowModules);
  d->FilterModel->invalidate();
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::scrollToSelectedModules()
{
  if (this->selectionModel()->selectedIndexes().count() > 0)
    {
    this->scrollTo(this->selectionModel()->selectedIndexes().at(0));
    }
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::addModules(const QStringList& moduleNames)
{
  foreach(const QString& moduleName, moduleNames)
    {
    this->addModule(moduleName);
    }
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::addModule(const QString& moduleName)
{
  Q_D(qSlicerModulesListView);
  Q_ASSERT(d->moduleItem(moduleName) == nullptr);
  QStandardItem * item = new QStandardItem();
  item->setData(moduleName, qSlicerModulesListViewPrivate::ModuleNameRole);
  d->updateItem(item);
  int index = d->sortedInsertionIndex(moduleName);
  d->ModulesListModel->insertRow(index, item);
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::updateModules()
{
  this->updateModules(this->modules());
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::updateModules(const QStringList& moduleNames)
{
  foreach(const QString& moduleName, moduleNames)
    {
    this->updateModule(moduleName);
    }
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::updateModule(const QString& moduleName)
{
  Q_D(qSlicerModulesListView);
  QStandardItem * item = d->moduleItem(moduleName);
  if (item == nullptr)
    {
    this->addModule(moduleName);
    }
  else
    {
    d->updateItem(item);
    }
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::onItemChanged(QStandardItem* item)
{
  Q_D(qSlicerModulesListView);
  if (item->data(Qt::CheckStateRole).isNull())
    {
    return;
    }
  QString moduleName = item->data(qSlicerModulesListViewPrivate::ModuleNameRole).toString();
  qSlicerAbstractCoreModule* module = d->FactoryManager->moduleInstance(moduleName);
  if (item->checkState() == Qt::Checked)
    {
    d->FactoryManager->removeModuleToIgnore(moduleName);
    // ensure dependencies are checked
    if (module)
      {
      foreach(const QString& dependency, module->dependencies())
        {
        d->FactoryManager->removeModuleToIgnore(dependency);
        }
      }
    }
  else
    {
    d->FactoryManager->addModuleToIgnore(moduleName);
    // ensure dependent modules are unchecked
    if (module)
      {
      foreach(const QString& dependentModule, d->FactoryManager->dependentModules(moduleName))
        {
        d->FactoryManager->addModuleToIgnore(dependentModule);
        }
      }
    }
}

// --------------------------------------------------------------------------
void qSlicerModulesListView::keyPressEvent(QKeyEvent * event)
{
  if (event->key() == Qt::Key_Delete ||
      event->key() == Qt::Key_Backspace)
    {
    this->hideSelectedModules();
    event->accept();
    return;
    }
  this->Superclass::keyPressEvent(event);
}

//---------------------------------------------------------------------------
void qSlicerModulesListView::changeEvent(QEvent* e)
{
  Q_D(qSlicerModulesListView);
  switch (e->type())
    {
    case QEvent::PaletteChange:
      {
      this->updateModules();
      break;
      }
    default:
      break;
    }
  QListView::changeEvent(e);
}
