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
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QShowEvent>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QSettings>
#include <QSet>
#include <QTableWidgetItem>

// CTK includes

// QtGUI includes
#include "qSlicerRelativePathMapper.h"
#include "qSlicerSettingsCachePanel.h"
#include "ui_qSlicerSettingsCachePanel.h"

// MRML includes
#include <vtkCacheManager.h>

// VTK includes
#include <vtkCommand.h>

namespace
{
enum CacheTableColumn
{
  ColumnModified = 0,
  ColumnName = 1,
  ColumnSize = 2,
};

// QTableWidgetItem that sorts by a stored numeric value rather than display text.
class NumericTableWidgetItem : public QTableWidgetItem
{
public:
  NumericTableWidgetItem(const QString& text, double sortValue)
    : QTableWidgetItem(text)
    , SortValue(sortValue)
  {
  }
  bool operator<(const QTableWidgetItem& other) const override
  {
    const auto* o = dynamic_cast<const NumericTableWidgetItem*>(&other);
    return o ? SortValue < o->SortValue : QTableWidgetItem::operator<(other);
  }
  double SortValue;
};
} // namespace

// --------------------------------------------------------------------------
// qSlicerSettingsCachePanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSettingsCachePanelPrivate : public Ui_qSlicerSettingsCachePanel
{
  Q_DECLARE_PUBLIC(qSlicerSettingsCachePanel);

protected:
  qSlicerSettingsCachePanel* const q_ptr;

public:
  qSlicerSettingsCachePanelPrivate(qSlicerSettingsCachePanel& object);
  void init();

  vtkCacheManager* CacheManager;
};

// --------------------------------------------------------------------------
// qSlicerSettingsCachePanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsCachePanelPrivate::qSlicerSettingsCachePanelPrivate(qSlicerSettingsCachePanel& object)
  : q_ptr(&object)
{
  this->CacheManager = nullptr;
}

// --------------------------------------------------------------------------
void qSlicerSettingsCachePanelPrivate::init()
{
  Q_Q(qSlicerSettingsCachePanel);

  this->setupUi(q);
  QObject::connect(this->CachePathButton, SIGNAL(directoryChanged(QString)), q, SLOT(setCachePath(QString)));
  QObject::connect(this->CacheSizeSpinBox, SIGNAL(valueChanged(int)), q, SLOT(setCacheSize(int)));
  QObject::connect(this->updateCacheUsageInformationButton, SIGNAL(clicked(bool)), q, SLOT(updateFromCacheManager()));
  QObject::connect(this->ForceRedownloadCheckBox, SIGNAL(toggled(bool)), q, SLOT(setForceRedownload(bool)));
  QObject::connect(this->RemoveSelectedCachePushButton, SIGNAL(clicked()), q, SLOT(removeSelectedCacheItems()));
  QObject::connect(this->FilesTableWidget, SIGNAL(itemSelectionChanged()), q, SLOT(updateRemoveSelectedButton()));
  QObject::connect(this->PruneCachePushButton, SIGNAL(clicked()), q, SLOT(pruneCache()));
  QObject::connect(this->ClearCachePushButton, SIGNAL(clicked()), q, SLOT(clearCache()));

  this->FilesTableWidget->setColumnCount(3);
  this->FilesTableWidget->setHorizontalHeaderLabels({ qSlicerSettingsCachePanel::tr("Modified"), qSlicerSettingsCachePanel::tr("Name"), qSlicerSettingsCachePanel::tr("Size") });
  this->FilesTableWidget->horizontalHeader()->setSectionResizeMode(ColumnModified, QHeaderView::ResizeToContents);
  this->FilesTableWidget->horizontalHeader()->setSectionResizeMode(ColumnName, QHeaderView::Stretch);
  this->FilesTableWidget->horizontalHeader()->setSectionResizeMode(ColumnSize, QHeaderView::ResizeToContents);
  this->FilesTableWidget->verticalHeader()->setVisible(false);
  this->FilesTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  this->FilesTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->FilesTableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->FilesTableWidget->setSortingEnabled(true);
  this->FilesTableWidget->sortByColumn(ColumnModified, Qt::DescendingOrder);
  this->RemoveSelectedCachePushButton->setEnabled(false);
}

// --------------------------------------------------------------------------
// qSlicerSettingsCachePanel methods

// --------------------------------------------------------------------------
qSlicerSettingsCachePanel::qSlicerSettingsCachePanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSettingsCachePanelPrivate(*this))
{
  Q_D(qSlicerSettingsCachePanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSettingsCachePanel::~qSlicerSettingsCachePanel() = default;

// --------------------------------------------------------------------------
void qSlicerSettingsCachePanel::showEvent(QShowEvent* event)
{
  this->Superclass::showEvent(event);
  this->updateFromCacheManager();
}

// --------------------------------------------------------------------------
void qSlicerSettingsCachePanel::setCacheManager(vtkCacheManager* cacheManager)
{
  Q_D(qSlicerSettingsCachePanel);
  if (d->CacheManager == cacheManager)
  {
    return;
  }
  qvtkReconnect(d->CacheManager, cacheManager, vtkCommand::ModifiedEvent, this, SLOT(updateFromCacheManager()));
  d->CacheManager = cacheManager;

  // Default values
  this->updateFromCacheManager();

  qSlicerRelativePathMapper* relativePathMapper = new qSlicerRelativePathMapper(d->CachePathButton, "directory", SIGNAL(directoryChanged(QString)));
  this->registerProperty("Cache/Path", relativePathMapper, "relativePath", SIGNAL(relativePathChanged(QString)));
  this->registerProperty("Cache/Size", d->CacheSizeSpinBox, /*no tr*/ "value", SIGNAL(valueChanged(int)));
  this->registerProperty("Cache/AutoPrune", d->AutoPruneCheckBox, /*no tr*/ "checked", SIGNAL(toggled(bool)));
  this->registerProperty("Cache/ForceRedownload", d->ForceRedownloadCheckBox, /*no tr*/ "checked", SIGNAL(toggled(bool)));
}

// --------------------------------------------------------------------------
void qSlicerSettingsCachePanel::updateFromCacheManager()
{
  Q_D(qSlicerSettingsCachePanel);
  this->setEnabled(d->CacheManager != nullptr);
  if (d->CacheManager == nullptr)
  {
    return;
  }
  d->CachePathButton->setDirectory(QString(d->CacheManager->GetRemoteCacheDirectory()));
  d->CacheSizeSpinBox->setValue(d->CacheManager->GetRemoteCacheLimit());

  d->CacheManager->UpdateCacheInformation();
  d->UsedCacheSizeLabel->setText(tr("%1MB used").arg(QString::number(qMax(d->CacheManager->GetCurrentCacheSize(), 0.f), 'f', 2)));
  d->FreeCacheSizeLabel->setText(tr("%1MB free").arg(QString::number(d->CacheManager->GetFreeCacheSpaceRemaining(), 'f', 2)));
  QPalette palette = this->palette();
  if (d->CacheManager->GetCurrentCacheSize() > d->CacheManager->GetRemoteCacheLimit())
  {
    palette.setColor(d->UsedCacheSizeLabel->foregroundRole(), Qt::red);
  }
  d->UsedCacheSizeLabel->setPalette(palette);
  d->FreeCacheSizeLabel->setPalette(palette);

  d->ForceRedownloadCheckBox->setChecked(d->CacheManager->GetEnableForceRedownload() == 1);

  d->FilesTableWidget->setSortingEnabled(false);
  d->FilesTableWidget->setRowCount(0);
  for (const vtkCacheManager::CacheEntry& entry : d->CacheManager->GetCacheEntries())
  {
    const QFileInfo fi(QString::fromStdString(entry.Path));
    const int row = d->FilesTableWidget->rowCount();
    d->FilesTableWidget->insertRow(row);

    // Modified column — yyyy-MM-dd sorts correctly as a string.
    auto* dateItem = new NumericTableWidgetItem(QDateTime::fromSecsSinceEpoch(entry.ModifiedTime).toString("yyyy-MM-dd"), static_cast<double>(entry.ModifiedTime));
    d->FilesTableWidget->setItem(row, ColumnModified, dateItem);

    // Name column — store full path in UserRole for deletion.
    QString nameText = fi.fileName();
    if (entry.IsDirectory)
    {
      nameText += tr(" [%1 files]").arg(entry.FileCount);
    }
    QTableWidgetItem* nameItem = new QTableWidgetItem(nameText);
    nameItem->setData(Qt::UserRole, fi.filePath());
    nameItem->setToolTip(fi.filePath());
    d->FilesTableWidget->setItem(row, ColumnName, nameItem);

    // Size column — numeric sort via NumericTableWidgetItem.
    const double sizeMB = static_cast<double>(entry.SizeBytes) / 1000000.0;
    const QString sizeText = sizeMB >= 0.005 ? tr("%1 MB").arg(sizeMB, 0, 'f', 2) : tr("%1 KB").arg(static_cast<double>(entry.SizeBytes) / 1000.0, 0, 'f', 1);
    auto* sizeItem = new NumericTableWidgetItem(sizeText, static_cast<double>(entry.SizeBytes));
    sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    sizeItem->setData(Qt::UserRole, static_cast<qulonglong>(entry.SizeBytes));
    d->FilesTableWidget->setItem(row, ColumnSize, sizeItem);

    if (entry.ExceedsCacheSize)
    {
      dateItem->setForeground(Qt::red);
      nameItem->setForeground(Qt::red);
      sizeItem->setForeground(Qt::red);
    }
  }
  d->FilesTableWidget->setSortingEnabled(true);
  this->updateRemoveSelectedButton();
}

// --------------------------------------------------------------------------
void qSlicerSettingsCachePanel::setCachePath(const QString& path)
{
  Q_D(qSlicerSettingsCachePanel);
  if (d->CacheManager == nullptr)
  {
    return;
  }

  const QString previousPath = QString::fromUtf8(d->CacheManager->GetRemoteCacheDirectory());
  const QString selectedPath = path;
  if (selectedPath == previousPath)
  {
    return;
  }

  // Create a missing cache directory before any sentinel checks.
  if (!QDir(selectedPath).exists())
  {
    QDir directory;
    if (!directory.mkpath(selectedPath))
    {
      const QSignalBlocker blocker(d->CachePathButton);
      d->CachePathButton->setDirectory(previousPath);
      QMessageBox::warning(this,
                           tr("Failed to create cache folder"),
                           tr("Failed to create cache directory %1. Make sure the parent directory is writable.").arg(QDir::toNativeSeparators(selectedPath)));
      return;
    }
  }

  // Warn the user if the folder contains files and is not marked as a cache folder
  // to avoid unintentional deletion of existing files in that folder later.
  if (!d->CacheManager->IsDirectoryEmpty(selectedPath.toStdString())              //
      && !d->CacheManager->HasSentinelFileInDirectory(selectedPath.toStdString()) //
      && !this->confirmCacheOperationWithMissingSentinel(tr("selection of cache folder"), selectedPath))
  {
    const QSignalBlocker blocker(d->CachePathButton);
    d->CachePathButton->setDirectory(previousPath);
    return;
  }

  // Ensure sentinel creation targets the newly selected cache path.
  d->CacheManager->SetRemoteCacheDirectory(selectedPath.toStdString());
  if (!d->CacheManager->HasSentinelFile() && !d->CacheManager->CreateSentinelFile())
  {
    d->CacheManager->SetRemoteCacheDirectory(previousPath.toStdString());
    const QSignalBlocker blocker(d->CachePathButton);
    d->CachePathButton->setDirectory(previousPath);
    QMessageBox::warning(this,
                         tr("Failed to mark folder for cache use"),
                         tr("Failed to create cache sentinel file in %1. Make sure the selected directory is writable.") //
                           .arg(QDir::toNativeSeparators(selectedPath)));
  }
}

// --------------------------------------------------------------------------
void qSlicerSettingsCachePanel::setCacheSize(int sizeInMB)
{
  Q_D(qSlicerSettingsCachePanel);
  d->CacheManager->SetRemoteCacheLimit(sizeInMB);
}

// --------------------------------------------------------------------------
void qSlicerSettingsCachePanel::setForceRedownload(bool force)
{
  Q_D(qSlicerSettingsCachePanel);
  d->CacheManager->SetEnableForceRedownload(force ? 1 : 0);
}

// --------------------------------------------------------------------------
void qSlicerSettingsCachePanel::clearCache()
{
  Q_D(qSlicerSettingsCachePanel);
  Q_ASSERT(d->CacheManager);
  if (!this->confirmCacheOperationWithMissingSentinel(tr("clearing the cache"), d->CacheManager->GetRemoteCacheDirectory()))
  {
    return;
  }

  d->CacheManager->ClearCache();
  this->updateFromCacheManager();
}

// --------------------------------------------------------------------------
void qSlicerSettingsCachePanel::updateRemoveSelectedButton()
{
  Q_D(qSlicerSettingsCachePanel);

  QSet<int> selectedRows;
  for (const QTableWidgetItem* item : d->FilesTableWidget->selectedItems())
  {
    selectedRows.insert(item->row());
  }

  if (selectedRows.isEmpty())
  {
    d->RemoveSelectedCachePushButton->setEnabled(false);
    d->RemoveSelectedCachePushButton->setText(tr("Remove selected"));
    return;
  }

  unsigned long long totalBytes = 0;
  for (int row : selectedRows)
  {
    totalBytes += d->FilesTableWidget->item(row, ColumnSize)->data(Qt::UserRole).toULongLong();
  }

  const double sizeMB = static_cast<double>(totalBytes) / 1000000.0;
  const QString sizeText = sizeMB >= 0.005 ? tr("%1 MB").arg(sizeMB, 0, 'f', 2) : tr("%1 KB").arg(static_cast<double>(totalBytes) / 1000.0, 0, 'f', 1);

  const int count = selectedRows.size();
  d->RemoveSelectedCachePushButton->setEnabled(true);
  d->RemoveSelectedCachePushButton->setText(tr("Remove selected (%1 %2, %3)").arg(count).arg(count == 1 ? tr("item") : tr("items")).arg(sizeText));
}

// --------------------------------------------------------------------------
void qSlicerSettingsCachePanel::removeSelectedCacheItems()
{
  Q_D(qSlicerSettingsCachePanel);
  Q_ASSERT(d->CacheManager);

  QSet<int> selectedRows;
  for (const QTableWidgetItem* item : d->FilesTableWidget->selectedItems())
  {
    selectedRows.insert(item->row());
  }
  if (selectedRows.isEmpty())
  {
    return;
  }

  if (!this->confirmCacheOperationWithMissingSentinel(tr("removing selected items"), d->CacheManager->GetRemoteCacheDirectory()))
  {
    return;
  }

  for (int row : selectedRows)
  {
    const QString path = d->FilesTableWidget->item(row, ColumnName)->data(Qt::UserRole).toString();
    d->CacheManager->DeleteFromCache(path.toStdString());
  }
  this->updateFromCacheManager();
}

// --------------------------------------------------------------------------
void qSlicerSettingsCachePanel::pruneCache()
{
  Q_D(qSlicerSettingsCachePanel);
  Q_ASSERT(d->CacheManager);
  if (!this->confirmCacheOperationWithMissingSentinel(tr("trimming the cache"), d->CacheManager->GetRemoteCacheDirectory()))
  {
    return;
  }

  d->CacheManager->PruneCache();
  this->updateFromCacheManager();
}

// --------------------------------------------------------------------------
bool qSlicerSettingsCachePanel::confirmCacheOperationWithMissingSentinel(const QString& operationText, const QString& cacheDirectory)
{
  Q_D(qSlicerSettingsCachePanel);
  if (!d->CacheManager || cacheDirectory.isEmpty())
  {
    return false;
  }
  if (d->CacheManager->HasSentinelFileInDirectory(cacheDirectory.toStdString()))
  {
    return true;
  }

  QMessageBox confirmationMessageBox(this);
  confirmationMessageBox.setIcon(QMessageBox::Warning);

  confirmationMessageBox.setWindowTitle(tr("Confirm %1").arg(operationText));
  confirmationMessageBox.setText(tr("Folder '%1' is not marked as a cache folder. Are you sure you want to proceed?").arg(cacheDirectory));

  QDir selectedDirectory(cacheDirectory);
  const QFileInfoList directoryEntries = selectedDirectory.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries, QDir::Name);
  if (!directoryEntries.empty())
  {
    const int maxDetailedEntries = 50;
    QStringList displayedEntries;
    const int displayedCount = qMin(directoryEntries.size(), maxDetailedEntries);
    for (int entryIndex = 0; entryIndex < displayedCount; ++entryIndex)
    {
      displayedEntries << QDir::toNativeSeparators(directoryEntries.at(entryIndex).fileName());
    }

    QString details = tr("Selected folder contents:") + "\n" + displayedEntries.join("\n");
    if (directoryEntries.size() > maxDetailedEntries)
    {
      details += "\n" + tr("... and %1 more item(s).").arg(directoryEntries.size() - maxDetailedEntries);
    }
    confirmationMessageBox.setDetailedText(details);
  }

  confirmationMessageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  confirmationMessageBox.setDefaultButton(QMessageBox::No);

  if (confirmationMessageBox.exec() != QMessageBox::Yes)
  {
    return false;
  }

  bool sentinelCreated;
  sentinelCreated = d->CacheManager->CreateSentinelFileInDirectory(cacheDirectory.toStdString());
  if (!sentinelCreated)
  {
    QMessageBox::warning(this, tr("Unable to Continue"), tr("Failed to create cache sentinel file in %1. Operation was canceled.").arg(cacheDirectory));
    return false;
  }

  return true;
}
