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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QAbstractTextDocumentLayout>
#include <QDebug>
#include <QDesktopServices>
#include <QFileInfo>
#include <QLabel>
#include <QLatin1String>
#include <QListWidget>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QSignalMapper>
#include <QStyledItemDelegate>
#include <QTextBlock>
#include <QTextDocument>
#include <QUrlQuery>

// Slicer includes
#include "qSlicerExtensionsManagerModel.h"
#include "qSlicerExtensionsLocalWidget.h"
#include "ui_qSlicerExtensionsButtonBox.h"

//-----------------------------------------------------------------------------
class qSlicerExtensionsLocalWidgetPrivate
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsLocalWidget);

protected:
  qSlicerExtensionsLocalWidget* const q_ptr;

public:
  typedef qSlicerExtensionsLocalWidgetPrivate Self;
  typedef qSlicerExtensionsManagerModel::ExtensionMetadataType ExtensionMetadataType;

  enum DataRoles
  {
    NameRole = Qt::UserRole,
    InstalledExtensionRevisionRole,
    InstalledExtensionSlicerVersionRole,
    InstalledExtensionUpdatedRole,
    OnServerExtensionRevisionRole,
    ChangeLogUrlRole,
    OnServerExtensionMissingRole, // confirmed that the extension is missing from the server
    OnServerExtensionUpdatedRole,
    UpdateAvailableRole,
    MoreLinkRole,
    DescriptionRole,
    CompatibleRole,
    LoadedRole,
    InstalledRole,
    BookmarkedRole,
    EnabledRole,
    ScheduledForUpdateRole,
    ScheduledForUninstallRole
  };

  qSlicerExtensionsLocalWidgetPrivate(qSlicerExtensionsLocalWidget& object);
  void init();

  QListWidgetItem* extensionItem(const QString& extensionName) const;

  // Add/update/remove extension item
  QListWidgetItem* updateExtensionItem(const QString& extensionName);

  QString extensionIconPath(const QString& extensionName, const QUrl& extensionIconUrl);
  QIcon extensionIcon(const QString& extensionName, const QUrl& extensionIconUrl);

  QSignalMapper InstallButtonMapper;
  QSignalMapper AddBookmarkButtonMapper;
  QSignalMapper RemoveBookmarkButtonMapper;
  QSignalMapper EnableButtonMapper;
  QSignalMapper DisableButtonMapper;
  QSignalMapper ScheduleUninstallButtonMapper;
  QSignalMapper CancelScheduledUninstallButtonMapper;
  QSignalMapper ScheduleUpdateButtonMapper;
  QSignalMapper CancelScheduledUpdateButtonMapper;

  QNetworkAccessManager IconDownloadManager;
  QSignalMapper IconDownloadMapper;
  QHash<QString, QNetworkReply*> IconDownloads;

  QString SearchText;

  qSlicerExtensionsManagerModel* ExtensionsManagerModel;
};

// --------------------------------------------------------------------------
qSlicerExtensionsLocalWidgetPrivate::qSlicerExtensionsLocalWidgetPrivate(qSlicerExtensionsLocalWidget& object)
  : q_ptr(&object)
{
  this->ExtensionsManagerModel = nullptr;
}

// --------------------------------------------------------------------------
namespace
{

// --------------------------------------------------------------------------
class qSlicerExtensionsButtonBox
  : public QWidget
  , public Ui_qSlicerExtensionsButtonBox
{
public:
  typedef QWidget Superclass;
  qSlicerExtensionsButtonBox(QListWidgetItem* widgetItem, QWidget* parent = nullptr)
    : Superclass(parent)
    , WidgetItem(widgetItem)
  {
    this->setupUi(this);
    this->InstallProgress->setVisible(false);
    this->UpdateProgress->setVisible(false);
  }

  void updateFromWidgetItem()
  {
    bool installed = this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::InstalledRole).toBool();
    bool compatible = this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::CompatibleRole).toBool();
    bool loaded = this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::LoadedRole).toBool();
    bool bookmarked = this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::BookmarkedRole).toBool();
    bool enabled = this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::EnabledRole).toBool();
    bool scheduledForUpdate =
      this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::ScheduledForUpdateRole).toBool();
    bool scheduledForUninstall =
      this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::ScheduledForUninstallRole).toBool();
    bool updateAvailable = this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::UpdateAvailableRole).toBool();
    // Available if already installed (e.g., from file) or has found a revision on server
    bool available = installed
                     || !this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::OnServerExtensionRevisionRole)
                           .toString()
                           .isEmpty();

    this->InstallButton->setVisible(!installed);
    this->InstallButton->setEnabled(compatible && available);

    if (installed && !loaded)
    {
      this->StatusLabel->setVisible(true);
      this->StatusLabel->setText(qSlicerExtensionsLocalWidget::tr("Install pending restart"));
    }
    else if (scheduledForUpdate)
    {
      this->StatusLabel->setVisible(true);
      this->StatusLabel->setText(qSlicerExtensionsLocalWidget::tr("Update pending restart"));
    }
    else if (scheduledForUninstall)
    {
      this->StatusLabel->setVisible(true);
      this->StatusLabel->setText(qSlicerExtensionsLocalWidget::tr("Uninstall pending restart"));
    }
    else
    {
      this->StatusLabel->setVisible(false);
    }

    this->AddBookmarkButton->setVisible(!bookmarked);
    this->RemoveBookmarkButton->setVisible(bookmarked);

    this->EnableButton->setVisible(!enabled && installed && !scheduledForUninstall);
    this->EnableButton->setEnabled(compatible);
    this->DisableButton->setVisible(enabled && loaded && installed && !scheduledForUninstall && !scheduledForUpdate);

    this->ScheduleForUninstallButton->setVisible(!scheduledForUninstall && installed);
    this->CancelScheduledForUninstallButton->setVisible(scheduledForUninstall);

    this->UpdateOptionsWidget->setVisible(updateAvailable);

    this->ScheduleForUpdateButton->setVisible(!scheduledForUpdate && !scheduledForUninstall);
    this->CancelScheduledForUpdateButton->setVisible(scheduledForUpdate);
  }

  QListWidgetItem* WidgetItem;
};

} // end of anonymous namespace

// --------------------------------------------------------------------------
class qSlicerExtensionsItemDelegate : public QStyledItemDelegate
{
public:
  qSlicerExtensionsItemDelegate(qSlicerExtensionsLocalWidget* list, QObject* parent = nullptr)
    : QStyledItemDelegate(parent)
    , List(list)
  {
  }

  // --------------------------------------------------------------------------
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
  {
    QStyleOptionViewItem modifiedOption = option;
    QListWidgetItem* const item = this->List->itemFromIndex(index);
    if (item && !item->data(qSlicerExtensionsLocalWidgetPrivate::EnabledRole).toBool())
    {
      modifiedOption.state &= ~QStyle::State_Enabled;
    }
    QStyledItemDelegate::paint(painter, modifiedOption, index);
  }

protected:
  qSlicerExtensionsLocalWidget* const List;
};

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidgetPrivate::init()
{
  Q_Q(qSlicerExtensionsLocalWidget);

  q->setAlternatingRowColors(true);
  q->setSelectionMode(QAbstractItemView::NoSelection);
  q->setIconSize(QSize(64, 64));
  q->setSpacing(1);
  q->setItemDelegate(new qSlicerExtensionsItemDelegate(q, q));

  QObject::connect(&this->InstallButtonMapper, SIGNAL(mapped(QString)), q, SLOT(installExtension(QString)));
  QObject::connect(&this->AddBookmarkButtonMapper, SIGNAL(mapped(QString)), q, SLOT(addBookmark(QString)));
  QObject::connect(&this->RemoveBookmarkButtonMapper, SIGNAL(mapped(QString)), q, SLOT(removeBookmark(QString)));
  QObject::connect(&this->EnableButtonMapper, SIGNAL(mapped(QString)), q, SLOT(setExtensionEnabled(QString)));
  QObject::connect(&this->DisableButtonMapper, SIGNAL(mapped(QString)), q, SLOT(setExtensionDisabled(QString)));
  QObject::connect(
    &this->ScheduleUninstallButtonMapper, SIGNAL(mapped(QString)), q, SLOT(scheduleExtensionForUninstall(QString)));
  QObject::connect(&this->CancelScheduledUninstallButtonMapper,
                   SIGNAL(mapped(QString)),
                   q,
                   SLOT(cancelExtensionScheduledForUninstall(QString)));
  QObject::connect(
    &this->ScheduleUpdateButtonMapper, SIGNAL(mapped(QString)), q, SLOT(scheduleExtensionForUpdate(QString)));
  QObject::connect(&this->CancelScheduledUpdateButtonMapper,
                   SIGNAL(mapped(QString)),
                   q,
                   SLOT(cancelExtensionScheduledForUpdate(QString)));
  QObject::connect(&this->IconDownloadMapper, SIGNAL(mapped(QString)), q, SLOT(onIconDownloadComplete(QString)));
}

// --------------------------------------------------------------------------
QString qSlicerExtensionsLocalWidgetPrivate::extensionIconPath(const QString& extensionName,
                                                               const QUrl& extensionIconUrl)
{
  return QString("%1/%2-icon.%3")
    .arg(this->ExtensionsManagerModel->extensionsInstallPath(),
         extensionName,
         QFileInfo(extensionIconUrl.path()).suffix());
}

// --------------------------------------------------------------------------
QIcon qSlicerExtensionsLocalWidgetPrivate::extensionIcon(const QString& extensionName, const QUrl& extensionIconUrl)
{
  Q_Q(qSlicerExtensionsLocalWidget);

  if (extensionIconUrl.isValid())
  {
    const QString iconPath = this->extensionIconPath(extensionName, extensionIconUrl);
    if (QFileInfo(iconPath).exists())
    {
      QPixmap pixmap(iconPath);
      pixmap = pixmap.scaled(q->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

      if (pixmap.isNull())
      {
        // Use default icon if unable to load extension icon
        return this->extensionIcon(QString(), QUrl());
      }

      QPixmap canvas(pixmap.size());
      canvas.fill(Qt::transparent);

      QPainter painter;
      painter.begin(&canvas);
      painter.setPen(Qt::NoPen);
      painter.setBrush(pixmap);
      painter.setRenderHint(QPainter::Antialiasing, true);
      painter.drawRoundedRect(QRect(QPoint(0, 0), pixmap.size()), 5, 5);
      painter.end();

      return QIcon(canvas);
    }

    if (!this->IconDownloads.contains(extensionName))
    {
      // Try to download icon
      QNetworkRequest req(extensionIconUrl);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
      // Icons are hosted on random servers, which often use redirects (301 redirect) to get the actual download URL.
      // In Qt6, redirects are followed by default, but it has to be manually enabled in Qt5.
      req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
#endif
      QNetworkReply* const reply = this->IconDownloadManager.get(req);

      this->IconDownloads.insert(extensionName, reply);
      this->IconDownloadMapper.setMapping(reply, extensionName);

      QObject::connect(reply, SIGNAL(finished()), &this->IconDownloadMapper, SLOT(map()));
    }
    else
    {
      qDebug() << "Icon download for " << extensionName
               << " already in progress. Active icon downloads: " << this->IconDownloads.keys();
    }
  }

  return QIcon(":/Icons/ExtensionDefaultIcon.png");
}

// --------------------------------------------------------------------------
QListWidgetItem* qSlicerExtensionsLocalWidgetPrivate::extensionItem(const QString& extensionName) const
{
  Q_Q(const qSlicerExtensionsLocalWidget);

  QAbstractItemModel* model = q->model();
  const QModelIndexList indices =
    model->match(model->index(0, 0, QModelIndex()), Self::NameRole, extensionName, 2, Qt::MatchExactly);

  Q_ASSERT(indices.count() < 2);
  if (indices.count() == 1)
  {
    return q->item(indices.first().row());
  }
  return nullptr;
}

namespace
{

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
class qSlicerExtensionsDescriptionLabel : public QLabel
{
public:
  typedef QLabel Superclass;

  // --------------------------------------------------------------------------
  qSlicerExtensionsDescriptionLabel(const QString& extensionName, const QString& slicerRevision, QListWidgetItem* item)
    : QLabel()
    , ExtensionName(extensionName)
    , SlicerRevision(slicerRevision)
    , WidgetItem(item)
  {
    QTextOption textOption = this->Text.defaultTextOption();
    textOption.setWrapMode(QTextOption::NoWrap);
    this->Text.setDefaultTextOption(textOption);
    this->MoreLinkText = qSlicerExtensionsLocalWidget::tr("More...");
  }

  // --------------------------------------------------------------------------
  void updateFromWidgetItem()
  {
    this->LastWidth = -1; // force update in next paint
    this->update();
  }

  QListWidgetItem* widgetItem() { return this->WidgetItem; }

  // --------------------------------------------------------------------------
  QSize sizeHint() const override
  {
    QSize hint = this->Superclass::sizeHint();
    hint.setHeight(qRound(this->Text.size().height() + 0.5) + this->margin() * 2);
    return hint;
  }

protected:
  QString versionString(const QString& revision, const QString& isoDateStr)
  {
    // Get formatted date
    QString formattedDate;
    QDateTime date = QDateTime::fromString(isoDateStr, Qt::ISODate);
    if (date.isValid())
    {
      formattedDate = date.toString("yyyy-MM-dd");
    }
    if (!revision.isEmpty() && !formattedDate.isEmpty())
    {
      return QString("%1 (%2)").arg(revision).arg(formattedDate);
    }
    else if (revision.isEmpty() && formattedDate.isEmpty())
    {
      return qSlicerExtensionsLocalWidget::tr("unknown");
    }
    else
    {
      return revision + formattedDate;
    }
  }

  // --------------------------------------------------------------------------
  void labelText(const QString& elidedDescription)
  {
    QString extensionDescription =
      this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::DescriptionRole).toString();

    QString labelText;

    labelText += QString("<h2>%1</h2>").arg(this->ExtensionName);

    // Warnings/notices
    bool compatible = this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::CompatibleRole).toBool();
    bool enabled = this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::EnabledRole).toBool();
    bool installed = this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::InstalledRole).toBool();
    // Available if already installed (e.g., from file) or has found a revision on server
    bool available = installed
                     || !this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::OnServerExtensionRevisionRole)
                           .toString()
                           .isEmpty();
    // Confirmed to be missing from server
    bool missing = this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::OnServerExtensionMissingRole).toBool();
    bool scheduledForUpdate =
      this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::ScheduledForUpdateRole).toBool();

    QString installedVersion = this->versionString(
      this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::InstalledExtensionRevisionRole).toString(),
      this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::InstalledExtensionUpdatedRole).toString());

    // Status line
    QString statusText;
    if (!available && missing) // "missing" is checked so that we don't display this message when there have been no
                               // update checks before
    {
      statusText += QString("<p style=\"font-weight: bold; font-size: 80%; color: %1;\">"
                            "<img style=\"float: left\" src=\":/Icons/ExtensionIncompatible.svg\"/> ")
                      .arg(this->WarningColor)
                    + qSlicerExtensionsLocalWidget::tr("Not found for this version of the application (r%1)")
                        .arg(this->SlicerRevision)
                    + QLatin1String("</p>");
    }
    if (!compatible)
    {
      statusText +=
        QLatin1String("<p style=\"font-weight: bold; font-size: 80%; color: %1;\">"
                      "<img style=\"float: left\" src=\":/Icons/ExtensionIncompatible.svg\"/> ")
          .arg(this->WarningColor)
        + qSlicerExtensionsLocalWidget::tr("Incompatible with Slicer r%1 [built for r%2]")
            .arg(this->SlicerRevision)
            .arg(this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::InstalledExtensionSlicerVersionRole)
                   .toString())
        + QLatin1String("</p>");
    }
    if (this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::UpdateAvailableRole).toBool()
        && !scheduledForUpdate)
    {
      QString onServerVersion = this->versionString(
        this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::OnServerExtensionRevisionRole).toString(),
        this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::OnServerExtensionUpdatedRole).toString());

      QString changeLogText;
      QString changeLogUrl = this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::ChangeLogUrlRole).toString();
      if (!changeLogUrl.isEmpty())
      {
        changeLogText =
          QString(" <a href=\"%1\">%2</a>").arg(changeLogUrl).arg(qSlicerExtensionsLocalWidget::tr("Change log..."));
      }
      statusText += QString("<p style=\"font-weight: bold; font-size: 80%; color: %1;\">"
                            "<img style=\"float: left\""
                            " src=\":/Icons/ExtensionUpdateAvailable.svg\"/> ")
                      .arg(this->InfoColor);
      statusText += qSlicerExtensionsLocalWidget::tr("An update is available. Installed: %1. Available: %2.")
                      .arg(installedVersion)
                      .arg(onServerVersion);
      statusText += changeLogText + QLatin1String("</p>");
    }
    if (statusText.isEmpty())
    {
      // if there are no notices than just add version information or an empty line (to make the layout more balanced)
      if (installed)
      {
        // Version

        if (!enabled || !compatible)
        {
          statusText += QLatin1String("<p>")
                        + qSlicerExtensionsLocalWidget::tr("Version: %1. Disabled.").arg(installedVersion)
                        + QLatin1String("</p>");
        }
        else
        {
          statusText += QLatin1String("<p>") + qSlicerExtensionsLocalWidget::tr("Version: %1").arg(installedVersion)
                        + QLatin1String("</p>");
        }
      }
      else
      {
        statusText += QLatin1String("<p>") + qSlicerExtensionsLocalWidget::tr("Not installed.") + QLatin1String("</p>");
      }
    }
    labelText += statusText;

    // Description and link
    QString linkText;
    QString moreLink = this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::MoreLinkRole).toString();
    if (!moreLink.isEmpty() && !missing)
    {
      linkText = QString(" <a href=\"%1\">%2</a>").arg(moreLink).arg(this->MoreLinkText);
    }
    labelText += QString("<p>%1%2</p>").arg(elidedDescription).arg(linkText);

    this->Text.setHtml(labelText);
    this->setToolTip(extensionDescription);
    this->setMouseTracking(!moreLink.isEmpty());
  }

  // --------------------------------------------------------------------------
  void paintEvent(QPaintEvent*) override
  {
    QPainter painter(this);
    const QRect cr = this->contentsRect();

    if (this->LastWidth != cr.width())
    {
      // Changed description text or available space, update elided text
      int margin = this->margin() * 2;
      bool moreLinkSpecified =
        !this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::MoreLinkRole).toString().isEmpty();
      if (moreLinkSpecified)
      {
        QString moreLinkText = QString(" %1").arg(this->MoreLinkText);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
        margin += this->fontMetrics().horizontalAdvance(moreLinkText);
#else
        margin += this->fontMetrics().width(moreLinkText);
#endif
      }
      QString extensionDescription =
        this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::DescriptionRole).toString();
      QString elidedExtensionDescription =
        this->fontMetrics().elidedText(extensionDescription, Qt::ElideRight, cr.width() - margin);
      this->labelText(elidedExtensionDescription);
      this->LastWidth = cr.width();
      this->Text.setTextWidth(this->LastWidth);
    }

    QAbstractTextDocumentLayout::PaintContext context;
    context.palette = this->palette();
    if (!this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::InstalledRole).toBool()
        || this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::ScheduledForUninstallRole).toBool()
        || !this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::CompatibleRole).toBool()
        || !this->WidgetItem->data(qSlicerExtensionsLocalWidgetPrivate::EnabledRole).toBool())
    {
      context.palette.setCurrentColorGroup(QPalette::Disabled);
    }

    painter.translate(cr.topLeft());
    this->Text.documentLayout()->draw(&painter, context);
  }

  // --------------------------------------------------------------------------
  QString linkUnderCursor(const QPoint& pos)
  {
    const int caretPos = this->Text.documentLayout()->hitTest(pos, Qt::FuzzyHit);
    if (caretPos < 0)
    {
      return QString();
    }

    const QTextBlock& block = this->Text.findBlock(caretPos);
    for (QTextBlock::iterator iter = block.begin(); !iter.atEnd(); ++iter)
    {
      const QTextFragment& fragment = iter.fragment();
      const int fp = fragment.position();
      if (fp <= caretPos && fp + fragment.length() > caretPos)
      {
        return fragment.charFormat().anchorHref();
      }
    }

    return QString();
  }

  // --------------------------------------------------------------------------
  void mouseMoveEvent(QMouseEvent* e) override
  {
    Superclass::mouseMoveEvent(e);

    QString href = this->linkUnderCursor(e->pos());
    if (href != this->LinkUnderCursor)
    {
      this->LinkUnderCursor = href;
      if (href.isEmpty())
      {
        this->unsetCursor();
      }
      else
      {
        this->setCursor(Qt::PointingHandCursor);
        emit this->linkHovered(href);
      }
    }
  }

  // --------------------------------------------------------------------------
  void mouseReleaseEvent(QMouseEvent* e) override
  {
    Superclass::mouseReleaseEvent(e);
    if (e->button() == Qt::LeftButton)
    {
      QString href = this->linkUnderCursor(e->pos());
      if (!href.isEmpty())
      {
        emit this->linkActivated(href);
      }
    }
  }

  const QString ExtensionName;
  const QString SlicerRevision;
  const QString WarningColor{ "#bd8530" };
  const QString InfoColor{ "#2c70c8" };
  QListWidgetItem* WidgetItem;

  QString MoreLinkText;    // used for computing available space for the extension description
  QString LinkUnderCursor; // used for detecting if the mouse is over the "More" link
  QTextDocument Text;      // stores displayed text and format
  int LastWidth{ -1 };     // to avoid assembling Text on every paint
};

// --------------------------------------------------------------------------
class qSlicerExtensionsItemWidget : public QWidget
{
public:
  qSlicerExtensionsItemWidget(qSlicerExtensionsDescriptionLabel* label, QWidget* parent = nullptr)
    : QWidget(parent)
    , Label(label)
  {
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(label, 1);
    this->ButtonBox = new qSlicerExtensionsButtonBox(label->widgetItem());
    layout->addWidget(this->ButtonBox);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);

    label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  }

  // --------------------------------------------------------------------------
  void updateFromWidgetItem()
  {
    this->Label->updateFromWidgetItem();
    this->ButtonBox->updateFromWidgetItem();
  }

  qSlicerExtensionsDescriptionLabel* Label;
  qSlicerExtensionsButtonBox* ButtonBox;
};

} // end of anonymous namespace

// --------------------------------------------------------------------------
QListWidgetItem* qSlicerExtensionsLocalWidgetPrivate::updateExtensionItem(const QString& extensionName)
{
  Q_Q(qSlicerExtensionsLocalWidget);
  if (extensionName.isEmpty())
  {
    qCritical() << "Missing metadata identified with 'extensionname' key";
    return nullptr;
  }
  QListWidgetItem* item = this->extensionItem(extensionName);
  qSlicerExtensionsManagerModel::ExtensionMetadataType metadata =
    this->ExtensionsManagerModel->extensionMetadata(extensionName);
  qSlicerExtensionsManagerModel::ExtensionMetadataType metadataFromServer =
    this->ExtensionsManagerModel->extensionMetadata(extensionName, qSlicerExtensionsManagerModel::MetadataServer);

  // Remove the item if it is no longer bookmarked nor installed
  bool removeItem = false;
  if (!metadata["bookmarked"].toBool() && !metadata["installed"].toBool())
  {
    removeItem = true;
  }

  // Remove the imte if does not match search text
  if (!removeItem && !this->SearchText.isEmpty())
  {
    // filtering by text is enabled, we may need to remove this item

    // Include extension if search text is found in extension name or description
    bool includeExtension = extensionName.contains(this->SearchText, Qt::CaseInsensitive);
    if (!includeExtension)
    {
      QString description = this->ExtensionsManagerModel->extensionDescription(extensionName);
      includeExtension = description.contains(this->SearchText, Qt::CaseInsensitive);
    }
    if (!includeExtension)
    {
      removeItem = true;
    }
  }

  // remove item as needed
  if (removeItem)
  {
    if (item)
    {
      // this item should not exist
      QAbstractItemModel* model = q->model();
      const QModelIndexList indices =
        model->match(model->index(0, 0, QModelIndex()), Self::NameRole, extensionName, 1, Qt::MatchExactly);
      if (indices.count() > 0)
      {
        q->model()->removeRow(indices.first().row());
      }
    }
    return nullptr;
  }

  // add new item as needed
  bool newItemCreated = false;
  if (!item)
  {
    item = new QListWidgetItem();
    newItemCreated = true;
  }

  item->setIcon(this->extensionIcon(extensionName, metadata.value("iconurl").toUrl()));

  QString moreLinkTarget;
  int serverAPI = q->extensionsManagerModel()->serverAPI();
  if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
  {
    moreLinkTarget = QString("slicer:%1").arg(extensionName);
  }
  else
  {
    qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << serverAPI;
  }

  QString installedRevision = metadata["installed"].toBool() ? metadata["revision"].toString() : QString();
  QString onServerRevision = metadataFromServer["revision"].toString();

  // Generate change log URL - only if repository is hosted on github or gitlab
  QUrl scmUrl = QUrl(metadataFromServer["scmurl"].toString());
  QString changeLogUrl;
  if (!installedRevision.isEmpty() && !onServerRevision.isEmpty())
  {
    QString scmUrlPath = scmUrl.path();
    // Path is usually like this: https://github.com/SlicerIGT/SlicerIGT.git
    // We need to remove the .git suffix to get the repository URL path.
    if (scmUrlPath.endsWith(".git"))
    {
      scmUrlPath.chop(4);
    }
    if (scmUrl.host().toLower() == "github.com")
    {
      scmUrl.setPath(QString("%1/compare/%2...%3").arg(scmUrlPath).arg(installedRevision).arg(onServerRevision));
      changeLogUrl = scmUrl.toString();
    }
    else if (scmUrl.host().toLower() == "gitlab.com")
    {
      scmUrl.setPath(QString("%1/-/compare/%2...%3").arg(scmUrlPath).arg(installedRevision).arg(onServerRevision));
      changeLogUrl = scmUrl.toString();
    }
  }

  // Save some metadata fields into the item to allow filtering and search

  item->setData(Self::NameRole, extensionName);
  item->setData(Self::InstalledExtensionRevisionRole, installedRevision);
  item->setData(Self::InstalledExtensionUpdatedRole,
                metadata["installed"].toBool() ? metadata["updated"].toString() : QString());
  item->setData(Self::InstalledExtensionSlicerVersionRole,
                metadata["installed"].toBool() ? metadata["slicer_revision"].toString() : QString());
  item->setData(Self::OnServerExtensionRevisionRole, onServerRevision);
  item->setData(Self::OnServerExtensionMissingRole,
                metadataFromServer["revision"].toString().isEmpty()
                  && this->ExtensionsManagerModel->lastUpdateTimeExtensionsMetadataFromServer().isValid());
  item->setData(Self::ChangeLogUrlRole, changeLogUrl);
  item->setData(Self::OnServerExtensionUpdatedRole, metadataFromServer["updated"].toString());
  item->setData(Self::UpdateAvailableRole, q->extensionsManagerModel()->isExtensionUpdateAvailable(extensionName));
  item->setData(Self::MoreLinkRole, moreLinkTarget);
  item->setData(Self::DescriptionRole, metadata["description"].toString());
  item->setData(Self::CompatibleRole, q->extensionsManagerModel()->isExtensionCompatible(extensionName).isEmpty());
  item->setData(Self::LoadedRole, metadata["loaded"].toBool());
  item->setData(Self::InstalledRole, metadata["installed"].toBool());
  item->setData(Self::BookmarkedRole, metadata["bookmarked"].toBool());
  item->setData(Self::EnabledRole, metadata["enabled"].toBool());
  item->setData(Self::ScheduledForUpdateRole,
                q->extensionsManagerModel()->isExtensionScheduledForUpdate(extensionName));
  item->setData(Self::ScheduledForUninstallRole,
                q->extensionsManagerModel()->isExtensionScheduledForUninstall(extensionName));

  if (newItemCreated)
  {

    // Initialize label
    qSlicerExtensionsDescriptionLabel* label =
      new qSlicerExtensionsDescriptionLabel(extensionName, q->extensionsManagerModel()->slicerRevision(), item);
    label->setMargin(6);
    QObject::connect(label, SIGNAL(linkActivated(QString)), q, SLOT(onLinkActivated(QString)));

    qSlicerExtensionsItemWidget* widget = new qSlicerExtensionsItemWidget(label);

    widget->updateFromWidgetItem();

    this->AddBookmarkButtonMapper.setMapping(widget->ButtonBox->AddBookmarkButton, extensionName);
    QObject::connect(
      widget->ButtonBox->AddBookmarkButton, SIGNAL(clicked()), &this->AddBookmarkButtonMapper, SLOT(map()));
    this->RemoveBookmarkButtonMapper.setMapping(widget->ButtonBox->RemoveBookmarkButton, extensionName);
    QObject::connect(
      widget->ButtonBox->RemoveBookmarkButton, SIGNAL(clicked()), &this->RemoveBookmarkButtonMapper, SLOT(map()));

    this->InstallButtonMapper.setMapping(widget->ButtonBox->InstallButton, extensionName);
    QObject::connect(widget->ButtonBox->InstallButton, SIGNAL(clicked()), &this->InstallButtonMapper, SLOT(map()));

    this->ScheduleUpdateButtonMapper.setMapping(widget->ButtonBox->ScheduleForUpdateButton, extensionName);
    QObject::connect(
      widget->ButtonBox->ScheduleForUpdateButton, SIGNAL(clicked()), &this->ScheduleUpdateButtonMapper, SLOT(map()));
    this->CancelScheduledUpdateButtonMapper.setMapping(widget->ButtonBox->CancelScheduledForUpdateButton,
                                                       extensionName);
    QObject::connect(widget->ButtonBox->CancelScheduledForUpdateButton,
                     SIGNAL(clicked()),
                     &this->CancelScheduledUpdateButtonMapper,
                     SLOT(map()));

    this->EnableButtonMapper.setMapping(widget->ButtonBox->EnableButton, extensionName);
    QObject::connect(widget->ButtonBox->EnableButton, SIGNAL(clicked()), &this->EnableButtonMapper, SLOT(map()));
    this->DisableButtonMapper.setMapping(widget->ButtonBox->DisableButton, extensionName);
    QObject::connect(widget->ButtonBox->DisableButton, SIGNAL(clicked()), &this->DisableButtonMapper, SLOT(map()));

    this->ScheduleUninstallButtonMapper.setMapping(widget->ButtonBox->ScheduleForUninstallButton, extensionName);
    QObject::connect(widget->ButtonBox->ScheduleForUninstallButton,
                     SIGNAL(clicked()),
                     &this->ScheduleUninstallButtonMapper,
                     SLOT(map()));
    this->CancelScheduledUninstallButtonMapper.setMapping(widget->ButtonBox->CancelScheduledForUninstallButton,
                                                          extensionName);
    QObject::connect(widget->ButtonBox->CancelScheduledForUninstallButton,
                     SIGNAL(clicked()),
                     &this->CancelScheduledUninstallButtonMapper,
                     SLOT(map()));

    QSize hint = label->sizeHint();
    hint.setWidth(hint.width() + 64);
    hint.setHeight(qMax(hint.height(), widget->ButtonBox->minimumSizeHint().height()));
    item->setSizeHint(hint);

    q->addItem(item);
    q->setItemWidget(item, widget);
  }
  else
  {
    qSlicerExtensionsItemWidget* widget = dynamic_cast<qSlicerExtensionsItemWidget*>(q->itemWidget(item));
    if (!widget)
    {
      qWarning() << Q_FUNC_INFO << " failed: cannot update list item for extension " << extensionName;
      return nullptr;
    }
    widget->updateFromWidgetItem();
  }
  return item;
}

// --------------------------------------------------------------------------
qSlicerExtensionsLocalWidget::qSlicerExtensionsLocalWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsLocalWidgetPrivate(*this))
{
  Q_D(qSlicerExtensionsLocalWidget);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsLocalWidget::~qSlicerExtensionsLocalWidget() = default;

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerExtensionsLocalWidget::extensionsManagerModel() const
{
  Q_D(const qSlicerExtensionsLocalWidget);
  return d->ExtensionsManagerModel;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::setExtensionsManagerModel(qSlicerExtensionsManagerModel* model)
{
  Q_D(qSlicerExtensionsLocalWidget);

  if (this->extensionsManagerModel() == model)
  {
    return;
  }

  disconnect(this, SLOT(onModelUpdated()));
  disconnect(this, SLOT(onExtensionInstalled(QString)));
  disconnect(this, SLOT(onExtensionUninstalled(QString)));
  disconnect(this, SLOT(onExtensionMetadataUpdated(QString)));
  disconnect(this, SLOT(onExtensionScheduledForUninstall(QString)));
  disconnect(this, SLOT(onExtensionCancelledScheduleForUninstall(QString)));
  disconnect(this, SLOT(onModelExtensionEnabledChanged(QString, bool)));
  disconnect(this, SLOT(onExtensionBookmarkedChanged(QString, bool)));
  disconnect(this, SLOT(setExtensionUpdateDownloadProgress(QString, qint64, qint64)));
  disconnect(this, SLOT(setExtensionInstallDownloadProgress(QString, qint64, qint64)));

  d->ExtensionsManagerModel = model;

  if (d->ExtensionsManagerModel)
  {
    connect(d->ExtensionsManagerModel, SIGNAL(modelUpdated()), this, SLOT(onModelUpdated()));
    connect(d->ExtensionsManagerModel, SIGNAL(extensionInstalled(QString)), this, SLOT(onExtensionInstalled(QString)));
    connect(
      d->ExtensionsManagerModel, SIGNAL(extensionUninstalled(QString)), this, SLOT(onExtensionUninstalled(QString)));
    connect(d->ExtensionsManagerModel,
            SIGNAL(extensionMetadataUpdated(QString)),
            this,
            SLOT(onExtensionMetadataUpdated(QString)));
    connect(d->ExtensionsManagerModel,
            SIGNAL(extensionBookmarkedChanged(QString, bool)),
            this,
            SLOT(onExtensionBookmarkedChanged(QString, bool)));
    connect(d->ExtensionsManagerModel,
            SIGNAL(extensionScheduledForUninstall(QString)),
            this,
            SLOT(onExtensionScheduledForUninstall(QString)));
    connect(d->ExtensionsManagerModel,
            SIGNAL(extensionCancelledScheduleForUninstall(QString)),
            this,
            SLOT(onExtensionCancelledScheduleForUninstall(QString)));
    connect(d->ExtensionsManagerModel,
            SIGNAL(extensionEnabledChanged(QString, bool)),
            this,
            SLOT(onModelExtensionEnabledChanged(QString, bool)));
    connect(d->ExtensionsManagerModel,
            SIGNAL(extensionUpdateAvailable(QString)),
            this,
            SLOT(setExtensionUpdateAvailable(QString)));
    connect(d->ExtensionsManagerModel,
            SIGNAL(extensionScheduledForUpdate(QString)),
            this,
            SLOT(setExtensionUpdateScheduled(QString)));
    connect(d->ExtensionsManagerModel,
            SIGNAL(extensionCancelledScheduleForUpdate(QString)),
            this,
            SLOT(setExtensionUpdateCanceled(QString)));
    connect(d->ExtensionsManagerModel,
            SIGNAL(updateDownloadProgress(QString, qint64, qint64)),
            this,
            SLOT(setExtensionUpdateDownloadProgress(QString, qint64, qint64)));
    connect(d->ExtensionsManagerModel,
            SIGNAL(installDownloadProgress(QString, qint64, qint64)),
            this,
            SLOT(setExtensionInstallDownloadProgress(QString, qint64, qint64)));
    this->onModelUpdated();
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::displayExtensionDetails(const QString& extensionName)
{
  Q_UNUSED(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::installExtension(const QString& extensionName)
{
  Q_D(const qSlicerExtensionsLocalWidget);
  if (!this->extensionsManagerModel())
  {
    return;
  }
  this->extensionsManagerModel()->downloadAndInstallExtensionByName(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::addBookmark(const QString& extensionName)
{
  if (!this->extensionsManagerModel())
  {
    return;
  }
  this->extensionsManagerModel()->setExtensionBookmarked(extensionName, true);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::removeBookmark(const QString& extensionName)
{
  if (!this->extensionsManagerModel())
  {
    return;
  }
  this->extensionsManagerModel()->setExtensionBookmarked(extensionName, false);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::setExtensionEnabled(const QString& extensionName)
{
  if (!this->extensionsManagerModel())
  {
    return;
  }
  this->extensionsManagerModel()->setExtensionEnabled(extensionName, true);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::setExtensionDisabled(const QString& extensionName)
{
  if (!this->extensionsManagerModel())
  {
    return;
  }
  this->extensionsManagerModel()->setExtensionEnabled(extensionName, false);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::scheduleExtensionForUninstall(const QString& extensionName)
{
  if (!this->extensionsManagerModel())
  {
    return;
  }
  if (this->extensionsManagerModel()->isExtensionLoaded(extensionName))
  {
    // Extension is loaded into memory, it cannot be uninstalled immediately,
    // but only at the next startup
    this->extensionsManagerModel()->scheduleExtensionForUninstall(extensionName);
  }
  else
  {
    // Extension is not loaded into memory yet, we can uninstall now
    this->extensionsManagerModel()->uninstallExtension(extensionName);
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::cancelExtensionScheduledForUninstall(const QString& extensionName)
{
  if (!this->extensionsManagerModel())
  {
    return;
  }
  this->extensionsManagerModel()->cancelExtensionScheduledForUninstall(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::onExtensionMetadataUpdated(const QString& extensionName)
{
  Q_D(qSlicerExtensionsLocalWidget);
  d->updateExtensionItem(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::onExtensionInstalled(const QString& extensionName)
{
  Q_D(qSlicerExtensionsLocalWidget);
  d->updateExtensionItem(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::onExtensionUninstalled(const QString& extensionName)
{
  Q_D(qSlicerExtensionsLocalWidget);
  d->updateExtensionItem(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::onExtensionBookmarkedChanged(const QString& extensionName, bool bookmarked)
{
  Q_UNUSED(bookmarked);
  Q_D(qSlicerExtensionsLocalWidget);
  d->updateExtensionItem(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::onExtensionScheduledForUninstall(const QString& extensionName)
{
  Q_D(qSlicerExtensionsLocalWidget);
  d->updateExtensionItem(extensionName);
}

// -------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::onExtensionCancelledScheduleForUninstall(const QString& extensionName)
{
  Q_D(qSlicerExtensionsLocalWidget);
  d->updateExtensionItem(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::setExtensionUpdateAvailable(const QString& extensionName)
{
  Q_D(qSlicerExtensionsLocalWidget);
  d->updateExtensionItem(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::scheduleExtensionForUpdate(const QString& extensionName)
{
  qSlicerExtensionsManagerModel* const model = this->extensionsManagerModel();
  if (!model)
  {
    return;
  }
  model->scheduleExtensionForUpdate(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::cancelExtensionScheduledForUpdate(const QString& extensionName)
{
  qSlicerExtensionsManagerModel* const model = this->extensionsManagerModel();
  if (!model)
  {
    return;
  }
  model->cancelExtensionScheduledForUpdate(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::setExtensionUpdateScheduled(const QString& extensionName)
{
  Q_D(qSlicerExtensionsLocalWidget);
  d->updateExtensionItem(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::setExtensionUpdateCanceled(const QString& extensionName)
{
  Q_D(qSlicerExtensionsLocalWidget);
  d->updateExtensionItem(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::setExtensionUpdateDownloadProgress(const QString& extensionName,
                                                                      qint64 received,
                                                                      qint64 total)
{
  Q_D(qSlicerExtensionsLocalWidget);
  QListWidgetItem* const item = d->extensionItem(extensionName);
  Q_ASSERT(item);
  qSlicerExtensionsItemWidget* const widget = dynamic_cast<qSlicerExtensionsItemWidget*>(this->itemWidget(item));
  Q_ASSERT(widget);

  if (total < 0)
  {
    widget->ButtonBox->UpdateProgress->setRange(0, 0);
    widget->ButtonBox->UpdateProgress->setValue(0);
  }
  else
  {
    while (total > (1LL << 31))
    {
      total >>= 1;
      received >>= 1;
    }

    widget->ButtonBox->UpdateProgress->setRange(0, static_cast<int>(total));
    widget->ButtonBox->UpdateProgress->setValue(static_cast<int>(received));
  }

  if (received == total)
  {
    d->updateExtensionItem(extensionName);
    widget->ButtonBox->UpdateProgress->setVisible(false);
  }
  else
  {
    d->updateExtensionItem(extensionName);
    widget->ButtonBox->UpdateProgress->setVisible(true);
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::setExtensionInstallDownloadProgress(const QString& extensionName,
                                                                       qint64 received,
                                                                       qint64 total)
{
  Q_D(qSlicerExtensionsLocalWidget);
  QListWidgetItem* const item = d->extensionItem(extensionName);
  if (!item)
  {
    return;
  }
  qSlicerExtensionsItemWidget* const widget = dynamic_cast<qSlicerExtensionsItemWidget*>(this->itemWidget(item));
  if (!widget)
  {
    return;
  }

  if (total < 0)
  {
    widget->ButtonBox->InstallProgress->setRange(0, 0);
    widget->ButtonBox->InstallProgress->setValue(0);
  }
  else
  {
    while (total > (1LL << 31))
    {
      total >>= 1;
      received >>= 1;
    }

    widget->ButtonBox->InstallProgress->setRange(0, static_cast<int>(total));
    widget->ButtonBox->InstallProgress->setValue(static_cast<int>(received));
  }

  if (received == total)
  {
    widget->ButtonBox->InstallProgress->setVisible(false);
    d->updateExtensionItem(extensionName);
  }
  else
  {
    d->updateExtensionItem(extensionName);
    widget->ButtonBox->InstallProgress->setVisible(true);
    widget->ButtonBox->InstallButton->setVisible(false);
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::onModelExtensionEnabledChanged(const QString& extensionName, bool enabled)
{
  Q_UNUSED(enabled);
  Q_D(qSlicerExtensionsLocalWidget);
  d->updateExtensionItem(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::onModelUpdated()
{
  Q_D(qSlicerExtensionsLocalWidget);
  this->clear();
  // Show extensions with available update at the top
  QStringList availableUpdateExtensions = d->ExtensionsManagerModel->availableUpdateExtensions();
  availableUpdateExtensions.sort();
  foreach (const QString& extensionName, availableUpdateExtensions)
  {
    d->updateExtensionItem(extensionName);
  }
  // Show all other extensions
  QStringList managedExtensions = d->ExtensionsManagerModel->managedExtensions();
  managedExtensions.sort();
  foreach (const QString& extensionName, managedExtensions)
  {
    if (availableUpdateExtensions.contains(extensionName))
    {
      // already in the list
      continue;
    }
    d->updateExtensionItem(extensionName);
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::onLinkActivated(const QString& link)
{
  Q_D(qSlicerExtensionsLocalWidget);
  if (link.startsWith("slicer:"))
  {
    // internal link (extension description page on the frontend server)
    QUrl url(d->ExtensionsManagerModel->frontendServerUrl());
    int serverAPI = this->extensionsManagerModel()->serverAPI();
    if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
    {
      {
        QString extensionName = link.mid(7); // remove leading "slicer:"
        url.setPath(url.path()
                    + QString("/view/%1/%2/%3")
                        .arg(extensionName)
                        .arg(this->extensionsManagerModel()->slicerRevision())
                        .arg(this->extensionsManagerModel()->slicerOs()));
      }
    }
    else
    {
      qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << serverAPI;
      return;
    }
    emit this->linkActivated(url);
  }
  else
  {
    // external link (e.g., change log), open in external browser
    QUrl url(link);
    QDesktopServices::openUrl(url);
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::onIconDownloadComplete(const QString& extensionName)
{
  Q_D(qSlicerExtensionsLocalWidget);

  QNetworkReply* const reply = d->IconDownloads.take(extensionName);
  if (!reply)
  {
    qWarning() << Q_FUNC_INFO << " failed: invalid reply";
    return;
  }

  if (reply->error() == QNetworkReply::NoError)
  {
    QFile iconFile(d->extensionIconPath(extensionName, reply->url()));
    if (iconFile.open(QIODevice::WriteOnly))
    {
      iconFile.write(reply->readAll());
      iconFile.close(); // Ensure file written to disk before we try to load it

      QListWidgetItem* item = d->extensionItem(extensionName);
      if (item)
      {
        item->setIcon(d->extensionIcon(extensionName, reply->url()));
      }
    }
  }

  reply->deleteLater();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::setSearchText(const QString& newText)
{
  Q_D(qSlicerExtensionsLocalWidget);
  if (d->SearchText == newText)
  {
    return;
  }
  d->SearchText = newText;
  this->onModelUpdated();
}

// --------------------------------------------------------------------------
QString qSlicerExtensionsLocalWidget::searchText() const
{
  Q_D(const qSlicerExtensionsLocalWidget);
  return d->SearchText;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsLocalWidget::refresh()
{
  Q_D(const qSlicerExtensionsLocalWidget);
  if (this->extensionsManagerModel())
  {
    this->extensionsManagerModel()->updateExtensionsMetadataFromServer();
    this->extensionsManagerModel()->updateModel();
  }
}
