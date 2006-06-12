#ifndef __ModuleParameter_h
#define __ModuleParameter_h

#include <map>
#include <string>
#include <iostream>
class ModuleParameter
{
public:
  ModuleParameter() {}
  virtual ~ModuleParameter() {}
  ModuleParameter(const ModuleParameter& parameter)
  {
    this->Properties = parameter.Properties;
  }
  void operator=(const ModuleParameter& parameter)
  {
    this->Properties = parameter.Properties;
  }

  virtual void SetType(const std::string &type) {
    this->SetProperty("type", type);
  }

  virtual std::string GetType() const {
    return this->GetProperty("type");
  }
  
  virtual void SetStringToType(const std::string &stringToType) {
    this->SetProperty("stringToType", stringToType);
  }

  virtual std::string GetStringToType() const {
    return this->GetProperty("stringToType");
  }
  
  virtual void SetName(const std::string &name) {
    this->SetProperty("name", name);
  }

  virtual std::string GetName() const {
    return this->GetProperty("name");
  }
  
  virtual void SetLongFlag(const std::string &longFlag) {
    this->SetProperty("longFlag", longFlag);
  }

  virtual std::string GetLongFlag() const {
    return this->GetProperty("longFlag");
  }
  
  virtual void SetShortFlag(const std::string &shortFlag) {
    this->SetProperty("shortFlag", shortFlag);
  }

  virtual std::string GetShortFlag() const {
    return this->GetProperty("shortFlag");
  }
  
  virtual void SetLabel(const std::string &label) {
    this->SetProperty("label", label);
  }
  
  virtual std::string GetLabel() const {
    return this->GetProperty("label");
  }

  virtual void SetDescription(const std::string &description) {
    this->SetProperty("description", description);
  }

  virtual std::string GetDescription() const {
    return this->GetProperty("description");
  }
  
  virtual void SetChannel(const std::string &channel) {
    this->SetProperty("channel", channel);
  }

  virtual std::string GetChannel() const {
    return this->GetProperty("channel");
  }
  
  virtual void SetIndex(const std::string &index) {
    this->SetProperty("index", index);
  }

  virtual std::string GetIndex() const {
    return this->GetProperty("index");
  }
  
  virtual void SetDefault(const std::string &def) {
    this->SetProperty("default", def);
  }

  virtual std::string GetDefault() const {
    return this->GetProperty("default");
  }
  
  virtual void SetFlag(const std::string &flag){
    this->SetProperty("flag", flag);
  }

  virtual std::string GetFlag() const {
    return this->GetProperty("flag");
  }

  virtual void SetConstraints() {
    this->SetProperty("constraints", "");
  }

  virtual std::string GetConstraints() const {
    return this->GetProperty("constraints");
  }
  
  virtual const std::map<std::string, std::string> &GetProperties() const {
    return Properties;
  }
  
protected:
  virtual void SetProperty(const std::string &key,
                           const std::string &value) {
    this->Properties[key] = value;
  }

  virtual std::string GetProperty(const std::string &key) const {
    std::map<std::string, std::string>::const_iterator it;

    it = this->Properties.find(key);
    if (it != this->Properties.end())
      {
      return it->second;
      }
    
    return std::string("");
  }

private:
  std::map<std::string, std::string> Properties;
};

inline std::ostream & operator<<(std::ostream &os, const ModuleParameter &parameter)
{ 
  std::map<std::string, std::string>::const_iterator it = parameter.GetProperties().begin();
  std::cout << "    Parameter" << std::endl;
  while (it != parameter.GetProperties().end())
    {
    os << "      " << (*it).first << ": " << (*it).second << std::endl;
    ++it;
    }
  return os;
}

#endif
