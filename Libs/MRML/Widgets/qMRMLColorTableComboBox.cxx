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

// CTK includes
#include <ctkTreeComboBox.h>

// qMRML includes
#include "qMRMLColorTableComboBox.h"
#include "qMRMLSceneColorTableModel.h"

//-----------------------------------------------------------------------------
class qMRMLColorTableComboBoxPrivate
{
  Q_DECLARE_PUBLIC(qMRMLColorTableComboBox);
protected:
  qMRMLColorTableComboBox* const q_ptr;
public:
  qMRMLColorTableComboBoxPrivate(qMRMLColorTableComboBox& object);
  void init();
};

// -----------------------------------------------------------------------------
qMRMLColorTableComboBoxPrivate
::qMRMLColorTableComboBoxPrivate(qMRMLColorTableComboBox& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qMRMLColorTableComboBoxPrivate::init()
{
  Q_Q(qMRMLColorTableComboBox);
  q->rootModel()->setParent(q);
  ctkTreeComboBox* comboBox = new ctkTreeComboBox;
  // only the first column is visible
  comboBox->setVisibleModelColumn(0);
  q->setComboBox(comboBox);
  q->setShowHidden(true);

  QStringList nodeTypes;
  nodeTypes << QString("vtkMRMLColorTableNode");
  nodeTypes << QString("vtkMRMLProceduralColorNode");
  q->setNodeTypes(nodeTypes);
  q->setAddEnabled(false);
  q->setRemoveEnabled(false);

  QIcon defaultIcon(":blankLUT");
  QList<QSize> iconSizes(defaultIcon.availableSizes());
  if (iconSizes.size() > 0)
    {
    comboBox->setIconSize(iconSizes[0]);
    }
}

// --------------------------------------------------------------------------
qMRMLColorTableComboBox::qMRMLColorTableComboBox(QWidget* parentWidget)
  : Superclass(this->createSceneModel(), parentWidget)
  , d_ptr(new qMRMLColorTableComboBoxPrivate(*this))
{
  Q_D(qMRMLColorTableComboBox);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLColorTableComboBox::~qMRMLColorTableComboBox() = default;

// --------------------------------------------------------------------------
QAbstractItemModel* qMRMLColorTableComboBox::createSceneModel()
{
  return new qMRMLSceneColorTableModel;
}

// --------------------------------------------------------------------------
void qMRMLColorTableComboBox::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  QModelIndex sceneIndex = this->comboBox()->model()->index(0,0);
  // index(0,0) is the scene.
  this->comboBox()->setRootModelIndex(sceneIndex);
}
