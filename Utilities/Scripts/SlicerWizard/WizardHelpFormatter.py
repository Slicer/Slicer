import argparse

#=============================================================================
class WizardHelpFormatter(argparse.HelpFormatter):
  """Custom formatter for |CLI| arguments.

  This formatter overrides :class:`argparse.HelpFormatter` in order to replace
  occurrences of the '<' and '>' characters with '[' and ']', respectively.
  This is done to work around the formatter's wrapping, which tries to break
  metavars if they contain these characters and then becomes confused (read:
  raises an assertion).
  """

  #---------------------------------------------------------------------------
  def _format_action_invocation(self, *args):
    text = super(WizardHelpFormatter, self)._format_action_invocation(*args)
    return text.replace("<", "[").replace(">", "]")

  #---------------------------------------------------------------------------
  def _format_usage(self, *args):
    text = super(WizardHelpFormatter, self)._format_usage(*args)
    return text.replace("<", "[").replace(">", "]")
