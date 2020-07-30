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

==============================================================================*/

// STD includes
#include <algorithm>

// Qt includes
#include <QKeyEvent>
#include <QPushButton>

// Slicer includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerApplication.h"
#include "qSlicerModuleFactoryFilterModel.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerModuleFinderDialog.h"
#include "qSlicerUtils.h"
#include "ui_qSlicerModuleFinderDialog.h"

// --------------------------------------------------------------------------
// qSlicerModuleFinderDialogPrivate

//-----------------------------------------------------------------------------
class qSlicerModuleFinderDialogPrivate: public Ui_qSlicerModuleFinderDialog
{
  Q_DECLARE_PUBLIC(qSlicerModuleFinderDialog);
protected:
  qSlicerModuleFinderDialog* const q_ptr;

public:
  qSlicerModuleFinderDialogPrivate(qSlicerModuleFinderDialog& object);

  /// Convenient method regrouping all initialization code
  void init();

  void makeSelectedItemVisible();

  QString CurrentModuleName;
};

// --------------------------------------------------------------------------
// qSlicerModuleFinderDialogPrivate methods

// --------------------------------------------------------------------------
qSlicerModuleFinderDialogPrivate::qSlicerModuleFinderDialogPrivate(qSlicerModuleFinderDialog& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerModuleFinderDialogPrivate::init()
{
  Q_Q(qSlicerModuleFinderDialog);

  this->setupUi(q);

  qSlicerCoreApplication * coreApp = qSlicerCoreApplication::application();
  qSlicerAbstractModuleFactoryManager* factoryManager = coreApp->moduleManager()->factoryManager();

  qSlicerModuleFactoryFilterModel* filterModel = this->ModuleListView->filterModel();

  // Hide modules that do not have GUI (user cannot switch to them)
  filterModel->setShowHidden(false);
  // Hide testing modules by default
  filterModel->setShowTesting(false);

  QObject::connect(this->SearchInAllTextCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setSearchInAllText(bool)));
  QObject::connect(this->ShowBuiltInCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setShowBuiltInModules(bool)));
  QObject::connect(this->ShowTestingCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setShowTestingModules(bool)));
  QObject::connect(this->FilterTitleSearchBox, SIGNAL(textChanged(QString)),
    q, SLOT(onModuleTitleFilterTextChanged()));

  QObject::connect(this->ModuleListView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
    q, SLOT(onSelectionChanged(QItemSelection, QItemSelection)));

  this->FilterTitleSearchBox->installEventFilter(q);
  this->ModuleListView->viewport()->installEventFilter(q);

  QPushButton* okButton = this->ButtonBox->button(QDialogButtonBox::Ok);
  okButton->setText(q->tr("Switch to module"));

  if (filterModel->rowCount() > 0)
    {
    // select first item
    this->ModuleListView->setCurrentIndex(filterModel->index(0, 0));
    }
}

// --------------------------------------------------------------------------
void qSlicerModuleFinderDialogPrivate::makeSelectedItemVisible()
{
  Q_Q(qSlicerModuleFinderDialog);
  qSlicerModuleFactoryFilterModel* filterModel = this->ModuleListView->filterModel();

  // Make sure that an item is selected
  if (!this->ModuleListView->currentIndex().isValid())
    {
    if (filterModel->rowCount() > 0)
      {
      // select first item
      this->ModuleListView->setCurrentIndex(filterModel->index(0, 0));
      }
    }
  // Make sure that the selected item is visible
  if (this->ModuleListView->currentIndex().isValid())
    {
    this->ModuleListView->scrollTo(this->ModuleListView->currentIndex());
    }
}

// --------------------------------------------------------------------------
// qSlicerModuleFinderDialog methods

// --------------------------------------------------------------------------
qSlicerModuleFinderDialog::qSlicerModuleFinderDialog(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerModuleFinderDialogPrivate(*this))
{
  Q_D(qSlicerModuleFinderDialog);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerModuleFinderDialog::~qSlicerModuleFinderDialog() = default;

void qSlicerModuleFinderDialog::setFactoryManager(qSlicerAbstractModuleFactoryManager* factoryManager)
{
  Q_D(qSlicerModuleFinderDialog);
  d->ModuleListView->setFactoryManager(factoryManager);
}

//------------------------------------------------------------------------------
void qSlicerModuleFinderDialog::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_UNUSED(deselected);
  Q_D(qSlicerModuleFinderDialog);

  QString moduleName;
  qSlicerAbstractCoreModule* module = nullptr;
  if (!selected.indexes().empty())
    {
    moduleName = selected.indexes().first().data(Qt::UserRole).toString();
    qSlicerCoreApplication* coreApp = qSlicerCoreApplication::application();
    qSlicerModuleManager* moduleManager = coreApp->moduleManager();
    qSlicerModuleFactoryManager* factoryManager = moduleManager->factoryManager();
    if (factoryManager->isLoaded(moduleName))
      {
      module = moduleManager->module(moduleName);
      }
    }

  d->CurrentModuleName = moduleName;

  if (module)
    {
    d->ModuleDescriptionBrowser->clear();
    QString html;

    // Title
    html.append(QString("<h2>%1</h2>").arg(module->title()));

    // Categories
    QStringList categories = module->categories();
    QStringList filteredCategories;
    foreach(QString category, categories)
      {
      if (category.isEmpty())
        {
        category = QLatin1String("[main]");
        }
      else
        {
        category.replace(".", "->");
        }
      filteredCategories << category;
      }
    html.append(QString("<p><b>Category:</b> %1</p>").arg(filteredCategories.join(", ")));

    // Help
    QString help = module->helpText();
    qSlicerCoreApplication* app = qSlicerCoreApplication::application();
    if (app)
      {
      QString wikiVersion = "Nightly";
      if (app->releaseType() == "Stable")
        {
        wikiVersion = QString("%1.%2").arg(app->majorVersion()).arg(app->minorVersion());
        }
      help = qSlicerUtils::replaceWikiUrlVersion(module->helpText(), wikiVersion);
      }
    help.replace("\\n", "<br>");
    help = help.trimmed();
    if (!help.isEmpty())
      {
      html.append(help.trimmed());
      }

    // Acknowledgments
    qSlicerAbstractModule* guiModule = qobject_cast<qSlicerAbstractModule*>(module);
    if (guiModule && !guiModule->logo().isNull())
      {
      d->ModuleDescriptionBrowser->document()->addResource(QTextDocument::ImageResource,
        QUrl("module://logo.png"), QVariant(guiModule->logo()));
      html.append(
        QString("<center><img src=\"module://logo.png\"/></center><br>"));
      }
    QString acknowledgement = module->acknowledgementText();
    if (!acknowledgement.isEmpty())
      {
      acknowledgement.replace("\\n", "<br>");
      acknowledgement = acknowledgement.trimmed();
      html.append("<p>");
      html.append(acknowledgement.trimmed());
      }

    // Contributors
    if (!module->contributors().isEmpty())
      {
      QString contributors = module->contributors().join(", ");
      QString contributorsText = QString("<p><b>Contributors:</b> ") + contributors + "</p>";
      html.append(contributorsText);
      }

    // Internal name
    if (module->name() != module->title())
      {
      html.append(QString("<p><b>Internal name:</b> %1</p>").arg(module->name()));
      }

    // Type
    QString type = QLatin1String("Core");
    // Use "inherits" instead of "qobject_cast" because "qSlicerBaseQTCLI" depends on "qSlicerQTGUI"
    if (module->inherits("qSlicerScriptedLoadableModule"))
      {
      type = QLatin1String("Python Scripted Loadable");
      }
    else if (module->inherits("qSlicerLoadableModule"))
      {
      type = QLatin1String("C++ Loadable");
      }
    else if (module->inherits("qSlicerCLIModule"))
      {
      type = QLatin1String("Command-Line Interface (CLI)");
      }
    if (module->isBuiltIn())
      {
      type += QLatin1String(", built-in");
      }
    html.append(QString("<p><b>Type:</b> %1</p>").arg(type));

    // Dependencies
    if (!module->dependencies().empty())
      {
      html.append(QString("<p><b>Require:</b> %1</p>").arg(module->dependencies().join(", ")));
      }

    // Location
    html.append(QString("<p><b>Location:</b> %1</p>").arg(module->path()));

    d->ModuleDescriptionBrowser->setHtml(html);
    }
  else
    {
    d->ModuleDescriptionBrowser->clear();
    if (!moduleName.isEmpty())
      {
      d->ModuleDescriptionBrowser->setText(QString("%1 module is not loaded").arg(moduleName));
      }
    }

  // scroll to the top
  QTextCursor cursor = d->ModuleDescriptionBrowser->textCursor();
  cursor.movePosition(QTextCursor::Start);
  d->ModuleDescriptionBrowser->setTextCursor(cursor);

  QPushButton* okButton = d->ButtonBox->button(QDialogButtonBox::Ok);
  okButton->setEnabled(module != nullptr);
}

//---------------------------------------------------------------------------
bool qSlicerModuleFinderDialog::eventFilter(QObject* target, QEvent* event)
{
  Q_D(qSlicerModuleFinderDialog);
  if (target == d->FilterTitleSearchBox)
    {
    // Prevent giving the focus to the previous/next widget if arrow keys are used
    // at the edge of the table (without this: if the current cell is in the top
    // row and user press the Up key, the focus goes from the table to the previous
    // widget in the tab order)
    if (event->type() == QEvent::KeyPress)
      {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
      qSlicerModuleFactoryFilterModel* filterModel = d->ModuleListView->filterModel();
      if (keyEvent != nullptr && filterModel->rowCount() > 0)
        {
        int currentRow = d->ModuleListView->currentIndex().row();
        int stepSize = 1;
        if (keyEvent->key() == Qt::Key_PageUp || keyEvent->key() == Qt::Key_PageDown)
          {
          stepSize = 5;
          }
        else if (keyEvent->key() == Qt::Key_Home || keyEvent->key() == Qt::Key_End)
          {
          stepSize = 10000;
          }
        if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_PageUp || keyEvent->key() == Qt::Key_Home)
          {
          if (currentRow > 0)
            {
            d->ModuleListView->setCurrentIndex(filterModel->index(std::max(0, currentRow - stepSize), 0));
            d->ModuleListView->scrollTo(d->ModuleListView->currentIndex());
            }
          return true;
          }
        else if (keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_PageDown || keyEvent->key() == Qt::Key_End)
          {
          if (currentRow + 1 < filterModel->rowCount())
            {
            d->ModuleListView->setCurrentIndex(filterModel->index(std::min(currentRow + stepSize, filterModel->rowCount()-1), 0));
            d->ModuleListView->scrollTo(d->ModuleListView->currentIndex());
            }
          return true;
          }
        }
      }
    }
  else if (target == d->ModuleListView->viewport() && event->type() == QEvent::MouseButtonDblClick)
    {
    // accept selection on double-click
    this->accept();
    return true;
    }
  return this->Superclass::eventFilter(target, event);
}

//---------------------------------------------------------------------------
QString qSlicerModuleFinderDialog::currentModuleName()const
{
  Q_D(const qSlicerModuleFinderDialog);
  return d->CurrentModuleName;
}

//---------------------------------------------------------------------------
void qSlicerModuleFinderDialog::setFocusToModuleTitleFilter()
{
  Q_D(qSlicerModuleFinderDialog);
  d->FilterTitleSearchBox->setFocus();
  d->makeSelectedItemVisible();
}

//---------------------------------------------------------------------------
void qSlicerModuleFinderDialog::setModuleTitleFilterText(const QString& text)
{
  Q_D(qSlicerModuleFinderDialog);
  d->FilterTitleSearchBox->setText(text);
}

//---------------------------------------------------------------------------
void qSlicerModuleFinderDialog::onModuleTitleFilterTextChanged()
{
  Q_D(qSlicerModuleFinderDialog);
  qSlicerModuleFactoryFilterModel* filterModel = d->ModuleListView->filterModel();
  filterModel->setFilterFixedString(d->FilterTitleSearchBox->text());
  d->makeSelectedItemVisible();
}

//---------------------------------------------------------------------------
void qSlicerModuleFinderDialog::setSearchInAllText(bool searchAll)
{
  Q_D(qSlicerModuleFinderDialog);
  qSlicerModuleFactoryFilterModel* filterModel = d->ModuleListView->filterModel();
  if (searchAll)
    {
    // qModuleListViewPrivate::FullTextSearchRole = Qt::UserRole + 4
    filterModel->setFilterRole(Qt::UserRole + 4);
    }
  else
    {
    // search in displayed module title
    filterModel->setFilterRole(Qt::DisplayRole);
    }
  d->makeSelectedItemVisible();
}

//---------------------------------------------------------------------------
void qSlicerModuleFinderDialog::setShowBuiltInModules(bool show)
{
  Q_D(qSlicerModuleFinderDialog);
  qSlicerModuleFactoryFilterModel* filterModel = d->ModuleListView->filterModel();
  filterModel->setShowBuiltIn(show);
  d->makeSelectedItemVisible();
}

//---------------------------------------------------------------------------
void qSlicerModuleFinderDialog::setShowTestingModules(bool show)
{
  Q_D(qSlicerModuleFinderDialog);
  qSlicerModuleFactoryFilterModel* filterModel = d->ModuleListView->filterModel();
  filterModel->setShowTesting(show);
  d->makeSelectedItemVisible();
}
