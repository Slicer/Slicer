"""Symbolic manipulation of CMake scripts.

This module provides a set of classes to support symbolic parsing of CMake
script. The use of symbolic parsing, as opposed to regular expressions, frees
the user from needing to worry about syntactic context (is that really a
function, or is it inside of a string or comment?) and provides a representation
that is more suitable to direct manipulation.

In addition to the several token classes provided, :class:`.CMakeScript`
provides an interface for bidirectional translation between raw text and
tokenized representations.

Unlike other parsers, this module is not a "pretty formatter", but rather is
specifically designed to preserve the original formatting of a script in order
to be able to perform convert losslessly from raw to parsed form and back,
while still providing a parsed form that is easy to use and manipulate. Care
should be used, however, when creating or manipulating scripts, as there are
effectively no safeguards against producing a script that is syntactically
invalid.
"""

import os
import re
import string

#=============================================================================
class Token(object):
  """Base class for CMake script tokens.

  This is the base class for CMake script tokens. An occurrence of a token
  whose type is exactly :class:`.Token` (i.e. not a subclass thereof) is a
  syntactic error unless the token text is empty.

  .. attribute:: text

    The textual content of the token.

  .. attribute:: indent

    The whitespace (including newlines) which preceded the token. As the parser
    is strictly preserving of whitespace, note that this must be non-empty in
    many cases in order to produce a syntactically correct script.
  """

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
  """String token.

  .. attribute:: text

    The textual content of the string. Note that escapes are not evaluated and
    will appear in their raw (escaped) form.

  .. attribute:: prefix

    The delimiter which starts this string. The delimiter may be empty,
    ``'"'``, or a lua-style long bracket (e.g. ``'[['``, ``'[===['``, etc.).

  .. attribute:: suffix

    The delimiter which ends this string, which shall match the :attr:`prefix`.

  String tokens appear as arguments to :class:`.Command`, as they are not valid
  outside of a command context.
  """

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
  """Comment token.

  .. attribute:: text

    The textual content of the comment.

  .. attribute:: prefix

    The delimiter which starts this comment: ``'#'``, optionally followed by a
    lua-style long bracket (e.g. ``'[['``, ``'[===['``, etc.).

  .. attribute:: suffix

    The delimiter which ends this comment: either empty, or a lua-style long
    bracket which shall match the long bracket in :attr:`prefix`.
  """

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
  """Command token.

  .. attribute:: text

    The name of the command.

  .. attribute:: prefix

    The delimiter which starts the command's argument list. This shall end with
    ``'('`` and may begin with whitespace if there is whitespace separating the
    command name from the '('.

  .. attribute:: suffix

    The delimiter which ends the command's argument list. This shall end with
    ``')'`` and may begin with whitespace if there is whitespace separating the
    last argument (or the opening '(' if there are no arguments) from the ')'.

  .. attribute:: arguments

    A :class:`list` of :class:`.String` tokens which comprise the arguments of
    the command.
  """

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
  """Tokenized representation of a CMake script.

  .. attribute:: tokens

    The :class:`list` of tokens which comprise the script. Manipulations of
    this list should be used to change the content of the script.
  """

  _reWhitespace = re.compile(r"\s")
  _reCommand = re.compile(r"([" + string.ascii_letters + r"]\w*)(\s*\()")
  _reComment = re.compile(r"#(\[=*\[)?")
  _reQuote = re.compile("\"")
  _reBracketQuote = re.compile(r"\[=*\[")
  _reEscape = re.compile(r"\\[\\\"nrt$ ]")

  #---------------------------------------------------------------------------
  def __init__(self, content):
    """
    :param content: Textual content of a CMake script.
    :type content: :class:`str`

    :raises:
      :exc:`~exceptions.SyntaxError` or :exc:`~exceptions.EOFError` if a
      parsing error occurs (i.e. if the input text is not syntactically valid).

    .. code-block:: python

      with open('CMakeLists.txt') as fi:
        script = CMakeParser.CMakeScript(f.read())

      with open('CMakeLists.txt.new', 'w') as fo:
        fo.write(str(script))
    """

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
      elif self._is(self._reComment):
        arguments.append(self._parseComment(self._match, argIndent))

      else:
        arguments.append(self._parseArgument(argIndent))

    raise EOFError("unexpected EOF while parsing command (expected ')')")

  #---------------------------------------------------------------------------
  def _is(self, regex):
    self._match = regex.match(self._content)
    return self._match is not None
