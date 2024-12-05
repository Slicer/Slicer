/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Seattle Children’s Hospital d/b/a Seattle Children’s Research Institute.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, EBATINCA, S.L.
  and was funded by by Murat Maga (Seattle Children’s Research Institute).

==============================================================================*/

// qMRML includes
#include "qSlicerTerminologyEditorWidget.h"
#include "ui_qSlicerTerminologyEditorWidget.h"

//------------------------------------------------------------------------------
class qSlicerTerminologyEditorWidgetPrivate : public Ui_qSlicerTerminologyEditorWidget
{
  Q_DECLARE_PUBLIC(qSlicerTerminologyEditorWidget);
protected:
  qSlicerTerminologyEditorWidget* const q_ptr;
public:
  qSlicerTerminologyEditorWidgetPrivate(qSlicerTerminologyEditorWidget& object);
  void init();
};

//------------------------------------------------------------------------------
qSlicerTerminologyEditorWidgetPrivate::qSlicerTerminologyEditorWidgetPrivate(qSlicerTerminologyEditorWidget& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qSlicerTerminologyEditorWidgetPrivate::init()
{
  Q_Q(qSlicerTerminologyEditorWidget);
  this->setupUi(q);
}

//------------------------------------------------------------------------------
qSlicerTerminologyEditorWidget::qSlicerTerminologyEditorWidget(QWidget *_parent)
  : QWidget(_parent)
  , d_ptr(new qSlicerTerminologyEditorWidgetPrivate(*this))
{
  Q_D(qSlicerTerminologyEditorWidget);
  d->init();
}

//------------------------------------------------------------------------------
qSlicerTerminologyEditorWidget::~qSlicerTerminologyEditorWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerTerminologyEditorWidget::terminologyInfo(qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle &terminologyInfo)
{
  Q_D(qSlicerTerminologyEditorWidget);

  terminologyInfo.GetTerminologyEntry()->GetCategoryObject()->SetCodeMeaning(
    d->categoryCodeMeaningLineEdit->text().toUtf8().constData());
  terminologyInfo.GetTerminologyEntry()->GetCategoryObject()->SetCodeValue(
    d->categoryCodeValueLineEdit->text().toUtf8().constData());
  terminologyInfo.GetTerminologyEntry()->GetCategoryObject()->SetCodingSchemeDesignator(
    d->categoryCSDLineEdit->text().toUtf8().constData());

  terminologyInfo.GetTerminologyEntry()->GetTypeObject()->SetCodeMeaning(
    d->typeCodeMeaningLineEdit->text().toUtf8().constData());
  terminologyInfo.GetTerminologyEntry()->GetTypeObject()->SetCodeValue(
    d->typeCodeValueLineEdit->text().toUtf8().constData());
  terminologyInfo.GetTerminologyEntry()->GetTypeObject()->SetCodingSchemeDesignator(
    d->typeCSDLineEdit->text().toUtf8().constData());

  terminologyInfo.GetTerminologyEntry()->GetTypeModifierObject()->SetCodeMeaning(
    d->typeModifierCodeMeaningLineEdit->text().toUtf8().constData());
  terminologyInfo.GetTerminologyEntry()->GetTypeModifierObject()->SetCodeValue(
    d->typeModifierCodeValueLineEdit->text().toUtf8().constData());
  terminologyInfo.GetTerminologyEntry()->GetTypeModifierObject()->SetCodingSchemeDesignator(
    d->typeModifierCSDLineEdit->text().toUtf8().constData());

  terminologyInfo.GetTerminologyEntry()->GetAnatomicRegionObject()->SetCodeMeaning(
    d->anatomicRegionCodeMeaningLineEdit->text().toUtf8().constData());
  terminologyInfo.GetTerminologyEntry()->GetAnatomicRegionObject()->SetCodeValue(
    d->anatomicRegionCodeValueLineEdit->text().toUtf8().constData());
  terminologyInfo.GetTerminologyEntry()->GetAnatomicRegionObject()->SetCodingSchemeDesignator(
    d->anatomicRegionCSDLineEdit->text().toUtf8().constData());

  terminologyInfo.GetTerminologyEntry()->GetAnatomicRegionModifierObject()->SetCodeMeaning(
    d->anatomicRegionModifierCodeMeaningLineEdit->text().toUtf8().constData());
  terminologyInfo.GetTerminologyEntry()->GetAnatomicRegionModifierObject()->SetCodeValue(
    d->anatomicRegionModifierCodeValueLineEdit->text().toUtf8().constData());
  terminologyInfo.GetTerminologyEntry()->GetAnatomicRegionModifierObject()->SetCodingSchemeDesignator(
    d->anatomicRegionModifierCSDLineEdit->text().toUtf8().constData());
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyEditorWidget::setTerminologyInfo(qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle &terminologyInfo)
{
  Q_D(qSlicerTerminologyEditorWidget);

  d->categoryCodeMeaningLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetCategoryObject()->GetCodeMeaning());
  d->categoryCodeValueLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetCategoryObject()->GetCodeValue());
  d->categoryCSDLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetCategoryObject()->GetCodingSchemeDesignator());

  d->typeCodeMeaningLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetTypeObject()->GetCodeMeaning());
  d->typeCodeValueLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetTypeObject()->GetCodeValue());
  d->typeCSDLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetTypeObject()->GetCodingSchemeDesignator());

  d->typeModifierCodeMeaningLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetTypeModifierObject()->GetCodeMeaning());
  d->typeModifierCodeValueLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetTypeModifierObject()->GetCodeValue());
  d->typeModifierCSDLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetTypeModifierObject()->GetCodingSchemeDesignator());

  d->anatomicRegionCodeMeaningLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetAnatomicRegionObject()->GetCodeMeaning());
  d->anatomicRegionCodeValueLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetAnatomicRegionObject()->GetCodeValue());
  d->anatomicRegionCSDLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetAnatomicRegionObject()->GetCodingSchemeDesignator());

  d->anatomicRegionModifierCodeMeaningLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetAnatomicRegionModifierObject()->GetCodeMeaning());
  d->anatomicRegionModifierCodeValueLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetAnatomicRegionModifierObject()->GetCodeValue());
  d->anatomicRegionModifierCSDLineEdit->setText(terminologyInfo.GetTerminologyEntry()->GetAnatomicRegionModifierObject()->GetCodingSchemeDesignator());
}
