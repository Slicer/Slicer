/*==============================================================================

  Program: 3D Slicer

  Copyright(c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLI18N_h
#define __vtkMRMLI18N_h

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkObject.h>

class vtkMRMLTranslator;

/// \brief Class that provide internationalization (i18n) features,
/// such as language translation or region-specific units and date formatting.
///
class VTK_MRML_EXPORT vtkMRMLI18N : public vtkObject
{
public:
  vtkTypeMacro(vtkMRMLI18N, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// Return the singleton instance with no reference counting.
  static vtkMRMLI18N* GetInstance();

  ///
  /// This is a singleton pattern New.  There will only be ONE
  /// reference to a vtkMRMLI18N object per process.  Clients that
  /// call this must call Delete on the object so that the reference
  /// counting will work. The single instance will be unreferenced when
  /// the program exits.
  static vtkMRMLI18N* New();

  /// Translate message with the current translator
  static std::string Translate(const char* context,
                               const char* sourceText,
                               const char* disambiguation = nullptr,
                               int n = -1);

  /// Set translator object. This class takes ownership of the translator
  /// and it releases it when the process quits.
  void SetTranslator(vtkMRMLTranslator* translator);

  /// Get translator object that can translate text that is displayed to the user
  /// to the currently chosen language.
  vtkGetObjectMacro(Translator, vtkMRMLTranslator);

protected:
  vtkMRMLI18N();
  ~vtkMRMLI18N() override;
  vtkMRMLI18N(const vtkMRMLI18N&);
  void operator=(const vtkMRMLI18N&);

  ///
  /// Singleton management functions.
  static void classInitialize();
  static void classFinalize();

  friend class vtkMRMLI18NInitialize;
  typedef vtkMRMLI18N Self;

  vtkMRMLTranslator* Translator{ nullptr };
};

/// Utility class to make sure qSlicerModuleManager is initialized before it is used.
class VTK_MRML_EXPORT vtkMRMLI18NInitialize
{
public:
  typedef vtkMRMLI18NInitialize Self;

  vtkMRMLI18NInitialize();
  ~vtkMRMLI18NInitialize();

private:
  static unsigned int Count;
};

/// This instance will show up in any translation unit that uses
/// vtkMRMLI18N.  It will make sure vtkMRMLI18N is initialized
/// before it is used.
static vtkMRMLI18NInitialize vtkMRMLI18NInitializer;

/// Translation function used in MRML classes
#define vtkMRMLTr(context, sourceText) vtkMRMLI18N::Translate(context, sourceText)

#endif
