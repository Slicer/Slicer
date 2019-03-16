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

#ifndef __qSlicerSegmentEditorEffectFactory_h
#define __qSlicerSegmentEditorEffectFactory_h

// Segmentations Editor Effects includes
#include "qSlicerSegmentationsEditorEffectsExport.h"

// Qt includes
#include <QObject>
#include <QList>

class qSlicerSegmentEditorAbstractEffect;
class qSlicerSegmentEditorEffectFactoryCleanup;

/// \ingroup SlicerRt_QtModules_Segmentations
/// \class qSlicerSegmentEditorEffectFactory
/// \brief Singleton class managing segment editor effect plugins
class Q_SLICER_SEGMENTATIONS_EFFECTS_EXPORT qSlicerSegmentEditorEffectFactory : public QObject
{
  Q_OBJECT

public:
  /// Instance getter for the singleton class
  /// \return Instance object
  Q_INVOKABLE static qSlicerSegmentEditorEffectFactory* instance();

  /// Allows cleanup of the singleton at application exit
  static void setInstance(qSlicerSegmentEditorEffectFactory* instance);

public:
  /// Register a effect
  /// \return True if effect registered successfully, false otherwise
  Q_INVOKABLE bool registerEffect(qSlicerSegmentEditorAbstractEffect* effect);

  /// Get list of registered effects
  Q_INVOKABLE QList<qSlicerSegmentEditorAbstractEffect*> registeredEffects() { return m_RegisteredEffects; };

  /// Copy list of registered effects to the container in a segment editor widget.
  /// Effects that are already in the list (have the same name) will not be modified.
  /// \return List of added effects (does not include effects that were already in the effects argument).
  Q_INVOKABLE QList<qSlicerSegmentEditorAbstractEffect*> copyEffects(QList<qSlicerSegmentEditorAbstractEffect*>& effects);

signals:
  /// Signals that a new effect has been registered.
  void effectRegistered(QString);

protected:
  /// List of registered effect instances
  QList<qSlicerSegmentEditorAbstractEffect*> m_RegisteredEffects;

public:
  /// Private constructor made public to enable python wrapping
  /// IMPORTANT: Should not be used for creating effect handler! Use instance() instead.
  qSlicerSegmentEditorEffectFactory(QObject* parent=nullptr);

  /// Private destructor made public to enable python wrapping
  ~qSlicerSegmentEditorEffectFactory() override;

private:
  Q_DISABLE_COPY(qSlicerSegmentEditorEffectFactory);
  friend class qSlicerSegmentEditorEffectFactoryCleanup;

private:
  /// Instance of the singleton
  static qSlicerSegmentEditorEffectFactory* m_Instance;
};

#endif
