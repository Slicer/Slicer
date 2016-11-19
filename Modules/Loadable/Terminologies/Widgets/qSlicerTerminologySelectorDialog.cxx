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

#include "qSlicerTerminologyNavigatorWidget.h"
#include "vtkSlicerTerminologyEntry.h"

// VTK includes
#include <vtkWeakPointer.h>

// Qt includes
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerTerminologySelectorDialogPrivate : public QDialog
{
  Q_DECLARE_PUBLIC(qSlicerTerminologySelectorDialog);
protected:
  qSlicerTerminologySelectorDialog* const q_ptr;
public:
  qSlicerTerminologySelectorDialogPrivate(qSlicerTerminologySelectorDialog& object);
  virtual ~qSlicerTerminologySelectorDialogPrivate();
public:
  void init();
private:
  qSlicerTerminologyNavigatorWidget* NavigatorWidget;
  QPushButton* SelectButton;
  QPushButton* CancelButton;

  /// Terminology entry object into which the selection is set
  vtkWeakPointer<vtkSlicerTerminologyEntry> TerminologyEntry;
  /// Selected color
  QColor Color;
};

//-----------------------------------------------------------------------------
qSlicerTerminologySelectorDialogPrivate::qSlicerTerminologySelectorDialogPrivate(qSlicerTerminologySelectorDialog& object)
  : q_ptr(&object)
{
  this->TerminologyEntry = NULL;
}

//-----------------------------------------------------------------------------
qSlicerTerminologySelectorDialogPrivate::~qSlicerTerminologySelectorDialogPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerTerminologySelectorDialogPrivate::init()
{
  Q_Q(qSlicerTerminologySelectorDialog);

  // Set up UI
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);

  this->NavigatorWidget = new qSlicerTerminologyNavigatorWidget();
  layout->addWidget(this->NavigatorWidget);

  QHBoxLayout* buttonsLayout = new QHBoxLayout();
  buttonsLayout->setSpacing(4);
  buttonsLayout->setContentsMargins(4, 0, 4, 0);

  this->SelectButton = new QPushButton("Select");
  buttonsLayout->addWidget(this->SelectButton);

  this->CancelButton = new QPushButton("Cancel");
  buttonsLayout->addWidget(this->CancelButton);

  layout->addLayout(buttonsLayout);

  // Make connections
  connect(this->SelectButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(this->CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

//-----------------------------------------------------------------------------
// qSlicerTerminologySelectorDialog methods

//-----------------------------------------------------------------------------
qSlicerTerminologySelectorDialog::qSlicerTerminologySelectorDialog(vtkSlicerTerminologyEntry* initialTerminology, QColor initialColor, QObject* parent)
  : QObject(parent)
  , d_ptr(new qSlicerTerminologySelectorDialogPrivate(*this))
{
  Q_D(qSlicerTerminologySelectorDialog);
  d->TerminologyEntry = initialTerminology;
  d->Color = initialColor;

  d->init();
}

//-----------------------------------------------------------------------------
qSlicerTerminologySelectorDialog::~qSlicerTerminologySelectorDialog()
{
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologySelectorDialog::exec()
{
  Q_D(qSlicerTerminologySelectorDialog);

  // Initialize dialog
  d->NavigatorWidget->setTerminologyEntry(d->TerminologyEntry);
  d->NavigatorWidget->setColor(d->Color);

  // Show dialog
  bool result = false;
  if (d->exec() != QDialog::Accepted)
    {
    return result;
    }

  // Save selection after clean exit
  result = d->NavigatorWidget->terminologyEntry(d->TerminologyEntry);
  d->Color = d->NavigatorWidget->customColor(); // Invalid if color has not been changed by the user from the recommended one from terminology

  return result;
}

//-----------------------------------------------------------------------------
bool qSlicerTerminologySelectorDialog::getTerminology(vtkSlicerTerminologyEntry* terminology, QColor &color, QObject* parent)
{
  if (!terminology)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid terminology argument";
    return false;
    }

  // Open terminology dialog and store result
  qSlicerTerminologySelectorDialog dialog(terminology, color, parent);
  bool result = dialog.exec();

  // Set color to output color argument
  if (dialog.color().isValid())
    {
    color.setRgb(dialog.color().rgb());
    }
  else
    {
    //TODO: Throws error message, but haven't found a better way to invalidate existing color object yet
    color.setRgb(-1,-1,-1); // Make it invalid, indicating that no custom color was set
    }

  return result;
}

//-----------------------------------------------------------------------------
QColor qSlicerTerminologySelectorDialog::color()
{
  Q_D(qSlicerTerminologySelectorDialog);
  return d->Color;
}
