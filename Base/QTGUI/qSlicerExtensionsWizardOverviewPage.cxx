/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressBar>
#include <QProgressDialog>
#include <QUrl>

// CTK includes
#include <ctkCheckableHeaderView.h>
#include <ctkLogger.h>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerExtensionsWizardOverviewPage.h"
#include "ui_qSlicerExtensionsWizardOverviewPage.h"

// Logic includes
#include "vtkSlicerExtensionsLogic.h"
#include "vtkArchive.h"

// VTK includes
#include <vtkSmartPointer.h>

static ctkLogger logger("org.commontk.libs.widgets.qSlicerExtensionsWizardOverviewPage");

// qSlicerExtensionsWizardOverviewPagePrivate

enum Columns
{
  ExtensionColumn = 0,
  StatusColumn,
  DescriptionColumn,
  HomepageColumn,
  BinaryURLColumn
};

enum Roles
{
  IsExtensionRole = Qt::UserRole
};

//-----------------------------------------------------------------------------
class qSlicerExtensionsWizardOverviewPagePrivate
  : public Ui_qSlicerExtensionsWizardOverviewPage
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsWizardOverviewPage);
protected:
  qSlicerExtensionsWizardOverviewPage* const q_ptr;

public:
  qSlicerExtensionsWizardOverviewPagePrivate(qSlicerExtensionsWizardOverviewPage& object);
  void init();

  QStringList scanExistingExtensions()const;
  
  void addExtension(ManifestEntry* extension);
  QTreeWidgetItem* categoryItem(const QString& category)const;
  QIcon iconFromStatus(int status)const;
  bool isExtensionInstalled(const QString& extension)const ;
  QTreeWidgetItem* item(const QUrl& url)const;

  void downloadExtension(QTreeWidgetItem* extensionItem);
  void installExtension(QTreeWidgetItem* extensionItem, const QString& archive);
  void uninstallExtension(QTreeWidgetItem* extensionItem);

  vtkSmartPointer<vtkSlicerExtensionsLogic> Logic;
  QNetworkAccessManager                     NetworkManager;
  QDir                                      ExtensionsDir;
};

// --------------------------------------------------------------------------
qSlicerExtensionsWizardOverviewPagePrivate::qSlicerExtensionsWizardOverviewPagePrivate(qSlicerExtensionsWizardOverviewPage& object)
  :q_ptr(&object)
{
  this->Logic = vtkSmartPointer<vtkSlicerExtensionsLogic>::New();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardOverviewPagePrivate::init()
{
  Q_Q(qSlicerExtensionsWizardOverviewPage);
  this->setupUi(q);
  
  //ctkCheckableHeaderView* headerView =
  //  new ctkCheckableHeaderView(Qt::Horizontal, this->ExtensionsTreeWidget);  
  //headerView->setPropagateToItems(true);

  //this->ExtensionsTreeWidget->setHeader(headerView);
  //this->ExtensionsTreeWidget->model()->setHeaderData(
  //  ExtensionColumn, Qt::Horizontal, Qt::Checked, Qt::CheckStateRole);
  this->ExtensionsTreeWidget->setColumnHidden(DescriptionColumn, true);
  this->ExtensionsTreeWidget->header()->setResizeMode(ExtensionColumn, QHeaderView::ResizeToContents);
  this->ExtensionsTreeWidget->header()->setResizeMode(StatusColumn, QHeaderView::ResizeToContents);
  
  QObject::connect( this->ExtensionsTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
                    q, SLOT(onItemClicked(QTreeWidgetItem*, int)));
  QObject::connect( this->InstallPushButton, SIGNAL(clicked()),
                    q, SLOT(installSelectedItems()));
  QObject::connect( this->UninstallPushButton, SIGNAL(clicked()),
                    q, SLOT(uninstallSelectedItems()));
  QObject::connect(&this->NetworkManager, SIGNAL(finished(QNetworkReply*)),
                   q, SLOT(downloadFinished(QNetworkReply*)));

  q->setProperty("installedExtensions", QStringList());
  q->registerField("installedExtensions", q, "installedExtensions");
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsWizardOverviewPagePrivate::scanExistingExtensions()const
{
  QStringList extensions;
  QFileInfoList entries = this->ExtensionsDir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot|QDir::Executable);
  foreach(const QFileInfo& entry, entries)
    {
    extensions << entry.absoluteFilePath();
    }
  return extensions;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsWizardOverviewPagePrivate
::isExtensionInstalled(const QString& extensionName)const
{
  QFileInfo extensionInfo(this->ExtensionsDir, extensionName);
  return extensionInfo.exists();
}

// --------------------------------------------------------------------------
QTreeWidgetItem* qSlicerExtensionsWizardOverviewPagePrivate::categoryItem(const QString& category)const
{
  if (category.isEmpty())
    {
    return this->ExtensionsTreeWidget->invisibleRootItem();
    }
  QList<QTreeWidgetItem*> matchingCategories =
    this->ExtensionsTreeWidget->findItems(category, Qt::MatchExactly);
  Q_ASSERT(matchingCategories.count() <= 1);
  if (matchingCategories.count() == 0)
    {
    return 0;
    }
  return matchingCategories[0];
}


// --------------------------------------------------------------------------
QTreeWidgetItem* qSlicerExtensionsWizardOverviewPagePrivate::item(const QUrl& url)const
{
  QList<QTreeWidgetItem*> items =
    this->ExtensionsTreeWidget->findItems("*", Qt::MatchWildcard | Qt::MatchRecursive);
  foreach(QTreeWidgetItem* item, items)
    {
    if (item->text(BinaryURLColumn) == url.toString())
      {
      return item;
      }
    }
  return 0;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardOverviewPagePrivate::addExtension(ManifestEntry* extension)
{
  QTreeWidgetItem* category = this->categoryItem(extension->Category.c_str());
  if (category == 0)
    {
    category = new QTreeWidgetItem(QStringList() << QString(extension->Category.c_str()));
    // categories can't be selected
    category->setFlags(Qt::ItemIsEnabled);
    //category->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsTristate | Qt::ItemIsEnabled);
    this->ExtensionsTreeWidget->addTopLevelItem(category);
    }
  Q_ASSERT(category);

  QTreeWidgetItem* extensionItem = new QTreeWidgetItem;
  //extensionItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
  //extensionItem->setData(ExtensionColumn, Qt::CheckStateRole, Qt::Unchecked);
  extensionItem->setData(ExtensionColumn, IsExtensionRole, true);
  extensionItem->setText(ExtensionColumn, extension->Name.c_str());
  extensionItem->setToolTip(ExtensionColumn, extension->Description.c_str());
  extensionItem->setIcon(ExtensionColumn, this->iconFromStatus(
    this->isExtensionInstalled(extension->Name.c_str()) ?
      vtkSlicerExtensionsLogic::StatusFoundOnDisk :
      vtkSlicerExtensionsLogic::StatusNotFoundOnDisk));
  extensionItem->setText(StatusColumn, extension->ExtensionStatus.c_str());
  extensionItem->setText(DescriptionColumn, extension->Description.c_str());
  extensionItem->setToolTip(DescriptionColumn, extension->Description.c_str());
  extensionItem->setText(HomepageColumn, extension->Homepage.c_str());
  extensionItem->setToolTip(HomepageColumn, "Click the address to open the page into your browser");
  extensionItem->setText(BinaryURLColumn, extension->URL.c_str());
  extensionItem->setToolTip(BinaryURLColumn, "Remote location of the extension");
  category->addChild(extensionItem);
}

// ----------------------------------------------------------------------------
QIcon qSlicerExtensionsWizardOverviewPagePrivate::iconFromStatus(int status)const
{
  Q_Q(const qSlicerExtensionsWizardOverviewPage);
  switch(status)
    {
    case vtkSlicerExtensionsLogic::StatusSuccess:
      return q->style()->standardIcon(QStyle::SP_DialogYesButton);
      break;
    case vtkSlicerExtensionsLogic::StatusDownloading:
    case vtkSlicerExtensionsLogic::StatusInstalling:
    case vtkSlicerExtensionsLogic::StatusUninstalling:
      return q->style()->standardIcon(QStyle::SP_BrowserReload);
      break;
    case vtkSlicerExtensionsLogic::StatusCancelled:
      return q->style()->standardIcon(QStyle::SP_DialogCancelButton);
      break;
    case vtkSlicerExtensionsLogic::StatusError:
      return q->style()->standardIcon(QStyle::SP_MessageBoxWarning);
      break;
    case vtkSlicerExtensionsLogic::StatusFoundOnDisk:
      return q->style()->standardIcon(QStyle::SP_DriveHDIcon);
      break;
    case vtkSlicerExtensionsLogic::StatusNotFoundOnDisk:
      return q->style()->standardIcon(QStyle::SP_DriveNetIcon);
      break;
    case vtkSlicerExtensionsLogic::StatusUnknown:
    default:
      break;
    };
  return QIcon();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardOverviewPagePrivate
::downloadExtension(QTreeWidgetItem* extensionItem)
{
  Q_Q(qSlicerExtensionsWizardOverviewPage);
  Q_ASSERT(extensionItem);
  if (!extensionItem->data(ExtensionColumn, IsExtensionRole).toBool())
    {
    return;
    }

  // Set the icon before the progress bar for update issues
  extensionItem->setIcon(ExtensionColumn, this->iconFromStatus(
    vtkSlicerExtensionsLogic::StatusDownloading));

  QProgressBar* progressBar = new QProgressBar(q);
  progressBar->setAutoFillBackground(true);
  progressBar->setRange(0,0);
  progressBar->setTextVisible(false);
  this->ExtensionsTreeWidget->setItemWidget(extensionItem, ExtensionColumn, progressBar);

  QUrl manifestURL(extensionItem->text(BinaryURLColumn));
  QNetworkRequest request(manifestURL);
  this->NetworkManager.get(request);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardOverviewPagePrivate
::installExtension(QTreeWidgetItem* extensionItem, const QString& archive)
{
  Q_ASSERT(extensionItem);
  if (!extensionItem->data(ExtensionColumn, IsExtensionRole).toBool())
    {
    return;
    }

  this->uninstallExtension(extensionItem);

  extensionItem->setIcon(ExtensionColumn,
    this->iconFromStatus(vtkSlicerExtensionsLogic::StatusInstalling));

  QString extensionName = extensionItem->text(ExtensionColumn);
  QString currentPath = QDir::currentPath();
  QDir extensionsDir(qSlicerCoreApplication::application()->extensionsPath());
  extensionsDir.mkdir(extensionName);
  extensionsDir.cd(extensionName);
  QDir::setCurrent(extensionsDir.absolutePath());
  extract_tar(archive.toLatin1(), true, true);
  QDir::setCurrent(currentPath);

  bool installed = this->isExtensionInstalled(extensionName);
  extensionItem->setIcon(ExtensionColumn, this->iconFromStatus(
    installed ? vtkSlicerExtensionsLogic::StatusFoundOnDisk :
                vtkSlicerExtensionsLogic::StatusError));
  if (installed)
    {
    this->UninstallPushButton->setEnabled(true);
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardOverviewPagePrivate
::uninstallExtension(QTreeWidgetItem* extensionItem)
{
  Q_ASSERT(extensionItem);
  if (!extensionItem->data(ExtensionColumn, IsExtensionRole).toBool())
    {
    return;
    }

  extensionItem->setIcon(ExtensionColumn, this->iconFromStatus(vtkSlicerExtensionsLogic::StatusUninstalling));

  QString extensionName = extensionItem->text(ExtensionColumn);
  QDir extensionsDir(qSlicerCoreApplication::application()->extensionsPath());
  QFileInfo extensionFileInfo(extensionsDir, extensionName);
  QDir extensionDir(extensionFileInfo.absoluteFilePath());
  // delete all files
  QDirIterator fileIt (extensionDir.absolutePath(), QDir::Files, QDirIterator::Subdirectories);
  while(fileIt.hasNext())
    {
    fileIt.next();
    QFileInfo file(fileIt.fileInfo());
    file.dir().remove(file.fileName());
    }
  // delete all empty directories
  QDirIterator dirIt (extensionDir.absolutePath(), QDir::Dirs, QDirIterator::Subdirectories);
  while(dirIt.hasNext())
    {
    dirIt.next();
    QFileInfo dir(dirIt.fileInfo());
    dir.dir().rmdir(dir.fileName());
    }

  extensionsDir.rmpath(extensionName);

  extensionItem->setIcon(ExtensionColumn, this->iconFromStatus(
    this->isExtensionInstalled(extensionName) ?
      vtkSlicerExtensionsLogic::StatusError:
      vtkSlicerExtensionsLogic::StatusNotFoundOnDisk));
  if (extensionItem->text(BinaryURLColumn).isEmpty())
    {
    delete extensionItem;
    }
  if (this->scanExistingExtensions().count() == 0)
    {
    this->UninstallPushButton->setEnabled(false);
    }
}

// qSlicerExtensionsWizardOverviewPage

// --------------------------------------------------------------------------
qSlicerExtensionsWizardOverviewPage::qSlicerExtensionsWizardOverviewPage(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsWizardOverviewPagePrivate(*this))
{
  Q_D(qSlicerExtensionsWizardOverviewPage);

  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsWizardOverviewPage::~qSlicerExtensionsWizardOverviewPage()
{
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardOverviewPage::initializePage()
{
  Q_D(qSlicerExtensionsWizardOverviewPage);

  d->ExtensionsDir = QDir(this->field("installPath").toString());
  QString extensionsURL = this->field("extensionsURL").toString();
  QString manifestFile = this->field("manifestFile").toString();

  bool install = this->field("installEnabled").toBool();
  bool uninstall = this->field("uninstallEnabled").toBool();

  foreach(QTreeWidgetItem* item, d->ExtensionsTreeWidget->invisibleRootItem()->takeChildren())
    {
    delete item;
    }
  d->Logic->ClearModules();
  d->Logic->SetRepositoryURL(extensionsURL.toLatin1());
  d->Logic->SetTemporaryDirectory(qSlicerCoreApplication::application()->temporaryPath().toLatin1());
  d->Logic->SetInstallPath(d->ExtensionsDir.absolutePath().toLatin1());

  QProgressDialog progressDialog(this);
  progressDialog.setLabelText("Populate server extensions");
  progressDialog.setRange(0,0);
  progressDialog.open();
  if (install)
    {
    bool res = d->Logic->UpdateModulesFromRepository(manifestFile.toStdString());
    if (!res)
      {
      qWarning() << "Failed to retrieve server modules";
      }
    d->InstallPushButton->setEnabled(res);
    }
  else
    {
    d->InstallPushButton->setEnabled(false);
    }
  qApp->processEvents();
  if (uninstall)
    {
    QStringList extensions = d->scanExistingExtensions();
    std::string extensionPaths = extensions.join(";").toStdString();
    // maybe there are some extensions locally but not remotely
    d->Logic->UpdateModulesFromDisk(extensionPaths);
    d->UninstallPushButton->setEnabled(extensions.size() > 0);
    }
  else
    {
    d->UninstallPushButton->setEnabled(false);
    }
  qApp->processEvents();
  d->ExtensionsTreeWidget->setSortingEnabled(false);
  const std::vector<ManifestEntry*>& modules = d->Logic->GetModules();
  std::vector<ManifestEntry*>::const_iterator it;
  for( it = modules.begin(); it != modules.end(); ++it)
    {
    d->addExtension(*it);
    }
  d->ExtensionsTreeWidget->setSortingEnabled(true);
  d->ExtensionsTreeWidget->expandAll();
  d->ExtensionsTreeWidget->sortItems(ExtensionColumn, Qt::AscendingOrder);
  d->ExtensionsTreeWidget->setMinimumSize(d->ExtensionsTreeWidget->sizeHint());
  // TODO: find a function in Qt that does it automatically.
  d->ExtensionsTreeWidget->resize(
    d->ExtensionsTreeWidget->frameWidth()
    + d->ExtensionsTreeWidget->header()->length()
    + d->ExtensionsTreeWidget->frameWidth(),
    d->ExtensionsTreeWidget->height() );
  progressDialog.close();
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsWizardOverviewPage::validatePage()
{
  Q_D(qSlicerExtensionsWizardOverviewPage);

  this->setProperty("installedExtensions", d->scanExistingExtensions());

  return true;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardOverviewPage::onItemClicked(QTreeWidgetItem* item, int column)
{
  switch(column)
    {
    case HomepageColumn:
      QDesktopServices::openUrl(item->text(HomepageColumn));
      break;
    default:
      break;
    };
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardOverviewPage::installSelectedItems()
{
  Q_D(qSlicerExtensionsWizardOverviewPage);

  QList<QTreeWidgetItem*> items = d->ExtensionsTreeWidget->selectedItems();

  foreach(QTreeWidgetItem* item, items)
    {
    d->downloadExtension(item);
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardOverviewPage::uninstallSelectedItems()
{
  Q_D(qSlicerExtensionsWizardOverviewPage);

  QList<QTreeWidgetItem*> items = d->ExtensionsTreeWidget->selectedItems();

  foreach(QTreeWidgetItem* item, items)
    {
    d->uninstallExtension(item);
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardOverviewPage::downloadFinished(QNetworkReply* reply)
{
  Q_D(qSlicerExtensionsWizardOverviewPage);

  QUrl extensionUrl = reply->url();
  QTreeWidgetItem* item = d->item(extensionUrl);
  Q_ASSERT(item);
  if (!item || reply->error())
    {
    qWarning() << "Failed downloading: " << extensionUrl.toString();
    d->ExtensionsTreeWidget->setItemWidget(item, ExtensionColumn,0);
    item->setIcon(ExtensionColumn, d->iconFromStatus(
      vtkSlicerExtensionsLogic::StatusError));
    return;
    }

  QFileInfo urlFileInfo(extensionUrl.path());
  QFileInfo fileInfo(qSlicerCoreApplication::application()->temporaryPath(),
    urlFileInfo.fileName());
  QFile file(fileInfo.absoluteFilePath());
  if (!file.open(QIODevice::WriteOnly))
    {
    qWarning() << "Could not open " << fileInfo.absoluteFilePath() << " for writing: %s" << file.errorString();
    // remove progress bar
    d->ExtensionsTreeWidget->setItemWidget(item, ExtensionColumn, 0);
    item->setIcon(ExtensionColumn, d->iconFromStatus(
      vtkSlicerExtensionsLogic::StatusError));
    return;
    }

  file.write(reply->readAll());
  file.close();
  // Delete the progress bar
  d->ExtensionsTreeWidget->setItemWidget(item, ExtensionColumn,0);

  d->installExtension(item, fileInfo.absoluteFilePath());
}
