import ast
import os


# =============================================================================
#
# _ui_CreateComponentDialog
#
# =============================================================================
# =============================================================================
#
# ModuleInfo
#
# =============================================================================
class ModuleInfo:
    # ---------------------------------------------------------------------------
    def __init__(self, path, key=None):
        self.path = path
        self.searchPath = os.path.dirname(path)

        if key is None:
            self.key = os.path.splitext(os.path.basename(path))[0]
        else:
            self.key = key

    # ---------------------------------------------------------------------------
    def __repr__(self):
        return "ModuleInfo(key=%(key)r, path=%(path)r)" % self.__dict__

    # ---------------------------------------------------------------------------
    def __str__(self):
        return self.path

    # ---------------------------------------------------------------------------
    @staticmethod
    def findModules(path, depth):
        result = []
        if os.path.isfile(path):
            entries = [path]
        else:
            entries = [os.path.join(path, entry) for entry in os.listdir(path)]
            # If the folder contains __init__.py, it means that this folder
            # is not a Slicer module but an embedded Python library that a module will load.
            if any(entry.endswith('__init__.py') for entry in entries):
                entries = []

        if depth > 0:
            for entry in filter(os.path.isdir, entries):
                result += ModuleInfo.findModules(entry, depth - 1)

        for entry in filter(os.path.isfile, entries):
            if not entry.endswith(".py"):
                continue

            # Criteria for a Slicer module to have a module class
            # that has the same name as the filename.

            try:
                # Find all class definitions
                with open(entry) as entry_file:
                    tree = ast.parse(entry_file.read())
                classes = [node for node in tree.body if isinstance(node, ast.ClassDef)]

                # Add file if module class is found
                filename = os.path.basename(entry)
                expectedClassName = os.path.splitext(filename)[0]
                for cls in classes:
                    if cls.name == expectedClassName:
                        result.append(ModuleInfo(entry))
            except:
                # Error while processing the file (e.g., syntax error),
                # it cannot be a Slicer module.
                pass

        return result
