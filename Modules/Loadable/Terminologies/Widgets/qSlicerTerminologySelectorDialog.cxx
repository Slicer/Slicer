/*==============================================================================

  Program: 3D Slicer

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

// Terminologies includes
#include "qSlicerTerminologySelectorDialog.h"

#include "vtkSlicerTerminologyEntry.h"

// Qt includes
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Terminologies_Widgets
class qSlicerTerminologySelectorDialogPrivate : public QDialog
{
  Q_DECLARE_PUBLIC(qSlicerTerminologySelectorDialog);
protected:
  qSlicerTerminologySelectorDialog* const q_ptr;
public:
  qSlicerTerminologySelectorDialogPrivate(qSlicerTerminologySelectorDialog& object);
  ~qSlicerTerminologySelectorDialogPrivate() override;
public:
  void init();
private:
  qSlicerTerminologyNavigatorWidget* NavigatorWidget{nullptr};
  QPushButton* SelectButton{nullptr};
  QPushButton* CancelButton{nullptr};

  /// Terminology and other metadata (name, color, auto-generated flags) into which the selection is set
  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle TerminologyInfo;
};

//-----------------------------------------------------------------------------
qSlicerTerminologySelectorDialogPrivate::qSlicerTerminologySelectorDialogPrivate(qSlicerTerminologySelectorDialog& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerTerminologySelectorDialogPrivate::~qSlicerTerminologySelectorDialogPrivate() = default;

//-----------------------------------------------------------------------------
void qSlicerTerminologySelectorDialogPrivate::init()
{
  Q_Q(qSlicerTerminologySelectorDialog);

  // Set up UI
  this->setWindowTitle("Terminology");

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);

  this->NavigatorWidget = new qSlicerTerminologyNavigatorWidget();
  layout->addWidget(this->NavigatorWidget);

  QHBoxLayout* buttonsLayout = new QHBoxLayout();
  buttonsLayout->setSpacing(4);
  buttonsLayout->setContentsMargins(4, 4, 4, 4);

  this->SelectButton = new QPushButton("Select");
  this->SelectButton->setDefault(true);
  this->SelectButton->setEnabled(false); // Disabled until terminology selection becomes valid
  buttonsLayout->addWidget(this->SelectButton, 2);

  this->CancelButton = new QPushButton("Cancel");
  buttonsLayout->addWidget(this->CancelButton, 1);

  layout->addLayout(buttonsLayout);

  // Make connections
  connect(this->NavigatorWidget, SIGNAL(selectionValidityChanged(bool)), q, SLOT(setSelectButtonEnabled(bool)));
  connect(this->NavigatorWidget, SIGNAL(typeDoubleClicked()), this, SLOT(accept()));
  connect(this->SelectButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(this->CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

//-----------------------------------------------------------------------------
// qSlicerTerminologySelectorDialog methods

//-----------------------------------------------------------------------------
qSlicerTerminologySelectorDialog::qSlicerTerminologySelectorDialog(QObject* parent)
  : QObject(parent)
  , d_ptr(new qSlicerTerminologySelectorDialogPrivate(*this))
{
  Q_D(qSlicerTerminologySelectorDialog);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerTerminologySelectorDialog::qSlicerTerminologySelectorDialog(
  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle &initialTerminologyInfo, QObject* parent)
  : QObject(parent)
  , d_ptr(new qSlicerTerminologySelectorDialogPrivate(*this))
{
  Q_D(qSlicerTerminologySelectorDialog);
  d->TerminologyInfo = initialTerminologyInfo;

  d->init();
}

//-----------------------------------------------------------------------------
qSlicerTerminologySelectorDialog::~qSlicerTerminologySelectorDialog() = default;

//-----------------------------------------------------------------------------
bool qSlicerTerminologySelectorDialog::exec()
{
  Q_D(qSlicerTerminologySelectorDialog);

  // Initialize dialog
  d->NavigatorWidget->setTerminologyInfo(d->TerminologyInfo);

  // Show dialog
  bool result = false;
  if (d->exec() != QDialog::Accepted)
    {
    return result;
    }

  // Save selection after clean exit
  d->NavigatorWidget->terminologyInfo(d->TerminologyInfo);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologySelectorDialog::getTerminology(
  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle &terminologyInfo, QObject* parent)
{
  // Open terminology dialog and store result
  qSlicerTerminologySelectorDialog dialog(terminologyInfo, parent);
  bool result = dialog.exec();
  dialog.terminologyInfo(terminologyInfo);
  return result;
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologySelectorDialog::getTerminology(vtkSlicerTerminologyEntry* terminologyEntry, QObject* parent)
{
  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle terminologyInfo;
  terminologyInfo.GetTerminologyEntry()->Copy(terminologyEntry);
  // Open terminology dialog and store result
  qSlicerTerminologySelectorDialog dialog(terminologyInfo, parent);
  dialog.setOverrideSectionVisible(false);
  if (!dialog.exec())
    {
    return false;
    }
  dialog.terminologyInfo(terminologyInfo);
  terminologyEntry->Copy(terminologyInfo.GetTerminologyEntry());
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerTerminologySelectorDialog::terminologyInfo(
  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle &terminologyInfo )
{
  Q_D(qSlicerTerminologySelectorDialog);
  terminologyInfo = d->TerminologyInfo;
}

//-----------------------------------------------------------------------------
void qSlicerTerminologySelectorDialog::setSelectButtonEnabled(bool enabled)
{
  Q_D(qSlicerTerminologySelectorDialog);
  d->SelectButton->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologySelectorDialog::overrideSectionVisible() const
{
  Q_D(const qSlicerTerminologySelectorDialog);
  return d->NavigatorWidget->overrideSectionVisible();
}

//-----------------------------------------------------------------------------
void qSlicerTerminologySelectorDialog::setOverrideSectionVisible(bool visible)
{
  Q_D(qSlicerTerminologySelectorDialog);
  d->NavigatorWidget->setOverrideSectionVisible(visible);
}
