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
#include <QEvent>
#include <QHBoxLayout>
#include <QDebug>

// CTK includes
#include <ctkColorDialog.h>
#include <ctkColorPickerButton.h>
#include <ctkDoubleSlider.h>
#include <ctkSliderWidget.h>
#include <ctkPopupWidget.h>
#include <ctkDoubleSpinBox.h>

// qMRML includes
#include "qMRMLColorModel.h"
#include "qMRMLItemDelegate.h"

//------------------------------------------------------------------------------
qMRMLItemDelegate::qMRMLItemDelegate(QObject *parent)
  : QStyledItemDelegate(parent)
{
  this->DummySpinBox = new ctkDoubleSpinBox(nullptr);
  this->DummySpinBox->setDecimals(2);
  this->DummySpinBox->setRange(0., 1.);
}

//------------------------------------------------------------------------------
qMRMLItemDelegate::~qMRMLItemDelegate()
{
  delete this->DummySpinBox;
  this->DummySpinBox = nullptr;
}

//------------------------------------------------------------------------------
bool qMRMLItemDelegate::isColor(const QModelIndex& index)const
{
  QVariant editData = index.data(Qt::EditRole);
  QVariant decorationData = index.data(Qt::DecorationRole);
  if (editData.isNull() &&
      decorationData.type() == QVariant::Color)
    {
    return true;
    }
  else if ( editData.isNull() &&
            decorationData.type() == QVariant::Pixmap &&
            index.data(qMRMLColorModel::ColorRole).type() == QVariant::Color )
    {
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
int qMRMLItemDelegate::colorRole(const QModelIndex& index)const
{
  QVariant decorationData = index.data(Qt::DecorationRole);
  QVariant colorData = index.data(qMRMLColorModel::ColorRole);
  if (decorationData.type() == QVariant::Color)
    {
    return Qt::DecorationRole;
    }
  else if (colorData.type() == QVariant::Color)
    {
    return qMRMLColorModel::ColorRole;
    }
  return -1;
}

//------------------------------------------------------------------------------
bool qMRMLItemDelegate::is0To1Value(const QModelIndex& index)const
{
  QVariant editData = index.data(Qt::EditRole);
  if (editData.type() != QVariant::String)
    {
    return false;
    }
  QRegExp regExp0To1With2Decimals("[01]\\.[0-9][0-9]");
  bool res=  regExp0To1With2Decimals.exactMatch(editData.toString());
  return res;
}

//------------------------------------------------------------------------------
QWidget *qMRMLItemDelegate
::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
               const QModelIndex &index) const
{
  if (this->isColor(index))
    {
    ctkColorPickerButton* colorPicker = new ctkColorPickerButton(parent);
    colorPicker->setProperty("changeColorOnSet", true);
    colorPicker->setDisplayColorName(false);
    ctkColorPickerButton::ColorDialogOptions options
      = ctkColorPickerButton::ShowAlphaChannel
      | ctkColorPickerButton::UseCTKColorDialog;
    colorPicker->setDialogOptions(options);
    connect(colorPicker, SIGNAL(colorChanged(QColor)),
            this, SLOT(commitAndClose()),Qt::QueuedConnection);
    return colorPicker;
    }
  else if (this->is0To1Value(index))
    {
    ctkSliderWidget* slider = new ctkSliderWidget(nullptr);
    slider->setDecimals(2);
    slider->setSingleStep(0.1);
    slider->setRange(0., 1.);
    slider->setPopupSlider(true);
    slider->popup()->layout()->setSizeConstraint(QLayout::SetMinimumSize);
    slider->setParent(parent);

    ctkDoubleSpinBox *spinBox = slider->spinBox();
    spinBox->setFrame(false);
    QObject::connect(slider, SIGNAL(valueChanged(double)),
                     this, SLOT(commitSenderData()));
    return slider;
    }
  return this->Superclass::createEditor(parent, option, index);
}

//------------------------------------------------------------------------------
void qMRMLItemDelegate::setEditorData(QWidget *editor,
                                      const QModelIndex &index) const
{
  if (this->isColor(index))
    {
    QColor color = index.data(this->colorRole(index)).value<QColor>();
    ctkColorPickerButton* colorPicker = qobject_cast<ctkColorPickerButton*>(editor);
    if (colorPicker) // colorPicker may be nullptr, don't make the application crash when that happens
      {
      colorPicker->blockSignals(true);
      colorPicker->setColor(color);
      colorPicker->blockSignals(false);
      if (colorPicker->property("changeColorOnSet").toBool())
        {
        colorPicker->setProperty("changeColorOnSet", false);
        colorPicker->changeColor();
        }
      }
    else
      {
      qWarning() << Q_FUNC_INFO << " failed: colorPicker is invalid";
      }
    }
  else if (this->is0To1Value(index))
    {
    ctkSliderWidget *sliderWidget = qobject_cast<ctkSliderWidget*>(editor);
    double value = index.data(Qt::EditRole).toDouble();
    if (sliderWidget) // sliderWidget may be nullptr, don't make the application crash when that happens
      {
      sliderWidget->setValue(value);
      }
    else
      {
      qWarning() << Q_FUNC_INFO << " failed: sliderWidget is invalid";
      }
    }
  else
    {
    this->Superclass::setEditorData(editor, index);
    }
}

//------------------------------------------------------------------------------
void qMRMLItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
  if (this->isColor(index))
    {
    ctkColorPickerButton* colorPicker = qobject_cast<ctkColorPickerButton*>(editor);
    Q_ASSERT(colorPicker);
    QColor color = colorPicker->color();
    // the color role depends on what is the underlying model.
    model->setData(index, color, this->colorRole(index));
    }
  else if (this->is0To1Value(index))
    {
    ctkSliderWidget *sliderWidget = qobject_cast<ctkSliderWidget*>(editor);
    QString value = QString::number(sliderWidget->value(), 'f', 2);
    model->setData(index, value, Qt::EditRole);
    }
  else
    {
    this->Superclass::setModelData(editor, model, index);
    }
}

//------------------------------------------------------------------------------
void qMRMLItemDelegate::commitSenderData()
{
  QWidget* editor = qobject_cast<QWidget*>(this->sender());
  emit commitData(editor);
}

//------------------------------------------------------------------------------
void qMRMLItemDelegate::commitAndClose()
{
  QWidget* editor = qobject_cast<QWidget*>(this->sender());
  emit commitData(editor);
  emit closeEditor(editor);
}

//------------------------------------------------------------------------------
QSize qMRMLItemDelegate
::sizeHint(const QStyleOptionViewItem &option,
           const QModelIndex &index) const
{
  if (this->is0To1Value(index))
    {
    return this->DummySpinBox->sizeHint();
    }
  return this->Superclass::sizeHint(option, index);
}

//------------------------------------------------------------------------------
void qMRMLItemDelegate
::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const
{
  if (this->isColor(index))
    {
    editor->setGeometry(option.rect);
    }
  else if (this->is0To1Value(index))
    {
    editor->setGeometry(option.rect);
    }
  else
    {
    this->Superclass::updateEditorGeometry(editor, option, index);
    }
}

//------------------------------------------------------------------------------
bool qMRMLItemDelegate::eventFilter(QObject *object, QEvent *event)
{
  ctkSliderWidget* editor = qobject_cast<ctkSliderWidget*>(object);
  if (editor &&
      (event->type() == QEvent::FocusOut ||
      (event->type() == QEvent::Hide && editor->isWindow())))
    {
    // The Hide event will take care of he editors that are in fact complete dialogs
    if (!editor->isActiveWindow() || (QApplication::focusWidget() != editor))
      {
      QWidget* widget = QApplication::focusWidget();
      while (widget)
        {
        ctkPopupWidget* sliderPopupWidget = qobject_cast<ctkPopupWidget*>(widget);
        if (sliderPopupWidget && sliderPopupWidget == qobject_cast<ctkSliderWidget*>(editor)->popup())
          {
          return false;
          }
        widget = widget->parentWidget();
        }
      }
    }
  return this->Superclass::eventFilter(object, event);
}

