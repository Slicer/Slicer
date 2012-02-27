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
#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QSignalMapper>

// Slicer includes
#include "qSlicerExtensionsManagerModel.h"
#include "qSlicerExtensionsManageWidget.h"
#include "ui_qSlicerExtensionsButtonBox.h"
#include "ui_qSlicerExtensionsManageWidget.h"

//-----------------------------------------------------------------------------
class qSlicerExtensionsManageWidgetPrivate: public Ui_qSlicerExtensionsManageWidget
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsManageWidget);
protected:
  qSlicerExtensionsManageWidget* const q_ptr;

public:
  typedef qSlicerExtensionsManageWidgetPrivate Self;
  typedef qSlicerExtensionsManagerModel::ExtensionMetadataType ExtensionMetadataType;
  qSlicerExtensionsManageWidgetPrivate(qSlicerExtensionsManageWidget& object);
  void init();

  enum ColumnsIds
    {
    IconColumn = 0,
    NameColumn,
    TextColumn,
    ButtonsColumn,
    ColumnCount
    };

  QTreeWidgetItem * extensionItem(const QString &extensionName)const;

  void addExtensionItem(const ExtensionMetadataType &metadata);

  QSignalMapper LabelLinkMapper;
  QSignalMapper EnableButtonMapper;
  QSignalMapper DisableButtonMapper;
  QSignalMapper UninstallButtonMapper;

  qSlicerExtensionsManagerModel * ExtensionsManagerModel;
};

// --------------------------------------------------------------------------
qSlicerExtensionsManageWidgetPrivate::qSlicerExtensionsManageWidgetPrivate(qSlicerExtensionsManageWidget& object)
  :q_ptr(&object)
{
  this->ExtensionsManagerModel = 0;
}

// --------------------------------------------------------------------------
namespace
{

// --------------------------------------------------------------------------
class qSlicerExtensionsButtonBox : public QWidget, public Ui_qSlicerExtensionsButtonBox
{
public:
  typedef QWidget Superclass;
  qSlicerExtensionsButtonBox(QWidget* parent = 0) : Superclass(parent)
  {
    this->setupUi(this);
  }
};

// --------------------------------------------------------------------------
QIcon extensionIcon(const QString& path, bool enabled)
{
  return  QIcon(QIcon(path).pixmap(QSize(64, 64), enabled ? QIcon::Normal : QIcon::Disabled));
}

} // end of anonymous namespace

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidgetPrivate::init()
{
  Q_Q(qSlicerExtensionsManageWidget);

  this->setupUi(q);

  this->ExtensionList->setColumnCount(Self::ColumnCount);
  this->ExtensionList->setColumnHidden(Self::NameColumn, true);
  this->ExtensionList->setRootIsDecorated(false);
  this->ExtensionList->setIconSize(QSize(64, 64));
  this->ExtensionList->header()->setResizeMode(Self::TextColumn, QHeaderView::Stretch);
  this->ExtensionList->setAllColumnsShowFocus(true);
  this->ExtensionList->setAlternatingRowColors(true);
  this->ExtensionList->setSelectionMode(QAbstractItemView::NoSelection);

  QObject::connect(&this->EnableButtonMapper, SIGNAL(mapped(QString)),
                   q, SLOT(setExtensionEnabled(QString)));

  QObject::connect(&this->DisableButtonMapper, SIGNAL(mapped(QString)),
                   q, SLOT(setExtensionDisabled(QString)));

  QObject::connect(&this->UninstallButtonMapper, SIGNAL(mapped(QString)),
                   q, SLOT(uninstallExtension(QString)));
}

// --------------------------------------------------------------------------
QTreeWidgetItem * qSlicerExtensionsManageWidgetPrivate::extensionItem(const QString& extensionName)const
{
  QList<QTreeWidgetItem*> items =
      this->ExtensionList->findItems(extensionName, Qt::MatchExactly, Self::NameColumn);
  Q_ASSERT(items.count() < 2);
  if (items.count() == 1)
    {
    return items.at(0);
    }
  return 0;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidgetPrivate::addExtensionItem(const ExtensionMetadataType& metadata)
{
  QString extensionName = metadata.value("extensionname").toString();
  if (extensionName.isEmpty())
    {
    qCritical() << "Missing metadata identified with 'extensionname' key";
    return;
    }
  Q_ASSERT(this->extensionItem(extensionName) == 0);
  QString description = metadata.value("description").toString();
  bool enabled = QVariant::fromValue(metadata.value("enabled")).toBool();

  QTreeWidgetItem * item = new QTreeWidgetItem();

  item->setIcon(qSlicerExtensionsManageWidgetPrivate::IconColumn,
                extensionIcon(":/Icons/ExtensionDefaultIcon.png", enabled));

  // See extensionItem(...) - Allow findItems() to work as expected
  item->setText(qSlicerExtensionsManageWidgetPrivate::NameColumn, extensionName);

  this->ExtensionList->addTopLevelItem(item);

  QString extensionNameAndDescription =
      QString("<b>%1</b><br><br>%2<br>").arg(extensionName).arg(description);
//  QString extensionMoreLink = QString("<a href=\"slicer:%3\">More</a>").arg(extensionName);
  QLabel * label = new QLabel(extensionNameAndDescription);
  label->setOpenExternalLinks(true);
  label->setMargin(9);
  this->ExtensionList->setItemWidget(item, qSlicerExtensionsManageWidgetPrivate::TextColumn, label);
  this->LabelLinkMapper.setMapping(label, extensionName);
  QObject::connect(label, SIGNAL(linkActivated(QString)), &this->LabelLinkMapper, SLOT(map()));

  qSlicerExtensionsButtonBox * buttonBox = new qSlicerExtensionsButtonBox();
  this->ExtensionList->setItemWidget(item, qSlicerExtensionsManageWidgetPrivate::ButtonsColumn, buttonBox);

  this->EnableButtonMapper.setMapping(buttonBox->EnableButton, extensionName);
  QObject::connect(buttonBox->EnableButton, SIGNAL(clicked()), &this->EnableButtonMapper, SLOT(map()));
  buttonBox->EnableButton->setVisible(!enabled);

  this->DisableButtonMapper.setMapping(buttonBox->DisableButton, extensionName);
  QObject::connect(buttonBox->DisableButton, SIGNAL(clicked()), &this->DisableButtonMapper, SLOT(map()));
  buttonBox->DisableButton->setVisible(enabled);

  this->UninstallButtonMapper.setMapping(buttonBox->UninstallButton, extensionName);
  QObject::connect(buttonBox->UninstallButton, SIGNAL(clicked()), &this->UninstallButtonMapper, SLOT(map()));
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
qSlicerExtensionsManageWidget::~qSlicerExtensionsManageWidget()
{
}

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
  disconnect(this, SLOT(onExtensionUninstalled(QString)));
  disconnect(this, SLOT(onModelExtensionEnabledChanged(QString,bool)));

  d->ExtensionsManagerModel = model;

  if (d->ExtensionsManagerModel)
    {
    this->onModelUpdated();
    connect(d->ExtensionsManagerModel, SIGNAL(modelUpdated()),
            this, SLOT(onModelUpdated()));
    connect(d->ExtensionsManagerModel, SIGNAL(extensionInstalled(QString)),
            this, SLOT(onExtensionInstalled(QString)));
    connect(d->ExtensionsManagerModel, SIGNAL(extensionUninstalled(QString)),
            this, SLOT(onExtensionUninstalled(QString)));
    connect(d->ExtensionsManagerModel, SIGNAL(extensionEnabledChanged(QString,bool)),
            this, SLOT(onModelExtensionEnabledChanged(QString,bool)));
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
void qSlicerExtensionsManageWidget::uninstallExtension(const QString& extensionName)
{
  if (!this->extensionsManagerModel())
    {
    return;
    }
  this->extensionsManagerModel()->uninstallExtension(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::onExtensionInstalled(const QString& extensionName)
{
  Q_D(qSlicerExtensionsManageWidget);
  d->addExtensionItem(d->ExtensionsManagerModel->extensionMetadata(extensionName));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::onExtensionUninstalled(const QString& extensionName)
{
  Q_D(qSlicerExtensionsManageWidget);
  QTreeWidgetItem * item = d->extensionItem(extensionName);
  Q_ASSERT(item);
  delete d->ExtensionList->takeTopLevelItem(d->ExtensionList->indexOfTopLevelItem(item));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::onModelExtensionEnabledChanged(const QString &extensionName, bool enabled)
{
  Q_D(qSlicerExtensionsManageWidget);
  QTreeWidgetItem * item = d->extensionItem(extensionName);
  Q_ASSERT(item);
  qSlicerExtensionsButtonBox * buttonBox =
      dynamic_cast<qSlicerExtensionsButtonBox*>(d->ExtensionList->itemWidget(item, qSlicerExtensionsManageWidgetPrivate::ButtonsColumn));
  Q_ASSERT(buttonBox);
  buttonBox->EnableButton->setVisible(!enabled);
  buttonBox->DisableButton->setVisible(enabled);
  item->setIcon(qSlicerExtensionsManageWidgetPrivate::IconColumn,
                extensionIcon(":/Icons/ExtensionDefaultIcon.png", enabled));

}

// --------------------------------------------------------------------------
void qSlicerExtensionsManageWidget::onModelUpdated()
{
  Q_D(qSlicerExtensionsManageWidget);
  d->ExtensionList->clear();
  foreach(const QString& extensionName, d->ExtensionsManagerModel->installedExtensions())
    {
    d->addExtensionItem(d->ExtensionsManagerModel->extensionMetadata(extensionName));
    }
}
