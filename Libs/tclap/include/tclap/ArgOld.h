
/****************************************************************************** 
 * 
 *  file:  Arg.h
 * 
 *  Copyright (c) 2003, Michael E. Smoot .
 *  Copyright (c) 2004, Michael E. Smoot, Daniel Aarno .
 *  All rights reverved.
 * 
 *  See the file COPYING in the top directory of this distribution for
 *  more information.
 *  
 *  THE SOFTWARE IS PROVIDED _AS IS_, WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 *  DEALINGS IN THE SOFTWARE.  
 *  
 *****************************************************************************/ 


#ifndef TCLAP_ARGUMENT_H
#define TCLAP_ARGUMENT_H

#include <string>
#include <vector>
#include <list>
#include <iostream>

#include <tclap/ArgException.h>
#include <tclap/Visitor.h>
#include <tclap/CmdLineInterface.h>

namespace TCLAP {

/** 
 * A virtual base class that defines the essential data for all arguments.
 * This class, or one of its existing children, must be subclassed to do
 * anything. 
 */
class Arg
{
private: 

  /**
     * Indicates whether the rest of the arguments should be ignored.
     */
  static bool& ignoreRestRef() { static bool ign = false; return ign; }

  /**
     * The delimiter that separates an argument flag/name from the
     * value.
     */
  static char& delimiterRef() { static char delim = ' '; return delim; } 

protected:

  /** 
     * The single char flag used to identify the argument.
     * This value (preceded by a dash {-}), can be used to identify 
     * an argument on the command line.  The _flag can be blank, 
     * in fact this is how unlabeled args work.  Unlabeled args must
     * override appropriate functions to get correct handling. Note 
     * that the _flag does NOT include the dash as part of the flag.
     */
  std::string _flag;

  /**
     * A single work namd indentifying the argument.
     * This value (preceded by two dashed {--}) can also be used 
     * to identify an argument on the command line.  Note that the
     * _name does NOT include the two dashes as part of the _name. The
     * _name cannot be blank.
     */
  std::string _name;

  /**
     * Description of the argument. 
     */
  std::string _description;

  /** 
     * Indicating whether the argument is required.
     */
  bool _required;

  /**
     * Label to be used in usage description.  Normally set to 
     * "required", but can be changed when necessary.
     */
  std::string _requireLabel;

  /**
     * Indicates whether a value is required for the argument.
     * Note that the value may be required but the argument/value
     * combination may not be, as specified by _required.
     */
  bool _valueRequired;

  /**
     * Indicates whether the argument has been set.
     * Indicates that a value on the command line has matched the
     * name/flag of this argument and the values have been set accordingly.
     */
  bool _alreadySet;

  /**
     * A pointer to a vistitor object.
     * The visitor allows special handling to occur as soon as the
     * argument is matched.  This defaults to NULL and should not
     * be used unless absolutely necessary.
     */
  Visitor* _visitor;

  /**
     * Whether this argument can be ignored, if desired.
     */
  bool _ignoreable;

  /**
     * Indicates that the arg was set as part of an XOR and not on the
     * command line.
     */
  bool _xorSet;

  bool _acceptsMultipleValues;

  /**
     * Performs the special handling described by the Vistitor.
     */
  void _checkWithVisitor() const;

  /**
     * Primary constructor. YOU (yes you) should NEVER construct an Arg 
     * directly, this is a base class that is extended by various children
     * that are meant to be used.  Use SwitchArg, ValueArg, MultiArg, 
     * UnlabeledValueArg, or UnlabeledMultiArg instead.
     *
     * \param flag - The flag identifying the argument.
     * \param name - The name identifying the argument.
     * \param desc - The description of the argument, used in the usage.
     * \param req - Whether the argument is required.
     * \param valreq - Whether the a value is required for the argument.
     * \param v - The visitor checked by the argument. Defaults to NULL.
     */
  Arg( const std::string& flag, 
       const std::string& name, 
       const std::string& desc, 
       bool req, 
       bool valreq,
       Visitor* v = NULL );

public:
  /**
     * Destructor.
     */
  virtual ~Arg();

  /**
     * Adds this to the specified list of Args.
     * \param argList - The list to add this to.
     */
  virtual void addToList( std::list<Arg*>& argList ) const;
    
  /**
     * Begin ignoring arguments since the "--" argument was specified.
     */
  static void beginIgnoring() { ignoreRestRef() = true; }
    
  /**
     * Whether to ignore the rest.
     */
  static bool ignoreRest() { return ignoreRestRef(); }

  /**
     * The delimiter that separates an argument flag/name from the
     * value.
     */
  static char delimiter() { return delimiterRef(); } 
    
  /**
     * The char used as a place holder when SwitchArgs are combined.
     * Currently set to '*', which shouldn't cause many problems since
     * *'s are expanded by most shells on the command line.  
     */
  static const char blankChar() { return '*'; }
    
  /**
     * The char that indicates the beginning of a flag.  Currently '-'.
     */
  static const char flagStartChar() { return '-'; }
    
  /**
     * The sting that indicates the beginning of a flag.  Currently "-".
     * Should be identical to flagStartChar.
     */
  static const std::string flagStartString() { return "-"; }
    
  /**
     * The sting that indicates the beginning of a name.  Currently "--".
     * Should be flagStartChar twice.
     */
  static const std::string nameStartString() { return "--"; }

  /**
     * The name used to identify the ignore rest argument.
     */
  static const std::string ignoreNameString() { return "ignore_rest"; }

  /**
     * Sets the delimiter for all arguments.
     * \param c - The character that delimits flags/names from values.
     */
  static void setDelimiter( char c ) { delimiterRef() = c; }

  /**
     * Pure virtual method meant to handle the parsing and value assignment
     * of the string on the command line. 
     * \param i - Pointer the the current argument in the list.
     * \param args - Mutable list of strings. What is 
     * passed in from main.
     */
  virtual bool processArg(int *i, std::vector<std::string>& args) = 0; 

  /**
     * Operator ==.
     * Equality operator. Must be virtual to handle unlabeled args.
     * \param a - The Arg to be compared to this.
     */
  virtual bool operator==(const Arg& a) const;

  /**
     * Returns the argument flag.
     */
  const std::string& getFlag() const;

  /**
     * Returns the argument name.
     */
  const std::string& getName() const;

  /**
     * Returns the argument description.
     */
  std::string getDescription() const;

  /**
     * Indicates whether the argument is required.
     */
  virtual bool isRequired() const;

  /**
     * Sets _required to true. This is used by the XorHandler.
     * You really have no reason to ever use it.
     */
  void forceRequired();

  /**
     * Sets the _alreadySet value to true.  This is used by the XorHandler.
     * You really have no reason to ever use it.
     */
  void xorSet();

  /**
     * Indicates whether a value must be specified for argument.
     */
  bool isValueRequired() const;

  /**
     * Indicates whether the argument has already been set.  Only true
     * if the arg has been matched on the command line.
     */
  bool isSet() const;

  /**
     * Indicates whether the argument can be ignored, if desired. 
     */
  bool isIgnoreable() const;

  /**
     * A method that tests whether a string matches this argument.
     * This is generally called by the processArg() method.  This
     * method could be re-implemented by a child to change how 
     * arguments are specified on the command line.
     * \param s - The string to be compared to the flag/name to determine
     * whether the arg matches.
     */
  virtual bool argMatches( const std::string& s ) const;

  /**
     * Returns a simple string representation of the argument.
     * Primarily for debugging.
     */
  virtual std::string toString() const;

  /**
     * Returns a short ID for the usage.
     * \param valueId - The value used in the id.
     */
  virtual std::string shortID( const std::string& valueId = "val" ) const;

  /**
     * Returns a long ID for the usage.
     * \param valueId - The value used in the id.
     */
  virtual std::string longID( const std::string& valueId = "val" ) const;

  /**
     * Trims a value off of the flag.
     * \param flag - The string from which the flag and value will be 
     * trimmed. Contains the flag once the value has been trimmed. 
     * \param value - Where the value trimmed from the string will
     * be stored.
     */
  virtual void trimFlag( std::string& flag, std::string& value ) const;

  /**
     * Checks whether a given string has blank chars, indicating that
     * it is a combined SwitchArg.  If so, return true, otherwise return
     * false.
     * \param s - string to be checked.
     */
  bool _hasBlanks( const std::string& s ) const;

  /**
     * Sets the requireLabel. Used by XorHandler.  You shouldn't ever
     * use this.
     * \param s - Set the requireLabel to this value.
     */
  void setRequireLabel( const std::string& s );

  virtual bool allowMore();
  virtual void printXMLDescription() {}
  virtual bool acceptsMultipleValues();

};

/**
 * Typedef of an Arg list iterator.
 */
typedef std::list<Arg*>::iterator ArgListIterator;

/**
 * Typedef of an Arg vector iterator.
 */
typedef std::vector<Arg*>::iterator ArgVectorIterator;

/**
 * Typedef of a Visitor list iterator.
 */
typedef std::list<Visitor*>::iterator VisitorListIterator;


//////////////////////////////////////////////////////////////////////
//BEGIN Arg.cpp
//////////////////////////////////////////////////////////////////////

inline Arg::Arg(const std::string& flag, 
                const std::string& name, 
                const std::string& desc, 
                bool req, 
                bool valreq,
                Visitor* v) :
  _flag(flag),
  _name(name),
  _description(desc),
  _required(req),
  _requireLabel("required"),
  _valueRequired(valreq),
  _alreadySet(false),
  _visitor( v ),
  _ignoreable(true),
  _xorSet(false),
  _acceptsMultipleValues(false)
{
  if ( _flag.length() > 1 ) 
    throw(SpecificationException(
            "Argument flag can only be one character long", toString() ) );

  if ( _name != ignoreNameString() &&  
       ( _flag == Arg::flagStartString() || 
         _flag == Arg::nameStartString() || 
         _flag == " " ) )
    throw(SpecificationException("Argument flag cannot be either '" + 
                                 Arg::flagStartString() + "' or '" + 
                                 Arg::nameStartString() + "' or a space.",
                                 toString() ) );

  if ( ( _name.find( Arg::flagStartString(), 0 ) != std::string::npos ) || 
       ( _name.find( Arg::nameStartString(), 0 ) != std::string::npos ) ||
       ( _name.find( " ", 0 ) != std::string::npos ) )
    throw(SpecificationException("Argument name cannot contain either '" + 
                                 Arg::flagStartString() + "' or '" + 
                                 Arg::nameStartString() + "' or space.",
                                 toString() ) );

}

inline Arg::~Arg() { }

inline std::string Arg::shortID( const std::string& valueId ) const
{
  std::string id = "";

  if ( _flag != "" )
    id = Arg::flagStartString() + _flag;
  else
    id = Arg::nameStartString() + _name;

  std::string delim = " "; 
  delim[0] = Arg::delimiter(); // ugly!!!
  
  if ( _valueRequired )
    id += delim + "<" + valueId  + ">";

  if ( !_required )
    id = "[" + id + "]";

  return id;
}

inline std::string Arg::longID( const std::string& valueId ) const
{
  std::string id = "";

  if ( _flag != "" )
    {
    id += Arg::flagStartString() + _flag;

    if ( _valueRequired )
      id += " <" + valueId + ">";
    
    id += ",  ";
    }

  id += Arg::nameStartString() + _name;

  if ( _valueRequired )
    id += " <" + valueId + ">";
      
  return id;

}

inline bool Arg::operator==(const Arg& a) const
{
  if ( ( _flag != "" && _flag == a._flag ) || _name == a._name)
    return true;
  else
    return false;
}

inline std::string Arg::getDescription() const 
{
  std::string desc = "";
  if ( _required )
    desc = "(" + _requireLabel + ")  ";

//  if ( _valueRequired )
//    desc += "(value required)  ";

  desc += _description;
  return desc; 
}

inline const std::string& Arg::getFlag() const { return _flag; }

inline const std::string& Arg::getName() const { return _name; } 

inline bool Arg::isRequired() const { return _required; }

inline bool Arg::isValueRequired() const { return _valueRequired; }

inline bool Arg::isSet() const 
{ 
  if ( _alreadySet && !_xorSet )
    return true;
  else
    return false;
}

inline bool Arg::isIgnoreable() const { return _ignoreable; }

inline void Arg::setRequireLabel( const std::string& s) 
{ 
  _requireLabel = s;
}

inline bool Arg::argMatches( const std::string& argFlag ) const
{
  if ( ( argFlag == Arg::flagStartString() + _flag && _flag != "" ) ||
       argFlag == Arg::nameStartString() + _name )
    return true;
  else
    return false;
}

inline std::string Arg::toString() const
{
  std::string s = "";

  if ( _flag != "" )
    s += Arg::flagStartString() + _flag + " ";

  s += "(" + Arg::nameStartString() + _name + ")";

  return s;
}

inline void Arg::_checkWithVisitor() const
{
  if ( _visitor != NULL )
    _visitor->visit();
}

/**
 * Implementation of trimFlag.
 */
inline void Arg::trimFlag(std::string& flag, std::string& value) const
{
  int stop = 0;
  for ( int i = 0; static_cast<unsigned int>(i) < flag.length(); i++ )
    if ( flag[i] == Arg::delimiter() )
      {
      stop = i;
      break;
      }

  if ( stop > 1 )
    {
    value = flag.substr(stop+1);
    flag = flag.substr(0,stop);
    }

}

/**
 * Implementation of _hasBlanks.
 */
inline bool Arg::_hasBlanks( const std::string& s ) const
{
  for ( int i = 1; static_cast<unsigned int>(i) < s.length(); i++ )
    if ( s[i] == Arg::blankChar() )
      return true;

  return false;
}

inline void Arg::forceRequired()
{
  _required = true;
}

inline void Arg::xorSet()
{
  _alreadySet = true;
  _xorSet = true;
}

/**
 * Overridden by Args that need to added to the end of the list.
 */
inline void Arg::addToList( std::list<Arg*>& argList ) const
{
  argList.push_front( const_cast<Arg*>(this) );
}

inline bool Arg::allowMore()
{
  return false;
}

inline bool Arg::acceptsMultipleValues()
{
  return _acceptsMultipleValues;
}

//////////////////////////////////////////////////////////////////////
//END Arg.cpp
//////////////////////////////////////////////////////////////////////

} //namespace TCLAP

#endif

