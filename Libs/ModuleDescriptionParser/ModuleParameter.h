#ifndef __ModuleParameter_h
#define __ModuleParameter_h

#include "ModuleDescriptionParserWin32Header.h"

//#include "itksys/hash_map.hxx"
#include <map>
#include <string>
#include <iostream>

// Need to define a hash function for strings
// template<>
// struct itksys::hash<std::string> {
//   itksys::hash<char*> h;
//   size_t operator()(const std::string &s) const {
//     return h(s.c_str());
//   };
// };


/** \class ModuleParameter
 *  \brief Class to describe a single parameter to a module.
 *
 * ModuleParameter describes a single parameters to a
 * module. Information on the parameter type, name, flag, label,
 * description, channel, index, default, and constraints can be
 * stored.
 *
 */
class ModuleDescriptionParser_EXPORT ModuleParameter
{
public:
  ModuleParameter() {}
  virtual ~ModuleParameter() {}
  ModuleParameter(const ModuleParameter& parameter);

  void operator=(const ModuleParameter& parameter);
  
  virtual void SetType(const std::string &type) {
    this->Type = type;
  }

  virtual std::string GetType() const {
    return this->Type;
  }
  
  virtual void SetStringToType(const std::string &stringToType) {
    this->StringToType = stringToType;
  }

  virtual std::string GetStringToType() const {
    return this->StringToType;
  }
  
  virtual void SetName(const std::string &name) {
    this->Name = name;
  }

  virtual std::string GetName() const {
    return this->Name;
  }
  
  virtual void SetLongFlag(const std::string &longFlag) {
    this->LongFlag = longFlag;
  }

  virtual std::string GetLongFlag() const {
    return this->LongFlag;
  }
  
  virtual void SetShortFlag(const std::string &shortFlag) {
    this->ShortFlag = shortFlag;
  }

  virtual std::string GetShortFlag() const {
    return this->ShortFlag;
  }
  
  virtual void SetLabel(const std::string &label) {
    this->Label = label;
  }
  
  virtual std::string GetLabel() const {
    return this->Label;
  }

  virtual void SetDescription(const std::string &description) {
    this->Description = description;
  }

  virtual std::string GetDescription() const {
    return this->Description;
  }
  
  virtual void SetChannel(const std::string &channel) {
    this->Channel = channel;
  }

  virtual std::string GetChannel() const {
    return this->Channel;
  }
  
  virtual void SetIndex(const std::string &index) {
    this->Index = index;
  }

  virtual std::string GetIndex() const {
    return this->Index;
  }
  
  virtual void SetDefault(const std::string &def) {
    this->Default = def;
  }

  virtual std::string GetDefault() const {
    return this->Default;
  }
  
  virtual void SetFlag(const std::string &flag){
    this->Flag = flag;
  }

  virtual std::string GetFlag() const {
    return this->Flag;
  }

  
protected:

  
private:
  std::string Name;
  std::string Description;
  std::string Label;
  std::string Type;
  std::string StringToType;
  std::string Default;
  std::string Flag;
  std::string ShortFlag;
  std::string LongFlag;
  std::string Channel;
  std::string Index;
};

ModuleDescriptionParser_EXPORT std::ostream & operator<<(std::ostream &os, const ModuleParameter &parameter);


#endif
