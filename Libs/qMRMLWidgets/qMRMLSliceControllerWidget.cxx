/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QWidgetAction>

// CTK includes
#include <ctkLogger.h>
#include <ctkPopupWidget.h>
#include <ctkSliderWidget.h>

// qMRML includes
#include "qMRMLSliceControllerWidget_p.h"

// MRMLLogic includes
#include <vtkMRMLSliceLayerLogic.h>

// MRML includes
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLSliceCompositeNode.h>

// VTK includes

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLSliceControllerWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qMRMLSliceControllerWidgetPrivate::qMRMLSliceControllerWidgetPrivate(qMRMLSliceControllerWidget& object)
  : q_ptr(&object)
{
  this->SliceLogic = 0;
  this->MRMLSliceNode = 0;
  this->MRMLSliceCompositeNode = 0;
  this->SliceLogics = 0;

  this->ControllerButtonGroup = 0;
  this->SliceOrientation = "Axial";

  qMRMLOrientation axialOrientation = {qMRMLSliceControllerWidget::tr("S: "), qMRMLSliceControllerWidget::tr("I <-----> S")};
  qMRMLOrientation sagittalOrientation = {qMRMLSliceControllerWidget::tr("R: "), qMRMLSliceControllerWidget::tr("L <-----> R")};
  qMRMLOrientation coronalOrientation = {qMRMLSliceControllerWidget::tr("A: "), qMRMLSliceControllerWidget::tr("P <-----> A")};
  qMRMLOrientation obliqueOrientation = {qMRMLSliceControllerWidget::tr(""), qMRMLSliceControllerWidget::tr("Oblique")};

  this->SliceOrientationToDescription["Axial"] = axialOrientation;
  this->SliceOrientationToDescription["Sagittal"] = sagittalOrientation;
  this->SliceOrientationToDescription["Coronal"] = coronalOrientation;
  this->SliceOrientationToDescription["Reformat"] = obliqueOrientation;

  this->PinButton = 0;
  this->SliceOffsetSlider = 0;
  this->PopupWidget = 0;

  this->LabelOpacitySlider = 0;
  this->LabelOpacityToggleButton = 0;
  this->LastLabelOpacity = 1.;

  this->LightBoxRowsSpinBox = 0;
  this->LightBoxColumnsSpinBox = 0;
}

//---------------------------------------------------------------------------
qMRMLSliceControllerWidgetPrivate::~qMRMLSliceControllerWidgetPrivate()
{
  delete this->PopupWidget;
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupUi(ctkPopupWidget* widget)
{
  Q_Q(qMRMLSliceControllerWidget);
  Q_ASSERT(widget == this->PopupWidget);

  this->Ui_qMRMLSliceControllerWidget::setupUi(widget);
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->BackgroundLayerNodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->ForegroundLayerNodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->LabelMapSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(this->actionFit_to_window, SIGNAL(triggered()),
                   q, SLOT(fitSliceToBackground()));
  QObject::connect(this->actionRotate_to_volume_plane, SIGNAL(triggered()),
                   q, SLOT(rotateSliceToBackground()));
  QObject::connect(this->actionShow_label_volume_outline, SIGNAL(toggled(bool)),
                   q, SLOT(showLabelOutline(bool)));
  QObject::connect(this->actionShow_reformat_widget, SIGNAL(toggled(bool)),
                   q, SLOT(showReformatWidget(bool)));
  QObject::connect(this->actionCompositingAlpha_blend, SIGNAL(triggered()),
                   q, SLOT(setCompositingToAlphaBlend()));
  QObject::connect(this->actionCompositingReverse_alpha_blend, SIGNAL(triggered()),
                   q, SLOT(setCompositingToReverseAlphaBlend()));
  QObject::connect(this->actionCompositingAdd, SIGNAL(triggered()),
                   q, SLOT(setCompositingToAdd()));
  QObject::connect(this->actionCompositingSubtract, SIGNAL(triggered()),
                   q, SLOT(setCompositingToSubtract()));
  QObject::connect(this->actionSliceSpacingModeAutomatic, SIGNAL(toggled(bool)),
                   q, SLOT(setSliceSpacingMode(bool)));
  QObject::connect(this->actionLightbox1x1_view, SIGNAL(triggered()),
                   q, SLOT(setLightboxTo1x1()));
  QObject::connect(this->actionLightbox1x2_view, SIGNAL(triggered()),
                   q, SLOT(setLightboxTo1x2()));
  QObject::connect(this->actionLightbox1x3_view, SIGNAL(triggered()),
                   q, SLOT(setLightboxTo1x3()));
  QObject::connect(this->actionLightbox1x4_view, SIGNAL(triggered()),
                   q, SLOT(setLightboxTo1x4()));
  QObject::connect(this->actionLightbox1x6_view, SIGNAL(triggered()),
                   q, SLOT(setLightboxTo1x6()));
  QObject::connect(this->actionLightbox1x8_view, SIGNAL(triggered()),
                   q, SLOT(setLightboxTo1x8()));
  QObject::connect(this->actionLightbox2x2_view, SIGNAL(triggered()),
                   q, SLOT(setLightboxTo2x2()));
  QObject::connect(this->actionLightbox3x3_view, SIGNAL(triggered()),
                   q, SLOT(setLightboxTo3x3()));
  QObject::connect(this->actionLightbox6x6_view, SIGNAL(triggered()),
                   q, SLOT(setLightboxTo6x6()));
  QObject::connect(this->actionForegroundInterpolation, SIGNAL(toggled(bool)),
                   q, SLOT(setForegroundInterpolation(bool)));
  QObject::connect(this->actionBackgroundInterpolation, SIGNAL(toggled(bool)),
                   q, SLOT(setBackgroundInterpolation(bool)));
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::init()
{
  Q_Q(qMRMLSliceControllerWidget);
  this->PopupWidget = new ctkPopupWidget;
  this->setupUi(this->PopupWidget);
  this->PopupWidget->setBaseWidget(q);
  this->PopupWidget->setAutoHide(true);

  this->SliceOffsetSlider = new ctkSliderWidget(q);
  this->SliceOffsetSlider->setToolTip(q->tr("Slice distance from RAS origin"));
  this->SliceOffsetSlider->setPageStep(1.);
  this->SliceOffsetSlider->spinBox()->setParent(this->PopupWidget);
  // Move the spinbox in the popup instead of having it in the slider bar
  dynamic_cast<QGridLayout*>(this->PopupWidget->layout())->addWidget(this->SliceOffsetSlider->spinBox(), 0, 0, 1, 2);

  this->PinButton = new QToolButton(q);
  this->PinButton->setCheckable(true);
  this->PinButton->setAutoRaise(true);
  this->PinButton->setFixedSize(15, 15);
  QIcon pushPinIcon;
  pushPinIcon.addFile(":/Icons/PushPinIn.png", QSize(), QIcon::Normal, QIcon::On);
  pushPinIcon.addFile(":/Icons/PushPinOut.png", QSize(), QIcon::Normal, QIcon::Off);
  this->PinButton->setIcon(pushPinIcon);
  QObject::connect(this->PinButton, SIGNAL(toggled(bool)),
                   this->PopupWidget, SLOT(pinPopup(bool)));
  this->PinButton->installEventFilter(this);

  QHBoxLayout* layout = new QHBoxLayout;
  layout->addWidget(this->SliceOffsetSlider);
  layout->addWidget(this->PinButton);
  QMargins margins = layout->contentsMargins();
  margins.setTop(0);
  margins.setBottom(0);
  q->setLayout(layout);
  layout->setContentsMargins(margins);

  vtkSmartPointer<vtkMRMLSliceLogic> defaultLogic =
    vtkSmartPointer<vtkMRMLSliceLogic>::New();
  q->setSliceLogic(defaultLogic);

  // Set selector attributes
  // Background and Foreground volume selectors can display LabelMap volumes. No
  // need to add the LabelMap attribute for them.
  this->LabelMapSelector->addAttribute("vtkMRMLVolumeNode", "LabelMap", "1");
  // Note: the min width is currently set in the UI file directly
  //// Set the slice controller widgets a min width.
  //int volumeSelectorMinWidth = this->LabelMapSelector->fontMetrics().width("Xxxxxxxx") + 20;
  //this->SliceOrientationSelector->setMinimumWidth(volumeSelectorMinWidth);
  //this->LabelMapSelector->setMinimumWidth(volumeSelectorMinWidth);
  //this->BackgroundLayerNodeSelector->setMinimumWidth(volumeSelectorMinWidth);
  //this->ForegroundLayerNodeSelector->setMinimumWidth(volumeSelectorMinWidth);

  // Populate the Linked menu
  this->setupLinkedOptionsMenu();

  // Populate Advanced menu
  this->setupMoreOptionsMenu();

  // Populate Foreground menu
  QMenu* foregroundMenu = new QMenu(tr("Advanced foreground layer"), this->ForegroundLayerOptionButton);
  foregroundMenu->addAction(this->actionForegroundInterpolation);
  this->ForegroundLayerOptionButton->setMenu(foregroundMenu);
  // Populate Background menu
  QMenu* backgroundMenu = new QMenu(tr("Advanced background layer"), this->BackgroundLayerOptionButton);
  backgroundMenu->addAction(this->actionBackgroundInterpolation);
  this->BackgroundLayerOptionButton->setMenu(backgroundMenu);

  // Connect Orientation selector
  this->connect(this->SliceOrientationSelector, SIGNAL(currentIndexChanged(QString)),
                q, SLOT(setSliceOrientation(QString)));

  // Connect Foreground layer selector
  this->connect(this->ForegroundLayerNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onForegroundLayerNodeSelected(vtkMRMLNode*)));
  // when the user select an entry already selected, we want to synchronize with the linked
  // slice logics as they mighy not have the same entry selected
  this->connect(this->ForegroundLayerNodeSelector, SIGNAL(nodeActivated(vtkMRMLNode*)),
                SLOT(onForegroundLayerNodeSelected(vtkMRMLNode*)));

  // Connect Background layer selector
  this->connect(this->BackgroundLayerNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onBackgroundLayerNodeSelected(vtkMRMLNode*)));
  // when the user select an entry already selected, we want to synchronize with the linked
  // slice logics as they mighy not have the same entry selected
  this->connect(this->BackgroundLayerNodeSelector, SIGNAL(nodeActivated(vtkMRMLNode*)),
                SLOT(onBackgroundLayerNodeSelected(vtkMRMLNode*)));

  // Connect Label map selector
  this->connect(this->LabelMapSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onLabelMapNodeSelected(vtkMRMLNode*)));
  // when the user select an entry already selected, we want to synchronize with the linked
  // slice logics as they mighy not have the same entry selected
  this->connect(this->LabelMapSelector, SIGNAL(nodeActivated(vtkMRMLNode*)),
                SLOT(onLabelMapNodeSelected(vtkMRMLNode*)));

  // Connect Slice offset slider
  this->connect(this->SliceOffsetSlider, SIGNAL(valueChanged(double)),
                q, SLOT(setSliceOffsetValue(double)), Qt::QueuedConnection);
  this->connect(this->SliceOffsetSlider, SIGNAL(valueIsChanging(double)),
                q, SLOT(trackSliceOffsetValue(double)), Qt::QueuedConnection);

  // Connect SliceCollapsibleButton
  // See qMRMLSliceControllerWidget::setControllerButtonGroup()
  //this->connect(this->SliceCollapsibleButton, SIGNAL(clicked()),
  //              SLOT(toggleControllerWidgetGroupVisibility()));

  // Connect Slice visibility toggle
  this->connect(this->SliceVisibilityButton, SIGNAL(clicked(bool)),
                q, SLOT(setSliceVisible(bool)));

  // Connect link toggle
  this->connect(this->SliceLinkButton, SIGNAL(clicked(bool)),
                q, SLOT(setSliceLink(bool)));
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::toggleControllerWidgetGroupVisibility()
{
  //bool visible = !this->ControllerWidgetGroup->isVisibleTo(
  //  this->ControllerWidgetGroup->parentWidget());
  //this->ControllerWidgetGroup->setVisible(visible);
  //this->SliceCollapsibleButton->setArrowType(
  //  visible ? Qt::UpArrow : Qt::DownArrow);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupLinkedOptionsMenu()
{
  Q_Q(qMRMLSliceControllerWidget);
  QMenu* linkedMenu = new QMenu(tr("Linked"),this->SliceLinkButton);
  linkedMenu->addAction(this->actionHotLinked);

  QObject::connect(this->actionHotLinked, SIGNAL(toggled(bool)),
                   q, SLOT(setHotLinked(bool)));

  this->SliceLinkButton->setMenu(linkedMenu);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupMoreOptionsMenu()
{
  Q_Q(qMRMLSliceControllerWidget);
  //QMenu* advancedMenu = new QMenu(tr("Advanced"),this->SliceMoreOptionButton);
  //QMenu* advancedMenu = new QMenu(tr("Advanced"));
  // Fit to window
  //advancedMenu->addAction(this->actionFit_to_window);
  this->FitToWindowToolButton->setDefaultAction(actionFit_to_window);
  // Rotate to Volume Plane
  //advancedMenu->addAction(this->actionRotate_to_volume_plane);
  this->RotateToVolumePlaneToolButton->setDefaultAction(actionRotate_to_volume_plane);
  // LabelMap opacity
  QMenu* labelOpacityMenu = new QMenu(tr("Adjust Labelmap opacity"));
  labelOpacityMenu->setIcon(QIcon(":Icons/SlicesLabelOpacity.png"));
  QWidget* labelOpacityWidget = new QWidget(labelOpacityMenu);
  QHBoxLayout* labelOpacityLayout = new QHBoxLayout(labelOpacityWidget);
  labelOpacityLayout->setContentsMargins(0,0,0,0);
  this->LabelOpacitySlider = new ctkSliderWidget(q);
  this->LabelOpacitySlider->setRange(0., 1.);
  this->LabelOpacitySlider->setSingleStep(0.05);
  this->LabelOpacitySlider->setValue(this->LastLabelOpacity);
  QObject::connect(this->LabelOpacitySlider, SIGNAL(valueChanged(double)),
                   q, SLOT(setLabelOpacity(double)));
  this->LabelOpacityToggleButton = new QToolButton(q);
  this->LabelOpacityToggleButton->setText("Toggle label opacity");
  QIcon visibilityIcon;
  visibilityIcon.addFile(":Icons/VisibleOn.png", QSize(), QIcon::Normal, QIcon::Off);
  visibilityIcon.addFile(":Icons/VisibleOff.png", QSize(), QIcon::Normal, QIcon::On);
  this->LabelOpacityToggleButton->setIcon(visibilityIcon);
  this->LabelOpacityToggleButton->setCheckable(true);
  // clicked is fired only if the user clicks on the button, not programatically
  QObject::connect(this->LabelOpacityToggleButton, SIGNAL(clicked(bool)),
                   this, SLOT(toggleLabelOpacity(bool)));
  labelOpacityLayout->addWidget(this->LabelOpacityToggleButton);
  labelOpacityLayout->addWidget(this->LabelOpacitySlider);
  labelOpacityWidget->setLayout(labelOpacityLayout);
  QWidgetAction* sliderAction = new QWidgetAction(labelOpacityMenu);
  sliderAction->setDefaultWidget(labelOpacityWidget);
  labelOpacityMenu->addAction(sliderAction);
  //advancedMenu->addMenu(labelOpacityMenu);
  this->LabelOpacityToolButton->setMenu(labelOpacityMenu);

  // Show label volume outline
  //advancedMenu->addAction(this->actionShow_label_volume_outline);
  this->ShowLabelVolumeOutlineToolButton->setDefaultAction(
    this->actionShow_label_volume_outline);
  // Show reformat widget
  //advancedMenu->addAction(this->actionShow_reformat_widget);
  this->ShowReformatWidgetToolButton->setDefaultAction(
    this->actionShow_reformat_widget);
  // Compositing
  QMenu* compositingMenu = new QMenu(tr("Compositing"));
  compositingMenu->setIcon(QIcon(":/Icons/SlicesComposite.png"));
  compositingMenu->addAction(this->actionCompositingAlpha_blend);
  compositingMenu->addAction(this->actionCompositingReverse_alpha_blend);
  compositingMenu->addAction(this->actionCompositingAdd);
  compositingMenu->addAction(this->actionCompositingSubtract);
  QActionGroup* compositingGroup = new QActionGroup(compositingMenu);
  compositingGroup->addAction(this->actionCompositingAlpha_blend);
  compositingGroup->addAction(this->actionCompositingReverse_alpha_blend);
  compositingGroup->addAction(this->actionCompositingAdd);
  compositingGroup->addAction(this->actionCompositingSubtract);
  //advancedMenu->addMenu(compositingMenu);
  this->CompositingToolButton->setMenu(compositingMenu);

  // Spacing mode
  QMenu* sliceSpacingMode = new QMenu(tr("Slice spacing mode"));
  sliceSpacingMode->setIcon(QIcon(":/Icons/SlicerAutomaticSliceSpacing.png"));
  sliceSpacingMode->addAction(this->actionSliceSpacingModeAutomatic);
  QMenu* sliceSpacingManualMode = new QMenu(tr("Manual"), sliceSpacingMode);
  sliceSpacingManualMode->setIcon(QIcon(":/Icon/SlicerManualSliceSpacing.png"));
  this->SliceSpacingSpinBox = new QDoubleSpinBox(sliceSpacingManualMode);
  this->SliceSpacingSpinBox->setDecimals(3);
  this->SliceSpacingSpinBox->setRange(0., VTK_LARGE_FLOAT);
  this->SliceSpacingSpinBox->setSingleStep(0.001);
  this->SliceSpacingSpinBox->setValue(1.);
  QObject::connect(this->SliceSpacingSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setSliceSpacing(double)));
  QWidgetAction* sliceSpacingAction = new QWidgetAction(sliceSpacingManualMode);
  sliceSpacingAction->setDefaultWidget(this->SliceSpacingSpinBox);
  sliceSpacingManualMode->addAction(sliceSpacingAction);
  sliceSpacingMode->addMenu(sliceSpacingManualMode);
  //advancedMenu->addMenu(sliceSpacingMode);
  this->SliceSpacingModeToolButton->setMenu(sliceSpacingMode);
  // Lightbox View
  QMenu* lightboxView = new QMenu(tr("Lightbox view"));
  lightboxView->setIcon(QIcon(":/Icons/LayoutLightboxView.png"));
  lightboxView->addAction(this->actionLightbox1x1_view);
  lightboxView->addAction(this->actionLightbox1x2_view);
  lightboxView->addAction(this->actionLightbox1x3_view);
  lightboxView->addAction(this->actionLightbox1x4_view);
  lightboxView->addAction(this->actionLightbox1x6_view);
  lightboxView->addAction(this->actionLightbox1x8_view);
  lightboxView->addAction(this->actionLightbox2x2_view);
  lightboxView->addAction(this->actionLightbox3x3_view);
  lightboxView->addAction(this->actionLightbox6x6_view);
  QMenu* customLightboxMenu = new QMenu(tr("Custom"), lightboxView);
  QWidget* customLightbox = new QWidget(lightboxView);
  QHBoxLayout* customLightboxLayout = new QHBoxLayout(customLightbox);
  this->LightBoxRowsSpinBox = new QSpinBox(customLightbox);
  this->LightBoxRowsSpinBox->setRange(1, 100);
  this->LightBoxRowsSpinBox->setValue(1);
  this->LightBoxColumnsSpinBox = new QSpinBox(customLightbox);
  this->LightBoxColumnsSpinBox->setRange(1, 100);
  this->LightBoxColumnsSpinBox->setValue(1);
  QPushButton* applyCustomLightboxButton = new QPushButton(tr("Apply"),customLightbox);
  QObject::connect(applyCustomLightboxButton, SIGNAL(clicked()),
                   this, SLOT(applyCustomLightbox()));
  customLightboxLayout->addWidget(this->LightBoxRowsSpinBox);
  customLightboxLayout->addWidget(this->LightBoxColumnsSpinBox);
  customLightboxLayout->addWidget(applyCustomLightboxButton);
  customLightbox->setLayout(customLightboxLayout);
  QWidgetAction* customLightboxAction = new QWidgetAction(customLightbox);
  customLightboxAction->setDefaultWidget(customLightbox);
  customLightboxMenu->addAction(customLightboxAction);
  lightboxView->addMenu(customLightboxMenu);
  QActionGroup* lightboxActionGroup = new QActionGroup(lightboxView);
  lightboxActionGroup->addAction(this->actionLightbox1x1_view);
  lightboxActionGroup->addAction(this->actionLightbox1x2_view);
  lightboxActionGroup->addAction(this->actionLightbox1x3_view);
  lightboxActionGroup->addAction(this->actionLightbox1x4_view);
  lightboxActionGroup->addAction(this->actionLightbox1x6_view);
  lightboxActionGroup->addAction(this->actionLightbox1x8_view);
  lightboxActionGroup->addAction(this->actionLightbox2x2_view);
  lightboxActionGroup->addAction(this->actionLightbox3x3_view);
  lightboxActionGroup->addAction(this->actionLightbox6x6_view);
  lightboxActionGroup->addAction(customLightboxAction);
  //advancedMenu->addMenu(lightboxView);
  this->LightBoxToolButton->setMenu(lightboxView);
  // Adjust display
  /*
  QMenu* adjustDisplay = new QMenu(tr("Adjust display"), advancedMenu);
  adjustDisplay->setIcon(QIcon(":/Icons/SlicesWinLevThreshCol.png"));
  adjustDisplay->addAction(this->actionAdjustDisplayForeground_volume);
  adjustDisplay->addAction(this->actionAdjustDisplayBackground_volume);
  advancedMenu->addMenu(adjustDisplay);
  */
  //this->SliceMoreOptionButton->setMenu(advancedMenu);
}

//---------------------------------------------------------------------------
bool qMRMLSliceControllerWidgetPrivate::eventFilter(QObject* object, QEvent* event)
{
  if (object == this->PinButton && event->type() == QEvent::Enter)
    {
    this->PopupWidget->showPopup();
    }
  return this->QObject::eventFilter(object, event);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setMRMLSliceNodeInternal(vtkMRMLSliceNode* newSliceNode)
{
  Q_Q(qMRMLSliceControllerWidget);

  if (newSliceNode == this->MRMLSliceNode)
    {
    return;
    }

  this->qvtkReconnect(this->MRMLSliceNode, newSliceNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRMLSliceNode()));

  this->MRMLSliceNode = newSliceNode;

  // Update widget state given the new slice node
  this->updateWidgetFromMRMLSliceNode();

  // Enable/disable widget
  q->setDisabled(newSliceNode == 0);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setMRMLSliceCompositeNodeInternal(vtkMRMLSliceCompositeNode* sliceComposite)
{
  if (this->MRMLSliceCompositeNode == sliceComposite)
    {
    return;
    }
  this->qvtkReconnect(this->MRMLSliceCompositeNode,
                      sliceComposite,
                      vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRMLSliceCompositeNode()));
  this->MRMLSliceCompositeNode = sliceComposite;

  if (this->MRMLSliceCompositeNode)
    {
    this->updateWidgetFromMRMLSliceCompositeNode();
    }
}

// --------------------------------------------------------------------------
vtkSmartPointer<vtkCollection> qMRMLSliceControllerWidgetPrivate::saveNodesForUndo(const QString& nodeTypes)
{
  Q_Q(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes;
  if (q->mrmlScene())
    {
    nodes.TakeReference(
      q->mrmlScene()->GetNodesByClass(nodeTypes.toLatin1()));
    }
  if (nodes.GetPointer())
    {
    q->mrmlScene()->SaveStateForUndo(nodes);
    }
  return nodes;
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::updateWidgetFromMRMLSliceNode()
{
  if (!this->MRMLSliceNode)
    {
    return;
    }

  logger.trace("updateWidgetFromMRMLSliceNode");

  // Update orientation selector state
  int index = this->SliceOrientationSelector->findText(
      QString::fromStdString(this->MRMLSliceNode->GetOrientationString()));
  Q_ASSERT(index>=0 && index <=4);
  this->SliceOrientationSelector->setCurrentIndex(index);

  // Update slice offset slider tooltip
  qMRMLOrientation orientation = this->SliceOrientationToDescription[
          QString::fromStdString(this->MRMLSliceNode->GetOrientationString())];
  this->SliceOffsetSlider->setToolTip(orientation.ToolTip);
  this->SliceOffsetSlider->setPrefix(orientation.Prefix);

  // Update slice visibility toggle
  this->SliceVisibilityButton->setChecked(this->MRMLSliceNode->GetSliceVisible());

  // Label Outline
  bool showOutline = this->MRMLSliceNode->GetUseLabelOutline();
  this->actionShow_label_volume_outline->setChecked(showOutline);
  this->actionShow_label_volume_outline->setText(showOutline ?
    tr("Hide label volume outlines") : tr("Show label volume outlines"));
  // Reformat
  bool showReformat = this->MRMLSliceNode->GetWidgetVisible();
  this->actionShow_reformat_widget->setChecked(showReformat);
  this->actionShow_reformat_widget->setText(
    showReformat ? tr("Hide reformat widget"): tr("Show reformat widget"));
  // Slice spacing mode
  this->actionSliceSpacingModeAutomatic->setChecked(
    this->MRMLSliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::AutomaticSliceSpacingMode);
  int rows = this->MRMLSliceNode->GetLayoutGridRows();
  int columns = this->MRMLSliceNode->GetLayoutGridColumns();
  this->actionLightbox1x1_view->setChecked(rows == 1 && columns == 1);
  this->actionLightbox1x2_view->setChecked(rows == 1 && columns == 2);
  this->actionLightbox1x3_view->setChecked(rows == 1 && columns == 3);
  this->actionLightbox1x4_view->setChecked(rows == 1 && columns == 4);
  this->actionLightbox1x6_view->setChecked(rows == 1 && columns == 6);
  this->actionLightbox1x8_view->setChecked(rows == 1 && columns == 8);
  this->actionLightbox2x2_view->setChecked(rows == 2 && columns == 2);
  this->actionLightbox3x3_view->setChecked(rows == 3 && columns == 3);
  this->actionLightbox6x6_view->setChecked(rows == 6 && columns == 6);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::updateWidgetFromMRMLSliceCompositeNode()
{
  Q_Q(qMRMLSliceControllerWidget);
  if (!q->mrmlScene() || q->mrmlScene()->GetIsUpdating())
    {// when we are loading, the scene might be in an inconsistent mode, where
    // the volumes pointed by the slice composite node don't exist yet
    return;
    }
  Q_ASSERT(this->MRMLSliceCompositeNode);

  logger.trace("updateWidgetFromMRMLSliceCompositeNode");

  // Update slice link toggle. Must be done first as its state controls
  // different behaviors when properties are set.
  this->SliceLinkButton->setChecked(this->MRMLSliceCompositeNode->GetLinkedControl());
  this->actionHotLinked->setChecked(this->MRMLSliceCompositeNode->GetHotLinkedControl());
  if (this->MRMLSliceCompositeNode->GetLinkedControl())
    {
    if (this->MRMLSliceCompositeNode->GetHotLinkedControl())
      {
      this->SliceLinkButton->setIcon(QIcon(":Icons/HotLinkOn.png"));
      }
    else
      {
      this->SliceLinkButton->setIcon(QIcon(":Icons/LinkOn.png"));
      }
    }
  else
    {
      this->SliceLinkButton->setIcon(QIcon(":Icons/LinkOff.png"));
    }
  
  // Update "foreground layer" node selector
  this->ForegroundLayerNodeSelector->setCurrentNode(
      q->mrmlScene()->GetNodeByID(this->MRMLSliceCompositeNode->GetForegroundVolumeID()));

  // Update "background layer" node selector
  this->BackgroundLayerNodeSelector->setCurrentNode(
      q->mrmlScene()->GetNodeByID(this->MRMLSliceCompositeNode->GetBackgroundVolumeID()));

  // Update "label map" node selector
  this->LabelMapSelector->setCurrentNode(
      q->mrmlScene()->GetNodeByID(this->MRMLSliceCompositeNode->GetLabelVolumeID()));

  // Label opacity
  this->LabelOpacitySlider->setValue(this->MRMLSliceCompositeNode->GetLabelOpacity());
  // Compositing
  switch(this->MRMLSliceCompositeNode->GetCompositing())
    {
    case vtkMRMLSliceCompositeNode::Alpha:
      this->actionCompositingAlpha_blend->setChecked(true);
      break;
    case vtkMRMLSliceCompositeNode::ReverseAlpha:
      this->actionCompositingReverse_alpha_blend->setChecked(true);
      break;
    case vtkMRMLSliceCompositeNode::Add:
      this->actionCompositingAdd->setChecked(true);
      break;
    case vtkMRMLSliceCompositeNode::Subtract:
      this->actionCompositingSubtract->setChecked(true);
      break;
    }

}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onForegroundLayerNodeSelected(vtkMRMLNode * node)
{
  Q_Q(qMRMLSliceControllerWidget);
  logger.trace(QString("sliceView: %1 - ForegroundLayerNodeSelected").arg(q->sliceOrientation()));

  if (!this->MRMLSliceCompositeNode)
    {
    return;
    }

  this->SliceLogic->StartSliceCompositeNodeInteraction(vtkMRMLSliceCompositeNode::ForegroundVolumeFlag);
  this->MRMLSliceCompositeNode->SetForegroundVolumeID(node ? node->GetID() : 0);
  this->SliceLogic->EndSliceCompositeNodeInteraction();

  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(node);
  vtkMRMLScalarVolumeDisplayNode* displayNode =
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
      volumeNode ? volumeNode->GetVolumeDisplayNode() : 0);
  this->qvtkReconnect(displayNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateFromForegroundDisplayNode(vtkObject*)));
  this->updateFromForegroundDisplayNode(displayNode);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onBackgroundLayerNodeSelected(vtkMRMLNode * node)
{
  Q_Q(qMRMLSliceControllerWidget);
  logger.trace(QString("sliceView: %1 - BackgroundLayerNodeSelected").arg(q->sliceOrientation()));

  if (!this->MRMLSliceCompositeNode)
    {
    return;
    }

  this->SliceLogic->StartSliceCompositeNodeInteraction(vtkMRMLSliceCompositeNode::BackgroundVolumeFlag);
  this->MRMLSliceCompositeNode->SetBackgroundVolumeID(node ? node->GetID() : 0);
  this->SliceLogic->EndSliceCompositeNodeInteraction();

  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(node);
  vtkMRMLScalarVolumeDisplayNode* displayNode =
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
      volumeNode ? volumeNode->GetVolumeDisplayNode(): 0);
  this->qvtkReconnect(displayNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateFromBackgroundDisplayNode(vtkObject*)));
  this->updateFromBackgroundDisplayNode(displayNode);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onLabelMapNodeSelected(vtkMRMLNode * node)
{
  Q_Q(qMRMLSliceControllerWidget);
  logger.trace(QString("sliceView: %1 - LabelMapNodeSelected").arg(q->sliceOrientation()));
  if (!this->MRMLSliceCompositeNode)
    {
    return;
    }

  this->SliceLogic->StartSliceCompositeNodeInteraction(vtkMRMLSliceCompositeNode::LabelVolumeFlag);
  this->MRMLSliceCompositeNode->SetLabelVolumeID(node ? node->GetID() : 0);
  this->SliceLogic->EndSliceCompositeNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onSliceLogicModifiedEvent()
{
  Q_Q(qMRMLSliceControllerWidget);

  this->setMRMLSliceNodeInternal(
    this->SliceLogic ? this->SliceLogic->GetSliceNode() : 0);
  this->setMRMLSliceCompositeNodeInternal(
    this->SliceLogic ? this->SliceLogic->GetSliceCompositeNode() : 0);

  // no op if they are the same
  // The imagedata of SliceLogic can change !?!?! it should probably not
  q->setImageData(this->SliceLogic ? this->SliceLogic->GetImageData() : 0);

  if (!this->SliceLogic)
    {
    return;
    }
  bool wasBlocking = this->SliceOffsetSlider->blockSignals(true);

  // Set the scale increments to match the z spacing (rotated into slice space)
  const double * sliceSpacing = this->SliceLogic->GetLowestVolumeSliceSpacing();
  Q_ASSERT(sliceSpacing);
  double offsetResolution = sliceSpacing ? sliceSpacing[2] : 0;
  q->setSliceOffsetResolution(offsetResolution);

  // Set slice offset range to match the field of view
  // Calculate the number of slices in the current range
  double sliceBounds[6] = {0, -1, 0, -1, 0, -1};
  this->SliceLogic->GetLowestVolumeSliceBounds(sliceBounds);
  Q_ASSERT(sliceBounds[4] <= sliceBounds[5]);
  q->setSliceOffsetRange(sliceBounds[4], sliceBounds[5]);

  // Update slider position
  this->SliceOffsetSlider->setValue(this->SliceLogic->GetSliceOffset());
  this->SliceOffsetSlider->blockSignals(wasBlocking);

  emit q->renderRequested();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::updateFromForegroundDisplayNode(vtkObject* node)
{
  vtkMRMLScalarVolumeDisplayNode* displayNode =
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(node);
  if (!displayNode)
    {
    return;
    }
  this->actionForegroundInterpolation->setChecked(displayNode->GetInterpolate());
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::updateFromBackgroundDisplayNode(vtkObject* node)
{
  vtkMRMLScalarVolumeDisplayNode* displayNode =
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(node);
  if (!displayNode)
    {
    return;
    }
  this->actionBackgroundInterpolation->setChecked(displayNode->GetInterpolate());
}

//---------------------------------------------------------------------------
vtkMRMLSliceLogic* qMRMLSliceControllerWidgetPrivate::compositeNodeLogic(vtkMRMLSliceCompositeNode* node)
{
  if (!this->SliceLogics)
    {
    return 0;
    }
  vtkMRMLSliceLogic* logic = 0;
  vtkCollectionSimpleIterator it;
  for (this->SliceLogics->InitTraversal(it);(logic = static_cast<vtkMRMLSliceLogic*>(
                                               this->SliceLogics->GetNextItemAsObject(it)));)
    {
    if (logic->GetSliceCompositeNode() == node)
      {
      return logic;
      }
    }
  return 0;
}

//---------------------------------------------------------------------------
vtkMRMLSliceLogic* qMRMLSliceControllerWidgetPrivate::sliceNodeLogic(vtkMRMLSliceNode* node)
{
  if (!this->SliceLogics)
    {
    return 0;
    }
  vtkMRMLSliceLogic* logic = 0;
  vtkCollectionSimpleIterator it;
  for (this->SliceLogics->InitTraversal(it);(logic = static_cast<vtkMRMLSliceLogic*>(
                                               this->SliceLogics->GetNextItemAsObject(it)));)
    {
    if (logic->GetSliceNode() == node)
      {
      return logic;
      }
    }
  return 0;
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::fitSliceToBackground(vtkMRMLSliceLogic* sliceLogic)
{
  if (!sliceLogic || !sliceLogic->GetSliceNode())
    {
    logger.warn("fitSliceToBackground - Failed because sliceLogic or sliceLogic->GetSliceNode() is NULL");
    return;
    }
  sliceLogic->FitSliceToAll();
  sliceLogic->GetSliceNode()->UpdateMatrices();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setForegroundInterpolation(vtkMRMLSliceLogic* sliceLogic, bool interpolate)
{
  Q_Q(qMRMLSliceControllerWidget);
  // TODO, update the QAction when the display node is modified
  vtkMRMLVolumeNode* volumeNode = sliceLogic->GetForegroundLayer()->GetVolumeNode();
  vtkMRMLScalarVolumeDisplayNode *displayNode = volumeNode ? vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
    volumeNode->GetVolumeDisplayNode()) : 0;
  if (displayNode)
    {
    q->mrmlScene()->SaveStateForUndo(displayNode);
    displayNode->SetInterpolate(interpolate);
    }
  // historic code that doesn't seem to work
  // vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
  //   sliceLogic->GetForegroundLayer()->GetVolumeDisplayNode());
  // if (displayNode)
  //   {
  //   q->mrmlScene()->SaveStateForUndo(displayNode);
  //   displayNode->SetInterpolate(interpolate);
  //   vtkMRMLVolumeNode* volumeNode = sliceLogic->GetForegroundLayer()->GetVolumeNode();
  //   if (volumeNode)
  //     {
  //     volumeNode->Modified();
  //     }
  //   }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setBackgroundInterpolation(vtkMRMLSliceLogic* sliceLogic, bool interpolate)
{
  Q_Q(qMRMLSliceControllerWidget);
  // TODO, update the QAction when the display node is modified
  vtkMRMLVolumeNode* volumeNode = sliceLogic->GetBackgroundLayer()->GetVolumeNode();
  vtkMRMLScalarVolumeDisplayNode *displayNode = volumeNode ? vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
    volumeNode->GetVolumeDisplayNode()) : 0;
  if (displayNode)
    {
    q->mrmlScene()->SaveStateForUndo(displayNode);
    displayNode->SetInterpolate(interpolate);
    }
  // historic code that doesn't seem to work
  // vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
  //   sliceLogic->GetBackgroundLayer()->GetVolumeDisplayNode());
  // if (displayNode)
  //   {
  //   q->mrmlScene()->SaveStateForUndo(displayNode);
  //   displayNode->SetInterpolate(interpolate);
  //   vtkMRMLVolumeNode* volumeNode = sliceLogic->GetBackgroundLayer()->GetVolumeNode();
  //   if (volumeNode)
  //     {
  //     volumeNode->Modified();
  //     }
  //   }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::toggleLabelOpacity(bool hide)
{
  this->LabelOpacitySlider->setValue(hide ? 0. : this->LastLabelOpacity);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::applyCustomLightbox()
{
  Q_Q(qMRMLSliceControllerWidget);
  q->setLightbox(this->LightBoxRowsSpinBox->value(), this->LightBoxColumnsSpinBox->value());
}

// --------------------------------------------------------------------------
// qMRMLSliceView methods

// --------------------------------------------------------------------------
qMRMLSliceControllerWidget::qMRMLSliceControllerWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLSliceControllerWidgetPrivate(*this))
{
  Q_D(qMRMLSliceControllerWidget);
  d->init();
  this->setSliceViewName("Red");
}

// --------------------------------------------------------------------------
qMRMLSliceControllerWidget::~qMRMLSliceControllerWidget()
{
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* qMRMLSliceControllerWidget::mrmlSliceNode()const
{
  Q_D(const qMRMLSliceControllerWidget);
  return d->MRMLSliceNode;
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLSliceControllerWidget);

  if (this->mrmlScene() == newScene)
    {
    return;
    }
  d->qvtkReconnect(this->mrmlScene(), newScene, vtkMRMLScene::SceneImportedEvent,
                   d, SLOT(updateWidgetFromMRMLSliceCompositeNode()));

  d->SliceLogic->SetMRMLScene(newScene);

  // Disable the node selectors as they would fire the signal currentIndexChanged(0)
  // meaning that there is no current node anymore. It's not true, it just means that
  // that the current node was not in the combo box list menu before
  bool backgroundBlockSignals = d->BackgroundLayerNodeSelector->blockSignals(true);
  bool foregroundBlockSignals = d->ForegroundLayerNodeSelector->blockSignals(true);
  bool labelmapBlockSignals = d->LabelMapSelector->blockSignals(true);

  this->Superclass::setMRMLScene(newScene);

  d->BackgroundLayerNodeSelector->blockSignals(backgroundBlockSignals);
  d->ForegroundLayerNodeSelector->blockSignals(foregroundBlockSignals);
  d->LabelMapSelector->blockSignals(labelmapBlockSignals);

  d->updateWidgetFromMRMLSliceCompositeNode();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode)
{
  Q_D(qMRMLSliceControllerWidget);
  // eventually calls vtkMRMLSliceLogic::ModifiedEvent which
  // eventually calls onSliceLogicModified.
  d->SliceLogic->SetSliceNode(newSliceNode);
  if (newSliceNode && newSliceNode->GetScene())
    {
    this->setMRMLScene(newSliceNode->GetScene());
    }
}

//---------------------------------------------------------------------------
CTK_GET_CPP(qMRMLSliceControllerWidget, vtkMRMLSliceLogic*, sliceLogic, SliceLogic);

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceLogic(vtkMRMLSliceLogic * newSliceLogic)
{
  Q_D(qMRMLSliceControllerWidget);
  if (d->SliceLogic == newSliceLogic)
    {
    return;
    }

  d->qvtkReconnect(d->SliceLogic, newSliceLogic, vtkCommand::ModifiedEvent,
                   d, SLOT(onSliceLogicModifiedEvent()));

  d->SliceLogic = newSliceLogic;

  if (d->SliceLogic && d->SliceLogic->GetMRMLScene())
    {
    this->setMRMLScene(d->SliceLogic->GetMRMLScene());
    }

  d->onSliceLogicModifiedEvent();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceLogics(vtkCollection* sliceLogics)
{
  Q_D(qMRMLSliceControllerWidget);
  d->SliceLogics = sliceLogics;
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setControllerButtonGroup(QButtonGroup* newButtonGroup)
{
  Q_D(qMRMLSliceControllerWidget);

  if (d->ControllerButtonGroup == newButtonGroup)
    {
    return;
    }

  if (d->ControllerButtonGroup)
    {
    // Remove SliceCollapsibleButton from ControllerButtonGroup
    //d->ControllerButtonGroup->removeButton(d->SliceCollapsibleButton);

    // Disconnect widget with buttonGroup
    this->disconnect(d->ControllerButtonGroup, SIGNAL(buttonClicked(int)),
                     d, SLOT(toggleControllerWidgetGroupVisibility()));
    }

  if (newButtonGroup)
    {
    if (newButtonGroup->exclusive())
      {
      logger.error("setControllerButtonGroup - newButtonGroup shouldn't be exclusive - "
                   "See QButtonGroup::setExclusive()");
      }

    // Disconnect sliceCollapsibleButton and  ControllerWidgetGroup
    //this->disconnect(d->SliceCollapsibleButton, SIGNAL(clicked()),
    //                 d, SLOT(toggleControllerWidgetGroupVisibility()));

    // Add SliceCollapsibleButton to newButtonGroup
    //newButtonGroup->addButton(d->SliceCollapsibleButton);

    // Connect widget with buttonGroup
    //this->connect(newButtonGroup, SIGNAL(buttonClicked(int)),
    //              d, SLOT(toggleControllerWidgetGroupVisibility()));
    }
  else
    {
    //this->connect(d->SliceCollapsibleButton, SIGNAL(clicked()),
    //              d, SLOT(toggleControllerWidgetGroupVisibility()));
    }

  d->ControllerButtonGroup = newButtonGroup;
}

//---------------------------------------------------------------------------
CTK_GET_CPP(qMRMLSliceControllerWidget, vtkMRMLSliceCompositeNode*,
            mrmlSliceCompositeNode, MRMLSliceCompositeNode);

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceViewSize(const QSize& newSize)
{
  Q_D(qMRMLSliceControllerWidget);
  logger.trace(QString("setSliceViewSize - newSize(%1, %2)").
               arg(newSize.width()).arg(newSize.height()));
  d->ViewSize = newSize;
  if (!d->SliceLogic)
    {
    return;
    }
  d->SliceLogic->ResizeSliceNode(newSize.width(), newSize.height());
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceViewName(const QString& newSliceViewName)
{
  Q_D(qMRMLSliceControllerWidget);

  if (d->MRMLSliceNode)
    {
    logger.error("setSliceViewName should be called before setMRMLSliceNode !");
    return;
    }

  if (d->SliceViewName == newSliceViewName)
    {
    return;
    }

  QPalette palette;

  // If name matches either 'Red, 'Green' or 'Yellow' set the
  // corresponding color (legacy colors). If the name matches an SVG color keyword
  // http://www.w3.org/TR/SVG/types.html#ColorKeywords, then use that.
  // Set Orange otherwise.
  QColor buttonColor;
  buttonColor.setRgbF(0.882352941176, 0.439215686275, 0.0705882352941); // orange

  QColor namedColor;
  namedColor.setNamedColor( newSliceViewName.toLower() );

  if (newSliceViewName == "Red")
    {
    buttonColor.setRgbF(0.952941176471, 0.290196078431, 0.2); // red
    }
  else if (newSliceViewName == "Green")
    {
    buttonColor.setRgbF(0.43137254902, 0.690196078431, 0.294117647059); // green
    }
  else if (newSliceViewName == "Yellow")
    {
    buttonColor.setRgbF(0.929411764706, 0.835294117647, 0.298039215686); // yellow
    }
  else if (namedColor.isValid())
    {
    buttonColor = namedColor;
    }
  //d->SliceCollapsibleButton->setPalette(QPalette(buttonColor));
  palette = d->SliceOffsetSlider->palette();
  QLinearGradient gradient(QPointF(0.,0.), QPointF(0.,1.));
  gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
  gradient.setColorAt(0., buttonColor.lighter(102));
  gradient.setColorAt(0.1, buttonColor);
  gradient.setColorAt(0.8, buttonColor.darker(102));
  gradient.setColorAt(1., buttonColor.darker(108));
  palette.setBrush(QPalette::Window, gradient);
  this->setAutoFillBackground(true);
  this->setPalette(palette);

  if (d->SliceLogic)
    {
    d->SliceLogic->SetName(newSliceViewName.toLatin1());
    }

  d->SliceViewName = newSliceViewName;
}

//---------------------------------------------------------------------------
CTK_GET_CPP(qMRMLSliceControllerWidget, QString, sliceViewName, SliceViewName);

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setImageData(vtkImageData* newImageData)
{
  Q_D(qMRMLSliceControllerWidget);

  if (d->ImageData == newImageData)
    {
    return;
    }

  d->ImageData = newImageData;

  emit this->imageDataChanged(d->ImageData);
}

//---------------------------------------------------------------------------
CTK_GET_CPP(qMRMLSliceControllerWidget, vtkImageData*, imageData, ImageData);

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceOffsetRange(double min, double max)
{
  Q_D(qMRMLSliceControllerWidget);
  d->SliceOffsetSlider->setRange(min, max);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceOffsetResolution(double resolution)
{
  Q_D(qMRMLSliceControllerWidget);
  d->SliceOffsetSlider->setSingleStep(resolution);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceOffsetValue(double offset)
{
  Q_D(qMRMLSliceControllerWidget);
  if (!d->SliceLogic)
    {
    return;
    }
  logger.trace(QString("setSliceOffsetValue: %1").arg(offset));
  d->SliceLogic->StartSliceOffsetInteraction();
  d->SliceLogic->SetSliceOffset(offset);
  d->SliceLogic->EndSliceOffsetInteraction();
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::trackSliceOffsetValue(double offset)
{
  Q_D(qMRMLSliceControllerWidget);
  if (!d->SliceLogic)
    {
    return;
    }
  logger.trace(QString("trackSliceOffsetValue"));
  d->SliceLogic->StartSliceOffsetInteraction();
  d->SliceLogic->SetSliceOffset(offset);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::fitSliceToBackground()
{
  Q_D(qMRMLSliceControllerWidget);
  logger.trace(QString("fitSliceToBackground"));
  if (!d->SliceLogics)
    {
    d->fitSliceToBackground(d->SliceLogic);
    return;
    }
  vtkMRMLSliceLogic* sliceLogic = 0;
  vtkCollectionSimpleIterator it;
  for (d->SliceLogics->InitTraversal(it);
       (sliceLogic = static_cast<vtkMRMLSliceLogic*>(
         d->SliceLogics->GetNextItemAsObject(it)));)
    {
    if (sliceLogic == d->SliceLogic || this->isLinked())
      {
      d->fitSliceToBackground(sliceLogic);
      }
    }
}

//---------------------------------------------------------------------------
QString qMRMLSliceControllerWidget::sliceOrientation()const
{
  Q_D(const qMRMLSliceControllerWidget);
  return d->SliceOrientationSelector->currentText();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceOrientation(const QString& orientation)
{
  Q_D(qMRMLSliceControllerWidget);

#ifndef QT_NO_DEBUG
  QStringList expectedOrientation;
  expectedOrientation << "Axial" << "Sagittal" << "Coronal" << "Reformat";
  Q_ASSERT(expectedOrientation.contains(orientation));
#endif

  if (!d->MRMLSliceNode || !d->MRMLSliceCompositeNode)
    {
    return;
    }

  d->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::OrientationFlag); 
  d->MRMLSliceNode->SetOrientation(orientation.toLatin1());
  d->SliceLogic->EndSliceNodeInteraction();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceVisible(bool visible)
{
  Q_D(qMRMLSliceControllerWidget);

  if (!d->MRMLSliceNode  || !d->MRMLSliceCompositeNode || !this->mrmlScene())
    {
    return;
    }
  vtkMRMLLayoutNode *layoutNode = vtkMRMLLayoutNode::SafeDownCast(
    this->mrmlScene()->GetNthNodeByClass(0,"vtkMRMLLayoutNode"));
  if (d->MRMLSliceCompositeNode->GetLinkedControl())
    {
    vtkCollection* sliceNodes = this->mrmlScene()->GetNodesByClass("vtkMRMLSliceNode");
    if (!sliceNodes)
      {
      return;
      }
    vtkMRMLSliceNode* sliceNode = 0;
    for(sliceNodes->InitTraversal();
        (sliceNode = vtkMRMLSliceNode::SafeDownCast(sliceNodes->GetNextItemAsObject()));)
      {
      // if compareview, send only compareview slices to 3D main viewer; otherwise,
      // only send red, yellow, and green to 3D main viewer.
      if (layoutNode ? ((QString(sliceNode->GetLayoutName()) == "Compare" &&
           layoutNode->GetViewArrangement() == vtkMRMLLayoutNode::SlicerLayoutCompareView) ||
          (QString(sliceNode->GetLayoutName()) != "Compare" &&
           layoutNode->GetViewArrangement() != vtkMRMLLayoutNode::SlicerLayoutCompareView))
          : true)
        {
        this->mrmlScene()->SaveStateForUndo(sliceNode);
        sliceNode->SetSliceVisible(visible);
        }
      }
    sliceNodes->Delete();
    }
  else
    {
    this->mrmlScene()->SaveStateForUndo(d->MRMLSliceNode);
    d->MRMLSliceNode->SetSliceVisible(visible);
    }
}

//---------------------------------------------------------------------------
bool qMRMLSliceControllerWidget::isLinked()const
{
  Q_D(const qMRMLSliceControllerWidget);
  // It is not really an assert here, what could have happen is that the
  // mrml slice composite node LinkedControl property has been changed but the
  // modified event has not been yet fired, updateWidgetFromMRMLSliceCompositeNode not having been
  // called yet, the slicelinkbutton state is not uptodate.
  //Q_ASSERT(!d->MRMLSliceCompositeNode ||
  //        d->MRMLSliceCompositeNode->GetLinkedControl() ==
  //         d->SliceLinkButton->isChecked());
  return d->MRMLSliceCompositeNode ? d->MRMLSliceCompositeNode->GetLinkedControl() : d->SliceLinkButton->isChecked();
}

//---------------------------------------------------------------------------
bool qMRMLSliceControllerWidget::isCompareView()const
{
  Q_D(const qMRMLSliceControllerWidget);
  return d->MRMLSliceNode && QString(d->MRMLSliceNode->GetLayoutName()).startsWith("Compare");
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceLink(bool linked)
{
  vtkCollection* sliceCompositeNodes = this->mrmlScene() ?
    this->mrmlScene()->GetNodesByClass("vtkMRMLSliceCompositeNode") : 0;
  if (!sliceCompositeNodes)
    {
    return;
    }
  vtkMRMLSliceCompositeNode* sliceCompositeNode = 0;
  for(sliceCompositeNodes->InitTraversal();
      (sliceCompositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(
        sliceCompositeNodes->GetNextItemAsObject()));)
    {
    sliceCompositeNode->SetLinkedControl(linked);
    }
  sliceCompositeNodes->Delete();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setHotLinked(bool linked)
{
  vtkCollection* sliceCompositeNodes = this->mrmlScene() ?
    this->mrmlScene()->GetNodesByClass("vtkMRMLSliceCompositeNode") : 0;
  if (!sliceCompositeNodes)
    {
    return;
    }
  vtkMRMLSliceCompositeNode* sliceCompositeNode = 0;
  for(sliceCompositeNodes->InitTraversal();
      (sliceCompositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(
        sliceCompositeNodes->GetNextItemAsObject()));)
    {
    sliceCompositeNode->SetHotLinkedControl(linked);
    }
  sliceCompositeNodes->Delete();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::rotateSliceToBackground()
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceNode");
  if(!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    vtkMRMLSliceLogic* nodeLogic = d->sliceNodeLogic(node);
    if (nodeLogic && (nodeLogic == d->SliceLogic.GetPointer() || this->isLinked()))
      {
      vtkMRMLVolumeNode* backgroundNode = nodeLogic->GetLayerVolumeNode(0);
      node->RotateToVolumePlane(backgroundNode);
      }
    }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setLabelOpacity(double opacity)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceCompositeNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    if (node == d->MRMLSliceCompositeNode || this->isLinked())
      {
      node->SetLabelOpacity(opacity);
      }
    }
  // LabelOpacityToggleButton won't fire the clicked(bool) signal here because
  // we change its check state programatically.
  d->LabelOpacityToggleButton->setChecked(opacity == 0.);
  if (opacity != 0.)
    {
    d->LastLabelOpacity = opacity;
    }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::showLabelOutline(bool show)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    if (node == d->MRMLSliceNode || this->isLinked())
      {
      node->SetUseLabelOutline(show);
      }
    }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::showReformatWidget(bool show)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    // When slice nodes are linked, only allow one slice node's reformat widget to be on at a time
    // If slice node's reformat widget was on, just turn all of them off
    // If slice node's reformat widget was off, turn it on and turn all the other ones off
    if (node == d->MRMLSliceNode || this->isLinked())
      {
      node->SetWidgetVisible(show && node == d->MRMLSliceNode);
      }
    }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setCompositing(int mode)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceCompositeNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    // When slice nodes are linked, only allow one slice node's reformat widget to be on at a time
    // If slice node's reformat widget was on, just turn all of them off
    // If slice node's reformat widget was off, turn it on and turn all the other ones off
    if (node == d->MRMLSliceCompositeNode || this->isLinked())
      {
      node->SetCompositing(mode);
      }
    }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setCompositingToAlphaBlend()
{
  this->setCompositing(vtkMRMLSliceCompositeNode::Alpha);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setCompositingToReverseAlphaBlend()
{
  this->setCompositing(vtkMRMLSliceCompositeNode::ReverseAlpha);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setCompositingToAdd()
{
  this->setCompositing(vtkMRMLSliceCompositeNode::Add);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setCompositingToSubtract()
{
  this->setCompositing(vtkMRMLSliceCompositeNode::Subtract);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceSpacingMode(bool automatic)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    if (node == d->MRMLSliceNode || this->isLinked())
      {
      if (automatic)
        {
        node->SetSliceSpacingModeToAutomatic();
        }
      else
        {
        node->SetSliceSpacingModeToPrescribed();
        }
      }
    }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceSpacing(double sliceSpacing)
{
  Q_D(qMRMLSliceControllerWidget);
  this->setSliceSpacingMode(false);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    if (node == d->MRMLSliceNode || this->isLinked())
      {
      double *current = node->GetPrescribedSliceSpacing();
      double spacing[3];
      spacing[0] = current[0];
      spacing[1] = current[1];
      spacing[2] = sliceSpacing;
      node->SetPrescribedSliceSpacing(spacing);
      }
    }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setLightbox(int rows, int columns)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    // only coronal layouts can be lightboxes ?
    if (node == d->MRMLSliceNode ||
        (this->isLinked() && this->isCompareView() &&
         QString(node->GetLayoutName()).startsWith("Compare")))
      {
      node->SetLayoutGrid(rows, columns);
      vtkMRMLSliceLogic* sliceLogic = d->sliceNodeLogic(node);
      if (sliceLogic)
        {
        // As the size (dimension+fov) of the slicenode depends on the
        // viewport size and the layout, we need to recompute the size
        sliceLogic->ResizeSliceNode(d->ViewSize.width(), d->ViewSize.height());
        }
      }
    }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setLightboxTo1x1()
{
  this->setLightbox(1,1);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setLightboxTo1x2()
{
  this->setLightbox(1, 2);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setLightboxTo1x3()
{
  this->setLightbox(1, 3);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setLightboxTo1x4()
{
  this->setLightbox(1, 4);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setLightboxTo1x6()
{
  this->setLightbox(1, 6);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setLightboxTo1x8()
{
  this->setLightbox(1, 8);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setLightboxTo2x2()
{
  this->setLightbox(2, 2);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setLightboxTo3x3()
{
  this->setLightbox(3, 3);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setLightboxTo6x6()
{
  this->setLightbox(6, 6);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setForegroundInterpolation(bool interpolate)
{
  Q_D(qMRMLSliceControllerWidget);
  if (!d->SliceLogics)
    {
    d->setForegroundInterpolation(d->SliceLogic, interpolate);
    return;
    }
  vtkMRMLSliceLogic* sliceLogic = 0;
  vtkCollectionSimpleIterator it;
  for (d->SliceLogics->InitTraversal(it);(sliceLogic = static_cast<vtkMRMLSliceLogic*>(
                                   d->SliceLogics->GetNextItemAsObject(it)));)
    {
    if (sliceLogic == d->SliceLogic || this->isLinked())
      {
      d->setForegroundInterpolation(sliceLogic, interpolate);
      }
    }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setBackgroundInterpolation(bool interpolate)
{
  Q_D(qMRMLSliceControllerWidget);
  if (!d->SliceLogics)
    {
    d->setBackgroundInterpolation(d->SliceLogic, interpolate);
    return;
    }
  vtkMRMLSliceLogic* sliceLogic = 0;
  vtkCollectionSimpleIterator it;
  for (d->SliceLogics->InitTraversal(it);(sliceLogic = static_cast<vtkMRMLSliceLogic*>(
                                   d->SliceLogics->GetNextItemAsObject(it)));)
    {
    if (sliceLogic == d->SliceLogic || this->isLinked())
      {
      d->setBackgroundInterpolation(sliceLogic, interpolate);
      }
    }
}
