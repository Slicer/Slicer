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

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"
#include "vtkMRMLLogicExport.h"

// MRML includes
class vtkMRMLAnnotationNode;
class vtkMRMLSliceNode;
class vtkMRMLTextNode;
class vtkXMLDataElement;
class vtkMRMLAbstractAnnotationPropertyValueProvider;

// STD includes
#include <cstdlib>
#include <array>
#include <string>

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
  static vtkMRMLTextNode *GetCornerAnnotations(vtkMRMLScene *,
                                               const int viewArrangement,
                                               const std::string& viewName);
  bool RegisterPropertyValueProvider(const std::string &pluginName,
                                vtkMRMLAbstractAnnotationPropertyValueProvider *);
  std::array<std::string, 8> GenerateAnnotations(vtkMRMLSliceNode *,
                                                vtkMRMLTextNode *,
                                                bool printWarnings = true);

  vtkGetMacro(SliceViewAnnotationsEnabled, bool);
  vtkSetMacro(SliceViewAnnotationsEnabled, bool);
  vtkBooleanMacro(SliceViewAnnotationsEnabled, bool);

  vtkGetMacro(BottomLeftEnabled, bool);
  vtkSetMacro(BottomLeftEnabled, bool);
  vtkBooleanMacro(BottomLeftEnabled, bool);

  vtkGetMacro(TopLeftEnabled, bool);
  vtkSetMacro(TopLeftEnabled, bool);
  vtkBooleanMacro(TopLeftEnabled, bool);

  vtkGetMacro(TopRightEnabled, bool);
  vtkSetMacro(TopRightEnabled, bool);
  vtkBooleanMacro(TopRightEnabled, bool);

  vtkGetMacro(DisplayStrictness, int);
  vtkSetMacro(DisplayStrictness, int);

  vtkGetMacro(FontSize, int);
  vtkSetMacro(FontSize, int);

  vtkGetMacro(FontFamily, std::string);
  vtkSetMacro(FontFamily, std::string);

protected:
  vtkMRMLCornerTextLogic();
  ~vtkMRMLCornerTextLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
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

  /// Parses the contents of an XMl-based text node into a vtkXMLDataElement.
  /// The vtkXMLDataElement will be used by other functions to generate the
  /// actual string intended to be rendered on a given slice view.
  vtkXMLDataElement* ParseTextNode(vtkMRMLTextNode*);

  bool SliceViewAnnotationsEnabled{true};
  bool BottomLeftEnabled{true}, TopLeftEnabled{true}, TopRightEnabled{true};
  int DisplayStrictness, FontSize;
  std::string FontFamily;
  std::unordered_map<
      std::string,
      vtkSmartPointer<vtkMRMLAbstractAnnotationPropertyValueProvider>>
      registeredProviders;
};

#endif
