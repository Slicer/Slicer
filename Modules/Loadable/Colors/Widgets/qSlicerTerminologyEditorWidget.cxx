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

// Colors includes
#include "qSlicerTerminologyEditorWidget.h"
#include "ui_qSlicerTerminologyEditorWidget.h"

// Terminologies includes
#include "qSlicerTerminologySelectorDialog.h"

//------------------------------------------------------------------------------
class qSlicerTerminologyEditorWidgetPrivate : public Ui_qSlicerTerminologyEditorWidget
{
  Q_DECLARE_PUBLIC(qSlicerTerminologyEditorWidget);
protected:
  qSlicerTerminologyEditorWidget* const q_ptr;
public:
  qSlicerTerminologyEditorWidgetPrivate(qSlicerTerminologyEditorWidget& object);
  void init();

  void updateGUIFromTerminologyInfo();

  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle TerminologyInfo;
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

  QObject::connect(this->selectFromTerminologyButton, SIGNAL(clicked()), q, SLOT(onSelectFromTerminology()));
}

//------------------------------------------------------------------------------
void qSlicerTerminologyEditorWidgetPrivate::updateGUIFromTerminologyInfo()
{
  Q_Q(qSlicerTerminologyEditorWidget);

  // Set terminology IDs on the GUI
  this->categoryCodeMeaningLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetCategoryObject()->GetCodeMeaning());
  this->categoryCodeValueLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetCategoryObject()->GetCodeValue());
  this->categoryCSDLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetCategoryObject()->GetCodingSchemeDesignator());

  this->typeCodeMeaningLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetTypeObject()->GetCodeMeaning());
  this->typeCodeValueLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetTypeObject()->GetCodeValue());
  this->typeCSDLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetTypeObject()->GetCodingSchemeDesignator());

  this->typeModifierCodeMeaningLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetTypeModifierObject()->GetCodeMeaning());
  this->typeModifierCodeValueLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetTypeModifierObject()->GetCodeValue());
  this->typeModifierCSDLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetTypeModifierObject()->GetCodingSchemeDesignator());

  this->anatomicRegionCodeMeaningLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetAnatomicRegionObject()->GetCodeMeaning());
  this->anatomicRegionCodeValueLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetAnatomicRegionObject()->GetCodeValue());
  this->anatomicRegionCSDLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetAnatomicRegionObject()->GetCodingSchemeDesignator());

  this->anatomicRegionModifierCodeMeaningLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetAnatomicRegionModifierObject()->GetCodeMeaning());
  this->anatomicRegionModifierCodeValueLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetAnatomicRegionModifierObject()->GetCodeValue());
  this->anatomicRegionModifierCSDLineEdit->setText(TerminologyInfo.GetTerminologyEntry()->GetAnatomicRegionModifierObject()->GetCodingSchemeDesignator());
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
  terminologyInfo = d->TerminologyInfo;
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyEditorWidget::setTerminologyInfo(qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle &terminologyInfo)
{
  Q_D(qSlicerTerminologyEditorWidget);
  d->TerminologyInfo = terminologyInfo;
  d->updateGUIFromTerminologyInfo();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyEditorWidget::onSelectFromTerminology()
{
  Q_D(qSlicerTerminologyEditorWidget);

  qSlicerTerminologySelectorDialog::getTerminology(d->TerminologyInfo, this);
  d->updateGUIFromTerminologyInfo();
}
