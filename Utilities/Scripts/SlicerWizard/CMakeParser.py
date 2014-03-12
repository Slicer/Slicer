#!/usr/bin/env python

import os
import re
import string

#=============================================================================
class Token(object):
  #---------------------------------------------------------------------------
  def __init__(self, text, indent=""):
    self.text = text
    self.indent = indent

  #---------------------------------------------------------------------------
  def __repr__(self):
    return "Token(text=%(text)r, indent=%(indent)r)" % self.__dict__

  #---------------------------------------------------------------------------
  def __str__(self):
    return self.indent + self.text

#=============================================================================
class String(Token):
  #---------------------------------------------------------------------------
  def __init__(self, text, indent="", prefix="", suffix=""):
    text = super(String, self).__init__(text, indent)
    self.prefix = prefix
    self.suffix = suffix

  #---------------------------------------------------------------------------
  def __repr__(self):
    return "String(prefix=%(prefix)r, suffix=%(suffix)r," \
           " text=%(text)r, indent=%(indent)r)" % self.__dict__

  #---------------------------------------------------------------------------
  def __str__(self):
    return self.indent + self.prefix + self.text + self.suffix

#=============================================================================
class Comment(Token):
  #---------------------------------------------------------------------------
  def __init__(self, prefix, text, indent="", suffix=""):
    text = super(Comment, self).__init__(text, indent)
    self.prefix = prefix
    self.suffix = suffix

  #---------------------------------------------------------------------------
  def __repr__(self):
    return "Comment(prefix=%(prefix)r, suffix=%(suffix)r," \
           " text=%(text)r, indent=%(indent)r)" % self.__dict__

  #---------------------------------------------------------------------------
  def __str__(self):
    return self.indent + self.prefix + self.text + self.suffix

#=============================================================================
class Command(Token):
  #---------------------------------------------------------------------------
  def __init__(self, text, arguments=[], indent="", prefix="(", suffix=")"):
    text = super(Command, self).__init__(text, indent)
    self.prefix = prefix
    self.suffix = suffix
    self.arguments = arguments

  #---------------------------------------------------------------------------
  def __repr__(self):
    return "Command(text=%(text)r, prefix=%(prefix)r," \
           " suffix=%(suffix)r, arguments=%(arguments)r," \
           " indent=%(indent)r)" % self.__dict__

  #---------------------------------------------------------------------------
  def __str__(self):
    args = "".join([str(a) for a in self.arguments])
    return self.indent + self.text + self.prefix + args + self.suffix

#=============================================================================
class CMakeScript(object):
  _reWhitespace = re.compile(r"\s")
  _reCommand = re.compile(r"([" + string.letters + r"]\w*)(\s*\()")
  _reComment = re.compile(r"#(\[=*\[)?")
  _reExpansion = re.compile(r"[$](?:ENV)?[{]")
  _reVariable = re.compile(r"[\w/.+-]+")
  _reQuote = re.compile("\"")
  _reBracketQuote = re.compile(r"\[=*\[")
  _reEscape = re.compile(r"\\[\\\"n$]")

  #---------------------------------------------------------------------------
  def __init__(self, content):
    self.tokens = []

    self._content = content
    self._match = None

    while len(self._content):
      indent = self._chompSpace()

      # Consume comments
      if self._is(self._reComment):
        self.tokens.append(self._parseComment(self._match, indent))

      # Consume commands
      elif self._is(self._reCommand):
        self.tokens.append(self._parseCommand(self._match, indent))

      # Consume other tokens (pedantically, if we get here, the script is
      # malformed, except at EOF)
      else:
        m = self._reWhitespace.search(self._content)
        n = m.start() if m is not None else len(self._content)
        self.tokens.append(Token(text=self._content[:n], indent=indent))
        self._content = self._content[n:]

  #---------------------------------------------------------------------------
  def __repr__(self):
    return repr(self.tokens)

  #---------------------------------------------------------------------------
  def __str__(self):
    return "".join([str(t) for t in self.tokens])

  #---------------------------------------------------------------------------
  def _chomp(self):
    if self._is(self._reExpansion):
      return self._chompExpansion(self._match.group(0))

    result = self._content[0]
    self._content = self._content[1:]
    return result

  #---------------------------------------------------------------------------
  def _chompSpace(self):
    result = ""

    while len(self._content) and self._content[0].isspace():
      result += self._content[0]
      self._content = self._content[1:]

    return result

  #---------------------------------------------------------------------------
  def _chompString(self, end, escapes):
    result = ""

    while len(self._content):
      if escapes and self._is(self._reEscape):
        e = self._match.group(0)
        result += e
        self._content = self._content[len(e):]

      elif self._content.startswith(end):
        self._content = self._content[len(end):]
        return result

      else:
        result += self._chomp()

    raise EOFError("unexpected EOF while parsing string (expected %r)" % end)

  #---------------------------------------------------------------------------
  def _chompExpansion(self, start):
    result = start
    self._content = self._content[len(start):]

    while len(self._content):
      if self._content[0] == "}":
        self._content = self._content[1:]
        return result + "}"

      if self._is(self._reExpansion):
        result += self._chompExpansion(self._match.group(0))
        continue

      m = self._reVariable.match(self._content)
      if m is not None:
        result += m.group(0)
        self._content = self._content[m.end():]
        continue

      raise SyntaxError("syntax error in expansion; expected '}', found %r" %
                        self._content[0])

    raise EOFError("unexpected EOF while parsing expansion (expected '}')")

  #---------------------------------------------------------------------------
  def _parseArgument(self, indent):
    text = ""

    while len(self._content):
      if self._is(self._reQuote) or self._is(self._reBracketQuote):
        prefix = self._match.group(0)
        self._content = self._content[len(prefix):]

        if prefix == "\"":
          suffix = prefix
          s = self._chompString(suffix, escapes=True)

        else:
          suffix = prefix.replace("[", "]")
          s = self._chompString(suffix, escapes=False)

        if not len(text):
          return String(prefix=prefix, suffix=suffix, text=s, indent=indent)

        text += prefix + s + suffix

      elif self._content[0].isspace():
        break

      elif self._is(self._reEscape):
        e = self._match.group(0)
        text += e
        self._content = self._content[len(e):]

      elif self._content[0] == ")":
        break

      else:
        text += self._chomp()

    return String(text=text, indent=indent)

  #---------------------------------------------------------------------------
  def _parseComment(self, match, indent):
    b = match.group(1)
    e = "\n" if b is None else b.replace("[", "]")
    n = self._content.find(e)
    if n < 0:
      raise EOFError("unexpected EOF while parsing comment (expected %r" % e)

    i = match.end()
    suffix = e.strip()
    token = Comment(prefix=self._content[:i], suffix=suffix,
                    text=self._content[i:n], indent=indent)

    self._content = self._content[n + len(suffix):]

    return token

  #---------------------------------------------------------------------------
  def _parseCommand(self, match, indent):
    command = match.group(1)
    prefix = match.group(2)
    arguments = []

    self._content = self._content[match.end():]

    while len(self._content):
      argIndent = self._chompSpace()

      if not len(self._content):
        break

      if self._content[0] == ")":
        self._content = self._content[1:]
        return Command(text=command, arguments=arguments, indent=indent,
                       prefix=prefix, suffix=argIndent + ")")

      else:
        arguments.append(self._parseArgument(argIndent))

    raise EOFError("unexpected EOF while parsing command (expected ')')")

  #---------------------------------------------------------------------------
  def _is(self, regex):
    self._match = regex.match(self._content)
    return self._match is not None
