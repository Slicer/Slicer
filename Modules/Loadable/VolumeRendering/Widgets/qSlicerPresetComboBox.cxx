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
#include <QApplication>
#include <QDesktopWidget>
#include <QListView>
#include <QScreen>
#include <QScrollBar>

// CTK includes
#include <ctkVTKWidgetsUtils.h>
#include <ctkWidgetsUtils.h>

/// Module logic includes
#include "vtkSlicerVolumeRenderingLogic.h"

// MRMLWidgets includes
#include <qMRMLSceneModel.h>
#include <qMRMLUtils.h>

// VolumeRenderingWidgets includes
#include "qSlicerPresetComboBox_p.h"

// MRML includes
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLVolumePropertyNode.h>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>

// STD includes
#include <cassert>

//-----------------------------------------------------------------------------
qSlicerIconComboBox::qSlicerIconComboBox(QWidget* parentWidget)
  : Superclass(parentWidget)
{
  QListView* listView = new QListView(nullptr);
  listView->setViewMode(QListView::IconMode);
  listView->setUniformItemSizes(true);
  listView->setWrapping(true);
  listView->setMovement(QListView::Static);
  listView->setFlow(QListView::LeftToRight);
  listView->setResizeMode(QListView::Adjust); // resize list view if widget width is changed
  listView->setSpacing(0);
  this->setView(listView);
}

//-----------------------------------------------------------------------------
void qSlicerIconComboBox::showPopup()
{
  QFrame* container = qobject_cast<QFrame*>(this->view()->parentWidget());
  QStyleOptionComboBox opt;
  initStyleOption(&opt);

  QRect listRect(this->style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxListBoxPopup, this));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
  QRect screen = this->screen()->availableGeometry();
#else
  QRect screen = QApplication::desktop()->availableGeometry(QApplication::desktop()->screenNumber(this));
#endif
  QPoint above = mapToGlobal(listRect.topLeft());

  // CustomSize
  // Screens are usually wide, therefore we allocate space for about 8 columns and up to 5 rows.

  const int numberOfItems = this->count();
  const int itemsPerRow = 8;
  // Maximum 5 rows (if items do not fit then a vertical scrollbar will be displayed)
  const int itemsPerColumns =
    std::min(5, itemsPerRow ? static_cast<int>(0.999 + static_cast<double>(numberOfItems) / itemsPerRow) : 0);

  int verticalScrollBarWidth = 0;
  if (this->view()->verticalScrollBar())
  {
    verticalScrollBarWidth = this->view()->verticalScrollBar()->width();
  }
  QMargins margins = container->contentsMargins();
  // Item size is not exactly the icon size, so we'll add 10% and 20% margin.
  // We don't need exact match, just approximate size match is sufficient.
  QSize itemSize = this->view()->iconSize();
  bool labelShown = (this->count() > 0) && (!this->itemText(0).isEmpty());
  listRect.setWidth(itemsPerRow * itemSize.width() * 1.1 + margins.left() + margins.right()
                    + 2.0 * container->frameWidth() + verticalScrollBarWidth);
  listRect.setHeight(itemsPerColumns * itemSize.height() * (labelShown ? 1.2 : 1.05) + margins.top() + margins.bottom()
                     + 2.0 * container->frameWidth());

  // Position horizontally.
  listRect.moveLeft(above.x());

  // Position vertically so the currently selected item lines up
  // with the combo box.
  const QRect currentItemRect = view()->visualRect(view()->currentIndex());
  const int offset = listRect.top() - currentItemRect.top();
  listRect.moveTop(above.y() + offset - listRect.top());

  if (listRect.width() > screen.width())
  {
    listRect.setWidth(screen.width());
  }

  container->setGeometry(listRect);
  container->raise();
  container->show();
  this->view()->setFocus();
  this->view()->scrollTo(this->view()->currentIndex(),
                         this->style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this)
                           ? QAbstractItemView::PositionAtCenter
                           : QAbstractItemView::EnsureVisible);
  container->update();
}

//-----------------------------------------------------------------------------
qSlicerPresetComboBoxPrivate::qSlicerPresetComboBoxPrivate(qSlicerPresetComboBox& object)
  : q_ptr(&object)
  , ShowIcons(true)
  , ShowLabelsInPopup(true)
  , IconSizeInPopup(64, 64)
{
}

//-----------------------------------------------------------------------------
void qSlicerPresetComboBoxPrivate::init()
{
  Q_Q(qSlicerPresetComboBox);

  q->setNodeTypes(QStringList("vtkMRMLVolumePropertyNode"));
  q->setSelectNodeUponCreation(false);
  q->setAddEnabled(false);
  q->setRemoveEnabled(false);
  q->setBaseName(qSlicerPresetComboBox::tr("Preset"));

  int iconSize = q->style()->pixelMetric(QStyle::PM_SmallIconSize) * 4;
  q->setIconSizeInPopup(QSize(iconSize, iconSize));

  this->updateLabelsIconsVisibility();

  QObject::connect(q, SIGNAL(nodeAdded(vtkMRMLNode*)), q, SLOT(setIconToPreset(vtkMRMLNode*)));
  QObject::connect(q, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SLOT(updateComboBoxTitleAndIcon(vtkMRMLNode*)));
}

// --------------------------------------------------------------------------
void qSlicerPresetComboBoxPrivate::updateLabelsIconsVisibility()
{
  Q_Q(qSlicerPresetComboBox);
  vtkMRMLScene* scene = q->mrmlScene();
  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(q->sortFilterProxyModel()->sourceModel());

  if (this->ShowIcons)
  {
    qSlicerIconComboBox* comboBox = new qSlicerIconComboBox;
    comboBox->forceDefault(true);
    q->setComboBox(comboBox);
    q->setIconSizeInPopup(q->iconSizeInPopup());
    sceneModel->setNameColumn(this->ShowLabelsInPopup ? 0 : -1);
  }
  else
  {
    ctkComboBox* comboBox = new ctkComboBox;
    comboBox->forceDefault(false);
    q->setComboBox(comboBox);
    sceneModel->setNameColumn(0);
  }

  // Update from scene
  QString currentNodeID = q->currentNodeID();
  q->setMRMLScene(nullptr);
  q->setMRMLScene(scene);
  q->updateComboBoxTitleAndIcon(nullptr);
  q->setCurrentNodeID(currentNodeID);
}

// --------------------------------------------------------------------------
qSlicerPresetComboBox::qSlicerPresetComboBox(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerPresetComboBoxPrivate(*this))
{
  Q_D(qSlicerPresetComboBox);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerPresetComboBox::~qSlicerPresetComboBox() = default;

// --------------------------------------------------------------------------
void qSlicerPresetComboBox::setIconToPreset(vtkMRMLNode* presetNode)
{
  Q_D(qSlicerPresetComboBox);
  if (presetNode == nullptr)
  {
    return;
  }

  qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(this->sortFilterProxyModel()->sourceModel());
  const QModelIndex& itemIndex = sceneModel->indexFromNode(presetNode);

  if (d->ShowIcons)
  {
    // Search corresponding icon
    QIcon presetIcon;
    vtkMRMLVolumeNode* iconVolume = vtkMRMLVolumeNode::SafeDownCast(
      presetNode->GetNodeReference(vtkSlicerVolumeRenderingLogic::GetIconVolumeReferenceRole()));
    if (iconVolume && iconVolume->GetImageData() != nullptr)
    {
      QImage qimage;
      qMRMLUtils::vtkImageDataToQImage(iconVolume->GetImageData(), qimage);
      // vtkITK loads 3D images with y axis flipped (compared to loading 2D images), flip it back now
      QImage qimageFlipped(qimage.mirrored(false, true));
      presetIcon.addPixmap(QPixmap::fromImage(qimageFlipped));
    }
    if (presetIcon.availableSizes().size() == 0)
    {
      // Check if an image is available for this preset in the stock preset images.
      // This may be faster than loading images using storage nodes from the scene.
      presetIcon = QIcon(QString(":/presets/") + presetNode->GetName());
    }
    if (presetIcon.availableSizes().size() == 0)
    {
      // Use generic icon (puzzle piece)
      presetIcon = QIcon(":/Icons/Extension.png");
    }

    // Set icon
    sceneModel->setData(itemIndex, presetIcon, Qt::DecorationRole);
  }

  // Set toolTip
  vtkMRMLVolumePropertyNode* volumePropertyNode = vtkMRMLVolumePropertyNode::SafeDownCast(presetNode);
  if (volumePropertyNode)
  {
    int previewSize = this->style()->pixelMetric(QStyle::PM_SmallIconSize);
    vtkScalarsToColors* colors = volumePropertyNode->GetVolumeProperty()
                                   ? volumePropertyNode->GetVolumeProperty()->GetRGBTransferFunction()
                                   : nullptr;
    assert(colors && colors->GetRange()[1] > colors->GetRange()[0]);
    QImage img = ctk::scalarsToColorsImage(colors, QSize(previewSize, previewSize));
    QString imgSrc = ctk::base64HTMLImageTagSrc(img);
    QString toolTip = QString("<img src=\"%1\"> %2").arg(imgSrc).arg(presetNode->GetName());
    sceneModel->setData(itemIndex, toolTip, Qt::ToolTipRole);
  }
}

// --------------------------------------------------------------------------
void qSlicerPresetComboBox::updateComboBoxTitleAndIcon(vtkMRMLNode* node)
{
  Q_D(qSlicerPresetComboBox);
  ctkComboBox* combo = qobject_cast<ctkComboBox*>(this->comboBox());
  if (node)
  {
    combo->setDefaultText(node->GetName());
    combo->setDefaultIcon(d->ShowIcons ? combo->itemIcon(combo->currentIndex()) : QIcon());
  }
  else
  {
    combo->setDefaultText(tr("Select a Preset"));
    combo->setDefaultIcon(QIcon());
  }
}

// --------------------------------------------------------------------------
bool qSlicerPresetComboBox::showIcons() const
{
  Q_D(const qSlicerPresetComboBox);
  return d->ShowIcons;
}

// --------------------------------------------------------------------------
void qSlicerPresetComboBox::setShowIcons(bool show)
{
  Q_D(qSlicerPresetComboBox);
  d->ShowIcons = show;
  d->updateLabelsIconsVisibility();
}

// --------------------------------------------------------------------------
bool qSlicerPresetComboBox::showLabelsInPopup() const
{
  Q_D(const qSlicerPresetComboBox);
  return d->ShowLabelsInPopup;
}

// --------------------------------------------------------------------------
void qSlicerPresetComboBox::setShowLabelsInPopup(bool show)
{
  Q_D(qSlicerPresetComboBox);
  d->ShowLabelsInPopup = show;
  d->updateLabelsIconsVisibility();
}

// --------------------------------------------------------------------------
QSize qSlicerPresetComboBox::iconSizeInPopup() const
{
  Q_D(const qSlicerPresetComboBox);
  return d->IconSizeInPopup;
}

// --------------------------------------------------------------------------
void qSlicerPresetComboBox::setIconSizeInPopup(const QSize& size)
{
  Q_D(qSlicerPresetComboBox);
  d->IconSizeInPopup = size;
  this->comboBox()->view()->setIconSize(d->IconSizeInPopup);
}
