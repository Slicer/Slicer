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
#include <QDoubleSpinBox>

// CTK includes
#include <ctkColorDialog.h>
#include <ctkColorPickerButton.h>

// qMRML includes
#include "qMRMLItemDelegate.h"

//------------------------------------------------------------------------------
qMRMLItemDelegate::qMRMLItemDelegate(QObject *parent)
  : QStyledItemDelegate(parent)
{
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
  else if (editData.type() == QVariant::Double ||
           static_cast<QMetaType::Type>(editData.type()) == QMetaType::Float)
    {
    // Right now we only have opacity, if you need to handle other kind of 
    // floating data, use a Qt::ItemRole to know what range to use. Maybe
    // tooltip or accessible ?
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setDecimals(2);
    editor->setSingleStep(0.1);
    editor->setMinimum(0.);
    editor->setMaximum(1.);
    return editor;
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
  else if (editData.type() == QVariant::Double ||
           static_cast<QMetaType::Type>(editData.type()) == QMetaType::Float)
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
  else if (editData.type() == QVariant::Double ||
           static_cast<QMetaType::Type>(editData.type()) == QMetaType::Float)
    {
    QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox*>(editor);
    spinBox->interpretText();
    double value = spinBox->value();
    model->setData(index, value, Qt::EditRole);
    }
  else
    {
    this->QStyledItemDelegate::setModelData(editor, model, index);
    }
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
  if (editData.type() == QVariant::Double ||
      static_cast<QMetaType::Type>(editData.type()) == QMetaType::Float)
    {
    QDoubleSpinBox dummySpinBox;
    dummySpinBox.setDecimals(2);
    dummySpinBox.setValue(1.00);
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
  else if (editData.type() == QVariant::Double ||
           static_cast<QMetaType::Type>(editData.type()) == QMetaType::Float)
    {
    editor->setGeometry(option.rect);
    }
  else
    {
    this->QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    }
}
