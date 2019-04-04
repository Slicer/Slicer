
import __main__

assert not hasattr(__main__, 'SOMEVAR')

assert not hasattr(__main__, 'ModuleA')
assert not hasattr(__main__, 'ModuleB')
assert not hasattr(__main__, 'ModuleC_WithoutWidget')
assert not hasattr(__main__, 'ModuleD_WithFileDialog_WithoutWidget')
assert not hasattr(__main__, 'ModuleE_WithFileWriter_WithoutWidget')

from types import ModuleType
import slicer

assert isinstance(slicer.modules, ModuleType)

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
assert isinstance(ModuleA, ModuleType)
assert isinstance(ModuleB, ModuleType)
assert isinstance(ModuleC_WithoutWidget, ModuleType)
assert isinstance(ModuleD_WithFileDialog_WithoutWidget, ModuleType)
assert isinstance(ModuleE_WithFileWriter_WithoutWidget, ModuleType)

# Check class type
assert isinstance(ModuleA.ModuleA, type)
assert isinstance(ModuleB.ModuleB, type)
assert isinstance(ModuleC_WithoutWidget.ModuleC_WithoutWidget, type)
assert isinstance(ModuleD_WithFileDialog_WithoutWidget.ModuleD_WithFileDialog_WithoutWidget, type)
assert isinstance(ModuleE_WithFileWriter_WithoutWidget.ModuleE_WithFileWriter_WithoutWidget, type)

assert isinstance(ModuleA.ModuleAWidget, type)
assert isinstance(ModuleB.ModuleBWidget, type)
assert not hasattr(ModuleC_WithoutWidget, 'ModuleC_WithoutWidgetWidget')
assert not hasattr(ModuleC_WithoutWidget, 'ModuleD_WithFileDialog_WithoutWidget')
assert not hasattr(ModuleC_WithoutWidget, 'ModuleE_WithFileWriter_WithoutWidget')


# Plugins
# XXX Will need to extend module API to list registered plugins
