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
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QWidgetAction>

// CTK includes
#include <ctkDoubleSlider.h>
#include <ctkLogger.h>
#include <ctkPopupWidget.h>
#include <ctkSignalMapper.h>
#include <ctkSliderWidget.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLSliceControllerWidget_p.h"

// MRMLLogic includes
#include <vtkMRMLSliceLayerLogic.h>

// MRML includes
#include <vtkMRMLCrosshairNode.h>
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
  : Superclass(object)
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

  this->CrosshairMapper = 0;
  this->CrosshairThicknessMapper = 0;

  this->LastLabelMapOpacity = 1.;
  this->LastForegroundOpacity = 1.;
  this->LastBackgroundOpacity = 1.;

  this->SliceOffsetSlider = 0;

  this->LightboxMenu = 0;
  this->CompositingMenu = 0;
  this->CrosshairMenu = 0;
  this->SliceSpacingMenu = 0;
  
  this->SliceSpacingSpinBox = 0;
  this->SliceFOVSpinBox = 0;
  this->LightBoxRowsSpinBox = 0;
  this->LightBoxColumnsSpinBox = 0;
}

//---------------------------------------------------------------------------
qMRMLSliceControllerWidgetPrivate::~qMRMLSliceControllerWidgetPrivate()
{
}

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

  this->LabelMapOpacitySlider->spinBox()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  this->ForegroundOpacitySlider->spinBox()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  this->BackgroundOpacitySlider->spinBox()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  this->LabelMapOpacitySlider->slider()->setOrientation(Qt::Vertical);
  this->ForegroundOpacitySlider->slider()->setOrientation(Qt::Vertical);
  this->BackgroundOpacitySlider->slider()->setOrientation(Qt::Vertical);

  this->LabelMapOpacitySlider->popup()->setHideDelay(400);
  this->ForegroundOpacitySlider->popup()->setHideDelay(400);
  this->BackgroundOpacitySlider->popup()->setHideDelay(400);

  this->LabelMapOpacitySlider->popup()->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  this->ForegroundOpacitySlider->popup()->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  this->BackgroundOpacitySlider->popup()->setAlignment(Qt::AlignBottom | Qt::AlignLeft);

  int popupHeight = this->PopupWidget->sizeHint().height() / 2;
  this->LabelMapOpacitySlider->popup()->setFixedHeight(popupHeight);
  this->ForegroundOpacitySlider->popup()->setFixedHeight(popupHeight);
  this->BackgroundOpacitySlider->popup()->setFixedHeight(popupHeight);

  // Set selector attributes
  // Background and Foreground volume selectors can display LabelMap volumes. No
  // need to add the LabelMap attribute for them.
  this->LabelMapComboBox->addAttribute("vtkMRMLVolumeNode", "LabelMap", "1");
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
                   q, SLOT(rotateSliceToBackground()));
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
  this->setupCrosshairMenu();

  // Visibility column
  this->connect(this->actionLabelMapVisibility, SIGNAL(triggered(bool)),
                q, SLOT(setLabelMapHidden(bool)));
  this->connect(this->actionForegroundVisibility, SIGNAL(triggered(bool)),
                q, SLOT(setForegroundHidden(bool)));
  this->connect(this->actionBackgroundVisibility, SIGNAL(triggered(bool)),
                q, SLOT(setBackgroundHidden(bool)));

  // Opacity column
  this->connect(this->LabelMapOpacitySlider, SIGNAL(valueChanged(double)),
                q, SLOT(setLabelMapOpacity(double)));
  this->connect(this->ForegroundOpacitySlider, SIGNAL(valueChanged(double)),
                q, SLOT(setForegroundOpacity(double)));
  this->connect(this->BackgroundOpacitySlider, SIGNAL(valueChanged(double)),
                q, SLOT(setBackgroundOpacity(double)));

  // Interpolation column
  QObject::connect(this->actionLabelMapOutline, SIGNAL(toggled(bool)),
                   q, SLOT(showLabelOutline(bool)));
  QObject::connect(this->actionForegroundInterpolation, SIGNAL(toggled(bool)),
                   q, SLOT(setForegroundInterpolation(bool)));
  QObject::connect(this->actionBackgroundInterpolation, SIGNAL(toggled(bool)),
                   q, SLOT(setBackgroundInterpolation(bool)));

  // Connect Label map selector
  this->connect(this->LabelMapComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onLabelMapNodeSelected(vtkMRMLNode*)));
  // when the user select an entry already selected, we want to synchronize with the linked
  // slice logics as they mighy not have the same entry selected
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
  // slice logics as they mighy not have the same entry selected
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
  // slice logics as they mighy not have the same entry selected
  this->connect(this->BackgroundComboBox, SIGNAL(nodeActivated(vtkMRMLNode*)),
               SLOT(onBackgroundLayerNodeSelected(vtkMRMLNode*)));
  //this->connect(this->BackgroundComboBox, SIGNAL(currentNodeChanged(bool)),
  //              this->actionBackgroundVisibility, SLOT(setEnabled(bool)));
  this->connect(this->BackgroundComboBox, SIGNAL(currentNodeChanged(bool)),
                this->actionBackgroundInterpolation, SLOT(setEnabled(bool)));

  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->LabelMapComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->ForegroundComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->BackgroundComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));

  // Connect actions to buttons
  this->SliceVisibilityButton->setDefaultAction(this->actionShow_in_3D);
  this->FitToWindowToolButton->setDefaultAction(this->actionFit_to_window);
  this->LightBoxToolButton->setMenu(this->LightboxMenu);
  this->ShowReformatWidgetToolButton->setDefaultAction(this->actionShow_reformat_widget);
  this->setupMoreOptionsMenu();

  this->LabelMapVisibilityButton->setDefaultAction(this->actionLabelMapVisibility);
  this->ForegroundVisibilityButton->setDefaultAction(this->actionForegroundVisibility);
  this->BackgroundVisibilityButton->setDefaultAction(this->actionBackgroundVisibility);

  this->LabelMapOutlineButton->setDefaultAction(this->actionLabelMapOutline);
  this->ForegroundInterpolationButton->setDefaultAction(this->actionForegroundInterpolation);
  this->BackgroundInterpolationButton->setDefaultAction(this->actionBackgroundInterpolation);
  
  this->CrosshairButton->setMenu(this->CrosshairMenu);
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::init()
{
  Q_Q(qMRMLSliceControllerWidget);

  this->Superclass::init();
  
  this->SliceOffsetSlider = new ctkSliderWidget(q);
  this->SliceOffsetSlider->setTracking(false);
  this->SliceOffsetSlider->setToolTip(q->tr("Slice distance from RAS origin"));

  //this->SliceOffsetSlider->spinBox()->setParent(this->PopupWidget);
  QDoubleSpinBox* spinBox = this->SliceOffsetSlider->spinBox();
  spinBox->setFrame(false);
  spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
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

  this->BarLayout->addWidget(this->SliceOffsetSlider);

  // Move the spinbox in the popup instead of having it in the slider bar
  //dynamic_cast<QGridLayout*>(this->PopupWidget->layout())->addWidget(
  //  this->SliceOffsetSlider->spinBox(), 0, 0, 1, 2);

  vtkSmartPointer<vtkMRMLSliceLogic> defaultLogic =
    vtkSmartPointer<vtkMRMLSliceLogic>::New();
  q->setSliceLogic(defaultLogic);

  q->setSliceViewName("Red");
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
void qMRMLSliceControllerWidgetPrivate::setupReformatOptionsMenu()
{
  Q_Q(qMRMLSliceControllerWidget);
  QMenu* reformatMenu = new QMenu(tr("Reformat"),this->ShowReformatWidgetToolButton);

  reformatMenu->addAction(this->actionLockNormalToCamera);

  QObject::connect(this->actionLockNormalToCamera, SIGNAL(triggered(bool)),
                   q, SLOT(lockReformatWidgetToCamera(bool)));

  this->ShowReformatWidgetToolButton->setMenu(reformatMenu);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupLightboxMenu()
{
  // Lightbox View
  this->LightboxMenu = new QMenu(tr("Lightbox view"));
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
  this->CompositingMenu = new QMenu(tr("Compositing"));
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
void qMRMLSliceControllerWidgetPrivate::setupCrosshairMenu()
{
  Q_Q(qMRMLSliceControllerWidget);

  // Crosshair
  QObject::connect(this->actionCrosshairNavigator, SIGNAL(triggered(bool)),
                   q, SLOT(setNavigatorEnabled(bool)));
  QActionGroup* crosshairActions = new QActionGroup(q);
  crosshairActions->setExclusive(true);
  crosshairActions->addAction(this->actionCrosshairNo_crosshair);
  crosshairActions->addAction(this->actionCrosshairBasic_crosshair);
  crosshairActions->addAction(this->actionCrosshairBasic_intersection);
  crosshairActions->addAction(this->actionCrosshairBasic_hashmarks);
  crosshairActions->addAction(this->actionCrosshairBasic_hashmarks_intersection);
  crosshairActions->addAction(this->actionCrosshairSmall_basic);
  crosshairActions->addAction(this->actionCrosshairSmall_basic_intersection);
  this->CrosshairMapper = new ctkSignalMapper(q);
  this->CrosshairMapper->setMapping(actionCrosshairNo_crosshair,
                                    vtkMRMLCrosshairNode::NoCrosshair);
  this->CrosshairMapper->setMapping(actionCrosshairBasic_crosshair,
                                    vtkMRMLCrosshairNode::ShowBasic);
  this->CrosshairMapper->setMapping(actionCrosshairBasic_intersection,
                                    vtkMRMLCrosshairNode::ShowIntersection);
  this->CrosshairMapper->setMapping(actionCrosshairBasic_hashmarks,
                                    vtkMRMLCrosshairNode::ShowHashmarks);
  this->CrosshairMapper->setMapping(actionCrosshairBasic_hashmarks_intersection,
                                    vtkMRMLCrosshairNode::ShowAll);
  this->CrosshairMapper->setMapping(actionCrosshairSmall_basic,
                                    vtkMRMLCrosshairNode::ShowSmallBasic);
  this->CrosshairMapper->setMapping(actionCrosshairSmall_basic_intersection,
                                    vtkMRMLCrosshairNode::ShowSmallIntersection);
  QObject::connect(crosshairActions, SIGNAL(triggered(QAction*)),
                   this->CrosshairMapper, SLOT(map(QAction*)));
  QObject::connect(this->CrosshairMapper, SIGNAL(mapped(int)),
                   q, SLOT(setCrosshairMode(int)));
  QActionGroup* crosshairThicknessActions = new QActionGroup(q);
  crosshairThicknessActions->setExclusive(true);
  crosshairThicknessActions->addAction(this->actionCrosshairFine);
  crosshairThicknessActions->addAction(this->actionCrosshairMedium);
  crosshairThicknessActions->addAction(this->actionCrosshairThick);
  this->CrosshairThicknessMapper = new ctkSignalMapper(q);
  this->CrosshairThicknessMapper->setMapping(actionCrosshairFine,
                                             vtkMRMLCrosshairNode::Fine);
  this->CrosshairThicknessMapper->setMapping(actionCrosshairMedium,
                                             vtkMRMLCrosshairNode::Medium);
  this->CrosshairThicknessMapper->setMapping(actionCrosshairThick,
                                             vtkMRMLCrosshairNode::Thick);
  QObject::connect(crosshairThicknessActions, SIGNAL(triggered(QAction*)),
                   this->CrosshairThicknessMapper, SLOT(map(QAction*)));
  QObject::connect(this->CrosshairThicknessMapper, SIGNAL(mapped(int)),
                   q, SLOT(setCrosshairThickness(int)));
  QObject::connect(this->actionCrosshairSlice_intersections, SIGNAL(triggered(bool)),
                   q, SLOT(setSliceIntersectionVisible(bool)));
  this->CrosshairMenu = new QMenu("Crosshair", q);
  this->CrosshairMenu->addAction(this->actionCrosshairNavigator);
  this->CrosshairMenu->addSeparator();
  this->CrosshairMenu->addActions(crosshairActions->actions());
  this->CrosshairMenu->addSeparator();
  this->CrosshairMenu->addActions(crosshairThicknessActions->actions());
  this->CrosshairMenu->addSeparator();
  this->CrosshairMenu->addAction(this->actionCrosshairSlice_intersections);
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::setupSliceSpacingMenu()
{
  Q_Q(qMRMLSliceControllerWidget);

  // Spacing mode
  this->SliceSpacingMenu = new QMenu(tr("Slice spacing mode"));
  this->SliceSpacingMenu->setIcon(QIcon(":/Icons/SlicerAutomaticSliceSpacing.png"));
  this->SliceSpacingMenu->addAction(this->actionSliceSpacingModeAutomatic);
  QMenu* sliceSpacingManualMode = new QMenu(tr("Manual spacing"), this->SliceSpacingMenu);
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
  this->SliceSpacingMenu->addMenu(sliceSpacingManualMode);

  QMenu* sliceFOVMenu = new QMenu(tr("Field of view"), this->SliceSpacingMenu);
  sliceFOVMenu->setIcon(QIcon(":/Icon/SlicesFieldOfView.png"));
  QWidget* sliceFOVWidget = new QWidget(this->SliceSpacingMenu);
  QHBoxLayout* sliceFOVLayout = new QHBoxLayout(sliceFOVWidget);
  sliceFOVLayout->setContentsMargins(0,0,0,0);
  this->SliceFOVSpinBox = new QDoubleSpinBox(sliceFOVWidget);
  this->SliceFOVSpinBox->setRange(0., 10000.);
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
void qMRMLSliceControllerWidgetPrivate::setupMoreOptionsMenu()
{
  QMenu* advancedMenu = new QMenu(tr("Advanced"), this->SliceMoreOptionButton);

  advancedMenu->addMenu(this->CompositingMenu);
  advancedMenu->addAction(this->actionRotate_to_volume_plane);
  advancedMenu->addMenu(this->SliceSpacingMenu);

  this->SliceMoreOptionButton->setMenu(advancedMenu);
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
void qMRMLSliceControllerWidgetPrivate::enableVisibilityButtons()
{
  bool hasBackground = this->MRMLSliceCompositeNode ?
    this->MRMLSliceCompositeNode->GetBackgroundVolumeID() != 0 : false;
  bool hasForeground = this->MRMLSliceCompositeNode ?
    this->MRMLSliceCompositeNode->GetForegroundVolumeID() != 0 : false;
  bool hasLabelMap = this->MRMLSliceCompositeNode ?
    this->MRMLSliceCompositeNode->GetLabelVolumeID() != 0 : false;

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
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::onMRMLSceneChanged(
  vtkObject* sender, void* calldata, unsigned long event, void* receiver)
{
  Q_Q(qMRMLSliceControllerWidget);
  Q_UNUSED(receiver);
  Q_UNUSED(sender);
  if (q->mrmlScene() != sender ||
      !q->mrmlScene() || 
      q->mrmlScene()->GetIsUpdating())
    {
    return;
    }
  if (event == vtkMRMLScene::NodeAddedEvent)
    {
    vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairNode::SafeDownCast(
      reinterpret_cast<vtkMRMLNode*>(calldata));
    if (crosshairNode)
      {
      qvtkReconnect(crosshairNode, vtkCommand::ModifiedEvent,
                  this, SLOT(updateWidgetFromCrosshairNode(vtkObject*)));
      this->updateWidgetFromCrosshairNode(crosshairNode);
      }
    }
  else if (event == vtkMRMLScene::SceneImportedEvent ||
           event == vtkMRMLScene::SceneRestoredEvent)
    {
    this->updateWidgetFromMRMLSliceCompositeNode();
    vtkMRMLNode* crosshairNode = q->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLCrosshairNode");
    if (crosshairNode)
      {
      this->onMRMLSceneChanged(q->mrmlScene(), crosshairNode, vtkMRMLScene::NodeAddedEvent, 0);
      }
    }
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
  this->actionShow_in_3D->setChecked(this->MRMLSliceNode->GetSliceVisible());

  // Label Outline
  bool showOutline = this->MRMLSliceNode->GetUseLabelOutline();
  this->actionLabelMapOutline->setChecked(showOutline);
  this->actionLabelMapOutline->setText(showOutline ?
    tr("Hide label volume outlines") : tr("Show label volume outlines"));
  // Reformat
  bool showReformat = this->MRMLSliceNode->GetWidgetVisible();
  this->actionShow_reformat_widget->setChecked(showReformat);
  this->actionShow_reformat_widget->setText(
    showReformat ? tr("Hide reformat widget"): tr("Show reformat widget"));
  // Slice spacing mode
  this->actionSliceSpacingModeAutomatic->setChecked(
    this->MRMLSliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::AutomaticSliceSpacingMode);
  double fov[3];
  this->MRMLSliceNode->GetFieldOfView(fov);
  this->SliceFOVSpinBox->setValue(fov[0] < fov[1] ? fov[0] : fov[1]);
  // Lightbox
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
  this->ForegroundComboBox->setCurrentNode(
      q->mrmlScene()->GetNodeByID(this->MRMLSliceCompositeNode->GetForegroundVolumeID()));

  // Update "background layer" node selector
  this->BackgroundComboBox->setCurrentNode(
      q->mrmlScene()->GetNodeByID(this->MRMLSliceCompositeNode->GetBackgroundVolumeID()));

  // Update "label map" node selector
  this->LabelMapComboBox->setCurrentNode(
      q->mrmlScene()->GetNodeByID(this->MRMLSliceCompositeNode->GetLabelVolumeID()));

  // Label opacity
  this->LabelMapOpacitySlider->setValue(this->MRMLSliceCompositeNode->GetLabelOpacity());

  //bool blocking = this->ForegroundOpacitySlider->blockSignals(true);
  //bool blocking = this->BackgroundOpacitySlider->blockSignals(true);
  this->ForegroundOpacitySlider->setValue(this->MRMLSliceCompositeNode->GetForegroundOpacity());
  //this->ForegroundOpacitySlider->blockSignals(blocking);
  //this->BackgroundOpacitySlider->setValue(1. - this->MRMLSliceCompositeNode->GetForegroundOpacity());
  //this->BackgroundOpacitySlider->blockSignals(blocking);

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

  this->actionCrosshairSlice_intersections->setChecked(
    this->MRMLSliceCompositeNode->GetSliceIntersectionVisibility());
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidgetPrivate::updateWidgetFromCrosshairNode(vtkObject* node)
{
  vtkMRMLCrosshairNode* xnode = vtkMRMLCrosshairNode::SafeDownCast(node);
  if (!xnode)
    {
    return;
    }
  // Crosshair
  this->actionCrosshairNavigator->setChecked(xnode->GetNavigation());
  // Crosshair Mode
  QAction* crosshairAction =
    qobject_cast<QAction*>(this->CrosshairMapper->mapping(xnode->GetCrosshairMode()));
  crosshairAction->setChecked(true);
  // Crosshair Thickness
  QAction* crosshairThicknessAction =
    qobject_cast<QAction*>(this->CrosshairThicknessMapper->mapping(xnode->GetCrosshairThickness()));
  crosshairThicknessAction->setChecked(true);
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

  this->enableVisibilityButtons();

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

  this->enableVisibilityButtons();

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

  this->enableVisibilityButtons();
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
void qMRMLSliceControllerWidgetPrivate::setForegroundInterpolation(vtkMRMLSliceLogic* sliceLogic, bool linear)
{
  Q_Q(qMRMLSliceControllerWidget);
  // TODO, update the QAction when the display node is modified
  vtkMRMLVolumeNode* volumeNode = sliceLogic->GetForegroundLayer()->GetVolumeNode();
  vtkMRMLScalarVolumeDisplayNode *displayNode = volumeNode ? vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
    volumeNode->GetVolumeDisplayNode()) : 0;
  if (displayNode)
    {
    q->mrmlScene()->SaveStateForUndo(displayNode);
    displayNode->SetInterpolate(linear);
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
void qMRMLSliceControllerWidgetPrivate::setBackgroundInterpolation(vtkMRMLSliceLogic* sliceLogic, bool linear)
{
  Q_Q(qMRMLSliceControllerWidget);
  // TODO, update the QAction when the display node is modified
  vtkMRMLVolumeNode* volumeNode = sliceLogic->GetBackgroundLayer()->GetVolumeNode();
  vtkMRMLScalarVolumeDisplayNode *displayNode = volumeNode ? vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
    volumeNode->GetVolumeDisplayNode()) : 0;
  if (displayNode)
    {
    q->mrmlScene()->SaveStateForUndo(displayNode);
    displayNode->SetInterpolate(linear);
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
void qMRMLSliceControllerWidgetPrivate::applyCustomLightbox()
{
  Q_Q(qMRMLSliceControllerWidget);
  q->setLightbox(this->LightBoxRowsSpinBox->value(), this->LightBoxColumnsSpinBox->value());
}

// --------------------------------------------------------------------------
// qMRMLSliceView methods

// --------------------------------------------------------------------------
qMRMLSliceControllerWidget::qMRMLSliceControllerWidget(QWidget* _parent)
  : Superclass(new qMRMLSliceControllerWidgetPrivate(*this), _parent)
{
  Q_D(qMRMLSliceControllerWidget);
  d->init();
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
  //d->qvtkReconnect(this->mrmlScene(), newScene, vtkMRMLScene::SceneImportedEvent,
  //                 d, SLOT(updateWidgetFromMRMLSliceCompositeNode()));
  d->qvtkReconnect(this->mrmlScene(), newScene, vtkMRMLScene::NodeAddedEvent,
                   d, SLOT(onMRMLSceneChanged(vtkObject*,void*,ulong,void*)));
  d->qvtkReconnect(this->mrmlScene(), newScene, vtkMRMLScene::SceneImportedEvent,
                   d, SLOT(onMRMLSceneChanged(vtkObject*,void*,ulong,void*)));
  d->qvtkReconnect(this->mrmlScene(), newScene, vtkMRMLScene::SceneRestoredEvent,
                   d, SLOT(onMRMLSceneChanged(vtkObject*,void*,ulong,void*)));
  d->SliceLogic->SetMRMLScene(newScene);

  // Disable the node selectors as they would fire the signal currentIndexChanged(0)
  // meaning that there is no current node anymore. It's not true, it just means that
  // that the current node was not in the combo box list menu before
  bool backgroundBlockSignals = d->BackgroundComboBox->blockSignals(true);
  bool foregroundBlockSignals = d->ForegroundComboBox->blockSignals(true);
  bool labelmapBlockSignals = d->LabelMapComboBox->blockSignals(true);

  this->Superclass::setMRMLScene(newScene);

  d->BackgroundComboBox->blockSignals(backgroundBlockSignals);
  d->ForegroundComboBox->blockSignals(foregroundBlockSignals);
  d->LabelMapComboBox->blockSignals(labelmapBlockSignals);

  //d->updateWidgetFromMRMLSliceCompositeNode();
  if (this->mrmlScene())
    {
    d->onMRMLSceneChanged(this->mrmlScene(), reinterpret_cast<void*>(this),
                          vtkMRMLScene::SceneImportedEvent, 0);
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
    //this->disconnect(d->ControllerButtonGroup, SIGNAL(buttonClicked(int)),
    //                 d, SLOT(toggleControllerWidgetGroupVisibility()));
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

  // Colors are now first class properties not derived from the
  // name...

  // // If name matches either 'Red, 'Green' or 'Yellow' set the
  // // corresponding color (legacy colors). If the name matches an SVG color keyword
  // // http://www.w3.org/TR/SVG/types.html#ColorKeywords, then use that.
  // // Set Orange otherwise.
  // QColor barColor = qMRMLSliceControllerWidget::sliceViewColor(newSliceViewName);
  // d->setColor(barColor);

  if (d->SliceLogic)
    {
    d->SliceLogic->SetName(newSliceViewName.toLatin1());
    }

  d->SliceViewName = newSliceViewName;
}

//---------------------------------------------------------------------------
CTK_GET_CPP(qMRMLSliceControllerWidget, QString, sliceViewName, SliceViewName);

//---------------------------------------------------------------------------
QColor qMRMLSliceControllerWidget::sliceViewColor(const QString& sliceViewName)
{
  // We try to use SVG named colors directly. However, the Slicer Red,
  // Green, Yellow colors do not match the SVG red, green, yellow. So
  // we use the Slicer colors instead. It would nice to use the Qt 4.7
  // static method QColor::isValidColor() but that is not available in
  // earlier versions of Qt. Instead, we could just try setting the
  // color and checking whether isValid() is true but setting an
  // invalid color emits a warning in debug builds. So we fall back to
  // searching for the color in the list of colorNames(). Note that
  // this is slightly different that isValidColor() as isValidColor()
  // would also check RGB dynamic range in the #RRGGBBB style
  // formats. 
  QColor color;
  if (sliceViewName == "Red")
    {
    color = qMRMLColors::sliceRed();
    }
  else if (sliceViewName == "Green")
    {
    color = qMRMLColors::sliceGreen();
    }
  else if (sliceViewName == "Yellow")
    {
    color =  qMRMLColors::sliceYellow();
    }
  else if (sliceViewName.startsWith("Compare"))
    {
    color = qMRMLColors::sliceOrange();
    }
  else if (sliceViewName.startsWith("Slice"))
    {
    color = qMRMLColors::sliceGray();
    }
  else if (QColor::colorNames().contains(sliceViewName, Qt::CaseInsensitive))
    {
    // This conditional should really have been "if (QColor::isValidColor(sliceViewName.toLower()))"
    color = QColor(sliceViewName.toLower());
    }
  else
    {
    color = qMRMLColors::sliceGray();
    }
  return color;
}

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceViewLabel(const QString& newSliceViewLabel)
{
  Q_D(qMRMLSliceControllerWidget);

  if (d->MRMLSliceNode)
    {
    logger.error("setSliceViewLabel should be called before setMRMLSliceNode !");
    return;
    }

  d->SliceViewLabel = newSliceViewLabel;
  d->ViewLabel->setText(d->SliceViewLabel);
}

//---------------------------------------------------------------------------
CTK_GET_CPP(qMRMLSliceControllerWidget, QString, sliceViewLabel, SliceViewLabel);

//---------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceViewColor(const QString& newSliceViewColor)
{
  Q_D(qMRMLSliceControllerWidget);

  if (d->MRMLSliceNode)
    {
    logger.error("setSliceViewColor should be called before setMRMLSliceNode !");
    return;
    }

  d->SliceViewColor = newSliceViewColor;

  QColor barColor(d->SliceViewColor);
  d->setColor(barColor);
}

//---------------------------------------------------------------------------
CTK_GET_CPP(qMRMLSliceControllerWidget, QString, sliceViewColor, SliceViewColor);

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
  d->SliceOffsetSlider->setPageStep(resolution);
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

  // This is implemented as sending a "reset field of view" command to
  // all the slice viewers. An alternative implementation is call
  // reset the field of view for the current slice and broadcast that
  // set of field of view parameters settings to the other viewers.
  // This can be done by changing the interaction flag to
  // vtkMRMLSliceNode::FieldOfViewFlag 
  d->SliceLogic->StartSliceNodeInteraction(vtkMRMLSliceNode::ResetFieldOfViewFlag);
  d->SliceLogic->FitSliceToAll();
  d->MRMLSliceNode->UpdateMatrices();
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
void qMRMLSliceControllerWidget::setLabelMapOpacity(double opacity)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLSliceCompositeNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);
       (node = static_cast<vtkMRMLSliceCompositeNode*>(
         nodes->GetNextItemAsObject(it)));)
    {
    if (node == d->MRMLSliceCompositeNode || this->isLinked())
      {
      node->SetLabelOpacity(opacity);
      }
    }
  // LabelOpacityToggleButton won't fire the clicked(bool) signal here because
  // we change its check state programatically.
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
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);
       (node = static_cast<vtkMRMLSliceCompositeNode*>(
         nodes->GetNextItemAsObject(it)));)
    {
    if (node == d->MRMLSliceCompositeNode || this->isLinked())
      {
      node->SetForegroundOpacity(opacity);
      }
    }
  // LabelOpacityToggleButton won't fire the clicked(bool) signal here because
  // we change its check state programatically.
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
  vtkMRMLSliceNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    if (node == d->MRMLSliceNode) //|| this->isLinked())
      {
      node->SetPlaneLockedToCamera(lock); //&& node == d->MRMLSliceNode);
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

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceFOV(double fov)
{
  Q_D(qMRMLSliceControllerWidget);
  double oldFov[3];
  d->MRMLSliceNode->GetFieldOfView(oldFov);
  if (qAbs(qMin(oldFov[0], oldFov[1])- fov) < 0.01)
    {
    return;
    }
  if (!d->SliceLogics)
    {
    d->SliceLogic->FitFOVToBackground(fov);
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
      sliceLogic->FitFOVToBackground(fov);
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
void qMRMLSliceControllerWidget::setForegroundInterpolation(bool linear)
{
  Q_D(qMRMLSliceControllerWidget);
  if (!d->SliceLogics)
    {
    d->setForegroundInterpolation(d->SliceLogic, linear);
    return;
    }
  vtkMRMLSliceLogic* sliceLogic = 0;
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
  vtkMRMLSliceLogic* sliceLogic = 0;
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
void qMRMLSliceControllerWidget::setNavigatorEnabled(bool enable)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLCrosshairNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLCrosshairNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetNavigation(enable);
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setCrosshairMode(int mode)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLCrosshairNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLCrosshairNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetCrosshairMode(mode);
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setCrosshairThickness(int thicknessMode)
{
  Q_D(qMRMLSliceControllerWidget);
  vtkSmartPointer<vtkCollection> nodes = d->saveNodesForUndo("vtkMRMLCrosshairNode");
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLCrosshairNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetCrosshairThickness(thicknessMode);
    }
}

// --------------------------------------------------------------------------
void qMRMLSliceControllerWidget::setSliceIntersectionVisible(bool visible)
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
    node->SetSliceIntersectionVisibility(visible);
    }
}
