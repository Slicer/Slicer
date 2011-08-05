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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// QT includes
#include <QApplication>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QHBoxLayout>

// CTK includes
#include <ctkColorDialog.h>
#include <ctkColorPickerButton.h>
#include <ctkDoubleSlider.h>
#include <ctkSliderWidget.h>
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLItemDelegate.h"

//------------------------------------------------------------------------------
qMRMLItemDelegate::qMRMLItemDelegate(QObject *parent)
  : QStyledItemDelegate(parent)
{
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
  QVariant editData = index.data(Qt::EditRole);
  QVariant decorationData = index.data(Qt::DecorationRole);
  
  if (editData.isNull() &&
      decorationData.type() == QVariant::Color)
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
    ctkSliderWidget* slider = new ctkSliderWidget;
    slider->setDecimals(2);
    slider->setSingleStep(0.1);
    slider->setRange(0., 1.);

    QDoubleSpinBox *spinBox = slider->spinBox();
    spinBox->setFrame(false);
    spinBox->setParent(parent);

    ctkPopupWidget* popupWidget = new ctkPopupWidget;
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(slider);
    layout->setContentsMargins(0,0,0,0);
    popupWidget->setLayout(layout);
    popupWidget->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    popupWidget->setOrientation(Qt::Horizontal);
    popupWidget->setHorizontalDirection(Qt::RightToLeft);
    popupWidget->setBaseWidget(spinBox);
    
    QObject::connect(spinBox, SIGNAL(destroyed(QObject*)),
                     popupWidget, SLOT(deleteLater()));

    QObject::connect(slider, SIGNAL(valueChanged(double)),
                     this, SLOT(commitSenderData()));
    return spinBox;
    }
  return this->QStyledItemDelegate::createEditor(parent, option, index);
}

//------------------------------------------------------------------------------
void qMRMLItemDelegate::setEditorData(QWidget *editor,
                                      const QModelIndex &index) const
{
  QVariant editData = index.data(Qt::EditRole);
  QVariant decorationData = index.data(Qt::DecorationRole);

  if (editData.isNull() &&
      decorationData.type() == QVariant::Color)
    {
    QColor color = decorationData.value<QColor>();
    ctkColorPickerButton* colorPicker = qobject_cast<ctkColorPickerButton*>(editor);
    Q_ASSERT(colorPicker);
    colorPicker->blockSignals(true);
    colorPicker->setColor(color);
    colorPicker->blockSignals(false);
    if (colorPicker->property("changeColorOnSet").toBool())
      {
      colorPicker->setProperty("changeColorOnSet", false);
      colorPicker->changeColor();
      }
    }
  else if (this->is0To1Value(index))
    {
    QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(editor);
    double value = index.data(Qt::EditRole).toDouble();
    spinBox->setValue(value);
    }
  else
    {
    this->QStyledItemDelegate::setEditorData(editor, index);
    }
}

//------------------------------------------------------------------------------
void qMRMLItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
  QVariant editData = index.data(Qt::EditRole);
  QVariant decorationData = index.data(Qt::DecorationRole);
  if (editData.isNull() &&
      decorationData.type() == QVariant::Color)
    {
    ctkColorPickerButton* colorPicker = qobject_cast<ctkColorPickerButton*>(editor);
    Q_ASSERT(colorPicker);
    QColor color = colorPicker->color();
    model->setData(index, color, Qt::DecorationRole);
    }
  else if (this->is0To1Value(index))
    {
    QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(editor);
    spinBox->interpretText();
    QString value = QString::number(spinBox->value(), 'f', 2);
    model->setData(index, value, Qt::EditRole);
    }
  else
    {
    this->QStyledItemDelegate::setModelData(editor, model, index);
    }
}

//------------------------------------------------------------------------------
void qMRMLItemDelegate::commitSenderData()
{
  QWidget* editor = qobject_cast<QWidget*>(this->sender());
  ctkSliderWidget* sliderEditor = qobject_cast<ctkSliderWidget*>(editor);
  if (sliderEditor)
    {
    editor = sliderEditor->spinBox();
    }
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
  QVariant editData = index.data(Qt::EditRole);
  if (this->is0To1Value(index))
    {
    QDoubleSpinBox dummySpinBox;
    dummySpinBox.setDecimals(2);
    dummySpinBox.setRange(0., 1.);
    return dummySpinBox.sizeHint();
    }
  return this->QStyledItemDelegate::sizeHint(option, index);   
}

//------------------------------------------------------------------------------
void qMRMLItemDelegate
::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const
{
  QVariant editData = index.data(Qt::EditRole);
  QVariant decorationData = index.data(Qt::DecorationRole);
  if (editData.isNull() &&
      decorationData.type() == QVariant::Color)
    {
    editor->setGeometry(option.rect);
    }
  else if (this->is0To1Value(index))
    {
    editor->setGeometry(option.rect);
    }
  else
    {
    this->QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    }
}

//------------------------------------------------------------------------------
bool qMRMLItemDelegate::eventFilter(QObject *object, QEvent *event)
{
  QWidget *editor = qobject_cast<QWidget*>(object);
  if (editor && 
      (event->type() == QEvent::FocusOut ||
      (event->type() == QEvent::Hide && editor->isWindow())))
    {
    //the Hide event will take care of he editors that are in fact complete dialogs
    if (!editor->isActiveWindow() || (QApplication::focusWidget() != editor))
      {
      QWidget* widget = QApplication::focusWidget();
      while (widget)
        {
        ctkSliderWidget* editorSliderWidget = qobject_cast<ctkSliderWidget*>(widget);
        if (editorSliderWidget && editorSliderWidget->spinBox() == editor)
          {
          return false;
          }
        widget = widget->parentWidget();
        }
      }
    }
  return this->QStyledItemDelegate::eventFilter(object, event);
}

