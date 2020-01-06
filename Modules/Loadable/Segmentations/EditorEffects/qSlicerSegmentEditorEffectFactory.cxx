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

// Segmentations EditorEffects includes
#include "qSlicerSegmentEditorEffectFactory.h"
#include "qSlicerSegmentEditorAbstractEffect.h"

// Qt includes
#include <QDebug>

//----------------------------------------------------------------------------
qSlicerSegmentEditorEffectFactory *qSlicerSegmentEditorEffectFactory::m_Instance = nullptr;

//----------------------------------------------------------------------------
/// \ingroup SlicerRt_QtModules_Segmentations
class qSlicerSegmentEditorEffectFactoryCleanup
{
public:
  inline void use()   {   }

  ~qSlicerSegmentEditorEffectFactoryCleanup()
    {
    if (qSlicerSegmentEditorEffectFactory::m_Instance)
      {
      qSlicerSegmentEditorEffectFactory::setInstance(nullptr);
      }
    }
};
static qSlicerSegmentEditorEffectFactoryCleanup qSlicerSegmentEditorEffectFactoryCleanupGlobal;

//-----------------------------------------------------------------------------
qSlicerSegmentEditorEffectFactory* qSlicerSegmentEditorEffectFactory::instance()
{
  if(!qSlicerSegmentEditorEffectFactory::m_Instance)
    {
    qSlicerSegmentEditorEffectFactoryCleanupGlobal.use();
    qSlicerSegmentEditorEffectFactory::m_Instance = new qSlicerSegmentEditorEffectFactory();
    }
  // Return the instance
  return qSlicerSegmentEditorEffectFactory::m_Instance;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorEffectFactory::setInstance(qSlicerSegmentEditorEffectFactory* instance)
{
  if (qSlicerSegmentEditorEffectFactory::m_Instance==instance)
    {
    return;
    }
  // Preferably this will be nullptr
  if (qSlicerSegmentEditorEffectFactory::m_Instance)
    {
    delete qSlicerSegmentEditorEffectFactory::m_Instance;
    }
  qSlicerSegmentEditorEffectFactory::m_Instance = instance;
  if (!instance)
    {
    return;
    }
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorEffectFactory::qSlicerSegmentEditorEffectFactory(QObject* parent)
  : QObject(parent)
{
  this->m_RegisteredEffects.clear();
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorEffectFactory::~qSlicerSegmentEditorEffectFactory()
{
  foreach(qSlicerSegmentEditorAbstractEffect* effect, m_RegisteredEffects)
  {
    delete effect;
  }
  this->m_RegisteredEffects.clear();
}

//---------------------------------------------------------------------------
bool qSlicerSegmentEditorEffectFactory::registerEffect(qSlicerSegmentEditorAbstractEffect* effectToRegister)
{
  if (effectToRegister == nullptr)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid effect to register!";
    return false;
    }
  if (effectToRegister->name().isEmpty())
    {
    qCritical() << Q_FUNC_INFO << ": Segment editor effect cannot be registered with empty name!";
    return false;
    }

  // Check if the same effect has already been registered
  qSlicerSegmentEditorAbstractEffect* currentEffect = nullptr;
  foreach (currentEffect, this->m_RegisteredEffects)
    {
    if (effectToRegister->name().compare(currentEffect->name()) == 0)
      {
      return false;
      }
    }

  // Add the effect to the list
  this->m_RegisteredEffects << effectToRegister;
  emit effectRegistered(effectToRegister->name());
  return true;
}

//---------------------------------------------------------------------------
QList<qSlicerSegmentEditorAbstractEffect*> qSlicerSegmentEditorEffectFactory::copyEffects(QList<qSlicerSegmentEditorAbstractEffect*>& effects)
{
  QList<qSlicerSegmentEditorAbstractEffect*> copiedEffects;
  foreach(qSlicerSegmentEditorAbstractEffect* effect, m_RegisteredEffects)
    {
    // If effect is added already then skip it
    bool effectAlreadyAdded = false;
    foreach(qSlicerSegmentEditorAbstractEffect* existingEffect, effects)
      {
      if (existingEffect->name() == effect->name())
        {
        // already in the list
        effectAlreadyAdded = true;
        break;
        }
      }
    if (effectAlreadyAdded)
      {
      continue;
      }

    // Effect not in the list yet, clone it and add it
    qSlicerSegmentEditorAbstractEffect* clonedEffect = effect->clone();
    if (!clonedEffect)
      {
      // make sure we don't put a nullptr pointer in the effect list
      qCritical() << Q_FUNC_INFO << " failed to clone effect: " << effect->name();
      continue;
      }
    effects << clonedEffect;
    copiedEffects << clonedEffect;
    }
  return copiedEffects;
}
