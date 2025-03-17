/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkMRMLCornerTextLogic - slicer logic class for corner annotations
// .SECTION Description
// This class manages the logic associated with rendering corner annotations


#ifndef __vtkMRMLCornerTextLogic_h
#define __vtkMRMLCornerTextLogic_h

// VTK includes
#include <vtkSetGet.h>
#include <vtkSmartPointer.h>
class vtkXMLDataElement;
class vtkXMLDataParser;

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"
#include "vtkMRMLLogicExport.h"

// MRML includes
class vtkMRMLAnnotationNode;
class vtkMRMLSliceNode;
class vtkMRMLTextNode;
class vtkMRMLAbstractAnnotationPropertyValueProvider;

// STD includes
#include <array>
#include <cstdlib>
#include <string>
#include <unordered_map>

class VTK_MRML_LOGIC_EXPORT vtkMRMLCornerTextLogic :
  public vtkMRMLAbstractLogic
{
public:
  enum TextLocation {
    CORNER_BL=0,
    CORNER_BR,
    CORNER_TL,
    CORNER_TR,
    EDGE_B,
    EDGE_R,
    EDGE_L,
    EDGE_T,
    TextLocation_Last
  };

  /// The Usual vtk class functions
  static vtkMRMLCornerTextLogic *New();
  vtkTypeMacro(vtkMRMLCornerTextLogic, vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// CornerText Logic functions
  static vtkMRMLTextNode* GetCornerAnnotations(vtkMRMLScene*, const int viewArrangement, const std::string& viewName);
  bool RegisterPropertyValueProvider(const std::string& pluginName, vtkMRMLAbstractAnnotationPropertyValueProvider*);
  std::array<std::string, 8> GenerateAnnotations(vtkMRMLSliceNode*, vtkMRMLTextNode*, bool printWarnings = true);

  /// @{
  /// Enable/disable the display of annotations in slice views.
  ///
  /// When enabled, annotations such as corner text will be shown on slice views.
  /// Disabling this will hide all slice view annotations.
  ///
  /// Default value: true.
  vtkGetMacro(SliceViewAnnotationsEnabled, bool);
  vtkSetMacro(SliceViewAnnotationsEnabled, bool);
  vtkBooleanMacro(SliceViewAnnotationsEnabled, bool);
  /// @}

  /// @{
  /// Enable/disable corner annotations in the bottom-left position.
  ///
  /// If enabled, corner text annotations will be displayed in the bottom-left
  /// corner of the slice view.
  ///
  /// Default value: true.
  ///
  /// \sa SetSliceViewAnnotationsEnabled()
  vtkGetMacro(BottomLeftEnabled, bool);
  vtkSetMacro(BottomLeftEnabled, bool);
  vtkBooleanMacro(BottomLeftEnabled, bool);
  /// @}

  /// @{
  /// Enable/disable corner annotations in the top-left position.
  ///
  /// If enabled, corner text annotations will be displayed in the top-left
  /// corner of the slice view.
  ///
  /// Default value: true.
  ///
  /// \sa SetSliceViewAnnotationsEnabled()
  vtkGetMacro(TopLeftEnabled, bool);
  vtkSetMacro(TopLeftEnabled, bool);
  vtkBooleanMacro(TopLeftEnabled, bool);
  /// @}

  /// @{
  /// Enable/disable corner annotations in the top-right position.
  ///
  /// If enabled, corner text annotations will be displayed in the top-right
  /// corner of the slice view.
  ///
  /// Default value: true.
  ///
  /// \sa SetSliceViewAnnotationsEnabled()
  vtkGetMacro(TopRightEnabled, bool);
  vtkSetMacro(TopRightEnabled, bool);
  vtkBooleanMacro(TopRightEnabled, bool);
  /// @}

  /// @{
  /// Set/Get the maximum display level to be included in corner annotations.
  ///
  /// This property defines the upper limit for display levels that should be
  /// considered when generating annotations. Display levels less than or equal
  /// to this value will be included.
  ///
  /// Example:
  /// - If set to 3, only levels 0, 1, 2, and 3 will be considered.
  ///
  /// Default value: 3.
  vtkGetMacro(IncludeDisplayLevelsLte, int);
  vtkSetMacro(IncludeDisplayLevelsLte, int);
  /// @}

  /// @{
  /// Set/Get the font size for corner text annotations.
  ///
  /// This property determines the font size used for displaying annotations.
  /// Larger values will increase the text size.
  ///
  /// Default value: 14.
  ///
  /// \sa SetFontFamily()
  vtkGetMacro(FontSize, int);
  vtkSetMacro(FontSize, int);
  /// @}

  /// @{
  /// Set/Get the font family used for corner text annotations.
  ///
  /// The font family determines the typeface used for rendering annotation text.
  /// Example values: "Arial", "Times", "Courier".
  ///
  /// Default value: "Times".
  ///
  /// \sa SetFontSize()
  vtkGetMacro(FontFamily, std::string);
  vtkSetMacro(FontFamily, std::string);
  /// @}

protected:
  vtkMRMLCornerTextLogic();
  ~vtkMRMLCornerTextLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;

  /// Parses the contents of an XMl-based text node into a vtkXMLDataElement.
  /// The vtkXMLDataElement will be used by other functions to generate the
  /// actual string intended to be rendered on a given slice view.
  vtkXMLDataElement* ParseTextNode(vtkXMLDataParser* parser, vtkMRMLTextNode* textNode);

private:

  const std::unordered_map<std::string, TextLocation> positionMap =
  {
      {"bottom-left", CORNER_BL},
      {"bottom-right", CORNER_BR},
      {"top-left", CORNER_TL},
      {"top-right", CORNER_TR},
      {"bottom", EDGE_B},
      {"right", EDGE_R},
      {"left", EDGE_L},
      {"top", EDGE_T}
  };

  vtkMRMLCornerTextLogic(const vtkMRMLCornerTextLogic&); // Not implemented
  void operator=(const vtkMRMLCornerTextLogic&); // Not implemented

  bool SliceViewAnnotationsEnabled{true};
  bool BottomLeftEnabled{true}, TopLeftEnabled{true}, TopRightEnabled{true};
  int IncludeDisplayLevelsLte{3};
  int FontSize{14};
  std::string FontFamily{"Times"};
  std::unordered_map<
      std::string,
      vtkSmartPointer<vtkMRMLAbstractAnnotationPropertyValueProvider>>
  RegisteredProviders;
};

#endif
