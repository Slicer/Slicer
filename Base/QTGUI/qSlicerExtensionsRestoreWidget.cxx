//QT includes
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
  qSlicerRestoreExtensionsItemDelegate(QObject * parent = 0)
    : QStyledItemDelegate(parent) {};

  bool editorEvent(QEvent *event,
    QAbstractItemModel *model,
    const QStyleOptionViewItem &option,
    const QModelIndex &index)
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
  void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
  {
    QRect r = option.rect;

    QPen enabledPen(QColor::fromRgb(0, 0, 0), 1, Qt::SolidLine);
    QPen disabledPen(QColor::fromRgb(125, 125, 125), 1, Qt::SolidLine);
    QPen candidatePen(QColor::fromRgb(0, 200, 50), 1, Qt::SolidLine);
    QPen installedPen(QColor::fromRgb(0, 50, 200), 1, Qt::SolidLine);

    //GET DATA
    const QString& title            = index.data(Qt::DisplayRole).toString();
    const bool isChecked            = index.data(qSlicerRestoreExtensions::CheckedRole).toBool();
    const QString& description      = index.data(qSlicerRestoreExtensions::DescriptionRole).toString();
    const bool isEnabled            = index.data(qSlicerRestoreExtensions::EnabledRole).toBool();
    const bool isRestoreCandidate   = index.data(qSlicerRestoreExtensions::RestoreCandidateRole).toBool();
    const bool isInstalled          = index.data(qSlicerRestoreExtensions::InstalledRole).toBool();

    //TITLE
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
  QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
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
  QPushButton *installButton = new QPushButton;
  this->checkOnStartup = new QCheckBox;
  this->silentInstallOnStartup = new QCheckBox;
  this->progressDialog = new QProgressDialog;
  this->extensionList = new QListWidget;
  this->progressBar = new QProgressBar;

  this->extensionList->setAlternatingRowColors(true);
  this->extensionList->setItemDelegate(new qSlicerRestoreExtensionsItemDelegate(q));
  this->checkOnStartup->setText(QObject::tr("Check previous extensions on startup"));
  this->silentInstallOnStartup->setText(QObject::tr("Install previous extensions without request"));

  // See https://issues.slicer.org/view.php?id=4641
  this->checkOnStartup->setVisible(false);
  this->silentInstallOnStartup->setVisible(false);

  this->progressDialog->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
  this->progressDialog->close();

  installButton->setText(QObject::tr("Install Selected"));
  layoutForProgressAndButton->addWidget(this->progressBar);
  layoutForProgressAndButton->addWidget(installButton);
  layoutForSettings->addWidget(this->checkOnStartup);
  layoutForSettings->addWidget(this->silentInstallOnStartup);
  mainLayout->addWidget(this->extensionList);
  mainLayout->addLayout(layoutForProgressAndButton);
  mainLayout->addLayout(layoutForSettings);
  q->setLayout(mainLayout);
  checkOnStartupSettingsKey = "ExtensionCheckOnStartup/dontCheck";
  silentInstallOnStartUpSettingsKey = "ExtensionCheckOnStartup/ifCheckInstallWithoutDialog";

  QObject::connect(installButton, SIGNAL(clicked()),
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
    const QVariantMap& currentInfo = extensionHistoryInformation.value(extensionName).toMap();
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
    const QStringList& candidateIds = extractInstallationCandidates(extensionHistoryInformation);

    if (candidateIds.length() > 0)
      {
      bool silentInstall = settings.value(this->silentInstallOnStartUpSettingsKey).toBool();
      if (silentInstall)
        {
        this->startDownloadAndInstallExtensionsHeadless(candidateIds);
        }
      else
        {
        const QString& text = QObject::tr(
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

    const QString& title                  = extensionName;
    const bool isCompatible               = currentInfo.value("IsCompatible").toBool();
    const bool isInstalled                = currentInfo.value("IsInstalled").toBool();
    const QString& usedLastInRevision     = currentInfo.value("UsedLastInRevision").toString();
    const bool wasInstalledInLastRevision = currentInfo.value("WasInstalledInLastRevision").toBool();
    const bool isItemEnabled              = isCompatible && !isInstalled;
    const bool isItemChecked              = isItemEnabled && wasInstalledInLastRevision;
    const QString& description =
      (isInstalled
       ? QObject::tr("currently installed")
       : (isCompatible
          ? (wasInstalledInLastRevision
             ? QObject::tr("was used in previously installed Slicer version (%1)").arg(usedLastInRevision)
             : QObject::tr("was last used in Slicer version %1").arg(usedLastInRevision))
          : QObject::tr("not compatible with current Slicer version (was last used in Slicer version %1)").arg(usedLastInRevision)));

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
  if (this->currentExtensionToInstall < this->nrOfExtensionsToInstall)
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
        QObject::tr("All extensions restored. Please restart Slicer."));
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
    this->progressDialog->setLabelText(
      QObject::tr("Installing %1 (%2/%3)")
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
::~qSlicerExtensionsRestoreWidget()
{
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerExtensionsRestoreWidget
::extensionsManagerModel()const
{
  Q_D(const qSlicerExtensionsRestoreWidget);
  return d->ExtensionsManagerModel;
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
