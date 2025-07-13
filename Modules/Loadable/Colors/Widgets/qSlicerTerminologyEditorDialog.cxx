/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Seattle Children's Hospital d/b/a Seattle Children's Research Institute.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, EBATINCA, S.L.
  and was funded by by Murat Maga (Seattle Children's Research Institute).

==============================================================================*/

// Colors includes
#include "qSlicerTerminologyEditorDialog.h"
#include "qSlicerTerminologyEditorWidget.h"

// Terminologies includes
#include "qSlicerTerminologyNavigatorWidget.h"
#include "vtkSlicerTerminologyEntry.h"

// Qt includes
#include <QDebug>
#include <QDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------
class qSlicerTerminologyEditorDialogPrivate : public QDialog
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qSlicerTerminologyEditorDialog);

protected:
  qSlicerTerminologyEditorDialog* const q_ptr;

public:
  explicit qSlicerTerminologyEditorDialogPrivate(qSlicerTerminologyEditorDialog& object, QWidget* parent);
  ~qSlicerTerminologyEditorDialogPrivate() override;

public:
  void init();

private:
  qSlicerTerminologyEditorWidget* EditorWidget{ nullptr };
  QPushButton* SaveButton{ nullptr };
  QPushButton* CancelButton{ nullptr };

  /// Terminology and other metadata (name, color, auto-generated flags) into which the selection is set
  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle TerminologyInfo;
};

//-----------------------------------------------------------------------------
qSlicerTerminologyEditorDialogPrivate::qSlicerTerminologyEditorDialogPrivate(qSlicerTerminologyEditorDialog& object, QWidget* parent)
  : QDialog(parent)
  , q_ptr(&object) // parent is passed to the private object to allow centering on the parent instead of on the screen
{
}

//-----------------------------------------------------------------------------
qSlicerTerminologyEditorDialogPrivate::~qSlicerTerminologyEditorDialogPrivate() = default;

//-----------------------------------------------------------------------------
void qSlicerTerminologyEditorDialogPrivate::init()
{
  Q_Q(qSlicerTerminologyEditorDialog);

  // Set up UI
  this->setWindowTitle("Edit terminology");

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);

  this->EditorWidget = new qSlicerTerminologyEditorWidget();
  layout->addWidget(this->EditorWidget);

  QHBoxLayout* buttonsLayout = new QHBoxLayout();
  buttonsLayout->setSpacing(4);
  buttonsLayout->setContentsMargins(4, 4, 4, 4);

  buttonsLayout->addSpacing(16);

  this->SaveButton = new QPushButton("Save");
  this->SaveButton->setDefault(true);
  buttonsLayout->addWidget(this->SaveButton, 2);

  this->CancelButton = new QPushButton("Cancel");
  buttonsLayout->addWidget(this->CancelButton, 1);

  layout->addLayout(buttonsLayout);

  // Make connections
  connect(this->SaveButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(this->CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

//-----------------------------------------------------------------------------
// qSlicerTerminologyEditorDialog methods

//-----------------------------------------------------------------------------
qSlicerTerminologyEditorDialog::qSlicerTerminologyEditorDialog(QObject* parent)
  : QObject(parent)
  , d_ptr(new qSlicerTerminologyEditorDialogPrivate(*this, qobject_cast<QWidget*>(parent)))
{
  Q_D(qSlicerTerminologyEditorDialog);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerTerminologyEditorDialog::qSlicerTerminologyEditorDialog(qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle& initialTerminologyInfo, QObject* parent)
  : QObject(parent)
  , d_ptr(new qSlicerTerminologyEditorDialogPrivate(*this, qobject_cast<QWidget*>(parent)))
{
  Q_D(qSlicerTerminologyEditorDialog);
  d->TerminologyInfo = initialTerminologyInfo;

  d->init();
}

//-----------------------------------------------------------------------------
qSlicerTerminologyEditorDialog::~qSlicerTerminologyEditorDialog() = default;

//-----------------------------------------------------------------------------
bool qSlicerTerminologyEditorDialog::exec()
{
  Q_D(qSlicerTerminologyEditorDialog);

  // Initialize dialog
  d->EditorWidget->setTerminologyInfo(d->TerminologyInfo);

  // Show dialog
  bool result = false;
  if (d->exec() != QDialog::Accepted)
  {
    return result;
  }

  // Save selection after clean exit
  d->EditorWidget->terminologyInfo(d->TerminologyInfo);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologyEditorDialog::getTerminology(qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle& terminologyInfo, QObject* parent)
{
  // Open terminology dialog and store result
  qSlicerTerminologyEditorDialog dialog(terminologyInfo, parent);
  bool result = dialog.exec();
  dialog.terminologyInfo(terminologyInfo);
  return result;
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologyEditorDialog::getTerminology(vtkSlicerTerminologyEntry* terminologyEntry, QObject* parent)
{
  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle terminologyInfo;
  terminologyInfo.GetTerminologyEntry()->Copy(terminologyEntry);
  // Open terminology dialog and store result
  qSlicerTerminologyEditorDialog dialog(terminologyInfo, parent);
  if (!dialog.exec())
  {
    return false;
  }
  dialog.terminologyInfo(terminologyInfo);
  terminologyEntry->Copy(terminologyInfo.GetTerminologyEntry());
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyEditorDialog::terminologyInfo(qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle& terminologyInfo)
{
  Q_D(qSlicerTerminologyEditorDialog);
  terminologyInfo = d->TerminologyInfo;
}

//-----------------------------------------------------------------------------
void qSlicerTerminologyEditorDialog::setSaveButtonEnabled(bool enabled)
{
  Q_D(qSlicerTerminologyEditorDialog);
  d->SaveButton->setEnabled(enabled);
}

#include "qSlicerTerminologyEditorDialog.moc"
