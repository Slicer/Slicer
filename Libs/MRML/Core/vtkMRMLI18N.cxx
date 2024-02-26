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

// MRML includes
#include "vtkMRMLI18N.h"
#include "vtkMRMLTranslator.h"

// VTK includes
#include <vtkObjectFactory.h>

vtkCxxSetObjectMacro(vtkMRMLI18N, Translator, vtkMRMLTranslator);

//----------------------------------------------------------------------------
// The i18n manager singleton.
// This MUST be default initialized to zero by the compiler and is
// therefore not initialized here.  The ClassInitialize and
// ClassFinalize methods handle this instance.
static vtkMRMLI18N* vtkMRMLI18NInstance;

//----------------------------------------------------------------------------
// Must NOT be initialized.  Default initialization to zero is necessary.
unsigned int vtkMRMLI18NInitialize::Count;

//----------------------------------------------------------------------------
// Implementation of vtkMRMLI18NInitialize class.
//----------------------------------------------------------------------------
vtkMRMLI18NInitialize::vtkMRMLI18NInitialize()
{
  if (++Self::Count == 1)
  {
    vtkMRMLI18N::classInitialize();
  }
}

//----------------------------------------------------------------------------
vtkMRMLI18NInitialize::~vtkMRMLI18NInitialize()
{
  if (--Self::Count == 0)
  {
    vtkMRMLI18N::classFinalize();
  }
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Up the reference count so it behaves like New
vtkMRMLI18N* vtkMRMLI18N::New()
{
  vtkMRMLI18N* ret = vtkMRMLI18N::GetInstance();
  ret->Register(nullptr);
  return ret;
}

//----------------------------------------------------------------------------
// Return the single instance of the vtkMRMLI18N
vtkMRMLI18N* vtkMRMLI18N::GetInstance()
{
  if (!vtkMRMLI18NInstance)
  {
    // Try the factory first
    vtkMRMLI18NInstance = (vtkMRMLI18N*)vtkObjectFactory::CreateInstance("vtkMRMLI18N");
    // if the factory did not provide one, then create it here
    if (!vtkMRMLI18NInstance)
    {
      vtkMRMLI18NInstance = new vtkMRMLI18N;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
      vtkMRMLI18NInstance->InitializeObjectBase();
#endif
    }
  }
  // return the instance
  return vtkMRMLI18NInstance;
}

//----------------------------------------------------------------------------
vtkMRMLI18N::vtkMRMLI18N()
{
  this->Translator = nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLI18N::~vtkMRMLI18N()
{
  if (this->Translator)
  {
    this->Translator->Delete();
  }
  this->Translator = nullptr;
}

//----------------------------------------------------------------------------
void vtkMRMLI18N::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "Translator:";
  if (this->GetTranslator())
  {
    this->GetTranslator()->PrintSelf(os, indent.GetNextIndent());
  }
  else
  {
    os << " (none)"
       << "\n";
  }
}

//----------------------------------------------------------------------------
void vtkMRMLI18N::classInitialize()
{
  // Allocate the singleton
  vtkMRMLI18NInstance = vtkMRMLI18N::GetInstance();
}

//----------------------------------------------------------------------------
void vtkMRMLI18N::classFinalize()
{
  vtkMRMLI18NInstance->Delete();
  vtkMRMLI18NInstance = nullptr;
}

//----------------------------------------------------------------------------
std::string vtkMRMLI18N::Translate(const char* context,
                                   const char* sourceText,
                                   const char* disambiguation /*=nullptr*/,
                                   int n /*=-1*/)
{
  vtkMRMLI18N* i18n = vtkMRMLI18N::GetInstance();
  vtkMRMLTranslator* translator = i18n ? i18n->GetTranslator() : nullptr;
  if (translator)
  {
    return translator->Translate(context, sourceText, disambiguation, n);
  }
  else
  {
    return sourceText ? sourceText : "";
  }
}
