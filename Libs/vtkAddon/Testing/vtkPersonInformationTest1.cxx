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

// vtkAddon includes
#include "vtkAddonTestingMacros.h"
#include "vtkPersonInformation.h"

// VTK includes
#include <vtkNew.h>

int vtkPersonInformationTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  vtkNew<vtkPersonInformation> userInformation;

  std::string name = "John Doe";
  std::string login = "foo:bar";
  std::string email = "valid.email@somewhere.com";
  std::string organization = "BWH";
  std::string organizationRole = "Referring";
  std::string procedureRole = "Some;Procedure;Role";

  CHECK_BOOL(userInformation->SetFromString(""), true);

  CHECK_BOOL(userInformation->SetName(name), true);
  CHECK_BOOL(userInformation->SetLogin(login), true);
  CHECK_BOOL(userInformation->SetEmail(email), true);
  CHECK_BOOL(userInformation->SetEmail("invalid email"), false);
  CHECK_BOOL(userInformation->SetOrganization(organization), true);
  CHECK_BOOL(userInformation->SetOrganizationRole(organizationRole), true);
  CHECK_BOOL(userInformation->SetProcedureRole(procedureRole), true);

  CHECK_STD_STRING(userInformation->GetName(), name);
  CHECK_STD_STRING(userInformation->GetLogin(), login);
  CHECK_STD_STRING(userInformation->GetEmail(), email);
  CHECK_STD_STRING(userInformation->GetOrganization(), organization);
  CHECK_STD_STRING(userInformation->GetOrganizationRole(), organizationRole);
  CHECK_STD_STRING(userInformation->GetProcedureRole(), procedureRole);

  std::string output = "Email:valid.email@somewhere.com;"
                       "Login:foo%3Abar;"
                       "Name:John Doe;"
                       "Organization:BWH;"
                       "OrganizationRole:Referring;"
                       "ProcedureRole:Some%3BProcedure%3BRole";
  CHECK_STD_STRING(userInformation->GetAsString(), output)
  vtkNew<vtkStringArray> keys;
  userInformation->GetKeys(keys.GetPointer());
  CHECK_INT(keys->GetNumberOfValues(), 6);
  CHECK_STRING(keys->GetValue(0), "Email");
  CHECK_STRING(keys->GetValue(5), "ProcedureRole");

  // Test key delete
  CHECK_BOOL(userInformation->SetName(""), true);
  std::string outputAfterNameDelete = "Email:valid.email@somewhere.com;"
                       "Login:foo%3Abar;"
                       "Organization:BWH;"
                       "OrganizationRole:Referring;"
                       "ProcedureRole:Some%3BProcedure%3BRole";
  CHECK_STD_STRING(userInformation->GetAsString(), outputAfterNameDelete);
  userInformation->GetKeys(keys.GetPointer());
  CHECK_INT(keys->GetNumberOfValues(), 5);
  CHECK_STRING(keys->GetValue(0), "Email");
  CHECK_STRING(keys->GetValue(4), "ProcedureRole");

  // Test all key delete
  CHECK_BOOL(userInformation->SetLogin(""), true);
  CHECK_BOOL(userInformation->SetEmail(""), true);
  CHECK_BOOL(userInformation->SetOrganization(""), true);
  CHECK_BOOL(userInformation->SetOrganizationRole(""), true);
  CHECK_BOOL(userInformation->SetProcedureRole(""), true);
  CHECK_STD_STRING(userInformation->GetAsString(), "")

  return EXIT_SUCCESS;
}
