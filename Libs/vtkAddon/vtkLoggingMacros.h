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
#ifndef __vtkLoggingMacros_h
#define __vtkLoggingMacros_h

#include "vtkAddon.h"

// Macro for logging informational messages
//
// Informational messages log important information and non-error events
// that can be used for approximately tracking user actions.
// The information is essential for troubleshooting errors (as the user
// can be asked to simply provide the application log instead of describing
// what he did in detail).
//
// A new vtkInfoMacro logging macro had to be added as vtkDebugMacro only logs
// the message if debug is enabled for the particular object (also, typically
// debug output contains many low-level details), and Error and Warning macros
// are reserved for reporting potential issues.

#ifndef vtkInfoWithObjectMacro
#define vtkInfoWithObjectMacro(self, x)                      \
  {                                                          \
  vtkOStreamWrapper::EndlType endl;                          \
  vtkOStreamWrapper::UseEndl(endl);                          \
  vtkOStrStreamWrapper vtkmsg;                               \
  vtkmsg << "Info: In " __FILE__ ", line " << __LINE__       \
        << "\n" << self->GetClassName() << " (" << self      \
        << "): " x << "\n\n";                                \
  vtkOutputWindowDisplayText(vtkmsg.str());                  \
  vtkmsg.rdbuf()->freeze(0);                                 \
  }
#endif

#ifndef vtkInfoWithoutObjectMacro
#define vtkInfoWithoutObjectMacro(x)                         \
  {                                                          \
  vtkOStreamWrapper::EndlType endl;                          \
  vtkOStreamWrapper::UseEndl(endl);                          \
  vtkOStrStreamWrapper vtkmsg;                               \
  vtkmsg << "Info: In " __FILE__ ", line " << __LINE__       \
        << "\n" x << "\n\n";                                 \
  vtkOutputWindowDisplayText(vtkmsg.str());                  \
  vtkmsg.rdbuf()->freeze(0);                                 \
  }
#endif

#ifndef vtkInfoMacro
#define vtkInfoMacro(x)                                      \
  vtkInfoWithObjectMacro(this,x);
#endif

#endif // __vtkLoggingMacros_h
