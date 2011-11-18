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
#include <QDebug>
#include <QDesktopServices>
#include <QDirIterator>
#include <QNetworkReply>
#include <QProgressBar>
#include <QProgressDialog>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerExtensionsWizardOverviewPage.h"
#include "ui_qSlicerExtensionsWizardOverviewPage.h"

// Logic includes
#include "vtkSlicerExtensionsLogic.h"
#include "vtkArchive.h"

// VTK includes
#include <vtkSmartPointer.h>

namespace
{

enum
{
  StatusSuccess = 0,
  StatusDownloading,
  StatusInstalling,
  StatusUninstalling,
  StatusCancelled,
  StatusError,
  StatusFoundOnDisk,
  StatusNotFoundOnDisk,
  StatusUnknown
};

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
// See http://stackoverflow.com/questions/2536524/copy-directory-using-qt
bool rmDir(const QString &dirPath)
{
  QDir dir(dirPath);
  if (!dir.exists()) { return true; }
  foreach(const QFileInfo &info, dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
    {
    if (info.isDir())
      {
      if (!rmDir(info.filePath())){ return false; }
      }
    else
      {
      if (!dir.remove(info.fileName())) { return false; }
      }
    }
  QDir parentDir(QFileInfo(dirPath).path());
  return parentDir.rmdir(QFileInfo(dirPath).fileName());
}

//-----------------------------------------------------------------------------
// See http://stackoverflow.com/questions/2536524/copy-directory-using-qt
bool cpDir(const QString &srcPath, const QString &dstPath)
{
  rmDir(dstPath);
  QDir parentDstDir(QFileInfo(dstPath).path());
  if (!parentDstDir.mkdir(QFileInfo(dstPath).fileName()))
    {
    return false;
    }

  QDir srcDir(srcPath);
  foreach(const QFileInfo &info, srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
    {
    QString srcItemPath = srcPath + "/" + info.fileName();
    QString dstItemPath = dstPath + "/" + info.fileName();
    if (info.isDir())
      {
      if (!cpDir(srcItemPath, dstItemPath))
        {
        return false;
        }
      }
    else if (info.isFile())
      {
      if (!QFile::copy(srcItemPath, dstItemPath))
        {
        return false;
        }
      }
    else
      {
      qDebug() << "Unhandled item" << info.filePath() << "in cpDir";
      }
    }
  return true;
}

//-----------------------------------------------------------------------------
QList<ManifestEntry*> UpdateModulesFromRepository(const QString& extensionServerURL,
                                                 const QList<QVariantMap>& retrievedListOfS4extFiles,
                                                 const QList<QVariantMap>& retrievedListOfPackageFiles,
                                                 vtkSlicerExtensionsLogic * extensionLogic)
{
  Q_ASSERT(extensionLogic);

  // Generate map of s4ext filename to the corresponding URL
  QHash<QString, QVariantMap> s4extFileNameToMap;
  foreach(const QVariantMap& file, retrievedListOfS4extFiles)
    {
    s4extFileNameToMap.insert(file.value("filename").toString(), file);
    }

  // The following regex has been developed with the help of http://rexv.org/
  // Matching group 1: Slicer revision - Could be 'svn revision' or 'git sha1'
  // Matching group 2: <platform>-<arch>
  // Matching group 3: Extension name
  // Matching group 4: (git|svn|local)
  // Matching group 5: (sha1|revision|"")
  QRegExp regex("^((?:[a-f0-9]{5,40})|(?:\\d+))\\-((?:linux|macosx|win|na)-(?:i386|amd64|na))\\-([a-zA-Z0-9_]+)\\-(svn|git|local)((?:[a-f0-9]{5,40})|(?:\\d*))\\-\\d{4}\\-\\d{2}\\-\\d{2}\\.tar.gz$");

  std::vector<ManifestEntry*> updatedManifestEntries;
  QStringList processedS4extFileNames;
  foreach(const QVariantMap& packageFile, retrievedListOfPackageFiles)
    {
    // file attributes: id, filename, filesize, sha1sum, url
    QString packageFileName = packageFile.value("filename").toString();
    if (!regex.exactMatch(packageFileName))
      {
      qWarning() << "The file name is not formatted properly:" << packageFileName;
      continue;
      }
    // Skip filename if it has already been processed
    if (processedS4extFileNames.contains(packageFileName))
      {
      continue;
      }
    processedS4extFileNames << packageFileName;

    QStringList capturedTexts = regex.capturedTexts();
    Q_ASSERT(capturedTexts.count() == 6);

    ManifestEntry * entry = new ManifestEntry;
    QString url = packageFile.value("url").toString();
    entry->URL = (extensionServerURL + "/" + url).toStdString();
    entry->Name = capturedTexts.at(3).toStdString();
    entry->Revision = capturedTexts.at(5).toStdString();

    // Lookup url of the associated s4ext files
    QString s4extFileName = QString(packageFileName).replace(QFileInfo(packageFileName).completeSuffix(), "s4ext");
    QHash<QString, QVariantMap>::const_iterator i =s4extFileNameToMap.find(s4extFileName);
    if (i != s4extFileNameToMap.end())
      {
      QString s4extURL = extensionServerURL + "/" + i.value().value("url").toString();
      extensionLogic->DownloadAndParseS4ext(s4extURL.toStdString(), entry);
      }
    extensionLogic->AddEntry(updatedManifestEntries, entry);
    }
  return QVector<ManifestEntry*>::fromStdVector(updatedManifestEntries).toList();
}

} // end of anonymous namespace

//-----------------------------------------------------------------------------
// qSlicerExtensionsWizardOverviewPagePrivate

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
  
  QObject::connect( this->ExtensionsTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
                    q, SLOT(onItemClicked(QTreeWidgetItem*,int)));
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
    if (QFile::exists(entry.absoluteFilePath() + "/qt-loadable-modules"))
      {
      extensions << entry.absoluteFilePath() + "/qt-loadable-modules";
      }
    if (QFile::exists(entry.absoluteFilePath() + "/qt-scripted-modules"))
      {
      extensions << entry.absoluteFilePath() + "/qt-scripted-modules";
      }
    if(QFile::exists(entry.absoluteFilePath() + "/plugins"))
      {
      extensions << entry.absoluteFilePath() + "/plugins";
      }
    if (!QFile::exists(entry.absoluteFilePath() + "/qt-loadable-modules") &&
        !QFile::exists(entry.absoluteFilePath() + "/qt-scripted-modules") &&
        !QFile::exists(entry.absoluteFilePath() + "/plugins"))
      {
      extensions << entry.absoluteFilePath();
      }
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
    this->isExtensionInstalled(extension->Name.c_str()) ? StatusFoundOnDisk : StatusNotFoundOnDisk));
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
    case StatusSuccess:
      return q->style()->standardIcon(QStyle::SP_DialogYesButton);
      break;
    case StatusDownloading:
    case StatusInstalling:
    case StatusUninstalling:
      return q->style()->standardIcon(QStyle::SP_BrowserReload);
      break;
    case StatusCancelled:
      return q->style()->standardIcon(QStyle::SP_DialogCancelButton);
      break;
    case StatusError:
      return q->style()->standardIcon(QStyle::SP_MessageBoxWarning);
      break;
    case StatusFoundOnDisk:
      return q->style()->standardIcon(QStyle::SP_DriveHDIcon);
      break;
    case StatusNotFoundOnDisk:
      return q->style()->standardIcon(QStyle::SP_DriveNetIcon);
      break;
    case StatusUnknown:
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
  extensionItem->setIcon(ExtensionColumn, this->iconFromStatus(StatusDownloading));

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
    this->iconFromStatus(StatusInstalling));

  QString extensionName = extensionItem->text(ExtensionColumn);

  // Make extension output directory
  QDir extensionsDir(qSlicerCoreApplication::application()->extensionsPath());
  extensionsDir.mkdir(extensionName);
  extensionsDir.cd(extensionName);

  // Save current directory
  QString currentPath = QDir::currentPath();

  // Set extension directory as current directory
  QDir::setCurrent(extensionsDir.absolutePath());

  // Extract into <extensionsPath>/<extensionName>/<archiveBaseName>/
  extract_tar(archive.toLatin1(), true, true);

  extensionsDir.cdUp();

  // Restore current directory
  QDir::setCurrent(currentPath);

  // Name of the sub-folder <archiveBaseName>
  QString archiveBaseName = QFileInfo(archive).baseName();

  // Rename <extensionName>/<archiveBaseName> into <extensionName>
  // => Such operation can't be done directly, we need intermediate steps ...

  //  Step1: <extensionName>/<archiveBaseName> -> <extensionName>-XXXXXX
  cpDir(extensionsDir.absolutePath() + "/" + extensionName + "/" + archiveBaseName,
        extensionsDir.absolutePath() + "/" + extensionName + "XXXXXX");

  //  Step2: <extensionName>-XXXXXX -> <extensionName>
  cpDir(extensionsDir.absolutePath() + "/" + extensionName + "XXXXXX",
        extensionsDir.absolutePath() + "/" + extensionName);

  //  Step3: Remove <extensionName>-XXXXXX
  rmDir(extensionsDir.absolutePath() + "/" + extensionName + "XXXXXX");

  bool installed = this->isExtensionInstalled(extensionName);
  extensionItem->setIcon(ExtensionColumn, this->iconFromStatus(
    installed ? StatusFoundOnDisk : StatusError));
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

  extensionItem->setIcon(ExtensionColumn, this->iconFromStatus(StatusUninstalling));

  QString extensionName = extensionItem->text(ExtensionColumn);
  rmDir(qSlicerCoreApplication::application()->extensionsPath() + "/" + extensionName);

  extensionItem->setIcon(ExtensionColumn, this->iconFromStatus(
    this->isExtensionInstalled(extensionName) ? StatusError: StatusNotFoundOnDisk));
  if (extensionItem->text(BinaryURLColumn).isEmpty())
    {
    delete extensionItem;
    }
  if (this->scanExistingExtensions().count() == 0)
    {
    this->UninstallPushButton->setEnabled(false);
    }
}

// --------------------------------------------------------------------------
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

  // Retrieve parameters set in the previous wizzard page
  d->ExtensionsDir = QDir(this->field("installPath").toString());

  QString extensionsServerURL = this->field("extensionsServerURL").toString();

  QList<QVariantMap> retrievedListOfS4extFiles = this->field("retrievedListOfS4extFiles").value< QList<QVariantMap> >();
  QList<QVariantMap> retrievedListOfPackageFiles = this->field("retrievedListOfPackageFiles").value< QList<QVariantMap> >();

  bool install = this->field("installEnabled").toBool();
  bool uninstall = this->field("uninstallEnabled").toBool();

  // Clear list of extensions
  foreach(QTreeWidgetItem* item, d->ExtensionsTreeWidget->invisibleRootItem()->takeChildren())
    {
    delete item;
    }
  d->Logic->ClearModules();

  d->Logic->SetRepositoryURL(extensionsServerURL.toLatin1());
  d->Logic->SetTemporaryDirectory(qSlicerCoreApplication::application()->temporaryPath().toLatin1());
  d->Logic->SetInstallPath(d->ExtensionsDir.absolutePath().toLatin1());

  QProgressDialog progressDialog(this);
  progressDialog.setLabelText("Populate server extensions");
  progressDialog.setRange(0,0);
  progressDialog.open();

  QList<ManifestEntry*> modules;
  if (install)
    {
    modules = UpdateModulesFromRepository(extensionsServerURL, retrievedListOfS4extFiles, retrievedListOfPackageFiles, d->Logic);
    d->InstallPushButton->setEnabled(modules.count() > 0);
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
  foreach(ManifestEntry* entry, modules)
    {
    d->addExtension(entry);
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
    d->ExtensionsTreeWidget->height());
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
    item->setIcon(ExtensionColumn, d->iconFromStatus(StatusError));
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
    item->setIcon(ExtensionColumn, d->iconFromStatus(StatusError));
    return;
    }

  file.write(reply->readAll());
  file.close();
  // Delete the progress bar
  d->ExtensionsTreeWidget->setItemWidget(item, ExtensionColumn,0);

  d->installExtension(item, fileInfo.absoluteFilePath());
}
