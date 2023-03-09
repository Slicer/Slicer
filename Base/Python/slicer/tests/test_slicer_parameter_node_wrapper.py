import dataclasses
import pathlib
import typing
from typing import Annotated
import unittest
import enum

import vtk
import slicer

from slicer import (
    vtkMRMLNode,
    vtkMRMLModelNode,
    vtkMRMLScalarVolumeNode,
    vtkMRMLLabelMapVolumeNode,
)
from slicer.parameterNodeWrapper import *


def newParameterNode():
    node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScriptedModuleNode")
    return node


@dataclasses.dataclass
class CustomClass:
    x: int
    y: int
    z: int


class EnumClass(enum.Enum):
    A = 0
    B = 1


@parameterNodeSerializer
class CustomClassSerializer(Serializer):
    @staticmethod
    def canSerialize(type_) -> bool:
        return type_ == CustomClass

    @staticmethod
    def create(type_):
        if CustomClassSerializer.canSerialize(type_):
            return ValidatedSerializer(CustomClassSerializer(), [NotNone(), IsInstance(CustomClass)])
        return None

    def default(self):
        return CustomClass(0, 0, 0)

    def isIn(self, parameterNode, name: str) -> bool:
        return parameterNode.HasParameter(name)

    def write(self, parameterNode, name: str, value) -> None:
        parameterNode.SetParameter(name, f"{value.x},{value.y},{value.z}")

    def read(self, parameterNode, name: str):
        val = parameterNode.GetParameter(name)
        vals = val.split(',')
        return CustomClass(int(vals[0]), int(vals[1]), int(vals[2]))

    def remove(self, parameterNode, name: str) -> None:
        parameterNode.UnsetParameter(name)


@dataclasses.dataclass
class AnotherCustomClass:
    a: int
    b: int
    c: int


# the big difference between this and CustomClassSerializer, is that the
# this supports writing None
@parameterNodeSerializer
class AnotherCustomClassSerializer(Serializer):
    @staticmethod
    def canSerialize(type_) -> bool:
        return type_ == AnotherCustomClass

    @staticmethod
    def create(type_):
        if AnotherCustomClassSerializer.canSerialize(type_):
            return ValidatedSerializer(AnotherCustomClassSerializer(), [IsInstance(AnotherCustomClass)])
        return None

    def default(self):
        return AnotherCustomClass(0, 0, 0)

    def isIn(self, parameterNode, name: str) -> bool:
        return parameterNode.HasParameter(name)

    def write(self, parameterNode, name: str, value) -> None:
        parameterNode.SetParameter(name, f"{value.a},{value.b},{value.c}" if value is not None else "None")

    def read(self, parameterNode, name: str):
        val = parameterNode.GetParameter(name)
        if val == "None":
            return None
        else:
            vals = val.split(',')
            return AnotherCustomClass(int(vals[0]), int(vals[1]), int(vals[2]))

    def remove(self, parameterNode, name: str) -> None:
        parameterNode.UnsetParameter(name)


# not registering as a @parameterNodeSerializer because we are overriding
# the serialization of a built in type and we don't want anyone else to accidentally
# use this.
class CustomIntSerializer(Serializer):
    """
    Stores ints as their negative.
    Used to test being able to use a specific serializer.
    """
    @staticmethod
    def canSerialize(type_) -> bool:
        return type_ == int

    @staticmethod
    def create(type_):
        if CustomIntSerializer.canSerialize(type_):
            return ValidatedSerializer(CustomIntSerializer(), [NotNone(), IsInstance(int)])
        return None

    def default(self):
        return 0

    def isIn(self, parameterNode, name: str) -> bool:
        return parameterNode.HasParameter(name)

    def write(self, parameterNode, name: str, value) -> None:
        parameterNode.SetParameter(name, str(-value))

    def read(self, parameterNode, name: str):
        return -int(parameterNode.GetParameter(name))

    def remove(self, parameterNode, name: str) -> None:
        parameterNode.UnsetParameter(name)


class TypedParameterNodeTest(unittest.TestCase):
    def setUp(self):
        slicer.mrmlScene.Clear(0)

    def test_node_serialier_has_parameter(self):
        nodeSerializer = NodeSerializer()

        parameterNode = newParameterNode()

        # a bug was caused by this, which is why it is explicitly tested
        self.assertFalse(nodeSerializer.isIn(parameterNode, "node"))
        nodeSerializer.write(parameterNode, "node", None)
        self.assertTrue(nodeSerializer.isIn(parameterNode, "node"))
        nodeSerializer.remove(parameterNode, "node")
        self.assertFalse(nodeSerializer.isIn(parameterNode, "node"))

    def test_removes(self):
        # for each serializer, make sure that calling remove from it
        # erases all trace of it in the parameterNode
        numberSerializer = NumberSerializer(int)
        stringSerializer = StringSerializer()
        pathSerializer = PathSerializer(pathlib.Path)
        boolSerializer = BoolSerializer()
        nodeSerializer = NodeSerializer()
        listSerializer = ListSerializer(NumberSerializer(float))
        tupleSerializer = TupleSerializer([NumberSerializer(float), BoolSerializer()])
        dictSerializer = DictSerializer(StringSerializer(), NumberSerializer(float))
        unionSerializer = UnionSerializer([StringSerializer(), NumberSerializer(float)])

        parameterNode = newParameterNode()

        numberSerializer.write(parameterNode, "number", 1)
        stringSerializer.write(parameterNode, "string", "1")
        pathSerializer.write(parameterNode, "path", pathlib.Path("."))
        boolSerializer.write(parameterNode, "bool", True)
        nodeSerializer.write(parameterNode, "node", slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode"))
        listSerializer.write(parameterNode, "list", [1])
        tupleSerializer.write(parameterNode, "tuple", (3.3, True))
        dictSerializer.write(parameterNode, "dict", {"a": 1, "b": 2})
        unionSerializer.write(parameterNode, "union", "hello")

        numberSerializer.remove(parameterNode, "number")
        stringSerializer.remove(parameterNode, "string")
        pathSerializer.remove(parameterNode, "path")
        boolSerializer.remove(parameterNode, "bool")
        nodeSerializer.remove(parameterNode, "node")
        listSerializer.remove(parameterNode, "list")
        tupleSerializer.remove(parameterNode, "tuple")
        dictSerializer.remove(parameterNode, "dict")
        unionSerializer.remove(parameterNode, "union")

        self.assertFalse(parameterNode.GetParameterNames())

    def test_stringIsIn(self):
        stringSerializer = StringSerializer()
        parameterNode = newParameterNode()
        name = "key"

        stringSerializer.write(parameterNode, name, "")

        self.assertEqual(stringSerializer.read(parameterNode, name), "")
        self.assertTrue(stringSerializer.isIn(parameterNode, name))

    def test_isCached(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            int_: int
            listInt: list[int]
            custom: Annotated[CustomClass, Default(CustomClass(0, 0, 0))]
            listCustom: list[CustomClass]

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("int_"))
        self.assertTrue(param.isCached("listInt"))
        self.assertFalse(param.isCached("custom"))
        self.assertFalse(param.isCached("listCustom"))
        with self.assertRaises(ValueError):
            param.isCached("notExistentParameter")

    def test_custom_validator(self):
        class Is777Or778(Validator):
            @staticmethod
            def validate(value):
                if value != 777 and value != 778:
                    raise ValueError("Twas not 777 or 778")

        @parameterNodeWrapper
        class ParameterNodeType:
            i: Annotated[int, Is777Or778, Default(777)]

        param = ParameterNodeType(newParameterNode())

        with self.assertRaises(ValueError):
            param.i = -2
        param.i = 778

    def test_custom_serializer(self):
        @parameterNodeWrapper
        class CustomClassParameterNode:
            # serializer for custom type will be found because it was decorate with @parameterNodeSerializer
            custom: Annotated[CustomClass, Default(CustomClass(0, 0, 0))]

            # custom serializer for built in type
            customInt: Annotated[int, CustomIntSerializer()]

        param = CustomClassParameterNode(newParameterNode())
        self.assertEqual(param.custom.x, 0)
        self.assertEqual(param.custom.y, 0)
        self.assertEqual(param.custom.z, 0)

        self.assertEqual(param.customInt, 0)

        param.customInt = 4
        self.assertEqual(param.parameterNode.GetParameter("customInt"), "-4")
        self.assertEqual(param.customInt, 4)

    def test_custom_serializer2(self):
        @parameterNodeWrapper
        class AnotherCustomClassParameterNode:
            normalDefault: AnotherCustomClass
            noneDefault: Annotated[AnotherCustomClass, Default(None)]
            nonNoneDefault: Annotated[AnotherCustomClass, Default(AnotherCustomClass(1, 2, 3))]

        param = AnotherCustomClassParameterNode(newParameterNode())
        self.assertEqual(param.normalDefault, AnotherCustomClass(0, 0, 0))
        self.assertIsNone(param.noneDefault)
        self.assertEqual(param.nonNoneDefault, AnotherCustomClass(1, 2, 3))

    def test_getSetValue(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            float_: float
            bool_: bool
            int_: Annotated[int, Default(4), Maximum(37)]
            string_: Annotated[str, Default("TypedParam")]

        param = ParameterNodeType(newParameterNode())
        self.assertEqual(param.getValue("float_"), 0.0)
        self.assertFalse(param.getValue("bool_"))
        self.assertEqual(param.getValue("int_"), 4)
        self.assertEqual(param.getValue("string_"), "TypedParam")

        param.setValue("float_", 9.9)
        self.assertEqual(param.getValue("float_"), 9.9)
        param.setValue("bool_", True)
        self.assertTrue(param.getValue("bool_"))
        param.setValue("int_", 36)
        self.assertEqual(param.getValue("int_"), 36)
        param.setValue("string_", "Another string")
        self.assertEqual(param.getValue("string_"), "Another string")

        with self.assertRaises(TypeError):
            param.setValue("float_", "this is not a float")
        self.assertEqual(param.getValue("float_"), 9.9)
        with self.assertRaises(ValueError):
            param.setValue("int_", 38)
        self.assertEqual(param.getValue("int_"), 36)

    def test_dataType(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            float_: float
            bool_: bool
            int_: int = 4
            string_: Annotated[str, Default("TypedParam")]

        self.assertEqual(ParameterNodeType.dataType("float_"), float)
        self.assertEqual(ParameterNodeType.dataType("bool_"), bool)
        self.assertEqual(ParameterNodeType.dataType("int_"), int)
        self.assertEqual(ParameterNodeType.dataType("string_"), Annotated[str, Default("TypedParam")])

    def test_primitives(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            int1: int
            float1: float
            bool1: bool
            string1: str
            int2: int = 4
            float2: Annotated[float, Default(9.9)]
            bool2: Annotated[bool, Default(True)]
            string2: str = "TypedParam"

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("int1"))
        self.assertTrue(param.isCached("int2"))
        self.assertTrue(param.isCached("float1"))
        self.assertTrue(param.isCached("float2"))
        self.assertTrue(param.isCached("bool1"))
        self.assertTrue(param.isCached("bool2"))
        self.assertTrue(param.isCached("string1"))
        self.assertTrue(param.isCached("string2"))

        self.assertEqual(param.int1, 0)
        self.assertEqual(param.int2, 4)
        self.assertAlmostEqual(param.float1, 0.0)
        self.assertAlmostEqual(param.float2, 9.9)
        self.assertFalse(param.bool1)
        self.assertTrue(param.bool2)
        self.assertEqual(param.string1, "")
        self.assertEqual(param.string2, "TypedParam")

        param.int1 = 7
        self.assertEqual(param.int1, 7)
        param.float2 = 4  # allow implicit conversion from int
        self.assertAlmostEqual(param.float2, 4.0)
        param.float2 = 7.5
        self.assertAlmostEqual(param.float2, 7.5)
        param.bool1 = True
        self.assertTrue(param.bool1)
        param.string1 = "Python"
        self.assertEqual(param.string1, "Python")

        with self.assertRaises(ValueError):
            param.int1 = None
        with self.assertRaises(ValueError):
            param.float1 = None
        with self.assertRaises(ValueError):
            param.bool1 = None
        with self.assertRaises(ValueError):
            param.string1 = None

        param2 = ParameterNodeType(param.parameterNode)

        self.assertEqual(param2.int1, 7)
        self.assertEqual(param2.int2, 4)
        self.assertAlmostEqual(param2.float1, 0.0)
        self.assertAlmostEqual(param2.float2, 7.5)
        self.assertTrue(param2.bool1)
        self.assertTrue(param2.bool2)
        self.assertEqual(param.string1, "Python")
        self.assertEqual(param2.string2, "TypedParam")

    def test_pathlib(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            path: pathlib.Path
            purePath: pathlib.PurePath
            purePosixPath: Annotated[pathlib.PurePosixPath, Default(pathlib.PurePosixPath("/root"))]
            pureWindowsPath: pathlib.PureWindowsPath

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("path"))
        self.assertTrue(param.isCached("purePath"))
        self.assertTrue(param.isCached("purePosixPath"))
        self.assertTrue(param.isCached("pureWindowsPath"))

        self.assertEqual(param.path, pathlib.Path())
        self.assertEqual(param.purePath, pathlib.PurePath())
        self.assertEqual(param.purePosixPath, pathlib.PurePosixPath("/root"))
        self.assertEqual(param.pureWindowsPath, pathlib.PureWindowsPath())

        self.assertIsInstance(param.path, pathlib.Path)
        self.assertIsInstance(param.purePath, pathlib.PurePath)
        self.assertIsInstance(param.purePosixPath, pathlib.PurePosixPath)
        self.assertIsInstance(param.pureWindowsPath, pathlib.PureWindowsPath)

        param.path = pathlib.Path("relativePath/folder")
        param.purePath = pathlib.PurePath("relativePath/folder")
        param.purePosixPath = pathlib.PurePosixPath("relativePath/folder")
        param.pureWindowsPath = pathlib.PureWindowsPath("relativePath/folder")

        self.assertEqual(param.path, pathlib.Path("relativePath/folder"))
        self.assertEqual(param.purePath, pathlib.PurePath("relativePath/folder"))
        self.assertEqual(param.purePosixPath, pathlib.PurePosixPath("relativePath/folder"))
        self.assertEqual(param.pureWindowsPath, pathlib.PureWindowsPath("relativePath/folder"))

        # test that it saved and can be read elsewhere
        param2 = ParameterNodeType(param.parameterNode)

        self.assertEqual(param2.path, pathlib.Path("relativePath/folder"))
        self.assertEqual(param2.purePath, pathlib.PurePath("relativePath/folder"))
        self.assertEqual(param2.purePosixPath, pathlib.PurePosixPath("relativePath/folder"))
        self.assertEqual(param2.pureWindowsPath, pathlib.PureWindowsPath("relativePath/folder"))

    def test_default_generator(self):
        count = 0

        def nextCount():
            nonlocal count
            count += 1
            return count - 1
        
        @parameterNodeWrapper
        class ParameterNodeType:
            x: Annotated[int, Default(generator=nextCount)]

        param = ParameterNodeType(newParameterNode())
        self.assertEqual(param.x, 0)
        param1 = ParameterNodeType(newParameterNode())
        self.assertEqual(param1.x, 1)

    def test_multiple_instances_are_independent(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            single: str
            multi: list[int]

        param1 = ParameterNodeType(newParameterNode())
        param2 = ParameterNodeType(newParameterNode())

        param1.single = "hi"
        param1.multi = [1, 2, 3]
        param2.single = "hello"
        param2.multi = [4, 5, 6]

        self.assertEqual(param1.single, "hi")
        self.assertEqual(param1.multi, [1, 2, 3])
        self.assertEqual(param2.single, "hello")
        self.assertEqual(param2.multi, [4, 5, 6])

    def test_list_int(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            p: list[int]

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("p"))

        p = param.p
        self.assertEqual(len(p), 0)

        p.append(4)
        p.append(1)
        p.append(7)
        self.assertEqual(param.p, [4, 1, 7])

        with self.assertRaises(TypeError):
            p.append("hi")
        self.assertEqual(param.p, [4, 1, 7])

        p.reverse()
        self.assertEqual(param.p, [7, 1, 4])

        p.sort()
        self.assertEqual(param.p, [1, 4, 7])

        self.assertEqual(4, p.pop(1))
        self.assertEqual(param.p, [1, 7])

        p.remove(7)
        self.assertEqual(param.p, [1])

        with self.assertRaises(ValueError):
            p.remove(44)  # not in the list
        self.assertEqual(param.p, [1])

        p += [3, 4, 5]
        self.assertEqual(param.p, [1, 3, 4, 5])

        p *= 2
        self.assertEqual(param.p, [1, 3, 4, 5, 1, 3, 4, 5])

        del p[2]
        self.assertEqual(param.p, [1, 3, 5, 1, 3, 4, 5])

        p.insert(4, 66)
        self.assertEqual(param.p, [1, 3, 5, 1, 66, 3, 4, 5])

        p.clear()
        self.assertEqual(param.p, [])

        p.extend([2, 3, 4])
        self.assertEqual(param.p, [2, 3, 4])

        p[0] = 7
        self.assertEqual(param.p, [7, 3, 4])

        self.assertEqual(param.p[0], 7)
        self.assertEqual(param.p[1], 3)
        self.assertEqual(param.p[2], 4)

        with self.assertRaises(NotImplementedError):
            _ = p + [4]
        with self.assertRaises(NotImplementedError):
            _ = [4] + p
        with self.assertRaises(NotImplementedError):
            _ = p * 2
        with self.assertRaises(NotImplementedError):
            _ = 2 * p

        p += [4, 5, 6]
        self.assertEqual(param.p, [7, 3, 4, 4, 5, 6])

        p *= 2
        self.assertEqual(param.p, [7, 3, 4, 4, 5, 6, 7, 3, 4, 4, 5, 6])

        # setting param.p will invalidate p
        param.p = [5, 1, 2]
        self.assertEqual(param.p, [5, 1, 2])

        # set from tuple
        param.p = (7, 3, 4)
        self.assertEqual(param.p, [7, 3, 4])

        param2 = ParameterNodeType(param.parameterNode)
        self.assertEqual(param2.p, [7, 3, 4])

        # allow explicit conversion to list, which breaks observation
        nonObserved = list(param.p)
        self.assertIsInstance(nonObserved, list)
        self.assertEqual(nonObserved, [7, 3, 4])

        nonObserved.append(1)
        self.assertEqual(nonObserved, [7, 3, 4, 1])
        self.assertEqual(param.p, [7, 3, 4])

    def test_list_custom(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            p: list[CustomClass]

        param = ParameterNodeType(newParameterNode())
        self.assertFalse(param.isCached("p"))

        p = param.p
        self.assertEqual(len(p), 0)

        param.p = [CustomClass(1, 2, 3), CustomClass(4, 5, 6)]
        self.assertEqual(param.p, [CustomClass(1, 2, 3), CustomClass(4, 5, 6)])

        # test that it saved and can be read elsewhere
        param2 = ParameterNodeType(param.parameterNode)
        self.assertEqual(len(param2.p), 2)
        self.assertEqual(param2.p, [CustomClass(1, 2, 3), CustomClass(4, 5, 6)])

    def test_list_of_annotated(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            choices: list[Annotated[str, Choice(["yes", "no", "maybe"])]]

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("choices"))

        param.choices.append("yes")

        self.assertEqual(param.choices, ["yes"])
        with self.assertRaises(ValueError):
            param.choices.append("invalid")
        self.assertEqual(param.choices, ["yes"])
        with self.assertRaises(ValueError):
            param.choices += ["no", "invalid"]
        self.assertEqual(param.choices, ["yes"])

        param.choices.extend(["maybe", "no"])
        self.assertEqual(param.choices, ["yes", "maybe", "no"])

    def test_list_of_lists(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            p: list[list[int]]
        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("p"))

        pl = param.p

        pl.append([])
        pl[0] = [1, 2, 3]
        pl.append([4, 5, 6])
        pl[1].append(7)

        pl[1][2] = 0

        self.assertEqual(param.p, [[1, 2, 3], [4, 5, 0, 7]])

        del pl[0]
        self.assertEqual(param.p, [[4, 5, 0, 7]])

        pl.insert(0, [6, 7])
        self.assertEqual(param.p, [[6, 7], [4, 5, 0, 7]])

        pl.pop(0)
        self.assertEqual(param.p, [[4, 5, 0, 7]])

        param2 = ParameterNodeType(param.parameterNode)
        self.assertEqual(param2.p, [[4, 5, 0, 7]])

    def test_tuple(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            a: tuple[Annotated[int, Minimum(0)], str]
            b: tuple[Annotated[float, Minimum(4)], bool] = (44.0, False)
            c: tuple[list[int]]

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("a"))
        self.assertTrue(param.isCached("b"))
        self.assertTrue(param.isCached("c"))

        self.assertEqual(param.a, (0, ""))
        self.assertEqual(param.b, (44.0, False))
        self.assertEqual(param.c, ([], ))

        param.a = (1, "a")
        param.b = (4.4, False)
        param.c = ([1, 4], )
        param.c[0].append(7)
        c0 = param.c[0]
        c0.pop(0)

        self.assertEqual(param.a, (1, "a"))
        self.assertEqual(param.b, (4.4, False))
        self.assertEqual(param.c, ([4, 7], ))

        with self.assertRaises(ValueError):
            param.a = (-2, "hi")
        with self.assertRaises(ValueError):
            param.b = (3.9, True)

        self.assertEqual(param.a, (1, "a"))
        self.assertEqual(param.b, (4.4, False))
        self.assertEqual(param.c, ([4, 7], ))

    def test_tuple_of_custom(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            a: tuple[CustomClass, int]

        param = ParameterNodeType(newParameterNode())
        self.assertFalse(param.isCached("a"))

        self.assertEqual(param.a, (CustomClass(0, 0, 0), 0))

        param.a = (CustomClass(7, 6, 5), 9)
        self.assertEqual(param.a, (CustomClass(7, 6, 5), 9))

    def test_dict(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            a: dict[str, int]

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("a"))

        self.assertEqual(len(param.a), 0)

        with self.assertRaises(KeyError):
            param.a["x"]

        param.a["x"] = 1
        self.assertEqual(param.a["x"], 1)

        param.a["q"] = 7
        self.assertEqual(param.a["q"], 7)

        self.assertTrue("q" in param.a)
        self.assertEqual(param.a, {"x": 1, "q": 7})
        self.assertEqual(list(param.a.items()), [("x", 1), ("q", 7)])
        self.assertEqual(list(param.a.keys()), ["x", "q"])
        self.assertEqual(list(param.a.values()), [1, 7])

        self.assertEqual(len(param.a), 2)

        del param.a["q"]
        self.assertEqual(len(param.a), 1)
        self.assertEqual(param.a, {"x": 1})

        self.assertEqual(1, param.a.pop("x"))
        self.assertEqual(param.a, {})

        param.a = {"w": 4, "c": 3}
        self.assertEqual(param.a, {"w": 4, "c": 3})

        self.assertEqual(("c", 3), param.a.popitem())
        self.assertEqual(param.a, {"w": 4})

        param.a.clear()
        self.assertEqual(param.a, {})

        param.a["z"] = 101
        param.a["aa"] = 102

        # comparing the dict as a list ensures the ordering is correct
        self.assertEqual(list(param.a.items()), [("z", 101), ("aa", 102)])

        param2 = ParameterNodeType(param.parameterNode)
        self.assertEqual(len(param2.a), 2)
        self.assertEqual(param2.a["z"], 101)
        self.assertEqual(param2.a["aa"], 102)
        self.assertEqual(list(param2.a.items()), [("z", 101), ("aa", 102)])

        # ensure we aren't accidentally cheating and using the same object
        self.assertIsNot(param, param2)
        self.assertIsNot(param.a, param2.a)

        param2.a["aa"] = 103
        self.assertEqual(param.a["aa"], 103)

    def test_dict_of_list(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            a: dict[int, list[str]]

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("a"))

        self.assertEqual(len(param.a), 0)

        param.a[0] = ["a", "b"]
        param.a[1] = ["c", "d"]

        param.a[0] += ["q", "r"]
        a1 = param.a[1]
        a1.pop(1)

        self.assertEqual(param.a[0], ["a", "b", "q", "r"])
        self.assertEqual(param.a[1], ["c"])
        self.assertEqual(list(param.a.items()), [(0, ["a", "b", "q", "r"]), (1, ["c"])])

    def test_node(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            inputs: list[vtkMRMLModelNode]
            output: vtkMRMLModelNode

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("inputs"))
        self.assertTrue(param.isCached("output"))

        inputs = [slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode') for _ in range(5)]
        param.inputs = inputs
        self.assertEqual(param.inputs, inputs)

        self.assertIsInstance(param.inputs[0], vtkMRMLModelNode)

        output = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode')
        param.output = output
        self.assertEqual(param.output, output)

        # cannot set a model to a markup
        with self.assertRaises(TypeError):
            param.output = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScalarVolumeNode')

        param.output = None
        self.assertIsNone(param.output)

    def test_node_baseclass(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            node: vtkMRMLNode

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("node"))

        param.node = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode')
        self.assertIsInstance(param.node, vtkMRMLModelNode)
        param.node = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScalarVolumeNode')
        self.assertIsInstance(param.node, vtkMRMLScalarVolumeNode)

        with self.assertRaises(TypeError):
            param.node = 4

    def test_union_simple(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            intOrStr: typing.Union[int, str]
            optBool: typing.Optional[bool]

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("intOrStr"))
        self.assertTrue(param.isCached("optBool"))

        self.assertEqual(param.intOrStr, 0)
        self.assertEqual(param.optBool, None)

        param.intOrStr = "hi"
        self.assertEqual(param.intOrStr, "hi")

        param.intOrStr = 99
        self.assertEqual(param.intOrStr, 99)

        param.optBool = True
        self.assertEqual(param.optBool, True)
        param.optBool = False
        self.assertEqual(param.optBool, False)
        param.optBool = None
        self.assertEqual(param.optBool, None)

        with self.assertRaises(TypeError):
            param.intOrStr = False
        self.assertEqual(param.intOrStr, 99)
        self.assertEqual(param.optBool, None)

    def test_union_many(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            value: typing.Union[bool, str, int, vtkMRMLModelNode]

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("value"))

        self.assertEqual(param.value, False)
        param.value = 4
        self.assertEqual(param.value, 4)
        param.value = True
        self.assertEqual(param.value, True)
        param.value = "string"
        self.assertEqual(param.value, "string")
        model = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode")
        param.value = model
        self.assertEqual(param.value, model)

        with self.assertRaises(TypeError):
            param.value = 4.0
        self.assertEqual(param.value, model)

    def test_union_multiple_nodes(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            value: typing.Union[vtkMRMLModelNode, vtkMRMLLabelMapVolumeNode, None]

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("value"))

        self.assertIsNone(param.value)

        model = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode')
        labelMapVolume = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLLabelMapVolumeNode')
        scalarVolume = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLScalarVolumeNode')

        param.value = model
        self.assertIs(param.value, model)
        param.value = labelMapVolume
        self.assertIs(param.value, labelMapVolume)
        with self.assertRaises(TypeError):
            param.value = scalarVolume
        self.assertIs(param.value, labelMapVolume)

    def test_validated_union(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            value: Annotated[typing.Union[
                Annotated[int, Minimum(5)],
                Annotated[str, Choice(["q", "r", "s"])],
            ], Default(7)]

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("value"))

        self.assertEqual(param.value, 7)
        param.value = 99
        self.assertEqual(param.value, 99)
        param.value = "q"
        self.assertEqual(param.value, "q")
        param.value = 96
        self.assertEqual(param.value, 96)

        with self.assertRaises(ValueError):
            param.value = 4
        self.assertEqual(param.value, 96)
        with self.assertRaises(ValueError):
            param.value = "p"
        self.assertEqual(param.value, 96)

        param.value = "s"
        self.assertEqual(param.value, "s")

    def test_enum(self):
        class Color(enum.Enum):
            RED = enum.auto()
            GREEN = enum.auto()
            BLUE = enum.auto()

            red = RED
            green = GREEN
            blue = BLUE

        class Other(enum.Enum):
            BAD = enum.auto()

        @parameterNodeWrapper
        class ParameterNodeType:
            fg: Color
            bg: Annotated[Color, Default(Color.BLUE)]

        # test default content
        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("fg"))
        self.assertTrue(param.isCached("bg"))

        self.assertIs(param.fg, Color.RED)
        self.assertIs(param.bg, Color.BLUE)

        param.fg = Color.GREEN
        self.assertIs(param.fg, Color.GREEN)

        # test cannot set non-Color value
        with self.assertRaises(TypeError):
            param.fg = "RED"
        with self.assertRaises(TypeError):
            param.fg = 0
        self.assertIs(param.fg, Color.GREEN)
        with self.assertRaises(TypeError):
            param.fg = Other.BAD
        self.assertIs(param.fg, Color.GREEN)

        # test alias
        param.bg = Color.blue
        self.assertIs(param.bg, Color.BLUE)

    def test_any(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            any: typing.Any
            anyDefaulted: Annotated[typing.Any, Default(7.7)]

        param = ParameterNodeType(newParameterNode())
        self.assertIsNone(param.any)
        self.assertEqual(param.anyDefaulted, 7.7)

        param.any = 1
        self.assertEqual(param.any, 1)
        param.any = "some string"
        self.assertEqual(param.any, "some string")
        param.any = EnumClass.B
        self.assertEqual(param.any, EnumClass.B)
        param.any = CustomClass(2, 3, 4)
        self.assertEqual(param.any, CustomClass(2, 3, 4))
        param.any = None
        self.assertEqual(param.any, None)
        param.any = [1, 2, 3]
        self.assertEqual(param.any, [1, 2, 3])
        param.any = [1, "multiple types", True]
        self.assertEqual(param.any, [1, "multiple types", True])
        param.any = {1: "a", "b": 2}
        self.assertEqual(param.any, {1: "a", "b": 2})

        param.anyDefaulted = 1
        self.assertEqual(param.anyDefaulted, 1)
        param.anyDefaulted = "some string"
        self.assertEqual(param.anyDefaulted, "some string")

    def test_events(self):
        class _Callback:
            def __init__(self):
                self.called = 0

            def call(self, caller, event):
                self.called += 1
        callback = _Callback()

        @parameterNodeWrapper
        class ParameterNodeType:
            i: int
            s: list[str]
            n: vtkMRMLNode

        param = ParameterNodeType(newParameterNode())
        self.assertTrue(param.isCached("i"))
        self.assertTrue(param.isCached("s"))
        self.assertTrue(param.isCached("n"))

        tag = param.AddObserver(vtk.vtkCommand.ModifiedEvent, callback.call)

        param.i = 4
        self.assertEqual(1, callback.called)

        obs = param.s
        obs += ('string1', 'string2')
        self.assertEqual(2, callback.called)

        list(param.s).append('Should not cause an event')
        self.assertEqual(2, callback.called)

        param.s = ['a', 'b', 'c']
        self.assertEqual(3, callback.called)

        param.n = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode')
        self.assertEqual(4, callback.called)

        param.n = None
        self.assertEqual(5, callback.called)

        with slicer.util.NodeModify(param):
            param.i = 9
            self.assertEqual(param.i, 9)
            param.s = []
            self.assertEqual(param.s, [])
            node = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode')
            param.n = node
            self.assertEqual(param.n, node)

        self.assertEqual(6, callback.called)

        param.RemoveObserver(tag)
        param.i = 7
        self.assertEqual(6, callback.called)

    def test_validators(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            min0: Annotated[int, Minimum(0)]
            max0: Annotated[int, Maximum(0)]
            within0_10: Annotated[float, WithinRange(0, 10)]
            choiceStr: Annotated[str, Choice(['a', 'b', 'c']), Default('a')]
            excludeStr: Annotated[str, Exclude(['a', 'b', 'c'])]

        param = ParameterNodeType(newParameterNode())

        param.min0 = 0
        param.min0 = 4
        with self.assertRaises(ValueError):
            param.min0 = -1
        self.assertEqual(param.min0, 4)

        param.max0 = 0
        param.max0 = -4
        with self.assertRaises(ValueError):
            param.max0 = 1
        self.assertEqual(param.max0, -4)

        param.within0_10 = 0
        param.within0_10 = 10
        param.within0_10 = 5
        with self.assertRaises(ValueError):
            param.within0_10 = 11
        with self.assertRaises(ValueError):
            param.within0_10 = -1
        self.assertEqual(param.within0_10, 5)

        param.choiceStr = 'a'
        param.choiceStr = 'b'
        param.choiceStr = 'c'
        with self.assertRaises(ValueError):
            param.choiceStr = 'd'

        param.excludeStr = 'd'
        param.excludeStr = ''
        param.excludeStr = 'e'
        with self.assertRaises(ValueError):
            param.excludeStr = 'a'
        with self.assertRaises(ValueError):
            param.excludeStr = 'b'
        with self.assertRaises(ValueError):
            param.excludeStr = 'c'

    def test_overlapping_members(self):
        @parameterNodeWrapper
        class ParameterNodeType1:
            x: int
            y: int

        @parameterNodeWrapper
        class ParameterNodeType2:
            x: Annotated[int, Default(6)]
            z: int

        parameterNode = newParameterNode()
        param1 = ParameterNodeType1(parameterNode, prefix="type1")
        param2 = ParameterNodeType2(parameterNode, prefix="type2")

        self.assertEqual(param1.x, 0)
        self.assertEqual(param1.y, 0)
        self.assertEqual(param2.x, 6)
        self.assertEqual(param2.z, 0)

        param1.x = 4
        param1.y = 3
        param2.x = 7
        param2.z = 8

        self.assertEqual(param1.x, 4)
        self.assertEqual(param1.y, 3)
        self.assertEqual(param2.x, 7)
        self.assertEqual(param2.z, 8)

    def test_parameter_node_reuse(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            x: int
            y: list[int]

        parameterNode = newParameterNode()
        param1 = ParameterNodeType(parameterNode, prefix="use1")
        param2 = ParameterNodeType(parameterNode, prefix="use2")

        self.assertEqual(param1.x, 0)
        self.assertEqual(param1.y, [])
        self.assertEqual(param2.x, 0)
        self.assertEqual(param2.y, [])

        param1.x = 4
        param1.y = [3]
        param2.x = 7
        param2.y = [8, 99]

        self.assertEqual(param1.x, 4)
        self.assertEqual(param1.y, [3])
        self.assertEqual(param2.x, 7)
        self.assertEqual(param2.y, [8, 99])

    def timing_test_timeit_read_int(self):
        """
        Manual test function that prints some benchmark timings.
        Can be used to test the speed of different MakeTypedParameterNode
        implementations.
        """
        @parameterNodeWrapper
        class ParameterNodeType:
            i: int
            mi: list[int]
            f: float
            mf: list[float]

        param = ParameterNodeType(newParameterNode())
        param.mi = [7] * 100
        param.mf = [7.0] * 100

        def seti():
            param.i = 7

        def setf():
            param.f = 7.0

        def setmi():
            param.mi = [7] * 100

        def setmf():
            param.mf = [7.0] * 100

        import timeit
        print("get int              ", timeit.timeit(lambda: param.i, number=100_000))
        print("set int              ", timeit.timeit(seti, number=100_000))
        print("get float            ", timeit.timeit(lambda: param.f, number=100_000))
        print("set float            ", timeit.timeit(setf, number=100_000))
        print("get list of 100 int  ", timeit.timeit(lambda: param.mi, number=100_000))
        print("set list of 100 int  ", timeit.timeit(setmi, number=100_000))
        print("get list of 100 float", timeit.timeit(lambda: param.mf, number=100_000))
        print("set list of 100 float", timeit.timeit(setmf, number=100_000))

        # failure so it shows up with ctest --output-on-failure
        self.assertTrue(False)
