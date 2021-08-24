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
#include <QButtonGroup>
#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QWidgetAction>

// CTK includes
#include <ctkDoubleSlider.h>
#include <ctkMessageBox.h>
#include <ctkPopupWidget.h>
#include <ctkSignalMapper.h>
#include <ctkDoubleSpinBox.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLSliceControllerWidget_p.h"
#include "qMRMLSliderWidget.h"

// MRMLLogic includes
#include <vtkMRMLSliceLayerLogic.h>

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkMRMLSegmentationDisplayNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>
#include <vtkMRMLUnitNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkStringArray.h>

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qMRMLSliceControllerWidgetPrivate::qMRMLSliceControllerWidgetPrivate(qMRMLSliceControllerWidget& object)
  : Superclass(object)
{
  this->SliceLogic = nullptr;
  this->MRMLSliceCompositeNode = nullptr;
  this->SliceLogics = nullptr;

  this->ControllerButtonGroup = nullptr;

  qMRMLOrientation axialOrientation = {qMRMLSliceControllerWidget::tr("S: "), qMRMLSliceControllerWidget::tr("I <-----> S")};
  qMRMLOrientation sagittalOrientation = {qMRMLSliceControllerWidget::tr("R: "), qMRMLSliceControllerWidget::tr("L <-----> R")};
  qMRMLOrientation coronalOrientation = {qMRMLSliceControllerWidget::tr("A: "), qMRMLSliceControllerWidget::tr("P <-----> A")};
  qMRMLOrientation obliqueOrientation = {"", qMRMLSliceControllerWidget::tr("Oblique")};

  this->SliceOrientationToDescription["Axial"] = axialOrientation;
  this->SliceOrientationToDescription["Sagittal"] = sagittalOrientation;
  this->SliceOrientationToDescription["Coronal"] = coronalOrientation;
  this->SliceOrientationToDescription["Reformat"] = obliqueOrientation;

  this->LastLabelMapOpacity = 1.;
  this->LastForegroundOpacity = 1.;
  this->LastBackgroundOpacity = 1.;

  this->FitToWindowToolButton = nullptr;
  this->SliceOffsetSlider = nullptr;

  this->LightboxMenu = nullptr;
  this->CompositingMenu = nullptr;
  this->SliceSpacingMenu = nullptr;
  this->SliceModelMenu = nullptr;
  this->SegmentationMenu = nullptr;
  this->LabelMapMenu = nullptr;
  this->OrientationMarkerMenu = nullptr;
  this->RulerMenu = nullptr;

  this->SliceSpacingSpinBox = nullptr;
  this->SliceFOVSpinBox = nullptr;
  this->LightBoxRowsSpinBox = nullptr;
  this->LightBoxColumnsSpinBox = nullptr;

  this->SliceModelFOVXSpinBox = nullptr;
  this->SliceModelFOVYSpinBox = nullptr;

  this->SliceModelOriginXSpinBox = nullptr;
  this->SliceModelOriginYSpinBox = nullptr;

  this->SliceModelDimensionXSpinBox = nullptr;
  this->SliceModelDimensionYSpinBox = nullptr;

}

//---------------------------------------------------------------------------
qMRMLSliceControllerWidgetPrivate::~qMRMLSliceControllerWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setColor(QColor barColor)
{
  //this->SliceOffsetSlider->spinBox()->setAutoFillBackground(true);
  this->Superclass::setColor(barColor);
  QPalette spinBoxPalette( this->SliceOffsetSlider->spinBox()->palette());
  spinBoxPalette.setColor(QPalette::Base, barColor.lighter(130));
  this->SliceOffsetSlider->spinBox()->setPalette(spinBoxPalette);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupPopupUi()
{
  Q_Q(qMRMLSliceControllerWidget);

  this->Superclass::setupPopupUi();
  this->Ui_qMRMLSliceControllerWidget::setupUi(this->PopupWidget);

  this->SegmentationOpacitySlider->spinBox()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  this->LabelMapOpacitySlider->spinBox()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  this->ForegroundOpacitySlider->spinBox()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  this->BackgroundOpacitySlider->spinBox()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  this->ForegroundOpacitySlider->slider()->setOrientation(Qt::Vertical);
  this->BackgroundOpacitySlider->slider()->setOrientation(Qt::Vertical);

  this->BackgroundOpacitySlider->popup()->setHideDelay(400);

  this->BackgroundOpacitySlider->popup()->setAlignment(Qt::AlignBottom | Qt::AlignLeft);

  int popupHeight = this->PopupWidget->sizeHint().height() / 2;
  this->BackgroundOpacitySlider->popup()->setFixedHeight(popupHeight);

  QGridLayout* popupLayout =
    qobject_cast<QGridLayout*>(this->PopupWidget->layout());
  popupLayout->addWidget(this->ForegroundOpacitySlider->spinBox(), 3, 2);
  this->connect(this->MoreButton, SIGNAL(toggled(bool)),
                this->ForegroundOpacitySlider->spinBox(), SLOT(setVisible(bool)));
  this->connect(this->ForegroundComboBox, SIGNAL(currentNodeChanged(bool)),
                this->ForegroundOpacitySlider->spinBox(), SLOT(setEnabled(bool)));
  // Set selector attributes
  // Background and Foreground volume selectors can display LabelMap volumes. No
  // need to add the LabelMap attribute for them.
  // Note: the min width is currently set in the UI file directly
  //// Set the slice controller widgets a min width.
  //int volumeSelectorMinWidth = this->LabelMapComboBox->fontMetrics().width("Xxxxxxxx") + 20;
  //this->SliceOrientationSelector->setMinimumWidth(volumeSelectorMinWidth);
  //this->LabelMapComboBox->setMinimumWidth(volumeSelectorMinWidth);
  //this->BackgroundComboBox->setMinimumWidth(volumeSelectorMinWidth);
  //this->ForegroundComboBox->setMinimumWidth(volumeSelectorMinWidth);

  // Populate the Linked menu
  this->setupLinkedOptionsMenu();

  // Populate the reformat menu
  this->setupReformatOptionsMenu();

  // Connect more button
  this->connect(this->MoreButton, SIGNAL(toggled(bool)),
                q, SLOT(moveBackgroundComboBox(bool)));
  this->connect(this->MoreButton, SIGNAL(toggled(bool)),
                q, SLOT(updateSegmentationControlsVisibility()));

  // Connect link toggle
  this->connect(this->SliceLinkButton, SIGNAL(clicked(bool)),
                q, SLOT(setSliceLink(bool)));

  // Connect Orientation selector
  this->connect(this->SliceOrientationSelector, SIGNAL(currentIndexChanged(QString)),
                q, SLOT(setSliceOrientation(QString)));

  QObject::connect(this->actionShow_in_3D, SIGNAL(toggled(bool)),
                   q, SLOT(setSliceVisible(bool)));
  QObject::connect(this->actionFit_to_window, SIGNAL(triggered()),
                   q, SLOT(fitSliceToBackground()));
  QObject::connect(this->actionRotate_to_volume_plane, SIGNAL(triggered()),
                   q, SLOT(rotateSliceToLowestVolumeAxes()));
  QObject::connect(this->actionShow_reformat_widget, SIGNAL(triggered(bool)),
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

  QObject::connect(this->actionSliceModelModeVolumes, SIGNAL(triggered()),
                   q, SLOT(setSliceModelModeVolumes()));
  QObject::connect(this->actionSliceModelMode2D, SIGNAL(triggered()),
                   q, SLOT(setSliceModelMode2D()));
  QObject::connect(this->actionSliceModelMode2D_Volumes, SIGNAL(triggered()),
                   q, SLOT(setSliceModelMode2D_Volumes()));
  QObject::connect(this->actionSliceModelModeVolumes_2D, SIGNAL(triggered()),
                   q, SLOT(setSliceModelModeVolumes_2D()));
  //QObject::connect(this->actionSliceModelModeCustom, SIGNAL(triggered()),
  //                 q, SLOT(setSliceModelModeCustom()));

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
  this->setupLightboxMenu();
  this->setupCompositingMenu();
  this->setupSliceSpacingMenu();
  this->setupSliceModelMenu();
  this->setupSegmentationMenu();
  this->setupLabelMapMenu();
  this->setupOrientationMarkerMenu();
  this->setupRulerMenu();

  // Visibility column
  this->connect(this->actionSegmentationVisibility, SIGNAL(triggered(bool)),
                q, SLOT(setSegmentationHidden(bool)));
  this->connect(this->actionLabelMapVisibility, SIGNAL(triggered(bool)),
                q, SLOT(setLabelMapHidden(bool)));
  this->connect(this->actionForegroundVisibility, SIGNAL(triggered(bool)),
                q, SLOT(setForegroundHidden(bool)));
  this->connect(this->actionBackgroundVisibility, SIGNAL(triggered(bool)),
                q, SLOT(setBackgroundHidden(bool)));

  // Opacity column
  this->connect(this->SegmentationOpacitySlider, SIGNAL(valueChanged(double)),
                q, SLOT(setSegmentationOpacity(double)));
  this->connect(this->LabelMapOpacitySlider, SIGNAL(valueChanged(double)),
                q, SLOT(setLabelMapOpacity(double)));
  this->connect(this->ForegroundOpacitySlider, SIGNAL(valueChanged(double)),
                q, SLOT(setForegroundOpacity(double)));
  this->connect(this->BackgroundOpacitySlider, SIGNAL(valueChanged(double)),
                q, SLOT(setBackgroundOpacity(double)));

  // Interpolation column
  QObject::connect(this->actionSegmentationOutlineFill, SIGNAL(triggered()),
                   q, SLOT(toggleSegmentationOutlineFill()));
  QObject::connect(this->actionLabelMapOutline, SIGNAL(toggled(bool)),
                   q, SLOT(showLabelOutline(bool)));
  QObject::connect(this->actionForegroundInterpolation, SIGNAL(toggled(bool)),
                   q, SLOT(setForegroundInterpolation(bool)));
  QObject::connect(this->actionBackgroundInterpolation, SIGNAL(toggled(bool)),
                   q, SLOT(setBackgroundInterpolation(bool)));

  // Connect Segmentation selector
  this->connect(this->SegmentSelectorWidget, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onSegmentationNodeSelected(vtkMRMLNode*)));
  //this->connect(this->SegmentSelectorWidget, SIGNAL(currentNodeChanged(bool)),
  //              this->actionSegmentationVisibility, SLOT(setEnabled(bool)));
  this->connect(this->SegmentSelectorWidget, SIGNAL(currentNodeChanged(bool)),
                this->actionSegmentationOutlineFill, SLOT(setEnabled(bool)));
  this->connect(this->SegmentSelectorWidget, SIGNAL(segmentSelectionChanged(QStringList)),
                this, SLOT(onSegmentVisibilitySelectionChanged(QStringList)));

  // Connect Label map selector
  this->connect(this->LabelMapComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onLabelMapNodeSelected(vtkMRMLNode*)));
  // when the user select an entry already selected, we want to synchronize with the linked
  // slice logics as they might not have the same entry selected
  this->connect(this->LabelMapComboBox, SIGNAL(nodeActivated(vtkMRMLNode*)),
                SLOT(onLabelMapNodeSelected(vtkMRMLNode*)));
  //this->connect(this->LabelMapComboBox, SIGNAL(currentNodeChanged(bool)),
  //              this->actionLabelMapVisibility, SLOT(setEnabled(bool)));
  this->connect(this->LabelMapComboBox, SIGNAL(currentNodeChanged(bool)),
                this->actionLabelMapOutline, SLOT(setEnabled(bool)));

  // Connect Foreground layer selector
  this->connect(this->ForegroundComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onForegroundLayerNodeSelected(vtkMRMLNode*)));
  // when the user select an entry already selected, we want to synchronize with the linked
  // slice logics as they might not have the same entry selected
  this->connect(this->ForegroundComboBox, SIGNAL(nodeActivated(vtkMRMLNode*)),
                SLOT(onForegroundLayerNodeSelected(vtkMRMLNode*)));
  //this->connect(this->ForegroundComboBox, SIGNAL(currentNodeChanged(bool)),
  //              this->actionForegroundVisibility, SLOT(setEnabled(bool)));
  this->connect(this->ForegroundComboBox, SIGNAL(currentNodeChanged(bool)),
                this->actionForegroundInterpolation, SLOT(setEnabled(bool)));

  // Connect Background layer selector
  this->connect(this->BackgroundComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onBackgroundLayerNodeSelected(vtkMRMLNode*)));
  // when the user select an entry already selected, we want to synchronize with the linked
  // slice logics as they might not have the same entry selected
  this->connect(this->BackgroundComboBox, SIGNAL(nodeActivated(vtkMRMLNode*)),
               SLOT(onBackgroundLayerNodeSelected(vtkMRMLNode*)));
  //this->connect(this->BackgroundComboBox, SIGNAL(currentNodeChanged(bool)),
  //              this->actionBackgroundVisibility, SLOT(setEnabled(bool)));
  this->connect(this->BackgroundComboBox, SIGNAL(currentNodeChanged(bool)),
                this->actionBackgroundInterpolation, SLOT(setEnabled(bool)));

  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->SegmentSelectorWidget, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->LabelMapComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->ForegroundComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->BackgroundComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));

  // Connect actions to buttons
  this->SliceVisibilityButton->setDefaultAction(this->actionShow_in_3D);
  this->LightBoxToolButton->setMenu(this->LightboxMenu);
  this->ShowReformatWidgetToolButton->setDefaultAction(this->actionShow_reformat_widget);

  this->SliceCompositeButton->setMenu(this->CompositingMenu);
  this->SliceSpacingButton->setMenu(this->SliceSpacingMenu);
  this->SliceVisibilityButton->setMenu(this->SliceModelMenu);
  this->SliceRotateToVolumePlaneButton->setDefaultAction(
    this->actionRotate_to_volume_plane);
  this->SliceMoreOptionButton->setVisible(false);
  //this->setupMoreOptionsMenu();

  this->SegmentationVisibilityButton->setDefaultAction(this->actionSegmentationVisibility);
  this->SegmentationVisibilityButton->setMenu(this->SegmentationMenu);

  this->LabelMapVisibilityButton->setDefaultAction(this->actionLabelMapVisibility);
  this->LabelMapVisibilityButton->setMenu(this->LabelMapMenu);

  this->SegmentationOutlineButton->setDefaultAction(this->actionSegmentationOutlineFill);
  this->LabelMapOutlineButton->setDefaultAction(this->actionLabelMapOutline);
  this->ForegroundInterpolationButton->setDefaultAction(this->actionForegroundInterpolation);
  this->BackgroundInterpolationButton->setDefaultAction(this->actionBackgroundInterpolation);

}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::init()
{
  Q_Q(qMRMLSliceControllerWidget);

  this->Superclass::init();

  // Fit to Window icon
  // Used to be in popup
  // <item>
  //  <widget class="QToolButton" name="FitToWindowToolButton">
  //   <property name="toolTip">
  //    <string>Adjust the Slice Viewer's field of view to match the extent of lowest non-None volume layer (bg, then fg, then label).</string>
  //   </property>
  //   <property name="icon">
  //    <iconset resource="../qMRMLWidgets.qrc">
  //     <normaloff>:/Icons/SlicesFitToWindow.png</normaloff>:/Icons/SlicesFitToWindow.png</iconset>
  //   </property>
  //   <property name="autoRaise">
  //    <bool>true</bool>
  //   </property>
  //  </widget>
  // </item>
  this->FitToWindowToolButton = new QToolButton(q);
  this->FitToWindowToolButton->setObjectName("FitToWindowToolButton");
  //this->FitToWindowToolButton->setToolTip(tr("Adjust the Slice Viewer's field of view to match the extent of lowest non-None volume layer (bg, then fg, then label)."));
  //QIcon fitToWindowIcon(":/Icons/SlicesFitToWindow.png");
  //this->FitToWindowToolButton->setIcon(fitToWindowIcon);
  this->FitToWindowToolButton->setAutoRaise(true);
  this->FitToWindowToolButton->setDefaultAction(this->actionFit_to_window);
  this->FitToWindowToolButton->setFixedSize(15, 15);
  this->BarLayout->insertWidget(2, this->FitToWindowToolButton);

  this->SliceOffsetSlider = new qMRMLSliderWidget(q);
  this->SliceOffsetSlider->setObjectName("SliceOffsetSlider");
  this->SliceOffsetSlider->setTracking(false);
  this->SliceOffsetSlider->setToolTip(qMRMLSliceControllerWidget::tr("Slice distance from RAS origin"));
  this->SliceOffsetSlider->setQuantity("length");
  this->SliceOffsetSlider->setUnitAwareProperties(
    qMRMLSliderWidget::Suffix|qMRMLSliderWidget::Precision|qMRMLSliderWidget::Scaling);
  this->SliceOffsetSlider->spinBox()->setDecimalsOption(
    ctkDoubleSpinBox::DecimalsByShortcuts |
    ctkDoubleSpinBox::DecimalsByKey |
    ctkDoubleSpinBox::DecimalsAsMin );

  //this->SliceOffsetSlider->spinBox()->setParent(this->PopupWidget);
  ctkDoubleSpinBox* spinBox = this->SliceOffsetSlider->spinBox();
  spinBox->setFrame(false);
  spinBox->spinBox()->setButtonSymbols(QAbstractSpinBox::NoButtons);
  spinBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Ignored);
  int targetHeight = spinBox->parentWidget()->layout()->sizeHint().height();//setSizeConstraint(QLayout::SetMinimumSize);
  int fontHeight = spinBox->fontMetrics().height();
  qreal heightRatio = static_cast<qreal>(targetHeight - 2) / fontHeight;
  if (heightRatio  < 1.)
    {
    QFont stretchedFont(spinBox->font());
    stretchedFont.setPointSizeF(stretchedFont.pointSizeF() * heightRatio);
    spinBox->setFont(stretchedFont);
    }

  // Connect Slice offset slider
  this->connect(this->SliceOffsetSlider, SIGNAL(valueChanged(double)),
                q, SLOT(setSliceOffsetValue(double)), Qt::QueuedConnection);
  this->connect(this->SliceOffsetSlider, SIGNAL(valueIsChanging(double)),
                q, SLOT(trackSliceOffsetValue(double)), Qt::QueuedConnection);
  this->connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                this->SliceOffsetSlider, SLOT(setMRMLScene(vtkMRMLScene*)));

  this->BarLayout->addWidget(this->SliceOffsetSlider);

  // Move the spinbox in the popup instead of having it in the slider bar
  //dynamic_cast<QGridLayout*>(this->PopupWidget->layout())->addWidget(
  //  this->SliceOffsetSlider->spinBox(), 0, 0, 1, 2);

  // Hide all buttons by default
  this->MoreButton->setChecked(false);

  vtkNew<vtkMRMLSliceLogic> defaultLogic;
  q->setSliceLogic(defaultLogic.GetPointer());
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setAndObserveSelectionNode()
{
  Q_Q(qMRMLSliceControllerWidget);

  vtkMRMLSelectionNode* selectionNode = nullptr;
  vtkMRMLScene* scene = q->mrmlScene();
  if (scene)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(scene->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
    }

  this->qvtkReconnect(this->SelectionNode, selectionNode,
    vtkMRMLSelectionNode::UnitModifiedEvent,
    this, SLOT(updateWidgetFromUnitNode()));
  this->SelectionNode = selectionNode;
  this->updateWidgetFromUnitNode();
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupLinkedOptionsMenu()
{
  Q_Q(qMRMLSliceControllerWidget);
  QMenu* linkedMenu = new QMenu(tr("Linked"), this->SliceLinkButton);
  linkedMenu->setObjectName("linkedMenu");

  linkedMenu->addAction(this->actionHotLinked);

  QObject::connect(this->actionHotLinked, SIGNAL(toggled(bool)),
                   q, SLOT(setHotLinked(bool)));

  this->SliceLinkButton->setMenu(linkedMenu);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupReformatOptionsMenu()
{
  Q_Q(qMRMLSliceControllerWidget);
  QMenu* reformatMenu = new QMenu(tr("Reformat"), this->ShowReformatWidgetToolButton);
  reformatMenu->setObjectName("reformatMenu");

  reformatMenu->addAction(this->actionLockNormalToCamera);

  QObject::connect(this->actionLockNormalToCamera, SIGNAL(triggered(bool)),
                   q, SLOT(lockReformatWidgetToCamera(bool)));

  this->ShowReformatWidgetToolButton->setMenu(reformatMenu);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupLightboxMenu()
{
  // Lightbox View
  this->LightboxMenu = new QMenu(tr("Lightbox view"), this->LightBoxToolButton);
  this->LightboxMenu->setObjectName("LightboxMenu");
  this->LightboxMenu->setIcon(QIcon(":/Icons/LayoutLightboxView.png"));
  this->LightboxMenu->addAction(this->actionLightbox1x1_view);
  this->LightboxMenu->addAction(this->actionLightbox1x2_view);
  this->LightboxMenu->addAction(this->actionLightbox1x3_view);
  this->LightboxMenu->addAction(this->actionLightbox1x4_view);
  this->LightboxMenu->addAction(this->actionLightbox1x6_view);
  this->LightboxMenu->addAction(this->actionLightbox1x8_view);
  this->LightboxMenu->addAction(this->actionLightbox2x2_view);
  this->LightboxMenu->addAction(this->actionLightbox3x3_view);
  this->LightboxMenu->addAction(this->actionLightbox6x6_view);
  QMenu* customLightboxMenu = new QMenu(tr("Custom"), this->LightboxMenu);
  customLightboxMenu->setObjectName("customLightboxMenu");
  QWidget* customLightbox = new QWidget(this->LightboxMenu);
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
  this->LightboxMenu->addMenu(customLightboxMenu);
  QActionGroup* lightboxActionGroup = new QActionGroup(this->LightboxMenu);
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
}
// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupCompositingMenu()
{
  // Compositing
  this->CompositingMenu = new QMenu(tr("Compositing"), this->SliceCompositeButton);
  this->CompositingMenu->setObjectName("CompositingMenu");
  this->CompositingMenu->setIcon(QIcon(":/Icons/SlicesComposite.png"));
  this->CompositingMenu->addAction(this->actionCompositingAlpha_blend);
  this->CompositingMenu->addAction(this->actionCompositingReverse_alpha_blend);
  this->CompositingMenu->addAction(this->actionCompositingAdd);
  this->CompositingMenu->addAction(this->actionCompositingSubtract);
  QActionGroup* compositingGroup = new QActionGroup(this->CompositingMenu);
  compositingGroup->addAction(this->actionCompositingAlpha_blend);
  compositingGroup->addAction(this->actionCompositingReverse_alpha_blend);
  compositingGroup->addAction(this->actionCompositingAdd);
  compositingGroup->addAction(this->actionCompositingSubtract);
}


// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupSliceSpacingMenu()
{
  Q_Q(qMRMLSliceControllerWidget);

  // Spacing mode
  this->SliceSpacingMenu = new QMenu(tr("Slice spacing mode"), this->SliceSpacingButton);
  this->SliceSpacingMenu->setObjectName("SlicerSpacingMenu");
  this->SliceSpacingMenu->setIcon(QIcon(":/Icons/SlicerAutomaticSliceSpacing.png"));
  this->SliceSpacingMenu->addAction(this->actionSliceSpacingModeAutomatic);
  QMenu* sliceSpacingManualMode = new QMenu(tr("Manual spacing"), this->SliceSpacingMenu);
  sliceSpacingManualMode->setObjectName("slicerSpacingManualMode");
  sliceSpacingManualMode->setIcon(QIcon(":/Icon/SlicerManualSliceSpacing.png"));
  this->SliceSpacingSpinBox = new ctkDoubleSpinBox(sliceSpacingManualMode);
  this->SliceSpacingSpinBox->setDecimals(3);
  this->SliceSpacingSpinBox->setRange(0.001, VTK_FLOAT_MAX);
  this->SliceSpacingSpinBox->setSingleStep(0.1);
  this->SliceSpacingSpinBox->setValue(1.);
  QObject::connect(this->SliceSpacingSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setSliceSpacing(double)));
  QWidgetAction* sliceSpacingAction = new QWidgetAction(sliceSpacingManualMode);
  sliceSpacingAction->setDefaultWidget(this->SliceSpacingSpinBox);
  sliceSpacingManualMode->addAction(sliceSpacingAction);
  this->SliceSpacingMenu->addMenu(sliceSpacingManualMode);

  QMenu* sliceFOVMenu = new QMenu(tr("Field of view"), this->SliceSpacingMenu);
  sliceFOVMenu->setObjectName("slicerFOVMenu");
  sliceFOVMenu->setIcon(QIcon(":/Icon/SlicesFieldOfView.png"));
  QWidget* sliceFOVWidget = new QWidget(this->SliceSpacingMenu);
  QHBoxLayout* sliceFOVLayout = new QHBoxLayout(sliceFOVWidget);
  sliceFOVLayout->setContentsMargins(0,0,0,0);
  this->SliceFOVSpinBox = new ctkDoubleSpinBox(sliceFOVWidget);
  this->SliceFOVSpinBox->setRange(0.01, 10000.);
  this->SliceFOVSpinBox->setValue(250.);
  QObject::connect(this->SliceFOVSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setSliceFOV(double)));
  sliceFOVLayout->addWidget(this->SliceFOVSpinBox);
  sliceFOVWidget->setLayout(sliceFOVLayout);
  QWidgetAction* sliceFOVAction = new QWidgetAction(sliceFOVMenu);
  sliceFOVAction->setDefaultWidget(sliceFOVWidget);
  sliceFOVMenu->addAction(sliceFOVAction);
  this->SliceSpacingMenu->addMenu(sliceFOVMenu);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupSliceModelMenu()
{
  Q_Q(qMRMLSliceControllerWidget);

  // Slice model mode
  this->SliceModelMenu = new QMenu(tr("Slice model mode"), this->SliceVisibilityButton);
  this->SliceModelMenu->setIcon(QIcon(":/Icons/SlicerAutomaticSliceSpacing.png"));
  this->SliceModelMenu->addAction(this->actionSliceModelModeVolumes);
  this->SliceModelMenu->addAction(this->actionSliceModelMode2D);
  this->SliceModelMenu->addAction(this->actionSliceModelMode2D_Volumes);
  this->SliceModelMenu->addAction(this->actionSliceModelModeVolumes_2D);
  //this->SliceModelMenu->addAction(this->actionSliceModelModeCustom);

  // TODO add custom sliders
  double UVWExtents[] = {256,256,256};
  double UVWOrigin[] = {0,0,0};
  int UVWDimensions[] = {256,256,256};
  vtkMRMLSliceNode* sliceNode = q->mrmlSliceNode();
  if (sliceNode)
  {
    sliceNode->GetUVWExtents(UVWExtents);
    sliceNode->GetUVWOrigin(UVWOrigin);
    sliceNode->GetUVWDimensions(UVWDimensions);
  }

  QMenu* fovSliceModelMenu = new QMenu(tr("Manual FOV"), this->SliceModelMenu);
  QWidget* fovSliceModel = new QWidget(this->SliceModelMenu);
  QHBoxLayout* fovSliceModelLayout = new QHBoxLayout(fovSliceModel);

  this->SliceModelFOVXSpinBox = new ctkDoubleSpinBox(fovSliceModel);
  this->SliceModelFOVXSpinBox->setRange(0.01, 10000.);
  this->SliceModelFOVXSpinBox->setValue(UVWExtents[0]);
  QObject::connect(this->SliceModelFOVXSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setSliceModelFOVX(double)));

  this->SliceModelFOVYSpinBox = new ctkDoubleSpinBox(fovSliceModel);
  this->SliceModelFOVYSpinBox->setRange(0.01, 10000.);
  this->SliceModelFOVYSpinBox->setValue(UVWExtents[1]);
  QObject::connect(this->SliceModelFOVYSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setSliceModelFOVY(double)));

  fovSliceModelLayout->addWidget(this->SliceModelFOVXSpinBox);
  fovSliceModelLayout->addWidget(this->SliceModelFOVYSpinBox);
  fovSliceModel->setLayout(fovSliceModelLayout);

  QWidgetAction* fovSliceModelAction = new QWidgetAction(fovSliceModel);
  fovSliceModelAction->setDefaultWidget(fovSliceModel);
  fovSliceModelMenu->addAction(fovSliceModelAction);
  this->SliceModelMenu->addMenu(fovSliceModelMenu);

  QMenu* dimensionsSliceModelMenu = new QMenu(tr("Manual Dimensions"), this->SliceModelMenu);
  QWidget* dimensionsSliceModel = new QWidget(this->SliceModelMenu);
  QHBoxLayout* dimensionsSliceModelLayout = new QHBoxLayout(dimensionsSliceModel);

  this->SliceModelDimensionXSpinBox = new QSpinBox(dimensionsSliceModel);
  this->SliceModelDimensionXSpinBox->setRange(1, 2000);
  this->SliceModelDimensionXSpinBox->setValue(UVWDimensions[0]);
  QObject::connect(this->SliceModelDimensionXSpinBox, SIGNAL(valueChanged(int)),
                   q, SLOT(setSliceModelDimensionX(int)));

  this->SliceModelDimensionYSpinBox = new QSpinBox(dimensionsSliceModel);
  this->SliceModelDimensionYSpinBox->setRange(1, 2000);
  this->SliceModelDimensionYSpinBox->setValue(UVWDimensions[1]);
  QObject::connect(this->SliceModelDimensionYSpinBox, SIGNAL(valueChanged(int)),
                   q, SLOT(setSliceModelDimensionY(int)));

  dimensionsSliceModelLayout->addWidget(this->SliceModelDimensionXSpinBox);
  dimensionsSliceModelLayout->addWidget(this->SliceModelDimensionYSpinBox);
  dimensionsSliceModel->setLayout(dimensionsSliceModelLayout);

  QWidgetAction* dimesnionsSliceModelAction = new QWidgetAction(dimensionsSliceModel);
  dimesnionsSliceModelAction->setDefaultWidget(dimensionsSliceModel);
  dimensionsSliceModelMenu->addAction(dimesnionsSliceModelAction);
  this->SliceModelMenu->addMenu(dimensionsSliceModelMenu);

  QMenu* originSliceModelMenu = new QMenu(tr("Manual Origin"), this->SliceModelMenu);
  QWidget* originSliceModel = new QWidget(this->SliceModelMenu);
  QHBoxLayout* originSliceModelLayout = new QHBoxLayout(originSliceModel);

  this->SliceModelOriginXSpinBox = new ctkDoubleSpinBox(originSliceModel);
  this->SliceModelOriginXSpinBox->setRange(-1000., 1000.);
  this->SliceModelOriginXSpinBox->setValue(UVWOrigin[0]);
  QObject::connect(this->SliceModelOriginXSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setSliceModelOriginX(double)));

  this->SliceModelOriginYSpinBox = new ctkDoubleSpinBox(originSliceModel);
  this->SliceModelOriginYSpinBox->setRange(-1000, 1000.);
  this->SliceModelOriginYSpinBox->setValue(UVWOrigin[1]);
  QObject::connect(this->SliceModelOriginYSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setSliceModelOriginY(double)));

  originSliceModelLayout->addWidget(this->SliceModelOriginXSpinBox);
  originSliceModelLayout->addWidget(this->SliceModelOriginYSpinBox);
  originSliceModel->setLayout(originSliceModelLayout);

  QWidgetAction* originSliceModelAction = new QWidgetAction(originSliceModel);
  originSliceModelAction->setDefaultWidget(originSliceModel);
  originSliceModelMenu->addAction(originSliceModelAction);
  this->SliceModelMenu->addMenu(originSliceModelMenu);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupSegmentationMenu()
{
  this->SegmentationMenu = new QMenu(tr("Segmentation"), this->SegmentationVisibilityButton);
  QWidgetAction* opacityAction = new QWidgetAction(this->SegmentationOpacitySlider);
  opacityAction->setDefaultWidget(this->SegmentationOpacitySlider->slider());
  this->SegmentationMenu->addAction(opacityAction);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupLabelMapMenu()
{
  this->LabelMapMenu = new QMenu(tr("LabelMap"), this->LabelMapVisibilityButton);
  QWidgetAction* opacityAction = new QWidgetAction(this->LabelMapOpacitySlider);
  opacityAction->setDefaultWidget(this->LabelMapOpacitySlider->slider());
  this->LabelMapMenu->addAction(opacityAction);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupMoreOptionsMenu()
{
  QMenu* advancedMenu = new QMenu(tr("Advanced"), this->SliceMoreOptionButton);
  advancedMenu->setObjectName("advancedMenu");
  advancedMenu->addMenu(this->CompositingMenu);
  advancedMenu->addAction(this->actionRotate_to_volume_plane);
  advancedMenu->addMenu(this->SliceSpacingMenu);
  advancedMenu->addMenu(this->SliceModelMenu);

  this->SliceMoreOptionButton->setMenu(advancedMenu);
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
      q->mrmlScene()->GetNodesByClass(nodeTypes.toUtf8()));
    q->mrmlScene()->SaveStateForUndo();
    }
  return nodes;
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::enableLayerWidgets()
{
  bool hasBackground = this->MRMLSliceCompositeNode ?
    this->MRMLSliceCompositeNode->GetBackgroundVolumeID() != nullptr : false;
  bool hasForeground = this->MRMLSliceCompositeNode ?
    this->MRMLSliceCompositeNode->GetForegroundVolumeID() != nullptr : false;
  bool hasLabelMap = this->MRMLSliceCompositeNode ?
    this->MRMLSliceCompositeNode->GetLabelVolumeID() != nullptr : false;

  int volumeCount = 0;
  volumeCount += hasBackground ? 1 : 0;
  volumeCount += hasForeground ? 1 : 0;
  volumeCount += hasLabelMap ? 1 : 0;

  bool enableVisibility = volumeCount >= 2;

  this->actionBackgroundVisibility->setEnabled(false);
  this->BackgroundOpacitySlider->setEnabled(false);

  this->actionForegroundVisibility->setEnabled(enableVisibility && hasForeground);
  this->ForegroundOpacitySlider->setEnabled(enableVisibility && hasForeground);

  this->actionLabelMapVisibility->setEnabled(enableVisibility && hasLabelMap);
  this->LabelMapOpacitySlider->setEnabled(enableVisibility && hasLabelMap);

  // enable the interpolation or outline modes
  this->actionLabelMapOutline->setEnabled(hasLabelMap);
  this->actionBackgroundInterpolation->setEnabled(hasBackground);
  this->actionForegroundInterpolation->setEnabled(hasForeground);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::updateFromMRMLScene()
{
  Q_Q(qMRMLSliceControllerWidget);
  this->updateWidgetFromMRMLSliceCompositeNode();
  q->updateSegmentationControlsVisibility();
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::updateSliceOrientationSelector(
    vtkMRMLSliceNode* sliceNode, QComboBox* sliceOrientationSelector)
{
  Q_ASSERT(sliceNode);
  Q_ASSERT(sliceOrientationSelector);

  vtkNew<vtkStringArray> orientationNames;
  sliceNode->GetSliceOrientationPresetNames(orientationNames.GetPointer());

  bool wasBlocked = sliceOrientationSelector->blockSignals(true);

  // Clear list
  sliceOrientationSelector->clear();

  // Add all available presets
  for (int idx = 0; idx < orientationNames->GetNumberOfValues(); ++idx)
    {
    sliceOrientationSelector->addItem(QString::fromStdString(orientationNames->GetValue(idx)));
    }

  QString currentOrientation = QString::fromStdString(sliceNode->GetOrientation());

  // Add "Reformat" only if current orientation is "Reformat"
  if (currentOrientation == "Reformat")
    {
    sliceOrientationSelector->addItem(currentOrientation);
    }

  // Update orientation selector state
  int index = sliceOrientationSelector->findText(currentOrientation);
  Q_ASSERT(index>=0);

  // Set current orientation
  sliceOrientationSelector->setCurrentIndex(index);

  sliceOrientationSelector->blockSignals(wasBlocked);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::updateWidgetFromMRMLSliceNode()
{
  Q_Q(qMRMLSliceControllerWidget);

  vtkMRMLSliceNode* sliceNode = q->mrmlSliceNode();
  if (!sliceNode)
    {
    return;
    }

  double* layoutColorVtk = sliceNode->GetLayoutColor();
  QColor layoutColor = QColor::fromRgbF(layoutColorVtk[0], layoutColorVtk[1], layoutColorVtk[2]);
  this->setColor(layoutColor);

  bool wasBlocked;

  // Update abbreviated slice view name
  this->ViewLabel->setText(sliceNode->GetLayoutLabel());

  Self::updateSliceOrientationSelector(sliceNode, this->SliceOrientationSelector);

  // Update slice offset slider tooltip
  qMRMLOrientation orientation = this->mrmlOrientation(
      QString::fromStdString(sliceNode->GetOrientation().c_str()));
  this->SliceOffsetSlider->setToolTip(orientation.ToolTip);
  this->SliceOffsetSlider->setPrefix(orientation.Prefix);

  // Update slice visibility toggle
  this->actionShow_in_3D->setChecked(sliceNode->GetSliceVisible());
  this->actionLockNormalToCamera->setChecked(
    sliceNode->GetWidgetNormalLockedToCamera());

  // Label Outline
  bool showOutline = sliceNode->GetUseLabelOutline();
  this->actionLabelMapOutline->setChecked(showOutline);
  this->actionLabelMapOutline->setText(showOutline ?
    tr("Hide label volume outlines") : tr("Show label volume outlines"));
  // Reformat
  bool showReformat = sliceNode->GetWidgetVisible();
  this->actionShow_reformat_widget->setChecked(showReformat);
  this->actionShow_reformat_widget->setText(
    showReformat ? tr("Hide reformat widget"): tr("Show reformat widget"));
  // Slice spacing mode
  this->SliceSpacingButton->setIcon(
    sliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::AutomaticSliceSpacingMode ?
      QIcon(":/Icons/SlicerAutomaticSliceSpacing.png") :
      QIcon(":/Icons/SlicerManualSliceSpacing.png"));
  this->actionSliceSpacingModeAutomatic->setChecked(
    sliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::AutomaticSliceSpacingMode);
  // Prescribed slice spacing
  double spacing[3] = {0.0, 0.0, 0.0};
  sliceNode->GetPrescribedSliceSpacing(spacing);
  this->SliceSpacingSpinBox->setValue(spacing[2]);
  // Field of view
  double fov[3]  = {0.0, 0.0, 0.0};
  sliceNode->GetFieldOfView(fov);
  wasBlocked = this->SliceFOVSpinBox->blockSignals(true);
  this->SliceFOVSpinBox->setValue(fov[0] < fov[1] ? fov[0] : fov[1]);
  this->SliceFOVSpinBox->blockSignals(wasBlocked);
  // Lightbox
  int rows = sliceNode->GetLayoutGridRows();
  int columns = sliceNode->GetLayoutGridColumns();
  this->actionLightbox1x1_view->setChecked(rows == 1 && columns == 1);
  this->actionLightbox1x2_view->setChecked(rows == 1 && columns == 2);
  this->actionLightbox1x3_view->setChecked(rows == 1 && columns == 3);
  this->actionLightbox1x4_view->setChecked(rows == 1 && columns == 4);
  this->actionLightbox1x6_view->setChecked(rows == 1 && columns == 6);
  this->actionLightbox1x8_view->setChecked(rows == 1 && columns == 8);
  this->actionLightbox2x2_view->setChecked(rows == 2 && columns == 2);
  this->actionLightbox3x3_view->setChecked(rows == 3 && columns == 3);
  this->actionLightbox6x6_view->setChecked(rows == 6 && columns == 6);

  this->actionSliceModelModeVolumes->setChecked(sliceNode->GetSliceResolutionMode() ==
                                                vtkMRMLSliceNode::SliceResolutionMatchVolumes);
  this->actionSliceModelMode2D->setChecked(sliceNode->GetSliceResolutionMode() ==
                                                vtkMRMLSliceNode::SliceResolutionMatch2DView);
  this->actionSliceModelMode2D_Volumes->setChecked(sliceNode->GetSliceResolutionMode() ==
                                                vtkMRMLSliceNode::SliceFOVMatch2DViewSpacingMatchVolumes);
  this->actionSliceModelModeVolumes_2D->setChecked(sliceNode->GetSliceResolutionMode() ==
                                                vtkMRMLSliceNode::SliceFOVMatchVolumesSpacingMatch2DView);
  //this->actionSliceModelModeCustom->setChecked(sliceNode->GetSliceResolutionMode() ==
  //                                              vtkMRMLSliceNode::SliceResolutionCustom);

  double UVWExtents[] = {256,256,256};
  double UVWOrigin[] = {0,0,0};
  int UVWDimensions[] = {256,256,256};

  sliceNode->GetUVWExtents(UVWExtents);
  sliceNode->GetUVWOrigin(UVWOrigin);
  sliceNode->GetUVWDimensions(UVWDimensions);

  wasBlocked = this->SliceModelFOVXSpinBox->blockSignals(true);
  this->SliceModelFOVXSpinBox->setValue(UVWExtents[0]);
  this->SliceModelFOVXSpinBox->blockSignals(wasBlocked);

  wasBlocked = this->SliceModelFOVYSpinBox->blockSignals(true);
  this->SliceModelFOVYSpinBox->setValue(UVWExtents[1]);
  this->SliceModelFOVYSpinBox->blockSignals(wasBlocked);

  wasBlocked = this->SliceModelDimensionXSpinBox->blockSignals(true);
  this->SliceModelDimensionXSpinBox->setValue(UVWDimensions[0]);
  this->SliceModelDimensionXSpinBox->blockSignals(wasBlocked);

  wasBlocked = this->SliceModelDimensionYSpinBox->blockSignals(true);
  this->SliceModelDimensionYSpinBox->setValue(UVWDimensions[1]);
  this->SliceModelDimensionYSpinBox->blockSignals(wasBlocked);

  wasBlocked = this->SliceModelOriginXSpinBox->blockSignals(true);
  this->SliceModelOriginXSpinBox->setValue(UVWOrigin[0]);
  this->SliceModelOriginXSpinBox->blockSignals(wasBlocked);

  wasBlocked = this->SliceModelOriginYSpinBox->blockSignals(true);
  this->SliceModelOriginYSpinBox->setValue(UVWOrigin[1]);
  this->SliceModelOriginYSpinBox->blockSignals(wasBlocked);

  // OrientationMarker (check the selected option)
  QAction* action = qobject_cast<QAction*>(this->OrientationMarkerTypesMapper->mapping(sliceNode->GetOrientationMarkerType()));
  if (action)
    {
    action->setChecked(true);
    }
  action = qobject_cast<QAction*>(this->OrientationMarkerSizesMapper->mapping(sliceNode->GetOrientationMarkerSize()));
  if (action)
    {
    action->setChecked(true);
    }

  // Ruler (check the selected option)
  action = qobject_cast<QAction*>(this->RulerTypesMapper->mapping(sliceNode->GetRulerType()));
  if (action)
    {
    action->setChecked(true);
    }

  // Ruler Color (check the selected option)
  action = qobject_cast<QAction*>(this->RulerColorMapper->mapping(sliceNode->GetRulerColor()));
  if (action)
    {
    action->setChecked(true);
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::updateWidgetFromMRMLSliceCompositeNode()
{
  Q_Q(qMRMLSliceControllerWidget);
  if (!q->mrmlScene() || q->mrmlScene()->IsBatchProcessing())
    {// when we are loading, the scene might be in an inconsistent mode, where
    // the volumes pointed by the slice composite node don't exist yet
    return;
    }
  if (!this->MRMLSliceCompositeNode)
    {
    return;
    }

  bool wasBlocked;

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
  wasBlocked = this->ForegroundComboBox->blockSignals(true);
  this->ForegroundComboBox->setCurrentNode(
      q->mrmlScene()->GetNodeByID(this->MRMLSliceCompositeNode->GetForegroundVolumeID()));
  this->ForegroundComboBox->blockSignals(wasBlocked);

  this->updateFromForegroundVolumeNode(
    q->mrmlScene()->GetNodeByID(this->MRMLSliceCompositeNode->GetForegroundVolumeID()));

  // Update "background layer" node selector
  wasBlocked = this->BackgroundComboBox->blockSignals(true);
  this->BackgroundComboBox->setCurrentNode(
      q->mrmlScene()->GetNodeByID(this->MRMLSliceCompositeNode->GetBackgroundVolumeID()));
  this->BackgroundComboBox->blockSignals(wasBlocked);

  this->updateFromBackgroundVolumeNode(
    q->mrmlScene()->GetNodeByID(this->MRMLSliceCompositeNode->GetBackgroundVolumeID()));

  // Update "label map" node selector
  wasBlocked = this->LabelMapComboBox->blockSignals(true);
  this->LabelMapComboBox->setCurrentNode(
      q->mrmlScene()->GetNodeByID(this->MRMLSliceCompositeNode->GetLabelVolumeID()));
  this->LabelMapComboBox->blockSignals(wasBlocked);

  // Label opacity
  this->LabelMapOpacitySlider->setValue(this->MRMLSliceCompositeNode->GetLabelOpacity());

  // Foreground opacity
  this->ForegroundOpacitySlider->setValue(this->MRMLSliceCompositeNode->GetForegroundOpacity());

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

  // Since we blocked the signals when setting the
  // Foreground/Background/Label volumes, we need to explicitly call
  // the function to enable the buttons, slides, etc.
  this->enableLayerWidgets();
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::updateWidgetFromUnitNode()
{
  Q_Q(qMRMLSliceControllerWidget);
  q->setSliceOffsetResolution(q->sliceOffsetResolution());
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onForegroundLayerNodeSelected(vtkMRMLNode * node)
{
  //Q_Q(qMRMLSliceControllerWidget);
  //qDebug() << "qMRMLSliceControllerWidgetPrivate::onForegroundLayerNodeSelected - sliceView:"
  //         << q->sliceOrientation();
  if (!this->MRMLSliceCompositeNode)
    {
    return;
    }

  this->SliceLogic->StartSliceCompositeNodeInteraction(vtkMRMLSliceCompositeNode::ForegroundVolumeFlag);
  this->MRMLSliceCompositeNode->SetForegroundVolumeID(node ? node->GetID() : nullptr);
  this->SliceLogic->EndSliceCompositeNodeInteraction();

  this->enableLayerWidgets();

  this->updateFromForegroundVolumeNode(node);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onBackgroundLayerNodeSelected(vtkMRMLNode * node)
{
  //Q_Q(qMRMLSliceControllerWidget);
  //qDebug() << "qMRMLSliceControllerWidgetPrivate::onBackgroundLayerNodeSelected - sliceView:"
  //         << q->sliceOrientation();
  if (!this->MRMLSliceCompositeNode)
    {
    return;
    }

  this->SliceLogic->StartSliceCompositeNodeInteraction(vtkMRMLSliceCompositeNode::BackgroundVolumeFlag);
  this->MRMLSliceCompositeNode->SetBackgroundVolumeID(node ? node->GetID() : nullptr);
  this->SliceLogic->EndSliceCompositeNodeInteraction();

  this->enableLayerWidgets();

  this->updateFromBackgroundVolumeNode(node);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onSegmentationNodeSelected(vtkMRMLNode* node)
{
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node);

  // Update segmentation visibility and opacity controls
  this->actionSegmentationVisibility->setEnabled(segmentationNode && segmentationNode->GetDisplayNodeID());
  this->SegmentationOpacitySlider->setEnabled(segmentationNode && segmentationNode->GetDisplayNodeID());

  if (!segmentationNode)
    {
    return;
    }

  // Update the controls with the new segmentation node
  this->onSegmentationNodeDisplayModifiedEvent(segmentationNode);

  // Connect segmentation and segment display change events
  this->qvtkReconnect(nullptr, segmentationNode, vtkMRMLDisplayableNode::DisplayModifiedEvent,
                      this, SLOT(onSegmentationNodeDisplayModifiedEvent(vtkObject*)) );
  this->qvtkReconnect(nullptr, segmentationNode, vtkSegmentation::SegmentAdded,
                      this, SLOT(onSegmentationNodeDisplayModifiedEvent(vtkObject*)));
  this->qvtkReconnect(nullptr, segmentationNode, vtkSegmentation::SegmentRemoved,
                      this, SLOT(onSegmentationNodeDisplayModifiedEvent(vtkObject*)));
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onSegmentationNodeDisplayModifiedEvent(vtkObject* nodeObject)
{
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(nodeObject);
  if (!segmentationNode || segmentationNode != this->SegmentSelectorWidget->currentNode())
    {
    return;
    }

  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
    segmentationNode->GetDisplayNode());

  // Update segmentation visibility and opacity controls
  this->actionSegmentationVisibility->setEnabled(displayNode != nullptr);
  this->SegmentationOpacitySlider->setEnabled(displayNode != nullptr);

  if (!displayNode)
    {
    return;
    }

  // Visibility
  this->SegmentationVisibilityButton->blockSignals(true);
  this->SegmentationVisibilityButton->setChecked(!displayNode->GetVisibility());
  this->SegmentationVisibilityButton->blockSignals(false);

  // Opacity
  this->SegmentationOpacitySlider->blockSignals(true);
  this->SegmentationOpacitySlider->setValue(displayNode->GetOpacity());
  this->SegmentationOpacitySlider->blockSignals(false);

  // Outline/fill
  this->updateSegmentationOutlineFillButton();

  // Segment visibilities
  std::vector<std::string> visibleSegmentIDsStd;
  displayNode->GetVisibleSegmentIDs(visibleSegmentIDsStd);
  QStringList visibleSegmentIDs;
  for (std::vector<std::string>::iterator segmentIDIt = visibleSegmentIDsStd.begin();
    segmentIDIt != visibleSegmentIDsStd.end(); ++segmentIDIt)
    {
    visibleSegmentIDs << segmentIDIt->c_str();
    }
  this->SegmentSelectorWidget->setSelectedSegmentIDs(visibleSegmentIDs);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::updateSegmentationOutlineFillButton()
{
  vtkMRMLSegmentationDisplayNode* displayNode = this->currentSegmentationDisplayNode();
  if (!displayNode)
    {
    return;
    }

  bool outline = displayNode->GetVisibility2DOutline();
  bool fill = displayNode->GetVisibility2DFill();

  if (outline && fill)
    {
    QIcon outlineFillIcon(":/Icons/SlicesLabelOutlineAndFill.png");
    this->SegmentationOutlineButton->setIcon(outlineFillIcon);
    }
  else if (fill)
    {
    QIcon fillIcon(":/Icons/SlicesLabelFill.png");
    this->SegmentationOutlineButton->setIcon(fillIcon);
    }
  else if (outline)
    {
    QIcon outlineIcon(":/Icons/SlicesLabelOutline.png");
    this->SegmentationOutlineButton->setIcon(outlineIcon);
    }
  else
    {
    qWarning() << Q_FUNC_INFO << ": Invalid segmentation outline/fill state: neither are on";
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onLabelMapNodeSelected(vtkMRMLNode * node)
{
  //Q_Q(qMRMLSliceControllerWidget);
  //qDebug() << "qMRMLSliceControllerWidgetPrivate::onLabelMapNodeSelected - sliceView:"
  //         << q->sliceOrientation();
  if (!this->MRMLSliceCompositeNode)
    {
    return;
    }

  this->SliceLogic->StartSliceCompositeNodeInteraction(vtkMRMLSliceCompositeNode::LabelVolumeFlag);
  this->MRMLSliceCompositeNode->SetLabelVolumeID(node ? node->GetID() : nullptr);
  this->SliceLogic->EndSliceCompositeNodeInteraction();

  this->enableLayerWidgets();
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onSliceLogicModifiedEvent()
{
  Q_Q(qMRMLSliceControllerWidget);

  vtkMRMLSliceNode* newSliceNode = this->SliceLogic ? this->SliceLogic->GetSliceNode() : nullptr;
  if (newSliceNode != q->mrmlSliceNode())
    {
    q->setMRMLViewNode(newSliceNode);
    }
  // Enable/disable widget
  q->setDisabled(newSliceNode == nullptr);

  this->setMRMLSliceCompositeNodeInternal(
    this->SliceLogic ? this->SliceLogic->GetSliceCompositeNode() : nullptr);

  // no op if they are the same
  // The imagedata of SliceLogic can change !?!?! it should probably not
  q->setImageDataConnection(
    this->SliceLogic ? this->SliceLogic->GetImageDataConnection() : nullptr);

  if (!this->SliceLogic)
    {
    return;
    }
  bool wasBlocking = this->SliceOffsetSlider->blockSignals(true);

  // Set slice offset range to match the field of view
  // Calculate the number of slices in the current range.
  // Extent is between the farthest voxel centers (not voxel sides).
  double sliceBounds[6] = {0, -1, 0, -1, 0, -1};
  this->SliceLogic->GetLowestVolumeSliceBounds(sliceBounds, true);

  const double * sliceSpacing = this->SliceLogic->GetLowestVolumeSliceSpacing();
  Q_ASSERT(sliceSpacing);
  double offsetResolution = sliceSpacing ? sliceSpacing[2] : 1.0;

  bool singleSlice = ((sliceBounds[5] - sliceBounds[4]) < offsetResolution);
  if (singleSlice)
    {
    // add one blank slice before and after the current slice to make the slider appear in the center when
    // we are centered on the slice
    double centerPos = (sliceBounds[4] + sliceBounds[5]) / 2.0;
    q->setSliceOffsetRange(centerPos - offsetResolution, centerPos + offsetResolution);
    }
  else
    {
    // there are at least two slices in the range
    q->setSliceOffsetRange(sliceBounds[4], sliceBounds[5]);
    }

  // Set the scale increments to match the z spacing (rotated into slice space)
  q->setSliceOffsetResolution(offsetResolution);

  // Update slider position
  this->SliceOffsetSlider->setValue(this->SliceLogic->GetSliceOffset());
  this->SliceOffsetSlider->blockSignals(wasBlocking);

  emit q->renderRequested();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::updateFromForegroundVolumeNode(vtkObject* node)
{
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(node);
  vtkMRMLScalarVolumeDisplayNode* displayNode =
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
      volumeNode ? volumeNode->GetVolumeDisplayNode(): nullptr);
  this->qvtkReconnect(displayNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateFromForegroundDisplayNode(vtkObject*)));
  this->updateFromForegroundDisplayNode(displayNode);
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
void qMRMLSliceControllerWidgetPrivate::updateFromBackgroundVolumeNode(vtkObject* node)
{
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(node);
  vtkMRMLScalarVolumeDisplayNode* displayNode =
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
      volumeNode ? volumeNode->GetVolumeDisplayNode(): nullptr);
  this->qvtkReconnect(displayNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateFromBackgroundDisplayNode(vtkObject*)));
  this->updateFromBackgroundDisplayNode(displayNode);
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
    return nullptr;
    }
  vtkMRMLSliceLogic* logic = nullptr;
  vtkCollectionSimpleIterator it;
  for (this->SliceLogics->InitTraversal(it);(logic = static_cast<vtkMRMLSliceLogic*>(
                                               this->SliceLogics->GetNextItemAsObject(it)));)
    {
    if (logic->GetSliceCompositeNode() == node)
      {
      return logic;
      }
    }
  return nullptr;
}

//---------------------------------------------------------------------------
vtkMRMLSliceLogic* qMRMLSliceControllerWidgetPrivate::sliceNodeLogic(vtkMRMLSliceNode* node)
{
  if (!this->SliceLogics)
    {
    return nullptr;
    }
  vtkMRMLSliceLogic* logic = nullptr;
  vtkCollectionSimpleIterator it;
  for (this->SliceLogics->InitTraversal(it);(logic = static_cast<vtkMRMLSliceLogic*>(
                                               this->SliceLogics->GetNextItemAsObject(it)));)
    {
    if (logic->GetSliceNode() == node)
      {
      return logic;
      }
    }
  return nullptr;
}


//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setForegroundInterpolation(vtkMRMLSliceLogic* sliceLogic, bool linear)
{
  Q_Q(qMRMLSliceControllerWidget);
  // TODO, update the QAction when the display node is modified
  vtkMRMLVolumeNode* volumeNode = sliceLogic->GetForegroundLayer()->GetVolumeNode();
  vtkMRMLScalarVolumeDisplayNode *displayNode = volumeNode ? vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
    volumeNode->GetVolumeDisplayNode()) : nullptr;
  if (displayNode)
    {
    q->mrmlScene()->SaveStateForUndo();
    displayNode->SetInterpolate(linear);
    }
  // historic code that doesn't seem to work
  // vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
  //   sliceLogic->GetForegroundLayer()->GetVolumeDisplayNode());
  // if (displayNode)
  //   {
  //   q->mrmlScene()->SaveStateForUndo();
  //   displayNode->SetInterpolate(interpolate);
  //   vtkMRMLVolumeNode* volumeNode = sliceLogic->GetForegroundLayer()->GetVolumeNode();
  //   if (volumeNode)
  //     {
  //     volumeNode->Modified();
  //     }
  //   }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setBackgroundInterpolation(vtkMRMLSliceLogic* sliceLogic, bool linear)
{
  Q_Q(qMRMLSliceControllerWidget);
  // TODO, update the QAction when the display node is modified
  vtkMRMLVolumeNode* volumeNode = sliceLogic->GetBackgroundLayer()->GetVolumeNode();
  vtkMRMLScalarVolumeDisplayNode *displayNode = volumeNode ? vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
    volumeNode->GetVolumeDisplayNode()) : nullptr;
  if (displayNode)
    {
    q->mrmlScene()->SaveStateForUndo();
    displayNode->SetInterpolate(linear);
    }
  // historic code that doesn't seem to work
  // vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
  //   sliceLogic->GetBackgroundLayer()->GetVolumeDisplayNode());
  // if (displayNode)
  //   {
  //   q->mrmlScene()->SaveStateForUndo();
  //   displayNode->SetInterpolate(interpolate);
  //   vtkMRMLVolumeNode* volumeNode = sliceLogic->GetBackgroundLayer()->GetVolumeNode();
  //   if (volumeNode)
  //     {
  //     volumeNode->Modified();
  //     }
  //   }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::applyCustomLightbox()
{
  Q_Q(qMRMLSliceControllerWidget);
  q->setLightbox(this->LightBoxRowsSpinBox->value(), this->LightBoxColumnsSpinBox->value());
}

//---------------------------------------------------------------------------
vtkMRMLSegmentationDisplayNode* qMRMLSliceControllerWidgetPrivate::currentSegmentationDisplayNode()
{
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    this->SegmentSelectorWidget->currentNode() );
  if (!segmentationNode)
    {
    return nullptr;
    }
  return vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupOrientationMarkerMenu()
{
  Q_Q(qMRMLSliceControllerWidget);

  // OrientationMarker actions
  // Type
  this->OrientationMarkerTypesMapper = new ctkSignalMapper(this->PopupWidget);
  this->OrientationMarkerTypesMapper->setMapping(this->actionOrientationMarkerTypeNone, vtkMRMLAbstractViewNode::OrientationMarkerTypeNone);
  this->OrientationMarkerTypesMapper->setMapping(this->actionOrientationMarkerTypeCube, vtkMRMLAbstractViewNode::OrientationMarkerTypeCube);
  this->OrientationMarkerTypesMapper->setMapping(this->actionOrientationMarkerTypeHuman, vtkMRMLAbstractViewNode::OrientationMarkerTypeHuman);
  this->OrientationMarkerTypesMapper->setMapping(this->actionOrientationMarkerTypeAxes, vtkMRMLAbstractViewNode::OrientationMarkerTypeAxes);
  QActionGroup* orientationMarkerTypesActions = new QActionGroup(this->PopupWidget);
  orientationMarkerTypesActions->setExclusive(true);
  orientationMarkerTypesActions->addAction(this->actionOrientationMarkerTypeNone);
  orientationMarkerTypesActions->addAction(this->actionOrientationMarkerTypeCube);
  orientationMarkerTypesActions->addAction(this->actionOrientationMarkerTypeHuman);
  orientationMarkerTypesActions->addAction(this->actionOrientationMarkerTypeAxes);
  QObject::connect(this->OrientationMarkerTypesMapper, SIGNAL(mapped(int)),q, SLOT(setOrientationMarkerType(int)));
  QObject::connect(orientationMarkerTypesActions, SIGNAL(triggered(QAction*)),this->OrientationMarkerTypesMapper, SLOT(map(QAction*)));
  // Size
  this->OrientationMarkerSizesMapper = new ctkSignalMapper(this->PopupWidget);
  this->OrientationMarkerSizesMapper->setMapping(this->actionOrientationMarkerSizeSmall, vtkMRMLAbstractViewNode::OrientationMarkerSizeSmall);
  this->OrientationMarkerSizesMapper->setMapping(this->actionOrientationMarkerSizeMedium, vtkMRMLAbstractViewNode::OrientationMarkerSizeMedium);
  this->OrientationMarkerSizesMapper->setMapping(this->actionOrientationMarkerSizeLarge, vtkMRMLAbstractViewNode::OrientationMarkerSizeLarge);
  QActionGroup* orientationMarkerSizesActions = new QActionGroup(this->PopupWidget);
  orientationMarkerSizesActions->setExclusive(true);
  orientationMarkerSizesActions->addAction(this->actionOrientationMarkerSizeSmall);
  orientationMarkerSizesActions->addAction(this->actionOrientationMarkerSizeMedium);
  orientationMarkerSizesActions->addAction(this->actionOrientationMarkerSizeLarge);
  QObject::connect(this->OrientationMarkerSizesMapper, SIGNAL(mapped(int)),q, SLOT(setOrientationMarkerSize(int)));
  QObject::connect(orientationMarkerSizesActions, SIGNAL(triggered(QAction*)),this->OrientationMarkerSizesMapper, SLOT(map(QAction*)));
  // Menu
  QMenu* orientationMarkerMenu = new QMenu(tr("Orientation marker"), this->PopupWidget);
  orientationMarkerMenu->setObjectName("orientationMarkerMenu");
  this->OrientationMarkerButton->setMenu(orientationMarkerMenu);
  orientationMarkerMenu->addActions(orientationMarkerTypesActions->actions());
  orientationMarkerMenu->addSeparator();
  orientationMarkerMenu->addActions(orientationMarkerSizesActions->actions());
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupRulerMenu()
{
  Q_Q(qMRMLSliceControllerWidget);
  // Ruler actions
  // Type
  this->RulerTypesMapper = new ctkSignalMapper(this->PopupWidget);
  this->RulerTypesMapper->setMapping(this->actionRulerTypeNone, vtkMRMLAbstractViewNode::RulerTypeNone);
  this->RulerTypesMapper->setMapping(this->actionRulerTypeThin, vtkMRMLAbstractViewNode::RulerTypeThin);
  this->RulerTypesMapper->setMapping(this->actionRulerTypeThick, vtkMRMLAbstractViewNode::RulerTypeThick);
  this->RulerColorMapper = new ctkSignalMapper(this->PopupWidget);
  this->RulerColorMapper->setMapping(this->actionRulerColorBlack, vtkMRMLAbstractViewNode::RulerColorBlack);
  this->RulerColorMapper->setMapping(this->actionRulerColorWhite, vtkMRMLAbstractViewNode::RulerColorWhite);
  this->RulerColorMapper->setMapping(this->actionRulerColorYellow, vtkMRMLAbstractViewNode::RulerColorYellow);
  QActionGroup* rulerTypesActions = new QActionGroup(this->PopupWidget);
  rulerTypesActions->setExclusive(true);
  rulerTypesActions->addAction(this->actionRulerTypeNone);
  rulerTypesActions->addAction(this->actionRulerTypeThin);
  rulerTypesActions->addAction(this->actionRulerTypeThick);
  QObject::connect(this->RulerTypesMapper, SIGNAL(mapped(int)),q, SLOT(setRulerType(int)));
  QObject::connect(rulerTypesActions, SIGNAL(triggered(QAction*)),this->RulerTypesMapper, SLOT(map(QAction*)));
  QActionGroup* rulerColorActions = new QActionGroup(this->PopupWidget);
  rulerColorActions->addAction(this->actionRulerColorWhite);
  rulerColorActions->addAction(this->actionRulerColorBlack);
  rulerColorActions->addAction(this->actionRulerColorYellow);
  QObject::connect(this->RulerColorMapper, SIGNAL(mapped(int)),q, SLOT(setRulerColor(int)));
  QObject::connect(rulerColorActions, SIGNAL(triggered(QAction*)),this->RulerColorMapper, SLOT(map(QAction*)));
  // Menu
  QMenu* rulerMenu = new QMenu(tr("Ruler"), this->PopupWidget);
  rulerMenu->setObjectName("rulerMenu");
  this->RulerButton->setMenu(rulerMenu);
  rulerMenu->addActions(rulerTypesActions->actions());
  rulerMenu->addSeparator();
  rulerMenu->addActions(rulerColorActions->actions());
}

// --------------------------------------------------------------------------
qMRMLOrientation qMRMLSliceControllerWidgetPrivate::mrmlOrientation(const QString &name)
{
  QHash<QString, qMRMLOrientation>::iterator it = this->SliceOrientationToDescription.find(name);
  if (it != this->SliceOrientationToDescription.end())
    {
    return it.value();
    }
  qMRMLOrientation obliqueOrientation = {"", qMRMLSliceControllerWidget::tr("Oblique")};
  return obliqueOrientation;
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onSegmentVisibilitySelectionChanged(QStringList selectedSegmentIDs)
{
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    this->SegmentSelectorWidget->currentNode() );
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentation node";
    return;
    }
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
    segmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    return;
    }

  std::vector<std::string> allSegmentIDs;
  segmentationNode->GetSegmentation()->GetSegmentIDs(allSegmentIDs);
  QStringList segmentIdsInSegmentSelectorWidget(this->SegmentSelectorWidget->segmentIDs());
  std::vector<std::string>::iterator segmentIDIt;
  for (segmentIDIt = allSegmentIDs.begin(); segmentIDIt != allSegmentIDs.end(); ++segmentIDIt)
    {
    QString segmentID(segmentIDIt->c_str());
    if (!segmentIdsInSegmentSelectorWidget.contains(segmentID))
      {
      // the segment selector widget does not know about this segment yet, so do not update the MRML node from it
      continue;
      }
    bool segmentVisibile = displayNode->GetSegmentVisibility(*segmentIDIt);
    // Hide segment that is visible but its checkbox has been unchecked
    if (segmentVisibile && !selectedSegmentIDs.contains(segmentID))
      {
      displayNode->SetSegmentVisibility(*segmentIDIt, false);
      return; // This event handler runs after each check/uncheck, so handling the first mismatch is enough
      }
    // Show segment that is not visible but its checkbox has been checked
    else if (!segmentVisibile && selectedSegmentIDs.contains(segmentID))
      {
      displayNode->SetSegmentVisibility(*segmentIDIt, true);
      return; // This event handler runs after each check/uncheck, so handling the first mismatch is enough
      }
    }
}


// --------------------------------------------------------------------------
// qMRMLSliceControllerWidget methods

//---------------------------------------------------------------------------
CTK_GET_CPP(qMRMLSliceControllerWidget, vtkMRMLSliceLogic*, sliceLogic, SliceLogic);
CTK_GET_CPP(qMRMLSliceControllerWidget, vtkAlgorithmOutput*, imageDataConnection, ImageDataConnection);
CTK_GET_CPP(qMRMLSliceControllerWidget, vtkMRMLSliceCompositeNode*,
            mrmlSliceCompositeNode, MRMLSliceCompositeNode);

// --------------------------------------------------------------------------
qMRMLSliceControllerWidget::qMRMLSliceControllerWidget(QWidget* _parent)
  : Superclass(new qMRMLSliceControllerWidgetPrivate(*this), _parent)
{
  Q_D(qMRMLSliceControllerWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLSliceControllerWidget::qMRMLSliceControllerWidget(
    qMRMLSliceControllerWidgetPrivate* pimpl, QWidget* _parent)
  : Superclass(pimpl, _parent)
{
  // Note: You are responsible to call init() in the constructor of derived class.
}

// --------------------------------------------------------------------------
qMRMLSliceControllerWidget::~qMRMLSliceControllerWidget() = default;

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLSliceControllerWidget);

  if (this->mrmlScene() == newScene)
    {
    return;
    }
  //d->qvtkReconnect(this->mrmlScene(), newScene, vtkMRMLScene::SceneImportedEvent,
  //                 d, SLOT(updateWidgetFromMRMLSliceCompositeNode()));
  d->qvtkReconnect(this->mrmlScene(), newScene, vtkMRMLScene::EndBatchProcessEvent,
                   d, SLOT(updateFromMRMLScene()));
  d->SliceLogic->SetMRMLScene(newScene);

  // Disable the node selectors as they would fire the signal currentIndexChanged(0)
  // meaning that there is no current node anymore. It's not true, it just means that
  // that the current node was not in the combo box list menu before
  bool backgroundBlockSignals = d->BackgroundComboBox->blockSignals(true);
  bool foregroundBlockSignals = d->ForegroundComboBox->blockSignals(true);
  bool labelmapBlockSignals = d->LabelMapComboBox->blockSignals(true);
  //TODO: If uncommented then the node selector inside the widget does not get the scene.
  //      Remove this pair of blockSignals calls if not using them does not cause trouble
  //bool segmentationBlockSignals = d->SegmentSelectorWidget->blockSignals(true);

  this->Superclass::setMRMLScene(newScene);

  d->BackgroundComboBox->blockSignals(backgroundBlockSignals);
  d->ForegroundComboBox->blockSignals(foregroundBlockSignals);
  d->LabelMapComboBox->blockSignals(labelmapBlockSignals);
  //d->SegmentSelectorWidget->blockSignals(segmentationBlockSignals); //TODO: See first blockSignals call above

  d->setAndObserveSelectionNode();

  //d->updateWidgetFromMRMLSliceCompositeNode();
  if (this->mrmlScene())
    {
    d->updateFromMRMLScene();
    }
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
vtkMRMLSliceNode* qMRMLSliceControllerWidget::mrmlSliceNode()const
{
  Q_D(const qMRMLSliceControllerWidget);
  return vtkMRMLSliceNode::SafeDownCast(this->mrmlViewNode());
}

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
    //this->disconnect(d->ControllerButtonGroup, SIGNAL(buttonClicked(int)),
    //                 d, SLOT(toggleControllerWidgetGroupVisibility()));
    }

  if (newButtonGroup)
    {
    if (newButtonGroup->exclusive())
      {
      qCritical() << "qMRMLSliceControllerWidget::setControllerButtonGroup - "
                     "newButtonGroup shouldn't be exclusive - See QButtonGroup::setExclusive()";
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
void qMRMLSliceControllerWidget::setSliceViewSize(const QSize& newSize)
{
  Q_D(qMRMLSliceControllerWidget);
  //qDebug() << QString("qMRMLSliceControllerWidget::setSliceViewSize - newSize(%1, %2)").
  //            arg(newSize.width()).arg(newSize.height());
  d->ViewSize = newSize;
  if (!d->SliceLogic)
    {
    return;
    }
  d->SliceLogic->ResizeSliceNode(newSize.width(), newSize.height());
}

//---------------------------------------------------------------------------
QString qMRMLSliceControllerWidget::sliceViewName() const
{
  Q_D(const qMRMLSliceControllerWidget);

  if (!this->mrmlSliceNode())
    {
    qCritical() << "qMRMLSliceControllerWidget::setSliceViewName failed: MRMLSliceNode is invalid";
    return QString();
    }
  return QString::fromUtf8(this->mrmlSliceNode()->GetLayoutName());
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceViewName(const QString& newSliceViewName)
{
  Q_D(qMRMLSliceControllerWidget);

  if (!this->mrmlSliceNode())
    {
    qCritical() << "qMRMLSliceControllerWidget::setSliceViewName failed: MRMLSliceNode is invalid";
    return;
    }

  this->mrmlSliceNode()->SetLayoutName(newSliceViewName.toUtf8().constData());
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceViewLabel(const QString& newSliceViewLabel)
{
  Q_D(qMRMLSliceControllerWidget);
  if (!this->mrmlSliceNode())
    {
    qCritical() << Q_FUNC_INFO << " failed: must set view node first";
    return;
    }
  this->mrmlSliceNode()->SetLayoutLabel(newSliceViewLabel.toUtf8());
}

//---------------------------------------------------------------------------
QString qMRMLSliceControllerWidget::sliceViewLabel()const
{
  Q_D(const qMRMLSliceControllerWidget);
  if (!this->mrmlSliceNode())
    {
    qCritical() << Q_FUNC_INFO << " failed: must set view node first";
    return QString();
    }
  return this->mrmlSliceNode()->GetLayoutLabel();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceViewColor(const QColor& newSliceViewColor)
{
  Q_D(qMRMLSliceControllerWidget);
  if (!this->mrmlSliceNode())
    {
    qCritical() << "qMRMLSliceControllerWidget::setSliceViewName failed: MRMLSliceNode is invalid";
    return;
    }
  // this will update the widget color
  this->mrmlSliceNode()->SetLayoutColor(newSliceViewColor.redF(), newSliceViewColor.greenF(), newSliceViewColor.blueF());
}

//---------------------------------------------------------------------------
QColor qMRMLSliceControllerWidget::sliceViewColor()const
{
  Q_D(const qMRMLSliceControllerWidget);
  return d->color();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget
::setImageDataConnection(vtkAlgorithmOutput* newImageDataConnection)
{
  Q_D(qMRMLSliceControllerWidget);

  if (d->ImageDataConnection == newImageDataConnection)
    {
    return;
    }

  d->ImageDataConnection = newImageDataConnection;

  emit this->imageDataConnectionChanged(d->ImageDataConnection);
}

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
  d->SliceOffsetResolution = resolution;
  resolution = qMax(resolution, 0.00000001);
  double displayCoeffiecient = 1.0;
  if (d->SelectionNode && this->mrmlScene())
    {
    vtkMRMLUnitNode* unitNode = vtkMRMLUnitNode::SafeDownCast(this->mrmlScene()->GetNodeByID(d->SelectionNode->GetUnitNodeID("length")));
    if (unitNode)
      {
      displayCoeffiecient = unitNode->GetDisplayCoefficient();
      }
    }
  d->SliceOffsetSlider->setSingleStep(resolution * displayCoeffiecient);
  d->SliceOffsetSlider->setPageStep(resolution * displayCoeffiecient);
}

//---------------------------------------------------------------------------
double qMRMLSliceControllerWidget::sliceOffsetResolution()
{
  Q_D(qMRMLSliceControllerWidget);
  return d->SliceOffsetResolution;
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceOffsetValue(double offset)
{
  Q_D(qMRMLSliceControllerWidget);
  if (!d->SliceLogic)
    {
    return;
    }
  //qDebug() << "qMRMLSliceControllerWidget::setSliceOffsetValue:" << offset;

  // This prevents desynchronized update of displayable managers during user interaction
  // (ie. slice intersection widget or segmentations lagging behind during slice translation)
  vtkMRMLApplicationLogic* applicationLogic =
    vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->GetMRMLApplicationLogic();
  if (applicationLogic)
    {
    applicationLogic->PauseRender();
    }

  d->SliceLogic->StartSliceOffsetInteraction();
  d->SliceLogic->SetSliceOffset(offset);
  d->SliceLogic->EndSliceOffsetInteraction();

  if (applicationLogic)
    {
    applicationLogic->ResumeRender();
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::trackSliceOffsetValue(double offset)
{
  Q_D(qMRMLSliceControllerWidget);
  if (!d->SliceLogic)
    {
    return;
    }
  //qDebug() << "qMRMLSliceControllerWidget::trackSliceOffsetValue";

  // This prevents desynchronized update of displayable managers during user interaction
  // (ie. slice intersection widget or segmentations lagging behind during slice translation)
    vtkMRMLApplicationLogic* applicationLogic =
    vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->GetMRMLApplicationLogic();
  if (applicationLogic)
    {
    applicationLogic->PauseRender();
    }

  d->SliceLogic->StartSliceOffsetInteraction();
  d->SliceLogic->SetSliceOffset(offset);

  if (applicationLogic)
    {
    applicationLogic->ResumeRender();
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::fitSliceToBackground()
{
  Q_D(qMRMLSliceControllerWidget);
  //qDebug() << "qMRMLSliceControllerWidget::fitSliceToBackground";

  // This is implemented as sending a "reset field of view" command to
  // all the slice viewers. An alternative implementation is call
  // reset the field of view for the current slice and broadcast that
  // set of field of view parameters settings to the other viewers.
  // This can be done by changing the interaction flag to
  // vtkMRMLSliceNode::FieldOfViewFlag
  d->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::ResetFieldOfViewFlag);
  d->SliceLogic->FitSliceToAll();
  this->mrmlSliceNode()->UpdateMatrices();
  d->SliceLogic->EndSliceNodeInteraction();
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

  if (!this->mrmlSliceNode() || !d->MRMLSliceCompositeNode)
    {
    return;
    }

  d->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::OrientationFlag);
  this->mrmlSliceNode()->SetOrientation(orientation.toUtf8());
  d->SliceLogic->EndSliceNodeInteraction();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceVisible(bool visible)
{
  Q_D(qMRMLSliceControllerWidget);

  if (!this->mrmlSliceNode() || !d->MRMLSliceCompositeNode || !this->mrmlScene())
    {
    return;
    }

  d->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::SliceVisibleFlag);
  this->mrmlSliceNode()->SetSliceVisible(visible);
  d->SliceLogic->EndSliceNodeInteraction();
}

//---------------------------------------------------------------------------
bool qMRMLSliceControllerWidget::isLinked()const
{
  Q_D(const qMRMLSliceControllerWidget);
  // It is not really an assert here, what could have happen is that the
  // mrml slice composite node LinkedControl property has been changed but the
  // modified event has not been yet fired, updateWidgetFromMRMLSliceCompositeNode not having been
  // called yet, the slicelinkbutton state is not up to date.
  //Q_ASSERT(!d->MRMLSliceCompositeNode ||
  //        d->MRMLSliceCompositeNode->GetLinkedControl() ==
  //         d->SliceLinkButton->isChecked());
  return d->MRMLSliceCompositeNode ? d->MRMLSliceCompositeNode->GetLinkedControl() : d->SliceLinkButton->isChecked();
}

//---------------------------------------------------------------------------
bool qMRMLSliceControllerWidget::isCompareView()const
{
  Q_D(const qMRMLSliceControllerWidget);
  return this->mrmlSliceNode() && QString(this->mrmlSliceNode()->GetLayoutName()).startsWith("Compare");
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceLink(bool linked)
{
  vtkCollection* sliceCompositeNodes = this->mrmlScene() ?
    this->mrmlScene()->GetNodesByClass("vtkMRMLSliceCompositeNode") : nullptr;
  if (!sliceCompositeNodes)
    {
    return;
    }
  vtkMRMLSliceCompositeNode* sliceCompositeNode = nullptr;
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
    this->mrmlScene()->GetNodesByClass("vtkMRMLSliceCompositeNode") : nullptr;
  if (!sliceCompositeNodes)
    {
    return;
    }
  vtkMRMLSliceCompositeNode* sliceCompositeNode = nullptr;
  for(sliceCompositeNodes->InitTraversal();
      (sliceCompositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(
        sliceCompositeNodes->GetNextItemAsObject()));)
    {
    sliceCompositeNode->SetHotLinkedControl(linked);
    }
  sliceCompositeNodes->Delete();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setMoreButtonVisible(bool visible)
{
  Q_D(qMRMLSliceControllerWidget);
  d->MoreButton->setVisible(visible);
}

//---------------------------------------------------------------------------
bool qMRMLSliceControllerWidget::isMoreButtonVisible() const
{
  Q_D(const qMRMLSliceControllerWidget);
  return d->MoreButton->isVisibleTo(const_cast<qMRMLSliceControllerWidget*>(this));
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::moveBackgroundComboBox(bool more)
{
  Q_D(qMRMLSliceControllerWidget);
  QLayout* oldParentLayout = d->BackgroundComboBox->parentWidget()->layout();
  oldParentLayout->takeAt(oldParentLayout->indexOf(d->BackgroundComboBox));
  if (more)
    {
    qobject_cast<QGridLayout*>(d->PopupWidget->layout())->addWidget(d->BackgroundComboBox, 4,4);
    }
  else
    {
    d->SliceFrame->layout()->addWidget(d->BackgroundComboBox);
    }
  d->BackgroundComboBox->setVisible(true);
  d->PopupWidget->resize(this->width(), d->PopupWidget->sizeHint().height());
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::rotateSliceToLowestVolumeAxes()
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceNode");
  if(!nodes.GetPointer())
    {
    return;
    }
  d->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::RotateToBackgroundVolumePlaneFlag);
  d->SliceLogic->RotateSliceToLowestVolumeAxes();
  d->SliceLogic->EndSliceNodeInteraction();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSegmentationHidden(bool hide)
{
  Q_D(qMRMLSliceControllerWidget);

  vtkMRMLSegmentationDisplayNode* displayNode = d->currentSegmentationDisplayNode();
  if (!displayNode)
    {
    return;
    }

  displayNode->SetVisibility(!hide);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setLabelMapHidden(bool hide)
{
  Q_D(qMRMLSliceControllerWidget);
  d->LabelMapOpacitySlider->setValue(hide ? 0. : d->LastLabelMapOpacity);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setForegroundHidden(bool hide)
{
  Q_D(qMRMLSliceControllerWidget);
  if (hide && d->ForegroundOpacitySlider->value() != 0.)
    {
    d->LastForegroundOpacity = d->ForegroundOpacitySlider->value();
    }
  d->ForegroundOpacitySlider->setValue(hide ? 0. : d->LastForegroundOpacity);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setBackgroundHidden(bool hide)
{
  Q_D(qMRMLSliceControllerWidget);
  if (hide && d->BackgroundOpacitySlider->value() != 0.)
    {
    d->LastBackgroundOpacity = 1. - d->BackgroundOpacitySlider->value();
    }
  d->BackgroundOpacitySlider->setValue(hide ? 0. : d->LastBackgroundOpacity);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSegmentationOpacity(double opacity)
{
  Q_D(qMRMLSliceControllerWidget);

  vtkMRMLSegmentationDisplayNode* displayNode = d->currentSegmentationDisplayNode();
  if (!displayNode)
    {
    return;
    }

  displayNode->SetOpacity(opacity);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setLabelMapOpacity(double opacity)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceCompositeNode");
  if (!nodes.GetPointer())
    {
    return;
    }

  d->SliceLogic->StartSliceCompositeNodeInteraction(vtkMRMLSliceCompositeNode::LabelOpacityFlag);
  d->MRMLSliceCompositeNode->SetLabelOpacity(opacity);
  d->SliceLogic->EndSliceCompositeNodeInteraction();

  // LabelOpacityToggleButton won't fire the clicked(bool) signal here because
  // we change its check state programmatically.
  d->actionLabelMapVisibility->setChecked(opacity == 0.);
  if (opacity != 0.)
    {
    d->LastLabelMapOpacity = opacity;
    }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setForegroundOpacity(double opacity)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceCompositeNode");
  if (!nodes.GetPointer())
    {
    return;
    }

  d->SliceLogic->StartSliceCompositeNodeInteraction(vtkMRMLSliceCompositeNode::ForegroundOpacityFlag);
  d->MRMLSliceCompositeNode->SetForegroundOpacity(opacity);
  d->SliceLogic->EndSliceCompositeNodeInteraction();

  // LabelOpacityToggleButton won't fire the clicked(bool) signal here because
  // we change its check state programmatically.
  d->actionForegroundVisibility->setChecked(opacity == 0.);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setBackgroundOpacity(double opacity)
{
  Q_D(qMRMLSliceControllerWidget);
  //this->setForegroundOpacity(1. - opacity);
  d->actionBackgroundVisibility->setChecked(opacity == 1.);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::toggleSegmentationOutlineFill()
{
  Q_D(qMRMLSliceControllerWidget);
  vtkMRMLSegmentationDisplayNode* displayNode = d->currentSegmentationDisplayNode();
  if (!displayNode)
    {
    return;
    }

  bool outline = displayNode->GetVisibility2DOutline();
  bool fill = displayNode->GetVisibility2DFill();

  if (outline && fill)
    {
    // If both are visible, turn off fill (toggle to outline only)
    displayNode->SetVisibility2DFill(false);
    }
  else if (outline)
    {
    // If only outline is visible, toggle to fill only
    displayNode->SetVisibility2DFill(true);
    displayNode->SetVisibility2DOutline(false);
    }
  else if (fill)
  {
    // If only fill is visible, show outline too (toggle to both)
    displayNode->SetVisibility2DFill(true);
    displayNode->SetVisibility2DOutline(true);
  }
  else
    {
    // Invalid selection, but may have been set programmatically or from Segmentation module UI.
    // Set to both outline and fill
    displayNode->SetVisibility2DFill(true);
    displayNode->SetVisibility2DOutline(true);
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

  if (!this->mrmlSliceNode())
    {
    return;
    }

  d->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::LabelOutlineFlag);
  this->mrmlSliceNode()->SetUseLabelOutline(show);
  d->SliceLogic->EndSliceNodeInteraction();
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
  vtkMRMLSliceNode* node = nullptr;
  vtkCollectionSimpleIterator it;

  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    // When slice nodes are linked, only allow one slice node's reformat widget to be on at a time
    // If slice node's reformat widget was on, just turn all of them off
    // If slice node's reformat widget was off, turn it on and turn all the other ones off
    if (node == this->mrmlSliceNode() || this->isLinked())
      {
      node->SetWidgetVisible(show);
      }
    }
  if(show)
    {
    this->setSliceVisible(true);
    }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::lockReformatWidgetToCamera(bool lock)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    if (node == this->mrmlSliceNode())
      {
      node->SetWidgetNormalLockedToCamera(lock);
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
  vtkMRMLSliceCompositeNode* node = nullptr;
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
  if (!this->mrmlSliceNode() || !d->MRMLSliceCompositeNode)
    {
    return;
    }
  d->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::SliceSpacingFlag);
  if (automatic)
    {
    this->mrmlSliceNode()->SetSliceSpacingModeToAutomatic();
    }
  else
    {
    this->mrmlSliceNode()->SetSliceSpacingModeToPrescribed();
    }
  d->SliceLogic->EndSliceNodeInteraction();
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceSpacing(double sliceSpacing)
{
  Q_D(qMRMLSliceControllerWidget);
  if (!this->mrmlSliceNode() || !d->MRMLSliceCompositeNode)
    {
    return;
    }
  d->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::SliceSpacingFlag);
  this->mrmlSliceNode()->SetSliceSpacingModeToPrescribed();
  double spacing[3] = {0.0, 0.0, 0.0};
  this->mrmlSliceNode()->GetPrescribedSliceSpacing(spacing);
  spacing[2] = sliceSpacing;
  this->mrmlSliceNode()->SetPrescribedSliceSpacing(spacing);
  d->SliceLogic->EndSliceNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceFOV(double fov)
{
  Q_D(qMRMLSliceControllerWidget);
  double oldFov[3];
  this->mrmlSliceNode()->GetFieldOfView(oldFov);
  if (qAbs(qMin(oldFov[0], oldFov[1])- fov) < 0.01)
    {
    return;
    }
  if (!d->SliceLogics)
    {
    d->SliceLogic->FitFOVToBackground(fov);
    return;
    }
  vtkMRMLSliceLogic* sliceLogic = nullptr;
  vtkCollectionSimpleIterator it;
  for (d->SliceLogics->InitTraversal(it);
       (sliceLogic = static_cast<vtkMRMLSliceLogic*>(
          d->SliceLogics->GetNextItemAsObject(it)));)
    {
    if (sliceLogic == d->SliceLogic || this->isLinked())
      {
      sliceLogic->FitFOVToBackground(fov);
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelFOV(int index, double fov)
{
  Q_D(qMRMLSliceControllerWidget);
  double oldFov[3];
  this->mrmlSliceNode()->GetUVWExtents(oldFov);
  if (qAbs(oldFov[index] - fov) < 0.01)
    {
    return;
    }
  oldFov[index] = fov;
  this->mrmlSliceNode()->SetSliceResolutionMode(vtkMRMLSliceNode::SliceResolutionCustom);
  this->mrmlSliceNode()->SetUVWExtents(oldFov);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelFOVX(double fov)
{
  this->setSliceModelFOV(0,fov);
}
// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelFOVY(double fov)
{
  this->setSliceModelFOV(1,fov);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelDimension(int index, int dimension)
{
  Q_D(qMRMLSliceControllerWidget);
  int oldDimension[3];
  this->mrmlSliceNode()->GetUVWDimensions(oldDimension);
  if (qAbs(oldDimension[index] - dimension) < 0.01)
    {
    return;
    }
  oldDimension[index] = dimension;
  this->mrmlSliceNode()->SetSliceResolutionMode(vtkMRMLSliceNode::SliceResolutionCustom);
  this->mrmlSliceNode()->SetUVWDimensions(oldDimension);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelDimensionX(int dimension)
{
  this->setSliceModelDimension(0,dimension);
}
// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelDimensionY(int dimension)
{
  this->setSliceModelDimension(1,dimension);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelOrigin(int index, double origin)
{
  Q_D(qMRMLSliceControllerWidget);
  double oldOrigin[3];
  this->mrmlSliceNode()->GetUVWOrigin(oldOrigin);
  if (qAbs(oldOrigin[index] - origin) < 0.01)
    {
    return;
    }
  oldOrigin[index] = origin;
  this->mrmlSliceNode()->SetSliceResolutionMode(vtkMRMLSliceNode::SliceResolutionCustom);
  this->mrmlSliceNode()->SetUVWOrigin(oldOrigin);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelOriginX(double origin)
{
  this->setSliceModelOrigin(0,origin);
}
// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelOriginY(double origin)
{
  this->setSliceModelOrigin(1,origin);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelModeVolumes()
{
  this->setSliceModelMode(vtkMRMLSliceNode::SliceResolutionMatchVolumes);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelMode2D()
{
  this->setSliceModelMode(vtkMRMLSliceNode::SliceResolutionMatch2DView);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelMode2D_Volumes()
{
  this->setSliceModelMode(vtkMRMLSliceNode::SliceFOVMatch2DViewSpacingMatchVolumes);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelModeVolumes_2D()
{
  this->setSliceModelMode(vtkMRMLSliceNode::SliceFOVMatchVolumesSpacingMatch2DView);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelModeCustom()
{
  this->setSliceModelMode(vtkMRMLSliceNode::SliceResolutionCustom);
  // TODO show sliders
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceModelMode(int mode)
{
  Q_D(qMRMLSliceControllerWidget);
  this->mrmlSliceNode()->SetSliceResolutionMode(mode);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setLightbox(int rows, int columns)
{
  Q_D(qMRMLSliceControllerWidget);
  // TBD: issue #1690: disable fiducials in light box mode
  int AA_EnableTesting = 1001; // XXX Copied from qSlicerCoreApplication
  bool isTestingEnabled = QCoreApplication::testAttribute(
        static_cast<Qt::ApplicationAttribute>(AA_EnableTesting));
  if (rows * columns != 1 && !isTestingEnabled)
    {
    ctkMessageBox disableFidsMsgBox;
    disableFidsMsgBox.setWindowTitle("Disable fiducials?");
    QString labelText = QString("Fiducials are disabled in light box mode. Press Continue to enter light box mode without fiducials.");
    disableFidsMsgBox.setText(labelText);
    QPushButton *continueButton =
       disableFidsMsgBox.addButton(tr("Continue"), QMessageBox::AcceptRole);
    disableFidsMsgBox.addButton(QMessageBox::Cancel);
    disableFidsMsgBox.setIcon(QMessageBox::Question);
    disableFidsMsgBox.setDontShowAgainVisible(true);
    disableFidsMsgBox.setDontShowAgainSettingsKey("SliceController/AlwaysEnterLightBoxWithDisabledFiducials");
    disableFidsMsgBox.exec();
    if (disableFidsMsgBox.clickedButton() != continueButton)
      {
      d->actionLightbox1x1_view->setChecked(true);
      return;
      }
    }

  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    // only coronal layouts can be lightboxes ?
    if (node == this->mrmlSliceNode() ||
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
void qMRMLSliceControllerWidget::setForegroundInterpolation(bool linear)
{
  Q_D(qMRMLSliceControllerWidget);
  if (!d->SliceLogics)
    {
    d->setForegroundInterpolation(d->SliceLogic, linear);
    return;
    }
  vtkMRMLSliceLogic* sliceLogic = nullptr;
  vtkCollectionSimpleIterator it;
  for (d->SliceLogics->InitTraversal(it);(sliceLogic = static_cast<vtkMRMLSliceLogic*>(
                                   d->SliceLogics->GetNextItemAsObject(it)));)
    {
    if (sliceLogic == d->SliceLogic || this->isLinked())
      {
      d->setForegroundInterpolation(sliceLogic, linear);
      }
    }
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setBackgroundInterpolation(bool linear)
{
  Q_D(qMRMLSliceControllerWidget);
  if (!d->SliceLogics)
    {
    d->setBackgroundInterpolation(d->SliceLogic, linear);
    return;
    }
  vtkMRMLSliceLogic* sliceLogic = nullptr;
  vtkCollectionSimpleIterator it;
  for (d->SliceLogics->InitTraversal(it);(sliceLogic = static_cast<vtkMRMLSliceLogic*>(
                                   d->SliceLogics->GetNextItemAsObject(it)));)
    {
    if (sliceLogic == d->SliceLogic || this->isLinked())
      {
      d->setBackgroundInterpolation(sliceLogic, linear);
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setOrientationMarkerType(int newOrientationMarkerType)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceNode*>(nodes->GetNextItemAsObject(it)));)
    {
    if (node == this->mrmlSliceNode() || this->isLinked())
      {
      node->SetOrientationMarkerType(newOrientationMarkerType);
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setOrientationMarkerSize(int newOrientationMarkerSize)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceNode*>(nodes->GetNextItemAsObject(it)));)
    {
    if (node == this->mrmlSliceNode() || this->isLinked())
      {
      node->SetOrientationMarkerSize(newOrientationMarkerSize);
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setRulerType(int newRulerType)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceNode*>(nodes->GetNextItemAsObject(it)));)
    {
    if (node == this->mrmlSliceNode() || this->isLinked())
      {
      node->SetRulerType(newRulerType);
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setRulerColor(int newRulerColor)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceNode* node = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceNode*>(nodes->GetNextItemAsObject(it)));)
    {
    if (node == this->mrmlSliceNode() || this->isLinked())
      {
      node->SetRulerColor(newRulerColor);
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::updateSegmentationControlsVisibility()
{
  if (!this->mrmlScene())
    {
    return;
    }

  Q_D(qMRMLSliceControllerWidget);

  bool popupVisible = d->MoreButton->isChecked();

  // If there are any segmentation nodes in the scene and selection is None, then select the first one
  if (!d->SegmentSelectorWidget->currentNode())
    {
    vtkMRMLNode* firstSegmentationNode = this->mrmlScene()->GetFirstNode(nullptr, "vtkMRMLSegmentationNode");
    if (firstSegmentationNode)
      {
      d->SegmentSelectorWidget->setCurrentNode(firstSegmentationNode);
      }
    }
  bool segmentationNodesPresent = (d->SegmentSelectorWidget->currentNode() != nullptr);

  // Show segmentation controls only if the popup is visible and if there are
  // segmentation nodes in the scene
  bool visible = segmentationNodesPresent && popupVisible;
  d->SegmentationIconLabel->setVisible(visible);
  d->SegmentationVisibilityButton->setVisible(visible);
  d->SegmentationOpacitySlider->setVisible(visible);
  d->SegmentationOutlineButton->setVisible(visible);
  d->SegmentSelectorWidget->setVisible(visible);
}

// --------------------------------------------------------------------------
qMRMLSliderWidget* qMRMLSliceControllerWidget::sliceOffsetSlider()
{
  Q_D(qMRMLSliceControllerWidget);
  return d->SliceOffsetSlider;
}

// --------------------------------------------------------------------------
QToolButton* qMRMLSliceControllerWidget::fitToWindowToolButton()
{
  Q_D(qMRMLSliceControllerWidget);
  return d->FitToWindowToolButton;
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::updateWidgetFromMRMLView()
{
  Q_D(qMRMLSliceControllerWidget);
  Superclass::updateWidgetFromMRMLView();
  d->updateWidgetFromMRMLSliceNode();
}
