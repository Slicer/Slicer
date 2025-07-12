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

==============================================================================*/

#ifndef __vtkSegmentationConversionPath_h
#define __vtkSegmentationConversionPath_h

// VTK includes
#include <vtkCollection.h>
#include <vtkObject.h>

// STD includes
#include <vector>

// Segmentation includes
#include "vtkSegmentationCoreConfigure.h"

class vtkSegmentationConverterRule;

/// \brief Store a segmentation conversion path.
/// \details
/// Stores conversion path as a list of conversion rules and their parameters.
class vtkSegmentationCore_EXPORT vtkSegmentationConversionPath : public vtkObject
{
public:
  static vtkSegmentationConversionPath* New();
  vtkTypeMacro(vtkSegmentationConversionPath, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Get conversion "cost". The lower the cost is the better (faster computation,
  /// higher accuracy, etc). The cost can be used to choose the best conversion paths
  /// when multiple paths are available.
  unsigned int GetCost();

  /// Add a rule to the end of the rule list. Return the index of this rule.
  int AddRule(vtkSegmentationConverterRule* rule);

  /// Concatenate all rules in "path" and to this conversion path.
  void AddRules(vtkSegmentationConversionPath* path);

  /// Return number of rules that make up this conversion path.
  int GetNumberOfRules();

  /// Get index-th rule.
  vtkSegmentationConverterRule* GetRule(int index) VTK_EXPECTS(0 <= index && index < GetNumberOfRules());

  /// Remove index-th rule.
  void RemoveRule(int index) VTK_EXPECTS(0 <= index && index < GetNumberOfRules());

  /// Remove all rules from this path.
  void RemoveAllRules();

  /// Replace paths with content of the source path.
  /// The rules are shallow-copied.
  void Copy(vtkSegmentationConversionPath* source);

protected:
  std::vector<vtkSmartPointer<vtkSegmentationConverterRule>> Rules;

protected:
  vtkSegmentationConversionPath();
  ~vtkSegmentationConversionPath() override;

private:
  vtkSegmentationConversionPath(const vtkSegmentationConversionPath&) = delete;
  void operator=(const vtkSegmentationConversionPath&) = delete;
};

/// \brief Store multiple segmentation conversion paths.
/// \details
/// Stores multiple conversion paths, each defined by a list of segmentation conversion rules.
class vtkSegmentationCore_EXPORT vtkSegmentationConversionPaths : public vtkCollection
{
public:
  vtkTypeMacro(vtkSegmentationConversionPaths, vtkCollection);
  static vtkSegmentationConversionPaths* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Add a path to the list.
  void AddPath(vtkSegmentationConversionPath*);

  /// Add paths to the list.
  void AddPaths(vtkSegmentationConversionPaths*);

  /// Get the next path in the list.
  vtkSegmentationConversionPath* GetNextPath();

  /// Get the index-th path the list.
  vtkSegmentationConversionPath* GetPath(int index)
  {
    return vtkSegmentationConversionPath::SafeDownCast(this->GetItemAsObject(index));
  }

  /// Reentrant safe way to get an object in a collection. Just pass the
  /// same cookie back and forth.
  vtkSegmentationConversionPath* GetNextPath(vtkCollectionSimpleIterator& cookie);

  /// Get number of conversion paths stored in the collection.
  int GetNumberOfPaths() { return this->GetNumberOfItems(); };

protected:
  vtkSegmentationConversionPaths() = default;
  ~vtkSegmentationConversionPaths() override = default;

private:
  // hide the standard AddItem from the user and the compiler.
  void AddItem(vtkObject* o) { this->vtkCollection::AddItem(o); }

private:
  vtkSegmentationConversionPaths(const vtkSegmentationConversionPaths&) = delete;
  void operator=(const vtkSegmentationConversionPaths&) = delete;
};

//----------------------------------------------------------------------------
inline void vtkSegmentationConversionPaths::AddPath(vtkSegmentationConversionPath* f)
{
  this->vtkCollection::AddItem(f);
}

//----------------------------------------------------------------------------
inline void vtkSegmentationConversionPaths::AddPaths(vtkSegmentationConversionPaths* paths)
{
  vtkSegmentationConversionPath* path = nullptr;
  vtkCollectionSimpleIterator it;
  for (paths->InitTraversal(it); (path = paths->GetNextPath(it));)
  {
    this->AddItem(path);
  }
}

//----------------------------------------------------------------------------
inline vtkSegmentationConversionPath* vtkSegmentationConversionPaths::GetNextPath()
{
  return static_cast<vtkSegmentationConversionPath*>(this->GetNextItemAsObject());
}

#endif
