// Qt includes
#include <QAction>
#include <QPushButton>
#include <QProgressBar>
#include <QLayout>
#include <iostream>
#include <QFileInfo>
#include <QDir>
#include <QListWidget>
#include <QDebug>
#include <QProgressDialog>
#include <ctkMessageBox.h>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QEvent>
#include <QApplication>
#include <qSlicerApplication.h>
#include <QCheckBox>
#include <QToolButton>

// QtGUI includes
#include "qSlicerExtensionsRestoreWidget.h"
#include "qSlicerExtensionsManagerModel.h"

// --------------------------------------------------------------------------
namespace qSlicerRestoreExtensions
{
enum ItemDataRole {
  IdRole = Qt::UserRole,
  CheckedRole,
  DescriptionRole,
  EnabledRole,
  RestoreCandidateRole,
  InstalledRole
  };
};

// --------------------------------------------------------------------------
class qSlicerRestoreExtensionsItemDelegate : public QStyledItemDelegate
{
public:
  qSlicerRestoreExtensionsItemDelegate(QObject * parent = nullptr)
    : QStyledItemDelegate(parent) {};

  bool editorEvent(QEvent *event,
    QAbstractItemModel *model,
    const QStyleOptionViewItem &option,
    const QModelIndex &index) override
  {
    bool isEnabled = index.data(qSlicerRestoreExtensions::EnabledRole).toBool();

    if (event->type() == QEvent::MouseButtonRelease && isEnabled)
      {
      bool value = index.data(qSlicerRestoreExtensions::CheckedRole).toBool();
      model->setData(index, !value, qSlicerRestoreExtensions::CheckedRole);
      return true;
      }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
  }

  // --------------------------------------------------------------------------
  void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override
  {
    QRect r = option.rect;

    QPalette palette = qSlicerApplication::application()->palette();
    QPen enabledPen(palette.color(QPalette::WindowText), 1, Qt::SolidLine);    
    QPen disabledPen(palette.color(QPalette::Disabled, QPalette::WindowText), 1, Qt::SolidLine);
    QPen candidatePen(palette.color(QPalette::Highlight), 1, Qt::SolidLine);
    QPen installedPen(palette.color(QPalette::WindowText), 1, Qt::SolidLine);

    // GET DATA
    QString title             = index.data(Qt::DisplayRole).toString();
    bool isChecked            = index.data(qSlicerRestoreExtensions::CheckedRole).toBool();
    QString description       = index.data(qSlicerRestoreExtensions::DescriptionRole).toString();
    bool isEnabled            = index.data(qSlicerRestoreExtensions::EnabledRole).toBool();
    bool isRestoreCandidate   = index.data(qSlicerRestoreExtensions::RestoreCandidateRole).toBool();
    bool isInstalled          = index.data(qSlicerRestoreExtensions::InstalledRole).toBool();

    // TITLE
    painter->setPen((isEnabled ? enabledPen : disabledPen));
    r = option.rect.adjusted(55, 10, 0, 0);
    painter->setFont(QFont("Arial", 13, QFont::Bold));
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignTop | Qt::AlignLeft, title, &r);
    //DESCRIPTION
    painter->setPen((isEnabled ? ( isRestoreCandidate ? candidatePen : enabledPen ) : ( isInstalled ? installedPen : disabledPen)));
    r = option.rect.adjusted(55, 30, -10, 0);
    painter->setFont(QFont("Arial", 9, QFont::Normal));
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignLeft, description, &r);
    //CHECKBOX
    QStyleOptionButton cbOpt;
    cbOpt.rect = option.rect.adjusted(20, 0, 0, 0);

    if (isChecked)
      {
      cbOpt.state |= QStyle::State_On;
      }
    else
      {
      cbOpt.state |= QStyle::State_Off;
      }

    QApplication::style()->drawControl(QStyle::CE_CheckBox, &cbOpt, painter);
  }
  QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const override
  {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(200, 60);
  }

};


//-----------------------------------------------------------------------------
class qSlicerExtensionsRestoreWidgetPrivate
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsRestoreWidget);
protected:
  qSlicerExtensionsRestoreWidget* const q_ptr;

public:
  qSlicerExtensionsRestoreWidgetPrivate(qSlicerExtensionsRestoreWidget& object);
  void init();
  void onShow();
  void setupUi();
  void setupList();
  QStringList getSelectedExtensions();
  void startDownloadAndInstallExtensions(QStringList extensionIds);
  void startDownloadAndInstallExtensionsHeadless(QStringList extensionIds);

  void downloadAndInstallNextExtension();
  void downloadProgress(const QString& extensionName, qint64 received, qint64 total);
  QStringList extractInstallationCandidates(QVariantMap extensionHistoryInformation);
  void processExtensionsHistoryInformationOnStartup(QVariantMap extensionHistoryInformation);
  void setCheckOnStartup(int state);
  void setSilentInstallOnStartup(int state);

  qSlicerExtensionsManagerModel *ExtensionsManagerModel;
  QProgressBar *progressBar;
  QProgressDialog *progressDialog;
  QToolButton* selectAllButton;
  QAction* selectAllAction;
  QToolButton* deselectAllButton;
  QAction* deselectAllAction;
  QToolButton* installSelectedButton;
  QAction* installSelectedAction;
  QCheckBox *checkOnStartup;
  QCheckBox *silentInstallOnStartup;
  QListWidget *extensionList;
  QStringList extensionsToInstall;
  QVariantMap extensionRestoreInformation;
  QString checkOnStartupSettingsKey;
  QString silentInstallOnStartUpSettingsKey;
  int nrOfExtensionsToInstall;
  int currentExtensionToInstall;
  bool headlessMode;
};

// --------------------------------------------------------------------------
qSlicerExtensionsRestoreWidgetPrivate::qSlicerExtensionsRestoreWidgetPrivate(qSlicerExtensionsRestoreWidget& object)
:q_ptr(&object)
{

}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidgetPrivate::init()
{
  this->nrOfExtensionsToInstall = 0;
  this->currentExtensionToInstall = -1;
  this->headlessMode = false;
  setupUi();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidgetPrivate::onShow()
{
  QSettings settings; // (this->ExtensionsManagerModel->extensionsSettingsFilePath(), QSettings::IniFormat);
  checkOnStartup->setChecked(!settings.value(checkOnStartupSettingsKey).toBool());
  silentInstallOnStartup->setChecked(settings.value(silentInstallOnStartUpSettingsKey).toBool());
  silentInstallOnStartup->setEnabled(checkOnStartup->isChecked());
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidgetPrivate
::setupUi()
{
  Q_Q(qSlicerExtensionsRestoreWidget);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  QHBoxLayout *layoutForProgressAndButton = new QHBoxLayout;
  QHBoxLayout *layoutForSettings= new QHBoxLayout;
  this->checkOnStartup = new QCheckBox;
  this->silentInstallOnStartup = new QCheckBox;
  this->progressDialog = new QProgressDialog;
  this->extensionList = new QListWidget;
  this->progressBar = new QProgressBar;

  this->selectAllAction = new QAction(q);
  this->selectAllAction->setText(qSlicerExtensionsRestoreWidget::tr("Select All"));

  this->selectAllButton = new QToolButton(q);
  this->selectAllButton->setDefaultAction(this->selectAllAction);

  this->deselectAllAction = new QAction(q);
  this->deselectAllAction->setText(qSlicerExtensionsRestoreWidget::tr("Deselect All"));

  this->deselectAllButton = new QToolButton(q);
  this->deselectAllButton->setDefaultAction(this->deselectAllAction);

  this->installSelectedAction = new QAction(q);
  this->installSelectedAction->setText(qSlicerExtensionsRestoreWidget::tr("&Install Selected..."));

  this->installSelectedButton = new QToolButton(q);
  this->installSelectedButton->setDefaultAction(this->installSelectedAction);

  this->extensionList->setAlternatingRowColors(true);
  this->extensionList->setItemDelegate(new qSlicerRestoreExtensionsItemDelegate(q));
  this->checkOnStartup->setText(qSlicerExtensionsRestoreWidget::tr("Check previous extensions on startup"));
  this->silentInstallOnStartup->setText(qSlicerExtensionsRestoreWidget::tr("Install previous extensions without request"));

  // See https://issues.slicer.org/view.php?id=4641
  this->checkOnStartup->setVisible(false);
  this->silentInstallOnStartup->setVisible(false);

  this->progressDialog->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
  this->progressDialog->close();

  layoutForProgressAndButton->addWidget(this->progressBar);
  layoutForProgressAndButton->addWidget(this->selectAllButton);
  layoutForProgressAndButton->addWidget(this->deselectAllButton);
  layoutForProgressAndButton->addWidget(this->installSelectedButton);
  layoutForSettings->addWidget(this->checkOnStartup);
  layoutForSettings->addWidget(this->silentInstallOnStartup);
  mainLayout->addWidget(this->extensionList);
  mainLayout->addLayout(layoutForProgressAndButton);
  mainLayout->addLayout(layoutForSettings);
  q->setLayout(mainLayout);
  checkOnStartupSettingsKey = "ExtensionCheckOnStartup/dontCheck";
  silentInstallOnStartUpSettingsKey = "ExtensionCheckOnStartup/ifCheckInstallWithoutDialog";

  QObject::connect(this->selectAllAction, SIGNAL(triggered(bool)),
      q, SLOT(onSelectAllExtensionsTriggered()));
  QObject::connect(this->deselectAllAction, SIGNAL(triggered(bool)),
      q, SLOT(onDeselectAllExtensionsTriggered()));
  QObject::connect(this->installSelectedAction, SIGNAL(triggered(bool)),
      q, SLOT(onInstallSelectedExtensionsTriggered()));

  QObject::connect(checkOnStartup, SIGNAL(stateChanged(int)),
    q, SLOT(onCheckOnStartupChanged(int)));
  QObject::connect(silentInstallOnStartup, SIGNAL(stateChanged(int)),
    q, SLOT(onSilentInstallOnStartupChanged(int)));
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsRestoreWidgetPrivate
::extractInstallationCandidates(QVariantMap extensionHistoryInformation)
{
  QStringList candidateIds;
  foreach(QString extensionName, extensionHistoryInformation.keys())
    {
    QVariantMap currentInfo = extensionHistoryInformation.value(extensionName).toMap();
    if (currentInfo.value("WasInstalledInLastRevision").toBool() && currentInfo.value("IsCompatible").toBool() && !currentInfo.value("IsInstalled").toBool())
      {
      candidateIds.append(currentInfo.value("ExtensionId").toString());
      }
    }
  return candidateIds;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidgetPrivate
::processExtensionsHistoryInformationOnStartup(QVariantMap extensionHistoryInformation)
{
  QSettings settings;// (this->ExtensionsManagerModel->extensionsSettingsFilePath(), QSettings::IniFormat);

  bool checkOnStartup = !settings.value(this->checkOnStartupSettingsKey).toBool();

  if (checkOnStartup)
    {
    QStringList candidateIds = extractInstallationCandidates(extensionHistoryInformation);

    if (candidateIds.length() > 0)
      {
      bool silentInstall = settings.value(this->silentInstallOnStartUpSettingsKey).toBool();
      if (silentInstall)
        {
        this->startDownloadAndInstallExtensionsHeadless(candidateIds);
        }
      else
        {
        QString text = qSlicerExtensionsRestoreWidget::tr(
          "%1 compatible extension(s) from a previous Slicer installation found. Do you want to install? "
          "(For details see: Extension Manager > Restore Extensions)").arg(candidateIds.length());

        ctkMessageBox checkHistoryMessage;
        checkHistoryMessage.setText(text);
        checkHistoryMessage.setIcon(QMessageBox::Information);
        checkHistoryMessage.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        checkHistoryMessage.setDontShowAgainVisible(true);

        if (checkHistoryMessage.exec() == QMessageBox::Yes)
          {
          this->startDownloadAndInstallExtensionsHeadless(candidateIds);
          }
        settings.setValue(checkOnStartupSettingsKey, checkHistoryMessage.dontShowAgain());
        }
      }
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidgetPrivate
::setupList()
{
  Q_Q(qSlicerExtensionsRestoreWidget);

  extensionList->clear();
  QVariantMap extensionInfo = q->extensionsManagerModel()->getExtensionHistoryInformation();

  foreach(QString extensionName, extensionInfo.keys())
    {
    QListWidgetItem* extensionItem = new QListWidgetItem;

    QVariantMap currentInfo = extensionInfo.value(extensionName).toMap();

    QString title                   = extensionName;
    bool isCompatible               = currentInfo.value("IsCompatible").toBool();
    bool isInstalled                = currentInfo.value("IsInstalled").toBool();
    QString usedLastInRevision      = currentInfo.value("UsedLastInRevision").toString();
    bool wasInstalledInLastRevision = currentInfo.value("WasInstalledInLastRevision").toBool();
    bool isItemEnabled              = isCompatible && !isInstalled;
    bool isItemChecked              = isItemEnabled && wasInstalledInLastRevision;
    QString description;

    if (isInstalled)
      {
      description = qSlicerExtensionsRestoreWidget::tr("currently installed");
      }
    else if (isCompatible)
      {
      if (wasInstalledInLastRevision)
        {
        description = qSlicerExtensionsRestoreWidget::tr("was used in previously installed Slicer version (%1)").arg(usedLastInRevision);
        }
      else
        {
        description = qSlicerExtensionsRestoreWidget::tr("was last used in Slicer version %1").arg(usedLastInRevision);
        }
      }
    else
      {
      description = qSlicerExtensionsRestoreWidget::tr(
            "not compatible with current Slicer version (was last used in Slicer version %1)").arg(usedLastInRevision);
      }

    extensionItem->setData(qSlicerRestoreExtensions::IdRole, currentInfo.value("ExtensionId").toString());
    extensionItem->setData(qSlicerRestoreExtensions::CheckedRole, isItemChecked);
    extensionItem->setData(qSlicerRestoreExtensions::DescriptionRole, description);
    extensionItem->setData(qSlicerRestoreExtensions::EnabledRole, isItemEnabled);
    extensionItem->setData(qSlicerRestoreExtensions::RestoreCandidateRole, wasInstalledInLastRevision); //details added for color coding
    extensionItem->setData(qSlicerRestoreExtensions::InstalledRole, isInstalled);
    extensionItem->setText(title);

    extensionList->addItem(extensionItem);
    }
}

// --------------------------------------------------------------------------
QStringList qSlicerExtensionsRestoreWidgetPrivate
::getSelectedExtensions()
{
  QStringList selectedExtensions;
  for (int i = 0; i < extensionList->count(); i++)
    {
    QListWidgetItem* currentItem = extensionList->item(i);
    if (currentItem->data(qSlicerRestoreExtensions::CheckedRole).toBool())
      {
      selectedExtensions.append(currentItem->data(qSlicerRestoreExtensions::IdRole).toString());
      }
    }
  return selectedExtensions;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidgetPrivate
::startDownloadAndInstallExtensionsHeadless(QStringList extensionIds)
{
  this->headlessMode = true;
  this->progressDialog->show();
  startDownloadAndInstallExtensions(extensionIds);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidgetPrivate
::startDownloadAndInstallExtensions(QStringList extensionIds)
{
  this->extensionsToInstall = extensionIds;
  this->nrOfExtensionsToInstall = extensionsToInstall.size();
  this->currentExtensionToInstall = -1;

  this->progressBar->setMaximum(this->nrOfExtensionsToInstall);
  this->progressDialog->setMaximum(this->nrOfExtensionsToInstall);

  downloadAndInstallNextExtension();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidgetPrivate
::downloadAndInstallNextExtension()
{
  Q_Q(qSlicerExtensionsRestoreWidget);
  this->currentExtensionToInstall++;
  if (this->currentExtensionToInstall >=0 && this->currentExtensionToInstall < this->nrOfExtensionsToInstall)
    {
    q->extensionsManagerModel()->downloadAndInstallExtension(extensionsToInstall.at(currentExtensionToInstall));
    }
  else
    {
    if (this->headlessMode)
      {
      this->progressDialog->close();
      this->headlessMode = false;
      static_cast<qSlicerApplication*>qApp->confirmRestart(
            qSlicerExtensionsRestoreWidget::tr("All extensions restored. Please restart Slicer."));
      }
    else
      {
      setupList();
      }
    }
}
// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidgetPrivate
::downloadProgress(const QString& extensionName, qint64 received, qint64 total)
{
  int value = float(currentExtensionToInstall) + (float(received) / float(total));
  if (this->headlessMode)
    {
    this->progressDialog->setValue(value);
    this->progressDialog->setLabelText(qSlicerExtensionsRestoreWidget::tr("Installing %1 (%2/%3)")
      .arg(extensionName)
      .arg(received)
      .arg(total));
    }
  else
    {
    this->progressBar->setValue(value);
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidgetPrivate
::setCheckOnStartup(int state)
{
  QSettings settings;// (this->ExtensionsManagerModel->extensionsSettingsFilePath(), QSettings::IniFormat);
  settings.setValue(checkOnStartupSettingsKey, !bool(state));
  this->silentInstallOnStartup->setEnabled(bool(state));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidgetPrivate
::setSilentInstallOnStartup(int state)
{
  QSettings settings;// (this->ExtensionsManagerModel->extensionsSettingsFilePath(), QSettings::IniFormat);
  settings.setValue(silentInstallOnStartUpSettingsKey, bool(state));
}

// --------------------------------------------------------------------------
// qSlicerExtensionsRestoreWidget methods

// --------------------------------------------------------------------------
qSlicerExtensionsRestoreWidget
::qSlicerExtensionsRestoreWidget(QWidget* _parent)
: Superclass(_parent)
, d_ptr(new qSlicerExtensionsRestoreWidgetPrivate(*this))
{
  Q_D(qSlicerExtensionsRestoreWidget);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsRestoreWidget
::~qSlicerExtensionsRestoreWidget() = default;

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerExtensionsRestoreWidget
::extensionsManagerModel()const
{
  Q_D(const qSlicerExtensionsRestoreWidget);
  return d->ExtensionsManagerModel;
}

//---------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidget::changeEvent(QEvent* e)
{
  Q_D(qSlicerExtensionsRestoreWidget);
  switch (e->type())
    {
    case QEvent::PaletteChange:
      {
      this->repaint();
      break;
      }
    default:
      break;
    }
  Superclass::changeEvent(e);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidget
::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  Q_D(qSlicerExtensionsRestoreWidget);
  d->onShow();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidget
::setExtensionsManagerModel(qSlicerExtensionsManagerModel* model)
{
  Q_D(qSlicerExtensionsRestoreWidget);

  if (this->extensionsManagerModel() == model)
    {
    return;
    }

  disconnect(this, SLOT(onProgressChanged(QString, qint64, qint64)));
  disconnect(this, SLOT(onInstallationFinished(QString)));
  disconnect(this, SLOT(onExtensionHistoryGatheredOnStartup(QVariantMap)));
  d->ExtensionsManagerModel = model;
  d->setupList();

  if (model)
    {
    connect(model, SIGNAL(installDownloadProgress(QString, qint64, qint64)),
      this, SLOT(onProgressChanged(QString, qint64, qint64)));
    connect(model, SIGNAL(extensionInstalled(QString)),
      this, SLOT(onInstallationFinished(QString)));
    connect(model, SIGNAL(extensionHistoryGatheredOnStartup(QVariantMap)),
      this, SLOT(onExtensionHistoryGatheredOnStartup(QVariantMap)));
    }
}

// --------------------------------------------------------------------------
QAction* qSlicerExtensionsRestoreWidget::selectAllAction() const
{
  Q_D(const qSlicerExtensionsRestoreWidget);
  return d->selectAllAction;
}

// --------------------------------------------------------------------------
QAction* qSlicerExtensionsRestoreWidget::deselectAllAction() const
{
  Q_D(const qSlicerExtensionsRestoreWidget);
  return d->deselectAllAction;
}

// --------------------------------------------------------------------------
QAction* qSlicerExtensionsRestoreWidget::installSelectedAction() const
{
  Q_D(const qSlicerExtensionsRestoreWidget);
  return d->installSelectedAction;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidget::onSelectAllExtensionsTriggered()
{
  Q_D(qSlicerExtensionsRestoreWidget);
  for (int index = 0; index < d->extensionList->count(); index++)
    {
    if (!d->extensionList->item(index)->data(qSlicerRestoreExtensions::EnabledRole).toBool())
      {
      continue;
      }
    d->extensionList->item(index)->setData(qSlicerRestoreExtensions::CheckedRole, true);
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidget::onDeselectAllExtensionsTriggered()
{
  Q_D(qSlicerExtensionsRestoreWidget);
  for (int index = 0; index < d->extensionList->count(); index++)
    {
    if (!d->extensionList->item(index)->data(qSlicerRestoreExtensions::EnabledRole).toBool())
      {
      continue;
      }
    d->extensionList->item(index)->setData(qSlicerRestoreExtensions::CheckedRole, false);
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidget
::onInstallSelectedExtensionsTriggered()
{
  Q_D(qSlicerExtensionsRestoreWidget);
  d->startDownloadAndInstallExtensions(d->getSelectedExtensions());
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidget
::onCheckOnStartupChanged(int state)
{
  Q_D(qSlicerExtensionsRestoreWidget);
  d->setCheckOnStartup(state);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidget
::onSilentInstallOnStartupChanged(int state)
{
  Q_D(qSlicerExtensionsRestoreWidget);
  d->setSilentInstallOnStartup(state);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidget::onProgressChanged(const QString& extensionName, qint64 received, qint64 total)
{
  Q_D(qSlicerExtensionsRestoreWidget);
  d->downloadProgress(extensionName, received, total);
};

// --------------------------------------------------------------------------
void qSlicerExtensionsRestoreWidget
::onInstallationFinished(QString extensionName)
{
  Q_UNUSED(extensionName);
  Q_D(qSlicerExtensionsRestoreWidget);
  d->downloadAndInstallNextExtension();
}

void qSlicerExtensionsRestoreWidget
::onExtensionHistoryGatheredOnStartup(const QVariantMap& extensionInfo)
{
  Q_D(qSlicerExtensionsRestoreWidget);
  d->processExtensionsHistoryInformationOnStartup(extensionInfo);
}
