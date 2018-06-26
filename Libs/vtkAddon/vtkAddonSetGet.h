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

==============================================================================*/

/// vtkAddonSetGet
///
/// vtkAddonSetGet adds additional Set/Get macros for currently unsupported types
/// Types:
///  stl containers,
///

#ifndef vtkAddonSetGet_h
#define vtkAddonSetGet_h

//
// Set macro for STL containers. Sets the local variable to be a copy of the specified container.
// Currently, STL containers cannot use the generic vtkSetMacro/vtkGetMacro types since those macros require a "<<" operator within vtkDebugMacro
//
#define vtkSetStdVectorMacro(name,type) \
virtual void Set##name (type _arg) \
{ \
  if (this->name != _arg) \
  { \
    this->name = _arg; \
    this->Modified(); \
  } \
}

//
// Get macro for STL containers. Returns a copy of the container.
// Currently, STL containers cannot use the generic vtkSetMacro/vtkGetMacro types since those macros require a "<<" operator within vtkDebugMacro
//
#define vtkGetStdVectorMacro(name,type) \
virtual type Get##name () { \
  return this->name; \
}

#endif
// VTK-HeaderTest-Exclude: vtkSetGet.h
