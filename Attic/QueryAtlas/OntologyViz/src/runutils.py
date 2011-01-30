import java
from javax.swing import SwingUtilities

class ProcRunnable(java.lang.Runnable):
    """Wrap up a callable object and its arguments to run later."""
    
    def __init__(self, func, *args, **kwargs):
        self._func = func
        self._args = args
        self._kwargs = kwargs
    def run(self):
        return self._func(*self._args, **self._kwargs)

def invokeLater(func, *args, **kwargs):
    """Convenience method for SwingUtilities.invokeLater()."""
    SwingUtilities.invokeLater(ProcRunnable(func, *args, **kwargs))
    return

def invokeAndWait(func, *args, **kwargs):
    """Convenience method for SwingUtilities.invokeAndWait()."""    
    SwingUtilities.invokeAndWait(ProcRunnable(func, *args, **kwargs))
    return

