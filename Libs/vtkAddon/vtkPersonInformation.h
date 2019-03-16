/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

/// \brief vtkPersonInformation - specify name and other information
/// to identify a person.
///

#ifndef __vtkPersonInformation_h
#define __vtkPersonInformation_h

#include "vtkAddon.h"

#include <vtkObject.h>
#include <vtkStringArray.h>

#include <map>

class VTK_ADDON_EXPORT vtkPersonInformation : public vtkObject
{
public:
  static vtkPersonInformation *New();
  vtkTypeMacro(vtkPersonInformation,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set/Get username.
  // Set method
  virtual bool SetName(const std::string& value);
  virtual std::string GetName();

  virtual bool SetLogin(const std::string& value);
  virtual std::string GetLogin();

  virtual bool SetEmail(const std::string& value);
  virtual std::string GetEmail();

  virtual bool SetOrganization(const std::string& value);
  virtual std::string GetOrganization();

  virtual bool SetOrganizationRole(const std::string& value);
  virtual std::string GetOrganizationRole();

  virtual bool SetProcedureRole(const std::string& value);
  virtual std::string GetProcedureRole();

  // Description:
  // Make a full, independent copy of another object into this object.
  virtual void DeepCopy(vtkPersonInformation* source);

  // Description:
  // Set a custom string value. Calls Modified() if the value is different from the previous value.
  // No validation is performed. If a standard string's key is used (Name, Email, ...)
  // then the standard string is overwritten without validation.
  // If value is set to empty then the key/value pair is deleted.
  virtual void SetCustomString(const std::string& key, const std::string& value);

  // Description:
  // Get a custom string value.
  // If a standard string's key is used (Name, Email, ...) then the corresponding value is returned.
  // Empty string is returned if the key has no associated value.
  virtual std::string GetCustomString(const std::string& key);

  virtual bool IsEmailValid(const std::string& value);

  // Description:
  // Information keys.
  static const char* KeyName() { return "Name"; };
  static const char* KeyLogin() { return "Login"; };
  static const char* KeyEmail() { return "Email"; };
  static const char* KeyOrganization() { return "Organization"; };
  static const char* KeyOrganizationRole() { return "OrganizationRole"; };
  static const char* KeyProcedureRole() { return "ProcedureRole"; };

  // Description:
  // Write all key/value pairs to a string.
  virtual std::string GetAsString();

  // Description:
  // Read all key/value pairs from a string.
  // All previous values are cleared.
  virtual bool SetFromString(const std::string& data);

  // Description:
  // Returns list of all keys.
  virtual void GetKeys(vtkStringArray *keys);

protected:
  vtkPersonInformation();
  ~vtkPersonInformation() override;

  std::string EncodeString(const std::string& value);
  std::string DecodeString(const std::string& value);

  std::map<std::string, std::string> Data;

private:
  vtkPersonInformation(const vtkPersonInformation&) = delete;
  void operator=(const vtkPersonInformation&) = delete;
};

#endif
