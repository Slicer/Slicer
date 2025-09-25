/*==============================================================================

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

// Segmentations includes
#include "qSlicerSegmentEditorAbstractEffect.h"
#include "qSlicerSegmentEditorAbstractEffect_p.h"

#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSegmentEditorNode.h"
#include "vtkSlicerSegmentEditorLogic.h"

#include "vtkSlicerSegmentationsModuleLogic.h"

// SegmentationCore includes
#include <vtkOrientedImageData.h>
#include <vtkOrientedImageDataResample.h>

// Qt includes
#include <QColor>
#include <QDebug>
#include <QFormLayout>
#include <QFrame>
#include <QImage>
#include <QLabel>
#include <QMainWindow>
#include <QPainter>
#include <QPaintDevice>
#include <QPixmap>
#include <QSettings>

// Slicer includes
#include "qMRMLSliceWidget.h"
#include "qMRMLSliceView.h"
#include "qMRMLThreeDWidget.h"
#include "qMRMLThreeDView.h"
#include "qSlicerApplication.h"
#include "qSlicerCoreApplication.h"
#include "vtkMRMLSliceLogic.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkImageConstantPad.h>
#include <vtkImageShiftScale.h>
#include <vtkImageThreshold.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkProp.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// CTK includes
#include <ctkMessageBox.h>

//-----------------------------------------------------------------------------
// qSlicerSegmentEditorAbstractEffectPrivate methods

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffectPrivate::qSlicerSegmentEditorAbstractEffectPrivate(qSlicerSegmentEditorAbstractEffect& object)
  : q_ptr(&object)
  , Scene(nullptr)
  , SavedCursor(QCursor(Qt::ArrowCursor))
  , OptionsFrame(nullptr)
  , Logic(nullptr)
{
  this->OptionsFrame = new QFrame();
  this->OptionsFrame->setFrameShape(QFrame::NoFrame);
  this->OptionsFrame->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
  QFormLayout* layout = new QFormLayout(this->OptionsFrame);
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffectPrivate::~qSlicerSegmentEditorAbstractEffectPrivate()
{
  if (this->OptionsFrame)
  {
    delete this->OptionsFrame;
    this->OptionsFrame = nullptr;
  }
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// qSlicerSegmentEditorAbstractEffect methods

//----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffect::qSlicerSegmentEditorAbstractEffect(QObject* parent)
  : Superclass(parent)
  , m_Name(QString())
  , d_ptr(new qSlicerSegmentEditorAbstractEffectPrivate(*this))
{
}

//----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffect::~qSlicerSegmentEditorAbstractEffect() = default;

//-----------------------------------------------------------------------------
QString qSlicerSegmentEditorAbstractEffect::name() const
{
  if (m_Name.isEmpty())
  {
    qCritical() << Q_FUNC_INFO << ": Empty effect name!";
  }
  return this->m_Name;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setName(QString name)
{
  Q_UNUSED(name);
  qCritical() << Q_FUNC_INFO << ": Cannot set effect name by method, only in constructor!";
}

//-----------------------------------------------------------------------------
QString qSlicerSegmentEditorAbstractEffect::title() const
{
  if (!this->m_Title.isEmpty())
  {
    return this->m_Title;
  }
  else if (!this->m_Name.isEmpty())
  {
    return this->m_Name;
  }
  else
  {
    qWarning() << Q_FUNC_INFO << ": Empty effect title!";
    return QString();
  }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setTitle(QString title)
{
  this->m_Title = title;
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::perSegment() const
{
  return this->m_PerSegment;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setPerSegment(bool perSegment)
{
  Q_UNUSED(perSegment);
  qCritical() << Q_FUNC_INFO << ": Cannot set per-segment flag by method, only in constructor!";
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::requireSegments() const
{
  return this->m_RequireSegments;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setRequireSegments(bool requireSegments)
{
  this->m_RequireSegments = requireSegments;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::activate()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  // Show options frame
  d->OptionsFrame->setVisible(true);

  this->m_Active = true;

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::deactivate()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  // Hide options frame
  d->OptionsFrame->setVisible(false);

  this->m_Active = false;
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::active()
{
  return m_Active;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCallbackSlots(QObject* receiver, const char* selectEffectSlot, const char* updateVolumeSlot, const char* saveStateForUndoSlot)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  QObject::connect(d, SIGNAL(selectEffectSignal(QString)), receiver, selectEffectSlot);
  QObject::connect(d, SIGNAL(updateVolumeSignal(void*, bool&)), receiver, updateVolumeSlot);
  QObject::connect(d, SIGNAL(saveStateForUndoSignal()), receiver, saveStateForUndoSlot);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::applyImageMask(vtkOrientedImageData* input, vtkOrientedImageData* mask, double fillValue, bool notMask /*=false*/)
{
  // The method is now moved to vtkOrientedImageDataResample::ApplyImageMask but kept here
  // for a while for backward compatibility.
  vtkOrientedImageDataResample::ApplyImageMask(input, mask, fillValue, notMask);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::modifySelectedSegmentByLabelmap(vtkOrientedImageData* modifierLabelmap, ModificationMode modificationMode, bool bypassMasking /*=false*/)
{
  int modificationExtent[6] = { 0, -1, 0, -1, 0, -1 };
  this->modifySelectedSegmentByLabelmap(modifierLabelmap, modificationMode, modificationExtent, bypassMasking);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::modifySelectedSegmentByLabelmap(vtkOrientedImageData* modifierLabelmap,
                                                                         ModificationMode modificationMode,
                                                                         QList<int> extent,
                                                                         bool bypassMasking /*=false*/)
{
  if (extent.size() != 6)
  {
    qCritical() << Q_FUNC_INFO << " failed: extent must have 6 int values";
    return;
  }
  int modificationExtent[6] = { extent[0], extent[1], extent[2], extent[3], extent[4], extent[5] };
  this->modifySelectedSegmentByLabelmap(modifierLabelmap, modificationMode, modificationExtent, bypassMasking);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::modifySelectedSegmentByLabelmap(vtkOrientedImageData* modifierLabelmap,
                                                                         ModificationMode modificationMode,
                                                                         const int modificationExtent[6],
                                                                         bool bypassMasking /*=false*/)
{
  this->modifySegmentByLabelmap(this->parameterSetNode()->GetSegmentationNode(),
                                this->parameterSetNode()->GetSelectedSegmentID() ? this->parameterSetNode()->GetSelectedSegmentID() : "",
                                modifierLabelmap,
                                modificationMode,
                                modificationExtent,
                                bypassMasking);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::modifySegmentByLabelmap(vtkMRMLSegmentationNode* segmentationNode,
                                                                 const char* segmentID,
                                                                 vtkOrientedImageData* modifierLabelmap,
                                                                 ModificationMode modificationMode,
                                                                 bool bypassMasking /*=false*/)
{
  int modificationExtent[6] = { 0, -1, 0, -1, 0, -1 };
  this->modifySegmentByLabelmap(segmentationNode, segmentID, modifierLabelmap, modificationMode, modificationExtent, bypassMasking);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::modifySegmentByLabelmap(vtkMRMLSegmentationNode* segmentationNode,
                                                                 const char* segmentID,
                                                                 vtkOrientedImageData* modifierLabelmapInput,
                                                                 ModificationMode modificationMode,
                                                                 const int modificationExtent[6],
                                                                 bool bypassMasking /*=false*/)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->Logic)
  {
    return;
  }

  d->Logic->ModifySegmentByLabelmap(segmentationNode,
                                    segmentID,
                                    modifierLabelmapInput,
                                    static_cast<vtkSlicerSegmentEditorLogic::ModificationMode>(modificationMode),
                                    modificationExtent,
                                    perSegment(),
                                    bypassMasking);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::selectEffect(QString effectName)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  emit d->selectEffectSignal(effectName);
}

//-----------------------------------------------------------------------------
QCursor qSlicerSegmentEditorAbstractEffect::createCursor(qMRMLWidget* viewWidget)
{
  Q_UNUSED(viewWidget); // The default cursor is not view-specific, but this method can be overridden

  QImage baseImage(":Icons/NullEffect.png");
  QIcon effectIcon(this->icon());
  if (effectIcon.isNull())
  {
    QPixmap cursorPixmap = QPixmap::fromImage(baseImage);
    return QCursor(cursorPixmap, baseImage.width() / 2, 0);
  }

  QImage effectImage(effectIcon.pixmap(effectIcon.availableSizes()[0]).toImage());
  int width = qMax(baseImage.width(), effectImage.width());
  int height = baseImage.height() + effectImage.height();
  width = height = qMax(width, height);
  int center = width / 2;
  QImage cursorImage(width, height, QImage::Format_ARGB32);
  QPainter painter;
  cursorImage.fill(0);
  painter.begin(&cursorImage);
  QPoint point(center - (baseImage.width() / 2), 0);
  painter.drawImage(point, baseImage);
  int draw_x_start = center - (effectImage.width() / 2);
  int draw_y_start = cursorImage.height() - effectImage.height();
  point.setX(draw_x_start);
  point.setY(draw_y_start);
  painter.drawImage(point, effectImage);
  QRectF rectangle(draw_x_start, draw_y_start, effectImage.width(), effectImage.height() - 1);
  painter.setPen(QColor("white"));
  painter.drawRect(rectangle);
  painter.end();

  QPixmap cursorPixmap = QPixmap::fromImage(cursorImage);
  // NullEffect.png arrow point at 5 pixels right and 2 pixels down from upper left (0,0) location
  int hotX = center - (baseImage.width() / 2) + 5;
  int hotY = 2;
  return QCursor(cursorPixmap, hotX, hotY);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::cursorOff(qMRMLWidget* viewWidget)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  d->SavedCursor = QCursor(viewWidget->cursor());
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (sliceWidget && sliceWidget->sliceView())
  {
    sliceWidget->sliceView()->setDefaultViewCursor(QCursor(Qt::BlankCursor));
  }
  else if (threeDWidget && threeDWidget->threeDView())
  {
    threeDWidget->threeDView()->setDefaultViewCursor(QCursor(Qt::BlankCursor));
  }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::cursorOn(qMRMLWidget* viewWidget)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (sliceWidget && sliceWidget->sliceView())
  {
    sliceWidget->sliceView()->setDefaultViewCursor(d->SavedCursor);
  }
  else if (threeDWidget && threeDWidget->threeDView())
  {
    threeDWidget->threeDView()->setDefaultViewCursor(d->SavedCursor);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::addActor3D(qMRMLWidget* viewWidget, vtkProp3D* actor)
{
  vtkRenderer* renderer = qSlicerSegmentEditorAbstractEffect::renderer(viewWidget);
  if (renderer)
  {
    renderer->AddViewProp(actor);
    this->scheduleRender(viewWidget);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get renderer for view widget";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::addActor2D(qMRMLWidget* viewWidget, vtkActor2D* actor)
{
  vtkRenderer* renderer = qSlicerSegmentEditorAbstractEffect::renderer(viewWidget);
  if (renderer)
  {
    renderer->AddActor2D(actor);
    this->scheduleRender(viewWidget);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get renderer for view widget";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::removeActor3D(qMRMLWidget* viewWidget, vtkProp3D* actor)
{
  vtkRenderer* renderer = qSlicerSegmentEditorAbstractEffect::renderer(viewWidget);
  if (renderer)
  {
    renderer->RemoveActor(actor);
    this->scheduleRender(viewWidget);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get renderer for view widget";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::removeActor2D(qMRMLWidget* viewWidget, vtkActor2D* actor)
{
  vtkRenderer* renderer = qSlicerSegmentEditorAbstractEffect::renderer(viewWidget);
  if (renderer)
  {
    renderer->RemoveActor2D(actor);
    this->scheduleRender(viewWidget);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get renderer for view widget";
  }
}

//-----------------------------------------------------------------------------
QFrame* qSlicerSegmentEditorAbstractEffect::optionsFrame()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  return d->OptionsFrame;
}

//-----------------------------------------------------------------------------
QFormLayout* qSlicerSegmentEditorAbstractEffect::optionsLayout()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  QFormLayout* formLayout = qobject_cast<QFormLayout*>(d->OptionsFrame->layout());
  return formLayout;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::addOptionsWidget(QWidget* newOptionsWidget)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  newOptionsWidget->setParent(d->OptionsFrame);
  newOptionsWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
  this->optionsLayout()->addRow(newOptionsWidget);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::addOptionsWidget(QLayout* newOptionsWidget)
{
  this->optionsLayout()->addRow(newOptionsWidget);
}

//-----------------------------------------------------------------------------
QWidget* qSlicerSegmentEditorAbstractEffect::addLabeledOptionsWidget(QString label, QWidget* newOptionsWidget)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  QLabel* labelWidget = new QLabel(label);
  newOptionsWidget->setParent(d->OptionsFrame);
  newOptionsWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
  this->optionsLayout()->addRow(labelWidget, newOptionsWidget);
  return labelWidget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerSegmentEditorAbstractEffect::addLabeledOptionsWidget(QString label, QLayout* newOptionsWidget)
{
  QLabel* labelWidget = new QLabel(label);
  if (dynamic_cast<QHBoxLayout*>(newOptionsWidget) == nullptr)
  {
    // for multiline layouts, align label to the top
    labelWidget->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  }
  this->optionsLayout()->addRow(labelWidget, newOptionsWidget);
  return labelWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qSlicerSegmentEditorAbstractEffect::scene()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  if (!d->ParameterSetNode)
  {
    return nullptr;
  }

  return d->ParameterSetNode->GetScene();
}

//-----------------------------------------------------------------------------
vtkMRMLSegmentEditorNode* qSlicerSegmentEditorAbstractEffect::parameterSetNode()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  return d->ParameterSetNode.GetPointer();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setParameterSetNode(vtkMRMLSegmentEditorNode* node)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);

  d->ParameterSetNode = node;
}

//-----------------------------------------------------------------------------
QString qSlicerSegmentEditorAbstractEffect::parameter(QString name)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
  {
    return QString();
  }

  // Get effect-specific prefixed parameter first
  QString attributeName = QString("%1.%2").arg(this->name()).arg(name);
  const char* value = d->ParameterSetNode->GetAttribute(attributeName.toUtf8().constData());
  // Look for common parameter if effect-specific one is not found
  if (!value)
  {
    value = d->ParameterSetNode->GetAttribute(name.toUtf8().constData());
  }
  if (!value)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter named " << name << " cannot be found for effect " << this->name();
    return QString();
  }

  return QString(value);
}

//-----------------------------------------------------------------------------
int qSlicerSegmentEditorAbstractEffect::integerParameter(QString name)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
  {
    return 0;
  }

  QString parameterStr = this->parameter(name);
  bool ok = false;
  int parameterInt = parameterStr.toInt(&ok);
  if (!ok)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter named " << name << " cannot be converted to integer!";
    return 0;
  }

  return parameterInt;
}

//-----------------------------------------------------------------------------
double qSlicerSegmentEditorAbstractEffect::doubleParameter(QString name)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
  {
    return 0.0;
  }

  QString parameterStr = this->parameter(name);
  bool ok = false;
  double parameterDouble = parameterStr.toDouble(&ok);
  if (!ok)
  {
    qCritical() << Q_FUNC_INFO << ": Parameter named " << name << " cannot be converted to floating point number!";
    return 0.0;
  }

  return parameterDouble;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerSegmentEditorAbstractEffect::nodeReference(QString name)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
  {
    return nullptr;
  }

  // Get effect-specific prefixed parameter first
  QString attributeName = QString("%1.%2").arg(this->name()).arg(name);
  vtkMRMLNode* node = d->ParameterSetNode->GetNodeReference(attributeName.toUtf8().constData());
  // Look for common parameter if effect-specific one is not found
  if (!node)
  {
    node = d->ParameterSetNode->GetNodeReference(name.toUtf8().constData());
  }
  return node;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setParameter(QString name, QString value)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node set to effect " << this->name();
    return;
  }

  // Set parameter as attribute
  QString attributeName = QString("%1.%2").arg(this->name()).arg(name);
  this->setCommonParameter(attributeName, value);
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::parameterDefined(QString name)
{
  QString attributeName = QString("%1.%2").arg(this->name()).arg(name);
  return this->commonParameterDefined(attributeName);
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::commonParameterDefined(QString name)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
  {
    return false;
  }
  const char* existingValue = d->ParameterSetNode->GetAttribute(name.toUtf8().constData());
  return (existingValue != nullptr && strlen(existingValue) > 0);
}

//-----------------------------------------------------------------------------
int qSlicerSegmentEditorAbstractEffect::confirmCurrentSegmentVisible()
{
  if (!this->parameterSetNode())
  {
    // no parameter set node - do not prevent operation
    return ConfirmedWithoutDialog;
  }
  vtkMRMLSegmentationNode* segmentationNode = this->parameterSetNode()->GetSegmentationNode();
  if (!segmentationNode)
  {
    // no segmentation node - do not prevent operation
    return ConfirmedWithoutDialog;
  }
  char* segmentID = this->parameterSetNode()->GetSelectedSegmentID();
  if (!segmentID || strlen(segmentID) == 0)
  {
    // no selected segment, probably this effect operates on the entire segmentation - do not prevent operation
    return ConfirmedWithoutDialog;
  }

  // If segment visibility is already confirmed for this segment then we don't need to ask again
  // (important for effects that are interrupted when painting/drawing on the image, because displaying a popup
  // interferes with painting on the image)
  vtkSegment* selectedSegment = nullptr;
  if (segmentationNode->GetSegmentation())
  {
    selectedSegment = segmentationNode->GetSegmentation()->GetSegment(segmentID);
  }
  if (this->m_AlreadyConfirmedSegmentVisible == selectedSegment)
  {
    return ConfirmedWithoutDialog;
  }

  int numberOfDisplayNodes = segmentationNode->GetNumberOfDisplayNodes();
  for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
  {
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetNthDisplayNode(displayNodeIndex));
    if (displayNode && displayNode->GetVisibility() && displayNode->GetSegmentVisibility(segmentID))
    {
      // segment already visible
      return ConfirmedWithoutDialog;
    }
  }

  qSlicerApplication* app = qSlicerApplication::application();
  QWidget* mainWindow = app ? app->mainWindow() : nullptr;

  ctkMessageBox* confirmCurrentSegmentVisibleMsgBox = new ctkMessageBox(mainWindow);
  confirmCurrentSegmentVisibleMsgBox->setAttribute(Qt::WA_DeleteOnClose);
  confirmCurrentSegmentVisibleMsgBox->setWindowTitle(tr("Operate on invisible segment?"));
  confirmCurrentSegmentVisibleMsgBox->setText(tr("The currently selected segment is hidden. Would you like to make it visible?"));

  confirmCurrentSegmentVisibleMsgBox->addButton(QMessageBox::Yes);
  confirmCurrentSegmentVisibleMsgBox->addButton(QMessageBox::No);
  confirmCurrentSegmentVisibleMsgBox->addButton(QMessageBox::Cancel);

  confirmCurrentSegmentVisibleMsgBox->setDontShowAgainVisible(true);
  confirmCurrentSegmentVisibleMsgBox->setDontShowAgainSettingsKey("Segmentations/ConfirmEditHiddenSegment");
  confirmCurrentSegmentVisibleMsgBox->addDontShowAgainButtonRole(QMessageBox::YesRole);
  confirmCurrentSegmentVisibleMsgBox->addDontShowAgainButtonRole(QMessageBox::NoRole);

  confirmCurrentSegmentVisibleMsgBox->setIcon(QMessageBox::Question);

  QSettings settings;
  int savedButtonSelection = settings.value(confirmCurrentSegmentVisibleMsgBox->dontShowAgainSettingsKey(), static_cast<int>(QMessageBox::InvalidRole)).toInt();

  int resultCode = confirmCurrentSegmentVisibleMsgBox->exec();

  // Cancel means that user did not let the operation to proceed
  if (resultCode == QMessageBox::Cancel)
  {
    return NotConfirmed;
  }

  // User chose to show the current segment
  if (resultCode == QMessageBox::Yes)
  {
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
    if (displayNode)
    {
      displayNode->SetVisibility(true);
      displayNode->SetSegmentVisibility(segmentID, true);
    }
  }
  else
  {
    // User confirmed that it is OK to work on this invisible segment
    this->m_AlreadyConfirmedSegmentVisible = selectedSegment;
  }

  // User confirmed that the operation can go on (did not click Cancel)
  if (savedButtonSelection == static_cast<int>(QMessageBox::InvalidRole))
  {
    return ConfirmedWithDialog;
  }
  else
  {
    return ConfirmedWithoutDialog;
  }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setParameterDefault(QString name, QString value)
{
  if (this->parameterDefined(name))
  {
    return;
  }
  this->setParameter(name, value);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCommonParameter(QString name, QString value)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node set to effect " << this->name();
    return;
  }

  const char* oldValue = d->ParameterSetNode->GetAttribute(name.toUtf8().constData());
  if (oldValue == nullptr && value.isEmpty())
  {
    // no change
    return;
  }
  if (value == QString(oldValue))
  {
    // no change
    return;
  }

  // Disable full modified events in all cases (observe EffectParameterModified instead if necessary)
  int disableState = d->ParameterSetNode->GetDisableModifiedEvent();
  d->ParameterSetNode->SetDisableModifiedEvent(1);

  // Set parameter as attribute
  d->ParameterSetNode->SetAttribute(name.toUtf8().constData(), value.toUtf8().constData());

  // Re-enable full modified events for parameter node
  d->ParameterSetNode->SetDisableModifiedEvent(disableState);

  // Emit parameter modified event if requested
  // Don't pass parameter name as char pointer, as custom modified events may be compressed and invoked after EndModify()
  // and by that time the pointer may not be valid anymore.
  d->ParameterSetNode->InvokeCustomModifiedEvent(vtkMRMLSegmentEditorNode::EffectParameterModified);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCommonParameterDefault(QString name, QString value)
{
  if (this->commonParameterDefined(name))
  {
    return;
  }
  this->setCommonParameter(name, value);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setParameter(QString name, int value)
{
  this->setParameter(name, QString::number(value));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setParameterDefault(QString name, int value)
{
  if (this->parameterDefined(name))
  {
    return;
  }
  this->setParameter(name, value);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCommonParameter(QString name, int value)
{
  this->setCommonParameter(name, QString::number(value));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCommonParameterDefault(QString name, int value)
{
  if (this->commonParameterDefined(name))
  {
    return;
  }
  this->setCommonParameter(name, value);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setParameter(QString name, double value)
{
  this->setParameter(name, QString::number(value));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setParameterDefault(QString name, double value)
{
  if (this->parameterDefined(name))
  {
    return;
  }
  this->setParameter(name, value);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCommonParameter(QString name, double value)
{
  this->setCommonParameter(name, QString::number(value));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCommonParameterDefault(QString name, double value)
{
  if (this->commonParameterDefined(name))
  {
    return;
  }
  this->setCommonParameter(name, value);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setNodeReference(QString name, vtkMRMLNode* node)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node set to effect " << this->name();
    return;
  }

  // Set parameter as attribute
  QString attributeName = QString("%1.%2").arg(this->name()).arg(name);
  this->setCommonNodeReference(attributeName, node);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setCommonNodeReference(QString name, vtkMRMLNode* node)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  if (!d->ParameterSetNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node set to effect " << this->name();
    return;
  }

  vtkMRMLNode* oldNode = d->ParameterSetNode->GetNodeReference(name.toUtf8().constData());
  if (node == oldNode)
  {
    // no change
    return;
  }

  // Set parameter as attribute
  d->ParameterSetNode->SetNodeReferenceID(name.toUtf8().constData(), node ? node->GetID() : nullptr);

  // Emit parameter modified event
  // Don't pass parameter name as char pointer, as custom modified events may be compressed and invoked after EndModify()
  // and by that time the pointer may not be valid anymore.
  d->ParameterSetNode->InvokeCustomModifiedEvent(vtkMRMLSegmentEditorNode::EffectParameterModified);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setVolumes(vtkOrientedImageData* alignedSourceVolume,
                                                    vtkOrientedImageData* modifierLabelmap,
                                                    vtkOrientedImageData* maskLabelmap,
                                                    vtkOrientedImageData* selectedSegmentLabelmap,
                                                    vtkOrientedImageData* referenceGeometryImage)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  d->ModifierLabelmap = modifierLabelmap;
  d->MaskLabelmap = maskLabelmap;
  d->AlignedSourceVolume = alignedSourceVolume;
  d->SelectedSegmentLabelmap = selectedSegmentLabelmap;
  d->ReferenceGeometryImage = referenceGeometryImage;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* qSlicerSegmentEditorAbstractEffect::defaultModifierLabelmap()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  bool success = false;
  emit d->updateVolumeSignal(d->ModifierLabelmap.GetPointer(), success); // this resets the labelmap and clears it
  if (!success)
  {
    return nullptr;
  }
  return d->ModifierLabelmap;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* qSlicerSegmentEditorAbstractEffect::modifierLabelmap()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  return d->ModifierLabelmap;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* qSlicerSegmentEditorAbstractEffect::maskLabelmap()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  bool success = false;
  emit d->updateVolumeSignal(d->MaskLabelmap.GetPointer(), success);
  if (!success)
  {
    return nullptr;
  }
  return d->MaskLabelmap;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* qSlicerSegmentEditorAbstractEffect::sourceVolumeImageData()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  bool success = false;
  emit d->updateVolumeSignal(d->AlignedSourceVolume.GetPointer(), success);
  if (!success)
  {
    return nullptr;
  }
  return d->AlignedSourceVolume;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* qSlicerSegmentEditorAbstractEffect::masterVolumeImageData()
{
  qWarning("qSlicerSegmentEditorAbstractEffect::masterVolumeImageData() method is deprecated,"
           " use sourceVolumeImageData method instead");
  return this->sourceVolumeImageData();
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* qSlicerSegmentEditorAbstractEffect::selectedSegmentLabelmap()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  bool success = false;
  emit d->updateVolumeSignal(d->SelectedSegmentLabelmap.GetPointer(), success);
  if (!success)
  {
    return nullptr;
  }
  return d->SelectedSegmentLabelmap;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* qSlicerSegmentEditorAbstractEffect::referenceGeometryImage()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  bool success = false;
  emit d->updateVolumeSignal(d->ReferenceGeometryImage.GetPointer(), success); // this resets the labelmap and clears it
  if (!success)
  {
    return nullptr;
  }
  return d->ReferenceGeometryImage;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::saveStateForUndo()
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  emit d->saveStateForUndoSignal();
}

//-----------------------------------------------------------------------------
vtkRenderWindow* qSlicerSegmentEditorAbstractEffect::renderWindow(qMRMLWidget* viewWidget)
{
  if (!viewWidget)
  {
    return nullptr;
  }

  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (sliceWidget)
  {
    if (!sliceWidget->sliceView())
    {
      // probably the application is closing
      return nullptr;
    }
    return sliceWidget->sliceView()->renderWindow();
  }
  else if (threeDWidget)
  {
    if (!threeDWidget->threeDView())
    {
      // probably the application is closing
      return nullptr;
    }
    return threeDWidget->threeDView()->renderWindow();
  }

  qCritical() << Q_FUNC_INFO << ": Unsupported view widget type!";
  return nullptr;
}

//-----------------------------------------------------------------------------
vtkRenderer* qSlicerSegmentEditorAbstractEffect::renderer(qMRMLWidget* viewWidget)
{
  vtkRenderWindow* renderWindow = qSlicerSegmentEditorAbstractEffect::renderWindow(viewWidget);
  if (!renderWindow)
  {
    return nullptr;
  }

  return vtkRenderer::SafeDownCast(renderWindow->GetRenderers()->GetItemAsObject(0));
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractViewNode* qSlicerSegmentEditorAbstractEffect::viewNode(qMRMLWidget* viewWidget)
{
  if (!viewWidget)
  {
    return nullptr;
  }

  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (sliceWidget)
  {
    return sliceWidget->sliceLogic()->GetSliceNode();
  }
  else if (threeDWidget)
  {
    return threeDWidget->mrmlViewNode();
  }

  qCritical() << Q_FUNC_INFO << ": Unsupported view widget type!";
  return nullptr;
}

//-----------------------------------------------------------------------------
QPoint qSlicerSegmentEditorAbstractEffect::rasToXy(double ras[3], qMRMLSliceWidget* sliceWidget)
{
  auto xy = vtkSlicerSegmentEditorLogic::RasToXy(ras, qSlicerSegmentEditorAbstractEffect::sliceNode(sliceWidget));
  return { xy[0], xy[1] };
}

//-----------------------------------------------------------------------------
QPoint qSlicerSegmentEditorAbstractEffect::rasToXy(QVector3D rasVector, qMRMLSliceWidget* sliceWidget)
{
  double ras[3] = { rasVector.x(), rasVector.y(), rasVector.z() };
  return qSlicerSegmentEditorAbstractEffect::rasToXy(ras, sliceWidget);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::xyzToRas(double inputXyz[3], double outputRas[3], qMRMLSliceWidget* sliceWidget)
{
  vtkSlicerSegmentEditorLogic::XyzToRas(inputXyz, outputRas, qSlicerSegmentEditorAbstractEffect::sliceNode(sliceWidget));
}

//-----------------------------------------------------------------------------
QVector3D qSlicerSegmentEditorAbstractEffect::xyzToRas(QVector3D inputXyzVector, qMRMLSliceWidget* sliceWidget)
{
  double inputXyz[3] = { inputXyzVector.x(), inputXyzVector.y(), inputXyzVector.z() };
  double outputRas[3] = { 0.0, 0.0, 0.0 };
  qSlicerSegmentEditorAbstractEffect::xyzToRas(inputXyz, outputRas, sliceWidget);
  QVector3D outputVector(outputRas[0], outputRas[1], outputRas[2]);
  return outputVector;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::xyToRas(QPoint xy, double outputRas[3], qMRMLSliceWidget* sliceWidget)
{
  double xyz[3] = { static_cast<double>(xy.x()), static_cast<double>(xy.y()), 0.0 };

  qSlicerSegmentEditorAbstractEffect::xyzToRas(xyz, outputRas, sliceWidget);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::xyToRas(double xy[2], double outputRas[3], qMRMLSliceWidget* sliceWidget)
{
  double xyz[3] = { xy[0], xy[1], 0.0 };
  qSlicerSegmentEditorAbstractEffect::xyzToRas(xyz, outputRas, sliceWidget);
}

//-----------------------------------------------------------------------------
QVector3D qSlicerSegmentEditorAbstractEffect::xyToRas(QPoint xy, qMRMLSliceWidget* sliceWidget)
{
  double outputRas[3] = { 0.0, 0.0, 0.0 };
  qSlicerSegmentEditorAbstractEffect::xyToRas(xy, outputRas, sliceWidget);
  QVector3D outputVector(outputRas[0], outputRas[1], outputRas[2]);
  return outputVector;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::xyzToIjk(double inputXyz[3],
                                                  int outputIjk[3],
                                                  qMRMLSliceWidget* sliceWidget,
                                                  vtkOrientedImageData* image,
                                                  vtkMRMLTransformNode* parentTransformNode /*=nullptr*/)
{
  vtkSlicerSegmentEditorLogic::XyzToIjk(inputXyz, outputIjk, qSlicerSegmentEditorAbstractEffect::sliceNode(sliceWidget), image, parentTransformNode);
}

//-----------------------------------------------------------------------------
QVector3D qSlicerSegmentEditorAbstractEffect::xyzToIjk(QVector3D inputXyzVector,
                                                       qMRMLSliceWidget* sliceWidget,
                                                       vtkOrientedImageData* image,
                                                       vtkMRMLTransformNode* parentTransformNode /*=nullptr*/)
{
  double inputXyz[3] = { inputXyzVector.x(), inputXyzVector.y(), inputXyzVector.z() };
  int outputIjk[3] = { 0, 0, 0 };
  qSlicerSegmentEditorAbstractEffect::xyzToIjk(inputXyz, outputIjk, sliceWidget, image, parentTransformNode);

  QVector3D outputVector(outputIjk[0], outputIjk[1], outputIjk[2]);
  return outputVector;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::xyToIjk(QPoint xy,
                                                 int outputIjk[3],
                                                 qMRMLSliceWidget* sliceWidget,
                                                 vtkOrientedImageData* image,
                                                 vtkMRMLTransformNode* parentTransformNode /*=nullptr*/)
{
  double xyz[3] = { static_cast<double>(xy.x()), static_cast<double>(xy.y()), 0.0 };
  qSlicerSegmentEditorAbstractEffect::xyzToIjk(xyz, outputIjk, sliceWidget, image, parentTransformNode);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::xyToIjk(double xy[2],
                                                 int outputIjk[3],
                                                 qMRMLSliceWidget* sliceWidget,
                                                 vtkOrientedImageData* image,
                                                 vtkMRMLTransformNode* parentTransformNode /*=nullptr*/)
{
  double xyz[3] = { xy[0], xy[0], 0.0 };
  qSlicerSegmentEditorAbstractEffect::xyzToIjk(xyz, outputIjk, sliceWidget, image, parentTransformNode);
}

//-----------------------------------------------------------------------------
QVector3D qSlicerSegmentEditorAbstractEffect::xyToIjk(QPoint xy, qMRMLSliceWidget* sliceWidget, vtkOrientedImageData* image, vtkMRMLTransformNode* parentTransformNode /*=nullptr*/)
{
  int outputIjk[3] = { 0, 0, 0 };
  qSlicerSegmentEditorAbstractEffect::xyToIjk(xy, outputIjk, sliceWidget, image, parentTransformNode);
  QVector3D outputVector(outputIjk[0], outputIjk[1], outputIjk[2]);
  return outputVector;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::forceRender(qMRMLWidget* viewWidget)
{
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (sliceWidget)
  {
    sliceWidget->sliceView()->forceRender();
  }
  if (threeDWidget)
  {
    threeDWidget->threeDView()->forceRender();
  }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::scheduleRender(qMRMLWidget* viewWidget)
{
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
  if (sliceWidget)
  {
    sliceWidget->sliceView()->scheduleRender();
  }
  if (threeDWidget)
  {
    threeDWidget->threeDView()->scheduleRender();
  }
}

//----------------------------------------------------------------------------
double qSlicerSegmentEditorAbstractEffect::sliceSpacing(qMRMLSliceWidget* sliceWidget)
{
  vtkMRMLSliceLogic* sliceLogic = sliceWidget->sliceLogic();
  return vtkSlicerSegmentEditorLogic::GetSliceSpacing(sliceLogic ? sliceLogic->GetSliceNode() : nullptr, sliceLogic);
}

//----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::showEffectCursorInSliceView()
{
  return m_ShowEffectCursorInSliceView;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setShowEffectCursorInSliceView(bool show)
{
  this->m_ShowEffectCursorInSliceView = show;
}

//----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::showEffectCursorInThreeDView()
{
  return m_ShowEffectCursorInThreeDView;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setShowEffectCursorInThreeDView(bool show)
{
  this->m_ShowEffectCursorInThreeDView = show;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::interactionNodeModified(vtkMRMLInteractionNode* interactionNode)
{
  if (interactionNode == nullptr)
  {
    return;
  }
  // Deactivate the effect if user switched to markup placement mode
  // to avoid double effect (e.g., paint & place fiducial at the same time)
  if (interactionNode->GetCurrentInteractionMode() != vtkMRMLInteractionNode::ViewTransform)
  {
    this->selectEffect("");
  }
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorAbstractEffect::segmentationDisplayableInView(vtkMRMLAbstractViewNode* viewNode)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  return d->Logic ? d->Logic->IsSegmentationDisplayableInView(viewNode) : false;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractEffect::setSegmentEditorLogic(vtkSlicerSegmentEditorLogic* segmentEditorLogic)
{
  Q_D(qSlicerSegmentEditorAbstractEffect);
  d->Logic = segmentEditorLogic;
}

//-----------------------------------------------------------------------------
vtkMRMLSliceNode* qSlicerSegmentEditorAbstractEffect::sliceNode(qMRMLSliceWidget* viewWidget)
{
  if (!viewWidget)
  {
    return nullptr;
  }

  return viewWidget->sliceLogic()->GetSliceNode();
}
