
import __main__

assert not hasattr(__main__, 'SOMEVAR')

assert not hasattr(__main__, 'ModuleA')
assert not hasattr(__main__, 'ModuleB')
assert not hasattr(__main__, 'ModuleC_WithoutWidget')
assert not hasattr(__main__, 'ModuleD_WithFileDialog_WithoutWidget')
assert not hasattr(__main__, 'ModuleE_WithFileWriter_WithoutWidget')

from types import ClassType, ModuleType
import slicer

assert type(slicer.modules) is ModuleType

# Global variable
assert slicer.modules.ModuleAInstance.somevar() == 'A'
assert slicer.modules.ModuleBInstance.somevar() == 'B'
assert slicer.modules.ModuleC_WithoutWidgetInstance.somevar() == 'C'
assert slicer.modules.ModuleD_WithFileDialog_WithoutWidgetInstance.somevar() == 'D'
assert slicer.modules.ModuleE_WithFileWriter_WithoutWidgetInstance.somevar() == 'E'

# Widget representation
assert isinstance(slicer.modules.modulea.widgetRepresentation(), slicer.qSlicerScriptedLoadableModuleWidget)
assert isinstance(slicer.modules.moduleb.widgetRepresentation(), slicer.qSlicerScriptedLoadableModuleWidget)
assert slicer.modules.modulec_withoutwidget.widgetRepresentation() is None
assert slicer.modules.moduled_withfiledialog_withoutwidget.widgetRepresentation() is None
assert slicer.modules.modulee_withfilewriter_withoutwidget.widgetRepresentation() is None

import ModuleA
import ModuleB
import ModuleC_WithoutWidget
import ModuleD_WithFileDialog_WithoutWidget
import ModuleE_WithFileWriter_WithoutWidget

# Check module type
assert type(ModuleA) is ModuleType
assert type(ModuleB) is ModuleType
assert type(ModuleC_WithoutWidget) is ModuleType
assert type(ModuleD_WithFileDialog_WithoutWidget) is ModuleType
assert type(ModuleE_WithFileWriter_WithoutWidget) is ModuleType

# Check class type
assert type(ModuleA.ModuleA) is ClassType
assert type(ModuleB.ModuleB) is ClassType
assert type(ModuleC_WithoutWidget.ModuleC_WithoutWidget) is ClassType
assert type(ModuleD_WithFileDialog_WithoutWidget.ModuleD_WithFileDialog_WithoutWidget) is ClassType
assert type(ModuleE_WithFileWriter_WithoutWidget.ModuleE_WithFileWriter_WithoutWidget) is ClassType

assert type(ModuleA.ModuleAWidget) is ClassType
assert type(ModuleB.ModuleBWidget) is ClassType
assert not hasattr(ModuleC_WithoutWidget, 'ModuleC_WithoutWidgetWidget')
assert not hasattr(ModuleC_WithoutWidget, 'ModuleD_WithFileDialog_WithoutWidget')
assert not hasattr(ModuleC_WithoutWidget, 'ModuleE_WithFileWriter_WithoutWidget')


# Plugins
# XXX Will need to extend module API to list registered plugins
