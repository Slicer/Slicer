
import __main__

# XXX #3549: This should not be a problem once issue #3549 is resolved.
#assert not hasattr(__main__, 'SOMEVAR')

# XXX #3549: By default, loaded module classes should not be in the global scope.
assert hasattr(__main__, 'ModuleA')
assert hasattr(__main__, 'ModuleB')

from types import ClassType, ModuleType

# XXX #3549: Once this is resolved, remove this.
assert type(ModuleA) is ClassType
assert type(ModuleB) is ClassType
assert type(ModuleAWidget) is ClassType
assert type(ModuleBWidget) is ClassType

import slicer

assert type(slicer.modules) is ModuleType

# XXX #3549: Once the issue is resolved, expected value should be changed to 'A'.
assert slicer.modules.ModuleAInstance.somevar() == 'B'
assert slicer.modules.ModuleBInstance.somevar() == 'B'

assert isinstance(slicer.modules.modulea.widgetRepresentation(), slicer.qSlicerScriptedLoadableModuleWidget)
assert isinstance(slicer.modules.moduleb.widgetRepresentation(), slicer.qSlicerScriptedLoadableModuleWidget)

import ModuleA
import ModuleB

assert type(ModuleA) is ModuleType
assert type(ModuleB) is ModuleType

assert type(ModuleA.ModuleA) is ClassType
assert type(ModuleB.ModuleB) is ClassType

assert type(ModuleA.ModuleAWidget) is ClassType
assert type(ModuleB.ModuleBWidget) is ClassType

