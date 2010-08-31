// Qt includes
#include <QDebug>
#include <QComboBox>
#include <QTreeView>

// CTK includes
#include <ctkTreeComboBox.h>

// qMRML includes
#include "qMRMLColorTableComboBox.h"
#include "qMRMLSceneColorTableModel.h"

//-----------------------------------------------------------------------------
class qMRMLColorTableComboBoxPrivate: public ctkPrivate<qMRMLColorTableComboBox>
{
public:
};

// --------------------------------------------------------------------------
qMRMLColorTableComboBox::qMRMLColorTableComboBox(QWidget* parentWidget)
  : Superclass(this->createSceneModel(), parentWidget)
{
  CTK_INIT_PRIVATE(qMRMLColorTableComboBox);
  this->rootModel()->setParent(this);
  ctkTreeComboBox* comboBox = new ctkTreeComboBox;
  this->setComboBox(comboBox);
  this->setShowHidden(true);
  QStringList nodeTypes;
  nodeTypes << QString("vtkMRMLColorTableNode");
  nodeTypes << QString("vtkMRMLProceduralColorNode");
  this->setNodeTypes(nodeTypes);

  QIcon defaultIcon(":blankLUT");
  QList<QSize> iconSizes(defaultIcon.availableSizes());
  if (iconSizes.size() > 0)
    {
    comboBox->setIconSize(iconSizes[0]);
    }
}

// --------------------------------------------------------------------------
qMRMLColorTableComboBox::~qMRMLColorTableComboBox()
{
}

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
  // must be called after the scene is set (model initialized)
  qobject_cast<ctkTreeComboBox*>(this->comboBox())->treeView()->hideColumn(1);
}
