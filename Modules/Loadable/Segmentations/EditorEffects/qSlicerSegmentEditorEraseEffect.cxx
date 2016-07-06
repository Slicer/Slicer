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

==============================================================================*/

// Segmentations includes
#include "qSlicerSegmentEditorEraseEffect.h"

//-----------------------------------------------------------------------------
class qSlicerSegmentEditorEraseEffectPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSegmentEditorEraseEffect);
protected:
  qSlicerSegmentEditorEraseEffect* const q_ptr;
public:
  qSlicerSegmentEditorEraseEffectPrivate(qSlicerSegmentEditorEraseEffect& object);
  ~qSlicerSegmentEditorEraseEffectPrivate();
public:
  QIcon EraseIcon;
};

//-----------------------------------------------------------------------------
qSlicerSegmentEditorEraseEffectPrivate::qSlicerSegmentEditorEraseEffectPrivate(qSlicerSegmentEditorEraseEffect& object)
  : q_ptr(&object)
{
  this->EraseIcon = QIcon(":Icons/Erase.png");
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorEraseEffectPrivate::~qSlicerSegmentEditorEraseEffectPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSegmentEditorEraseEffect methods

//----------------------------------------------------------------------------
qSlicerSegmentEditorEraseEffect::qSlicerSegmentEditorEraseEffect(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSegmentEditorEraseEffectPrivate(*this) )
{
  this->m_Name = QString("Erase");
  this->m_Erase = true;
}

//----------------------------------------------------------------------------
qSlicerSegmentEditorEraseEffect::~qSlicerSegmentEditorEraseEffect()
{
}

//---------------------------------------------------------------------------
QIcon qSlicerSegmentEditorEraseEffect::icon()
{
  Q_D(qSlicerSegmentEditorEraseEffect);

  return d->EraseIcon;
}

//---------------------------------------------------------------------------
QString const qSlicerSegmentEditorEraseEffect::helpText()const
{
  return QString("Left-click and drag in slice or 3D viewers to erase from current segment with a round brush.");
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffect* qSlicerSegmentEditorEraseEffect::clone()
{
  return new qSlicerSegmentEditorEraseEffect();
}
