/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QLabel>

// SubjectHierarchy includes
#include "qMRMLSubjectHierarchyComboBox.h"

#include "qMRMLSubjectHierarchyTreeView.h"
#include "qMRMLSubjectHierarchyModel.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"

#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

// MRML includes
#include <vtkMRMLScene.h>

//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qMRMLSubjectHierarchyComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSubjectHierarchyComboBox);

protected:
  qMRMLSubjectHierarchyComboBox* const q_ptr;

public:
  qMRMLSubjectHierarchyComboBoxPrivate(qMRMLSubjectHierarchyComboBox& object);
  virtual void init();

public:
  int MaximumNumberOfShownItems;
  bool AlignPopupVertically;

  qMRMLSubjectHierarchyTreeView* TreeView;
  QLabel* NoItemLabel;
};

//------------------------------------------------------------------------------
qMRMLSubjectHierarchyComboBoxPrivate::qMRMLSubjectHierarchyComboBoxPrivate(qMRMLSubjectHierarchyComboBox& object)
  : q_ptr(&object)
  , MaximumNumberOfShownItems(20)
  , AlignPopupVertically(true)
  , TreeView(nullptr)
  , NoItemLabel(nullptr)
{
}

// --------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBoxPrivate::init()
{
  Q_Q(qMRMLSubjectHierarchyComboBox);

  q->forceDefault(true);

  q->setDefaultText("Select subject hierarchy item");
  q->setDefaultIcon(QIcon());
  q->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

  // Setup tree view
  this->TreeView = new qMRMLSubjectHierarchyTreeView(q);
  this->TreeView->setMultiSelection(false);
  this->TreeView->setColumnHidden(this->TreeView->model()->visibilityColumn(), true);
  this->TreeView->setColumnHidden(this->TreeView->model()->transformColumn(), true);
  this->TreeView->setColumnHidden(this->TreeView->model()->idColumn(), true);
  this->TreeView->setHeaderHidden(true);
  this->TreeView->setContextMenuEnabled(false);

  // No item label
  this->NoItemLabel = new QLabel("No items");
  this->NoItemLabel->setMargin(4);

  // Add tree view to container
  QFrame* container = qobject_cast<QFrame*>(q->view()->parentWidget());
  container->layout()->addWidget(this->NoItemLabel);
  container->layout()->addWidget(this->TreeView);

  // Make connections
  QObject::connect(this->TreeView, SIGNAL(currentItemChanged(vtkIdType)),
                   q, SLOT(updateComboBoxTitleAndIcon(vtkIdType)));
  QObject::connect(this->TreeView, SIGNAL(currentItemModified(vtkIdType)),
                   q, SLOT(updateComboBoxTitleAndIcon(vtkIdType)));
  QObject::connect(this->TreeView, SIGNAL(currentItemChanged(vtkIdType)),
                   q, SIGNAL(currentItemChanged(vtkIdType)));
  QObject::connect(this->TreeView, SIGNAL(currentItemModified(vtkIdType)),
                   q, SIGNAL(currentItemModified(vtkIdType)));
}

// --------------------------------------------------------------------------
// qMRMLSubjectHierarchyComboBox

// --------------------------------------------------------------------------
qMRMLSubjectHierarchyComboBox::qMRMLSubjectHierarchyComboBox(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLSubjectHierarchyComboBoxPrivate(*this))
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLSubjectHierarchyComboBox::~qMRMLSubjectHierarchyComboBox() = default;

//------------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* qMRMLSubjectHierarchyComboBox::subjectHierarchyNode()const
{
  Q_D(const qMRMLSubjectHierarchyComboBox);
  return d->TreeView->subjectHierarchyNode();
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLSubjectHierarchyComboBox::mrmlScene()const
{
  Q_D(const qMRMLSubjectHierarchyComboBox);
  return d->TreeView->mrmlScene();
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(const qMRMLSubjectHierarchyComboBox);

  if (this->mrmlScene() == scene)
    {
    return;
    }

  d->TreeView->setMRMLScene(scene);

  vtkMRMLSubjectHierarchyNode* shNode = d->TreeView->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }

  // Set tree root item to be the new scene, and disable showing it
  d->TreeView->setRootItem(shNode->GetSceneItemID());
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::clearSelection()
{
  Q_D(const qMRMLSubjectHierarchyComboBox);
  d->TreeView->clearSelection();
}

//------------------------------------------------------------------------------
vtkIdType qMRMLSubjectHierarchyComboBox::currentItem()const
{
  Q_D(const qMRMLSubjectHierarchyComboBox);
  return d->TreeView->currentItem();
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::setCurrentItem(vtkIdType itemID)
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  return d->TreeView->setCurrentItem(itemID);
}

//--------------------------------------------------------------------------
qMRMLSortFilterSubjectHierarchyProxyModel* qMRMLSubjectHierarchyComboBox::sortFilterProxyModel()const
{
  Q_D(const qMRMLSubjectHierarchyComboBox);
  return d->TreeView->sortFilterProxyModel();
}

//--------------------------------------------------------------------------
qMRMLSubjectHierarchyModel* qMRMLSubjectHierarchyComboBox::model()const
{
  Q_D(const qMRMLSubjectHierarchyComboBox);
  return d->TreeView->model();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::setShowRootItem(bool show)
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  d->TreeView->setShowRootItem(show);
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyComboBox::showRootItem()const
{
  Q_D(const qMRMLSubjectHierarchyComboBox);
  return d->TreeView->showRootItem();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::setRootItem(vtkIdType rootItemID)
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  d->TreeView->setRootItem(rootItemID);
}

//--------------------------------------------------------------------------
vtkIdType qMRMLSubjectHierarchyComboBox::rootItem()const
{
  Q_D(const qMRMLSubjectHierarchyComboBox);
  return d->TreeView->rootItem();
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyComboBox::highlightReferencedItems()const
{
  Q_D(const qMRMLSubjectHierarchyComboBox);
  return d->TreeView->highlightReferencedItems();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::setHighlightReferencedItems(bool highlightOn)
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  d->TreeView->setHighlightReferencedItems(highlightOn);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::setPluginWhitelist(QStringList whitelist)
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  d->TreeView->setPluginWhitelist(whitelist);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::setPluginBlacklist(QStringList blacklist)
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  d->TreeView->setPluginBlacklist(blacklist);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::disablePlugin(QString plugin)
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  d->TreeView->disablePlugin(plugin);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::setAttributeFilter(const QString& attributeName, const QVariant& attributeValue/*=QVariant()*/)
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  d->TreeView->setAttributeFilter(attributeName, attributeValue);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::removeAttributeFilter()
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  d->TreeView->removeAttributeFilter();
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::setLevelFilter(QStringList &levelFilter)
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  d->TreeView->setLevelFilter(levelFilter);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::setNodeTypes(const QStringList& types)
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  d->TreeView->setNodeTypes(types);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::setHideChildNodeTypes(const QStringList& types)
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  d->TreeView->setHideChildNodeTypes(types);
}

//--------------------------------------------------------------------------
int qMRMLSubjectHierarchyComboBox::maximumNumberOfShownItems()const
{
  Q_D(const qMRMLSubjectHierarchyComboBox);
  return d->MaximumNumberOfShownItems;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::setMaximumNumberOfShownItems(int maxNumberOfShownItems)
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  d->MaximumNumberOfShownItems = maxNumberOfShownItems;
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyComboBox::alignPopupVertically()const
{
  Q_D(const qMRMLSubjectHierarchyComboBox);
  return d->AlignPopupVertically;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::setAlignPopupVertically(bool align)
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  d->AlignPopupVertically = align;
}

//-----------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::showPopup()
{
  Q_D(qMRMLSubjectHierarchyComboBox);

  QFrame* container = qobject_cast<QFrame*>(this->view()->parentWidget());
  QStyleOptionComboBox opt;
  this->initStyleOption(&opt);

  QRect listRect(this->style()->subControlRect(QStyle::CC_ComboBox, &opt,
                                               QStyle::SC_ComboBoxListBoxPopup, this));
  QRect screen = QApplication::desktop()->availableGeometry(QApplication::desktop()->screenNumber(this));
  QPoint below = mapToGlobal(listRect.bottomLeft());
  QPoint above = mapToGlobal(listRect.topLeft());

  // Custom Height
  int popupHeight = 0;
  int displayedItemCount = d->TreeView->displayedItemCount();
  if (displayedItemCount == 0)
    {
    // If there is no items, find what message to show instead
    vtkMRMLSubjectHierarchyNode* shNode = d->TreeView->subjectHierarchyNode();
    vtkIdType rootItem = d->TreeView->rootItem();
    std::vector<vtkIdType> childItemIDs;
    shNode->GetItemChildren(rootItem, childItemIDs, false);
    if (childItemIDs.empty())
      {
      if (rootItem!= shNode->GetSceneItemID())
        {
        std::string rootName = shNode->GetItemName(rootItem);
        QString label = QString("No items in branch: ") + QString::fromStdString(rootName);
        d->NoItemLabel->setText(label);
        }
      else
        {
        d->NoItemLabel->setText("No items in scene");
        }
      }
    else
      {
      d->NoItemLabel->setText("No items accepted by current filters");
      }

      // Show no item label instead of tree view
      d->NoItemLabel->show();
      d->TreeView->hide();
      popupHeight = d->NoItemLabel->sizeHint().height();
    }
  else
    {
    // Height based on the number of items
    const int numberOfRows = qMin(displayedItemCount, d->MaximumNumberOfShownItems);
    popupHeight = numberOfRows * d->TreeView->sizeHintForRow(0);

    // Add tree view margins for the height
    // NB: not needed for the width as the item labels will be cropped
    // without displaying an horizontal scroll bar
    int tvMarginLeft, tvMarginTop, tvMarginRight, tvMarginBottom;
    d->TreeView->getContentsMargins(&tvMarginLeft, &tvMarginTop, &tvMarginRight, &tvMarginBottom);
    popupHeight += tvMarginTop + tvMarginBottom;

    d->NoItemLabel->hide();
    d->TreeView->show();
    }

  // Add container margins for the height
  int marginLeft, marginTop, marginRight, marginBottom;
  container->getContentsMargins(&marginLeft, &marginTop, &marginRight, &marginBottom);
  popupHeight += marginTop + marginBottom;

  // Position of the container
  if(d->AlignPopupVertically)
    {
    // Position horizontally
    listRect.moveLeft(above.x());

    // Position vertically so the currently selected item lines up with the combo box
    const QRect currentItemRect = d->TreeView->visualRect(d->TreeView->currentIndex());
    const int offset = listRect.top() - currentItemRect.top();
    listRect.moveTop(above.y() + offset - listRect.top());

    if (listRect.width() > screen.width() )
      {
      listRect.setWidth(screen.width());
      }
    if (mapToGlobal(listRect.bottomRight()).x() > screen.right())
      {
      below.setX(screen.x() + screen.width() - listRect.width());
      above.setX(screen.x() + screen.width() - listRect.width());
      }
    if (mapToGlobal(listRect.topLeft()).x() < screen.x() )
      {
      below.setX(screen.x());
      above.setX(screen.x());
      }
    }
  else
    {
    // Position below the combobox
    listRect.moveTo(below);
    }

  container->move(listRect.topLeft());
  container->setFixedHeight(popupHeight);
  container->setFixedWidth(this->width());
  container->raise();
  container->show();

  this->view()->setFocus();
  this->view()->scrollTo( this->view()->currentIndex(),
                          this->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)
                             ? QAbstractItemView::PositionAtCenter
                             : QAbstractItemView::EnsureVisible );
  container->update();
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::mousePressEvent(QMouseEvent* e)
{
  // Disable context menu altogether
  if (e->button() == Qt::RightButton)
    {
    return;
    }

  // Perform default mouse press event (make selections etc.)
  this->Superclass::mousePressEvent(e);
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyComboBox::updateComboBoxTitleAndIcon(vtkIdType selectedShItemID)
{
  Q_D(qMRMLSubjectHierarchyComboBox);
  vtkMRMLSubjectHierarchyNode* shNode = d->TreeView->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    this->setDefaultText("Error: Invalid subject hierarchy");
    this->setDefaultIcon(QIcon());
    return;
    }
  if (!selectedShItemID)
    {
    this->setDefaultText("Select subject hierarchy item");
    this->setDefaultIcon(QIcon());
    return;
    }

  // Hide popup
  QFrame* container = qobject_cast<QFrame*>(this->view()->parentWidget());
  container->hide();

  // Assemble title for selected item
  QString titleText(shNode->GetItemName(selectedShItemID).c_str());
  vtkIdType parentItemID = shNode->GetItemParent(selectedShItemID);
  while (parentItemID != shNode->GetSceneItemID() && parentItemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    titleText.prepend(" / ");
    QString parentItemName(shNode->GetItemName(parentItemID).c_str());
    if (parentItemName.length() > 21)
      {
      // Truncate item name if too long
      parentItemName = parentItemName.left(9) + "..." + parentItemName.right(9);
      }
    titleText.prepend(parentItemName);
    parentItemID = shNode->GetItemParent(parentItemID);
    }
  this->setDefaultText(titleText);

  // Get icon for selected item
  std::string ownerPluginName = shNode->GetItemOwnerPluginName(selectedShItemID);
  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin =
    qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName(ownerPluginName.c_str());
  if (ownerPlugin)
    {
    this->setDefaultIcon(ownerPlugin->icon(selectedShItemID));
    }
  else
    {
    qCritical() << Q_FUNC_INFO << ": No owner plugin for subject hierarchy item " << shNode->GetItemName(selectedShItemID).c_str();
    this->setDefaultIcon(QIcon());
    }
}
