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
  and CANARIE.

==============================================================================*/
// Segmentations includes
#include "qMRMLSegmentationGeometryDialog.h"

#include "vtkMRMLSegmentationNode.h"

// Qt includes
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Segmentations_Widgets
class qMRMLSegmentationGeometryDialogPrivate : public QDialog
{
  Q_DECLARE_PUBLIC(qMRMLSegmentationGeometryDialog);
protected:
  qMRMLSegmentationGeometryDialog* const q_ptr;
public:
  qMRMLSegmentationGeometryDialogPrivate(qMRMLSegmentationGeometryDialog& object);
  ~qMRMLSegmentationGeometryDialogPrivate() override;
public:
  void init();
private:
  vtkMRMLSegmentationNode* SegmentationNode;
  bool ResampleLabelmaps;

  qMRMLSegmentationGeometryWidget* GeometryWidget;
  QPushButton* OKButton;
  QPushButton* CancelButton;
};

//-----------------------------------------------------------------------------
qMRMLSegmentationGeometryDialogPrivate::qMRMLSegmentationGeometryDialogPrivate(qMRMLSegmentationGeometryDialog& object)
  : q_ptr(&object)
  , ResampleLabelmaps(false)
{
}

//-----------------------------------------------------------------------------
qMRMLSegmentationGeometryDialogPrivate::~qMRMLSegmentationGeometryDialogPrivate() = default;

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryDialogPrivate::init()
{
  Q_Q(qMRMLSegmentationGeometryDialog);

  // Set up UI
  this->setWindowTitle("Segmentation geometry");

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setSpacing(4);
  layout->setContentsMargins(4, 4, 4, 6);

  this->GeometryWidget = new qMRMLSegmentationGeometryWidget();
  this->GeometryWidget->setSegmentationNode(this->SegmentationNode);
  layout->addWidget(this->GeometryWidget);

  //layout->addStretch(1);

  QHBoxLayout* buttonsLayout = new QHBoxLayout();
  buttonsLayout->setSpacing(4);
  buttonsLayout->setContentsMargins(4, 0, 4, 0);

  this->OKButton = new QPushButton("OK");
  buttonsLayout->addWidget(this->OKButton);

  this->CancelButton = new QPushButton("Cancel");
  buttonsLayout->addWidget(this->CancelButton);
  this->CancelButton->setVisible(this->GeometryWidget->editEnabled());

  layout->addLayout(buttonsLayout);

  // Make connections
  connect(this->OKButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(this->CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

//-----------------------------------------------------------------------------
// qMRMLSegmentationGeometryDialog methods

//-----------------------------------------------------------------------------
qMRMLSegmentationGeometryDialog::qMRMLSegmentationGeometryDialog(vtkMRMLSegmentationNode* segmentationNode, QObject* parent)
  : QObject(parent)
  , d_ptr(new qMRMLSegmentationGeometryDialogPrivate(*this))
{
  Q_D(qMRMLSegmentationGeometryDialog);
  d->SegmentationNode = segmentationNode;

  d->init();
}

//-----------------------------------------------------------------------------
qMRMLSegmentationGeometryDialog::~qMRMLSegmentationGeometryDialog() = default;

//-----------------------------------------------------------------------------
bool qMRMLSegmentationGeometryDialog::editEnabled()const
{
  Q_D(const qMRMLSegmentationGeometryDialog);
  return d->GeometryWidget->editEnabled();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryDialog::setEditEnabled(bool aEditEnabled)
{
  Q_D(qMRMLSegmentationGeometryDialog);
  d->GeometryWidget->setEditEnabled(aEditEnabled);
  d->CancelButton->setVisible(aEditEnabled);
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentationGeometryDialog::resampleLabelmaps()const
{
  Q_D(const qMRMLSegmentationGeometryDialog);
  return d->ResampleLabelmaps;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryDialog::setResampleLabelmaps(bool aResampleLabelmaps)
{
  Q_D(qMRMLSegmentationGeometryDialog);
  d->ResampleLabelmaps = aResampleLabelmaps;
  if (aResampleLabelmaps)
    {
    d->OKButton->setToolTip("Set reference image geometry and resample all segment labelmaps");
    }
  else
    {
    d->OKButton->setToolTip("Set reference image geometry (do not resample)");
    }
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentationGeometryDialog::exec()
{
  Q_D(qMRMLSegmentationGeometryDialog);

  // Initialize dialog
  d->GeometryWidget->setSegmentationNode(d->SegmentationNode);

  // Show dialog
  bool result = false;
  if (d->exec() != QDialog::Accepted)
    {
    return result;
    }

  MRMLNodeModifyBlocker blocker(d->SegmentationNode);

  // Apply geometry after clean exit
  if (d->GeometryWidget->editEnabled())
    {
    d->GeometryWidget->setReferenceImageGeometryForSegmentationNode();
    if (d->ResampleLabelmaps)
      {
      d->GeometryWidget->resampleLabelmapsInSegmentationNode();
      }
    }
  return true;
}
