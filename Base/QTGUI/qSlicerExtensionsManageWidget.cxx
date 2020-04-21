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
#include <QFileInfo>
#include <QLabel>
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
#include "qSlicerExtensionsManageWidget.h"
#include "ui_qSlicerExtensionsButtonBox.h"

//-----------------------------------------------------------------------------
class qSlicerExtensionsManageWidgetPrivate
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsManageWidget);
protected:
  qSlicerExtensionsManageWidget* const q_ptr;

public:
  typedef qSlicerExtensionsManageWidgetPrivate Self;
  typedef qSlicerExtensionsManagerModel::ExtensionMetadataType ExtensionMetadataType;


  enum DataRoles
    {
    NameRole = Qt::UserRole,
    EnabledRole,
    };

  qSlicerExtensionsManageWidgetPrivate(qSlicerExtensionsManageWidget& object);
  void init();

  QListWidgetItem * extensionItem(const QString &extensionName) const;

  void addExtensionItem(const ExtensionMetadataType &metadata);

  QString extensionIconPath(const QString& extensionName,
                            const QUrl& extensionIconUrl);
  QIcon extensionIcon(const QString& extensionName,
                      const QUrl& extensionIconUrl);

  QSignalMapper EnableButtonMapper;
  QSignalMapper DisableButtonMapper;
  QSignalMapper ScheduleUninstallButtonMapper;
  QSignalMapper CancelScheduledUninstallButtonMapper;
  QSignalMapper ScheduleUpdateButtonMapper;
  QSignalMapper CancelScheduledUpdateButtonMapper;

  QNetworkAccessManager DownloadManager;
  QSignalMapper DownloadMapper;
  QHash<QString, QNetworkReply*> Downloads;

  qSlicerExtensionsManagerModel * ExtensionsManagerModel;
};

// --------------------------------------------------------------------------
qSlicerExtensionsManageWidgetPrivate::qSlicerExtensionsManageWidgetPrivate(qSlicerExtensionsManageWidget& object)
  :q_ptr(&object)
{
  this->ExtensionsManagerModel = nullptr;
}

// --------------------------------------------------------------------------
namespace
{

// --------------------------------------------------------------------------
class qSlicerExtensionsButtonBox : public QWidget, public Ui_qSlicerExtensionsButtonBox
{
public:
  typedef QWidget Superclass;
  qSlicerExtensionsButtonBox(QWidget* parent = nullptr) : Superclass(parent)
  {
    this->setupUi(this);
  }
};

} // end of anonymous namespace

// --------------------------------------------------------------------------
class qSlicerExtensionsItemDelegate : public QStyledItemDelegate
{
public:
  qSlicerExtensionsItemDelegate(qSlicerExtensionsManageWidget * list,
                                QObject * parent = nullptr)
    : QStyledItemDelegate(parent), List(list) {}

  // --------------------------------------------------------------------------
  void paint(QPainter * painter, const QStyleOptionViewItem& option,
                     const QModelIndex& index) const override
  {
    QStyleOptionViewItem modifiedOption = option;
    QListWidgetItem * const item = this->List->itemFromIndex(index);
    if (item && !item->data(qSlicerExtensionsManageWidgetPrivate::EnabledRole).toBool())
      {
      modifiedOption.state &= ~QStyle::State_Enabled;
      }
    QStyledItemDelegate::paint(painter, modifiedOption, index);
  }

protected:
  qSlicerExtensionsManageWidget * const List;
};

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidgetPrivate::init()
{
  Q_Q(qSlicerExtensionsManageWidget);

  q->setAlternatingRowColors(true);
  q->setSelectionMode(QAbstractItemView::NoSelection);
  q->setIconSize(QSize(64, 64));
  q->setSpacing(1);
  q->setItemDelegate(new qSlicerExtensionsItemDelegate(q, q));

  QObject::connect(&this->EnableButtonMapper, SIGNAL(mapped(QString)),
                   q, SLOT(setExtensionEnabled(QString)));

  QObject::connect(&this->DisableButtonMapper, SIGNAL(mapped(QString)),
                   q, SLOT(setExtensionDisabled(QString)));

  QObject::connect(&this->ScheduleUninstallButtonMapper, SIGNAL(mapped(QString)),
                   q, SLOT(scheduleExtensionForUninstall(QString)));

  QObject::connect(&this->CancelScheduledUninstallButtonMapper, SIGNAL(mapped(QString)),
                   q, SLOT(cancelExtensionScheduledForUninstall(QString)));

  QObject::connect(&this->ScheduleUpdateButtonMapper,
                   SIGNAL(mapped(QString)),
                   q, SLOT(scheduleExtensionForUpdate(QString)));

  QObject::connect(&this->CancelScheduledUpdateButtonMapper,
                   SIGNAL(mapped(QString)),
                   q, SLOT(cancelExtensionScheduledForUpdate(QString)));

  QObject::connect(&this->DownloadMapper, SIGNAL(mapped(QString)),
                   q, SLOT(onIconDownloadComplete(QString)));
}

// --------------------------------------------------------------------------
QString qSlicerExtensionsManageWidgetPrivate::extensionIconPath(
  const QString& extensionName, const QUrl& extensionIconUrl)
{
  return QString("%1/%2-icon.%3").arg(
    this->ExtensionsManagerModel->extensionsInstallPath(),
    extensionName, QFileInfo(extensionIconUrl.path()).suffix());
}

// --------------------------------------------------------------------------
QIcon qSlicerExtensionsManageWidgetPrivate::extensionIcon(
  const QString& extensionName, const QUrl& extensionIconUrl)
{
  Q_Q(qSlicerExtensionsManageWidget);

  if (extensionIconUrl.isValid())
    {
    const QString iconPath = this->extensionIconPath(extensionName,
                                                     extensionIconUrl);
    if (QFileInfo(iconPath).exists())
      {
      QPixmap pixmap(iconPath);
      pixmap = pixmap.scaled(q->iconSize(), Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);

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

    Q_ASSERT(!this->Downloads.contains(extensionName));

    // Try to download icon
    QNetworkReply* const reply =
      this->DownloadManager.get(QNetworkRequest(extensionIconUrl));

    this->Downloads.insert(extensionName, reply);
    this->DownloadMapper.setMapping(reply, extensionName);

    QObject::connect(reply, SIGNAL(finished()),
                     &this->DownloadMapper, SLOT(map()));
    }

  return QIcon(":/Icons/ExtensionDefaultIcon.png");
}

// --------------------------------------------------------------------------
QListWidgetItem * qSlicerExtensionsManageWidgetPrivate::extensionItem(const QString& extensionName) const
{
  Q_Q(const qSlicerExtensionsManageWidget);

  QAbstractItemModel* model = q->model();
  const QModelIndexList indices =
    model->match(model->index(0, 0, QModelIndex()), Self::NameRole,
                 extensionName, 2, Qt::MatchExactly);

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
class qSlicerExtensionsDescriptionLabel : public QLabel
{
public:
  typedef QLabel Superclass;

  // --------------------------------------------------------------------------
  qSlicerExtensionsDescriptionLabel(const QString& extensionSlicerVersion, const QString& slicerRevision,
                                    const QString& extensionId, const QString& extensionName,
                                    const QString& extensionDescription, bool extensionEnabled,
                                    bool extensionCompatible)
    : QLabel(), ExtensionSlicerVersion(extensionSlicerVersion), SlicerRevision(slicerRevision),
      ExtensionIncompatible(!extensionCompatible), WarningColor("#bd8530"),
      ExtensionUpdateAvailable(false), InfoColor("#2c70c8"),
      ExtensionDisabled(!extensionEnabled), ExtensionId(extensionId),
      ExtensionName(extensionName), ExtensionDescription(extensionDescription),
      LastWidth(0), LastElidedDescription(extensionDescription)
  {
    QTextOption textOption = this->Text.defaultTextOption();
    textOption.setWrapMode(QTextOption::NoWrap);
    this->Text.setDefaultTextOption(textOption);

    this->prepareText(extensionDescription); // Ensure reasonable initial height for size hint
    this->setToolTip(extensionDescription);

    this->setMouseTracking(!extensionId.isEmpty());
  }

  // --------------------------------------------------------------------------
  void setExtensionDisabled(bool state)
  {
    if (this->ExtensionDisabled != state)
      {
      this->ExtensionDisabled = state;
      this->prepareText(this->LastElidedDescription);
      this->update();
      }
  }

  // --------------------------------------------------------------------------
  void setExtensionUpdateAvailable(bool state)
  {
    if (this->ExtensionUpdateAvailable != state)
      {
      this->ExtensionUpdateAvailable = state;
      this->prepareText(this->LastElidedDescription);
      this->update();
      }
  }

  // --------------------------------------------------------------------------
  QString incompatibleExtensionText()
  {
    return QString("<p style=\"font-weight: bold; font-size: 80%; color: %2;\">"
                   "<img style=\"float: left\" src=\":/Icons/ExtensionIncompatible.svg\"/> "
                   "Incompatible with Slicer r%3 [built for r%4]</p>").
        arg(this->WarningColor, this->SlicerRevision, this->ExtensionSlicerVersion);
  }

  // --------------------------------------------------------------------------
  QString extensionUpdateAvailableText()
  {
    static const char* const text =
      "<p style=\"font-weight: bold; font-size: 80%; color: %2;\">"
      "<img style=\"float: left\""
      " src=\":/Icons/ExtensionUpdateAvailable.svg\"/> "
      "An update is available</p>";
    return QString(text).arg(this->InfoColor);
  }

  // --------------------------------------------------------------------------
  QString descriptionAsRichText(const QString& elidedDescription)
  {
    static const QString format = "%1<h2>%2%3</h2><p>%4%5</p>";

    QString linkText;
    QString warningText;
    QString enabledText = (this->ExtensionDisabled ? " (disabled)" : "");
    if (this->ExtensionIncompatible)
      {
      warningText += this->incompatibleExtensionText();
      enabledText = " (disabled)";
      }
    if (this->ExtensionUpdateAvailable)
      {
      warningText += this->extensionUpdateAvailableText();
      }
    if (!this->ExtensionId.isEmpty())
      {
      linkText = QString(" <a href=\"slicer:%1\">More</a>").arg(this->ExtensionId);
      }
    return format.arg(warningText, this->ExtensionName, enabledText, elidedDescription, linkText);
  }

  // --------------------------------------------------------------------------
  void prepareText(const QString& elidedDescription)
  {
    this->LastElidedDescription = elidedDescription;
    this->Text.setHtml(this->descriptionAsRichText(elidedDescription));
  }

  // --------------------------------------------------------------------------
  QSize sizeHint() const override
  {
    QSize hint = this->Superclass::sizeHint();
    hint.setHeight(qRound(this->Text.size().height() + 0.5) +
                   this->margin() * 2);
    return hint;
  }

  // --------------------------------------------------------------------------
  void paintEvent(QPaintEvent *) override
  {
    QPainter painter(this);
    const QRect cr = this->contentsRect();

    if (this->LastWidth != cr.width())
      {
      int margin = this->margin() * 2;
      if (!this->ExtensionId.isEmpty())
        {
        margin += this->fontMetrics().width(" More");
        }
      this->prepareText(
        this->fontMetrics().elidedText(this->ExtensionDescription,
                                       Qt::ElideRight, cr.width() - margin));
      this->Text.setTextWidth(this->LastWidth = cr.width());
      }

    QAbstractTextDocumentLayout::PaintContext context;
    context.palette = this->palette();
    if (this->ExtensionIncompatible || this->ExtensionDisabled)
      {
      context.palette.setCurrentColorGroup(QPalette::Disabled);
      }

    painter.translate(cr.topLeft());
    this->Text.documentLayout()->draw(&painter, context);
  }

  // --------------------------------------------------------------------------
  QString linkUnderCursor(const QPoint& pos)
  {
    const int caretPos =
      this->Text.documentLayout()->hitTest(pos, Qt::FuzzyHit);
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
  void mouseMoveEvent(QMouseEvent * e) override
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
  void mouseReleaseEvent(QMouseEvent * e) override
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

  QString ExtensionSlicerVersion;
  QString SlicerRevision;

  bool ExtensionIncompatible;
  const QString WarningColor;

  bool ExtensionUpdateAvailable;
  const QString InfoColor;

  bool ExtensionDisabled;

  QString ExtensionId;
  QString ExtensionName;
  QString ExtensionDescription;

  QTextDocument Text;

  int LastWidth;
  QString LastElidedDescription;

  QString LinkUnderCursor;
};

// --------------------------------------------------------------------------
class qSlicerExtensionsItemWidget : public QWidget
{
public:
  qSlicerExtensionsItemWidget(qSlicerExtensionsDescriptionLabel * label, QWidget* parent = nullptr)
    : QWidget(parent), Label(label)
  {
    QHBoxLayout * layout = new QHBoxLayout;
    layout->addWidget(label, 1);
    layout->addWidget(this->ButtonBox = new qSlicerExtensionsButtonBox);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);

    label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  }

  qSlicerExtensionsDescriptionLabel * Label;
  qSlicerExtensionsButtonBox * ButtonBox;
};

} // end of anonymous namespace

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidgetPrivate::addExtensionItem(const ExtensionMetadataType& metadata)
{
  Q_Q(qSlicerExtensionsManageWidget);

  QString extensionId = metadata.value("extension_id").toString();
  QString extensionName = metadata.value("extensionname").toString();
  if (extensionName.isEmpty())
    {
    qCritical() << "Missing metadata identified with 'extensionname' key";
    return;
    }
  Q_ASSERT(this->extensionItem(extensionName) == nullptr);
  QString description = metadata.value("description").toString();
  QString extensionSlicerRevision = metadata.value("slicer_revision").toString();
  bool enabled = QVariant::fromValue(metadata.value("enabled")).toBool();

  QListWidgetItem * item = new QListWidgetItem();

  item->setIcon(this->extensionIcon(extensionName,
                                    metadata.value("iconurl").toUrl()));

  item->setData(Self::NameRole, extensionName); // See extensionItem(...)
  item->setData(Self::EnabledRole, enabled);

  q->addItem(item);

  bool isExtensionCompatible =
      q->extensionsManagerModel()->isExtensionCompatible(extensionName).isEmpty();

  qSlicerExtensionsDescriptionLabel * label = new qSlicerExtensionsDescriptionLabel(
        extensionSlicerRevision, q->extensionsManagerModel()->slicerRevision(),
        extensionId, extensionName, description, enabled, isExtensionCompatible);
  label->setMargin(6);
  QObject::connect(label, SIGNAL(linkActivated(QString)), q, SLOT(onLinkActivated(QString)));

  const bool isExtensionUpdateAvailable =
    q->extensionsManagerModel()->isExtensionUpdateAvailable(extensionName);
  label->setExtensionUpdateAvailable(isExtensionUpdateAvailable);

  qSlicerExtensionsItemWidget * widget = new qSlicerExtensionsItemWidget(label);
  q->setItemWidget(item, widget);

  this->EnableButtonMapper.setMapping(widget->ButtonBox->EnableButton, extensionName);
  QObject::connect(widget->ButtonBox->EnableButton, SIGNAL(clicked()), &this->EnableButtonMapper, SLOT(map()));
  widget->ButtonBox->EnableButton->setVisible(!enabled);
  widget->ButtonBox->EnableButton->setEnabled(isExtensionCompatible);

  this->DisableButtonMapper.setMapping(widget->ButtonBox->DisableButton, extensionName);
  QObject::connect(widget->ButtonBox->DisableButton, SIGNAL(clicked()), &this->DisableButtonMapper, SLOT(map()));
  widget->ButtonBox->DisableButton->setVisible(enabled);

  bool scheduledForUninstall = this->ExtensionsManagerModel->isExtensionScheduledForUninstall(extensionName);

  this->ScheduleUninstallButtonMapper.setMapping(widget->ButtonBox->ScheduleForUninstallButton, extensionName);
  QObject::connect(widget->ButtonBox->ScheduleForUninstallButton, SIGNAL(clicked()), &this->ScheduleUninstallButtonMapper, SLOT(map()));
  widget->ButtonBox->ScheduleForUninstallButton->setVisible(!scheduledForUninstall);

  this->CancelScheduledUninstallButtonMapper.setMapping(widget->ButtonBox->CancelScheduledForUninstallButton, extensionName);
  QObject::connect(widget->ButtonBox->CancelScheduledForUninstallButton, SIGNAL(clicked()), &this->CancelScheduledUninstallButtonMapper, SLOT(map()));
  widget->ButtonBox->CancelScheduledForUninstallButton->setVisible(scheduledForUninstall);

  const bool scheduledForUpdate =
    this->ExtensionsManagerModel->isExtensionScheduledForUpdate(extensionName);

  widget->ButtonBox->UpdateOptionsWidget->setVisible(isExtensionUpdateAvailable);
  widget->ButtonBox->UpdateProgress->setVisible(false);

  this->ScheduleUpdateButtonMapper.setMapping(
    widget->ButtonBox->ScheduleForUpdateButton, extensionName);
  QObject::connect(
    widget->ButtonBox->ScheduleForUpdateButton, SIGNAL(clicked()),
    &this->ScheduleUpdateButtonMapper, SLOT(map()));
  widget->ButtonBox->ScheduleForUpdateButton->setVisible(!scheduledForUpdate);

  this->CancelScheduledUpdateButtonMapper.setMapping(
    widget->ButtonBox->CancelScheduledForUpdateButton, extensionName);
  QObject::connect(
    widget->ButtonBox->CancelScheduledForUpdateButton, SIGNAL(clicked()),
    &this->CancelScheduledUpdateButtonMapper, SLOT(map()));
  widget->ButtonBox->CancelScheduledForUpdateButton->setVisible(scheduledForUpdate);

  QSize hint = label->sizeHint();
  hint.setWidth(hint.width() + 64);
  hint.setHeight(qMax(hint.height(), widget->ButtonBox->minimumSizeHint().height()));
  item->setSizeHint(hint);
}

// --------------------------------------------------------------------------
qSlicerExtensionsManageWidget::qSlicerExtensionsManageWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsManageWidgetPrivate(*this))
{
  Q_D(qSlicerExtensionsManageWidget);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsManageWidget::~qSlicerExtensionsManageWidget() = default;

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerExtensionsManageWidget::extensionsManagerModel()const
{
  Q_D(const qSlicerExtensionsManageWidget);
  return d->ExtensionsManagerModel;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::setExtensionsManagerModel(qSlicerExtensionsManagerModel* model)
{
  Q_D(qSlicerExtensionsManageWidget);

  if (this->extensionsManagerModel() == model)
    {
    return;
    }

  disconnect(this, SLOT(onModelUpdated()));
  disconnect(this, SLOT(onExtensionInstalled(QString)));
  disconnect(this, SLOT(onExtensionScheduledForUninstall(QString)));
  disconnect(this, SLOT(onExtensionCancelledScheduleForUninstall(QString)));
  disconnect(this, SLOT(onModelExtensionEnabledChanged(QString,bool)));

  d->ExtensionsManagerModel = model;

  if (d->ExtensionsManagerModel)
    {
    this->onModelUpdated();
    connect(d->ExtensionsManagerModel, SIGNAL(modelUpdated()),
            this, SLOT(onModelUpdated()));
    connect(d->ExtensionsManagerModel, SIGNAL(extensionInstalled(QString)),
            this, SLOT(onExtensionInstalled(QString)));
    connect(d->ExtensionsManagerModel, SIGNAL(extensionScheduledForUninstall(QString)),
            this, SLOT(onExtensionScheduledForUninstall(QString)));
    connect(d->ExtensionsManagerModel, SIGNAL(extensionCancelledScheduleForUninstall(QString)),
            this, SLOT(onExtensionCancelledScheduleForUninstall(QString)));
    connect(d->ExtensionsManagerModel, SIGNAL(extensionEnabledChanged(QString,bool)),
            this, SLOT(onModelExtensionEnabledChanged(QString,bool)));
    connect(d->ExtensionsManagerModel,
            SIGNAL(extensionUpdateAvailable(QString)),
            this, SLOT(setExtensionUpdateAvailable(QString)));
    connect(d->ExtensionsManagerModel,
            SIGNAL(extensionScheduledForUpdate(QString)),
            this, SLOT(setExtensionUpdateScheduled(QString)));
    connect(d->ExtensionsManagerModel,
            SIGNAL(extensionCancelledScheduleForUpdate(QString)),
            this, SLOT(setExtensionUpdateCanceled(QString)));
    connect(d->ExtensionsManagerModel,
            SIGNAL(updateDownloadProgress(QString,qint64,qint64)),
            this,
            SLOT(setExtensionUpdateDownloadProgress(QString,qint64,qint64)));
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::displayExtensionDetails(const QString& extensionName)
{
  Q_UNUSED(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::setExtensionEnabled(const QString& extensionName)
{
  if (!this->extensionsManagerModel())
    {
    return;
    }
  this->extensionsManagerModel()->setExtensionEnabled(extensionName, true);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::setExtensionDisabled(const QString& extensionName)
{
  if (!this->extensionsManagerModel())
    {
    return;
    }
  this->extensionsManagerModel()->setExtensionEnabled(extensionName, false);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::scheduleExtensionForUninstall(const QString& extensionName)
{
  if (!this->extensionsManagerModel())
    {
    return;
    }
  this->extensionsManagerModel()->scheduleExtensionForUninstall(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::cancelExtensionScheduledForUninstall(const QString& extensionName)
{
  if (!this->extensionsManagerModel())
    {
    return;
    }
  this->extensionsManagerModel()->cancelExtensionScheduledForUninstall(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::onExtensionInstalled(const QString& extensionName)
{
  Q_D(qSlicerExtensionsManageWidget);
  d->addExtensionItem(d->ExtensionsManagerModel->extensionMetadata(extensionName));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::onExtensionScheduledForUninstall(const QString& extensionName)
{
  Q_D(qSlicerExtensionsManageWidget);
  QListWidgetItem * item = d->extensionItem(extensionName);
  Q_ASSERT(item);
  qSlicerExtensionsItemWidget * widget =
      dynamic_cast<qSlicerExtensionsItemWidget*>(this->itemWidget(item));
  Q_ASSERT(widget);
  widget->ButtonBox->ScheduleForUpdateButton->setEnabled(false);
  widget->ButtonBox->CancelScheduledForUninstallButton->setVisible(true);
  widget->ButtonBox->ScheduleForUninstallButton->setVisible(false);
}

// -------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::onExtensionCancelledScheduleForUninstall(const QString& extensionName)
{
  Q_D(qSlicerExtensionsManageWidget);
  QListWidgetItem * item = d->extensionItem(extensionName);
  Q_ASSERT(item);
  qSlicerExtensionsItemWidget * widget =
      dynamic_cast<qSlicerExtensionsItemWidget*>(this->itemWidget(item));
  Q_ASSERT(widget);
  widget->ButtonBox->ScheduleForUpdateButton->setEnabled(true);
  widget->ButtonBox->CancelScheduledForUninstallButton->setVisible(false);
  widget->ButtonBox->ScheduleForUninstallButton->setVisible(true);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::setExtensionUpdateAvailable(
  const QString& extensionName)
{
  Q_D(qSlicerExtensionsManageWidget);
  QListWidgetItem* const item = d->extensionItem(extensionName);
  Q_ASSERT(item);
  qSlicerExtensionsItemWidget* const widget =
    dynamic_cast<qSlicerExtensionsItemWidget*>(this->itemWidget(item));
  Q_ASSERT(widget);
  widget->Label->setExtensionUpdateAvailable(true);

  widget->ButtonBox->UpdateOptionsWidget->setVisible(true);
  if (!widget->ButtonBox->UpdateProgress->isVisible())
    {
    const bool scheduled =
      d->ExtensionsManagerModel &&
      d->ExtensionsManagerModel->isExtensionScheduledForUpdate(extensionName);

    widget->ButtonBox->ScheduleForUpdateButton->setVisible(!scheduled);
    widget->ButtonBox->CancelScheduledForUpdateButton->setVisible(scheduled);
    }

  QSize hint = widget->Label->sizeHint();
  hint.setWidth(hint.width() + 64);
  hint.setHeight(qMax(hint.height(), widget->sizeHint().height()));
  item->setSizeHint(hint);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::scheduleExtensionForUpdate(
  const QString& extensionName)
{
  qSlicerExtensionsManagerModel* const model =
    this->extensionsManagerModel();
  if (!model)
    {
    return;
    }
  model->scheduleExtensionForUpdate(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::cancelExtensionScheduledForUpdate(
  const QString& extensionName)
{
  qSlicerExtensionsManagerModel* const model =
    this->extensionsManagerModel();
  if (!model)
    {
    return;
    }
  model->cancelExtensionScheduledForUpdate(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::setExtensionUpdateScheduled(
  const QString& extensionName)
{
  Q_D(qSlicerExtensionsManageWidget);
  QListWidgetItem* const item = d->extensionItem(extensionName);
  Q_ASSERT(item);
  qSlicerExtensionsItemWidget* const widget =
    dynamic_cast<qSlicerExtensionsItemWidget*>(this->itemWidget(item));
  Q_ASSERT(widget);
  widget->ButtonBox->ScheduleForUpdateButton->setVisible(false);
  widget->ButtonBox->CancelScheduledForUpdateButton->setVisible(true);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::setExtensionUpdateCanceled(
  const QString& extensionName)
{
  Q_D(qSlicerExtensionsManageWidget);
  QListWidgetItem* const item = d->extensionItem(extensionName);
  Q_ASSERT(item);
  qSlicerExtensionsItemWidget* const widget =
    dynamic_cast<qSlicerExtensionsItemWidget*>(this->itemWidget(item));
  Q_ASSERT(widget);
  widget->ButtonBox->ScheduleForUpdateButton->setVisible(true);
  widget->ButtonBox->CancelScheduledForUpdateButton->setVisible(false);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::setExtensionUpdateDownloadProgress(
  const QString& extensionName, qint64 received, qint64 total)
{
  Q_D(qSlicerExtensionsManageWidget);
  QListWidgetItem* const item = d->extensionItem(extensionName);
  Q_ASSERT(item);
  qSlicerExtensionsItemWidget* const widget =
    dynamic_cast<qSlicerExtensionsItemWidget*>(this->itemWidget(item));
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
    const bool scheduled =
      d->ExtensionsManagerModel &&
      d->ExtensionsManagerModel->isExtensionScheduledForUpdate(extensionName);

    widget->ButtonBox->UpdateProgress->setVisible(false);
    widget->ButtonBox->ScheduleForUpdateButton->setVisible(!scheduled);
    widget->ButtonBox->CancelScheduledForUpdateButton->setVisible(scheduled);
    }
  else
    {
    widget->ButtonBox->UpdateProgress->setVisible(true);
    widget->ButtonBox->ScheduleForUpdateButton->setVisible(false);
    widget->ButtonBox->CancelScheduledForUpdateButton->setVisible(false);
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::onModelExtensionEnabledChanged(const QString &extensionName, bool enabled)
{
  Q_D(qSlicerExtensionsManageWidget);
  QListWidgetItem * item = d->extensionItem(extensionName);
  Q_ASSERT(item);
  item->setData(qSlicerExtensionsManageWidgetPrivate::EnabledRole, enabled);
  qSlicerExtensionsItemWidget * widget =
      dynamic_cast<qSlicerExtensionsItemWidget*>(this->itemWidget(item));
  Q_ASSERT(widget);
  widget->Label->setExtensionDisabled(!enabled);
  widget->ButtonBox->EnableButton->setVisible(!enabled);
  widget->ButtonBox->DisableButton->setVisible(enabled);

}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::onModelUpdated()
{
  Q_D(qSlicerExtensionsManageWidget);
  this->clear();
  foreach(const QString& extensionName, d->ExtensionsManagerModel->installedExtensions())
    {
    d->addExtensionItem(d->ExtensionsManagerModel->extensionMetadata(extensionName));
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::onLinkActivated(const QString& link)
{
  Q_D(qSlicerExtensionsManageWidget);

  QUrl url = d->ExtensionsManagerModel->serverUrl();
  url.setPath(url.path() + "/slicerappstore/extension/view");
  QUrlQuery urlQuery;
  urlQuery.addQueryItem("extensionId", link.mid(7)); // remove leading "slicer:"
  urlQuery.addQueryItem("breadcrumbs", "none");
  urlQuery.addQueryItem("layout", "empty");
  url.setQuery(urlQuery);

  emit this->linkActivated(url);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::onIconDownloadComplete(
  const QString& extensionName)
{
  Q_D(qSlicerExtensionsManageWidget);

  QNetworkReply* const reply = d->Downloads.take(extensionName);
  Q_ASSERT(reply);

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
