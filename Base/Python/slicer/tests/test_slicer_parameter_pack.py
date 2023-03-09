import unittest
from typing import Annotated, Any, Union

import slicer
from slicer import vtkMRMLModelNode
from slicer.parameterNodeWrapper import *
from slicer.parameterNodeWrapper.parameterPack import ParameterPackSerializer


def newParameterNode():
    node = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScriptedModuleNode")
    return node


@parameterPack
class Point:
    x: float
    y: float


@parameterPack
class BoundingBox:
    topLeft: Point
    bottomRight: Point


class BadDateException(ValueError):
    pass


@parameterPack
class Date:
    _month: int = 1
    _day: int = 1
    _year: int = 1970

    @staticmethod
    def checkDate(month, day, year):
        if month < 1 or month > 12 or day < 1 or day > 31:
            raise BadDateException(f"Bad date: {month}/{day}/{year}")
        if month == 2 and day > 28:
            raise BadDateException(f"Bad date: {month}/{day}/{year}")
        if month in (4, 6, 9, 11) and day > 30:
            raise BadDateException(f"Bad date: {month}/{day}/{year}")

    def __init__(self, month=None, day=None, year=None) -> None:
        if month is not None:
            self._month = month
        if day is not None:
            self._day = day
        if year is not None:
            self._year = year
        self.checkDate(self._month, self._day, self._year)

    def __str__(self) -> str:
        return f"Date(month={self.month}, day={self.day}, year={self.year})"

    @property
    def month(self):
        return self._month

    @month.setter
    def month(self, value):
        self.checkDate(value, self.day, self.year)
        self._month = value

    @property
    def day(self):
        return self._day

    @day.setter
    def day(self, value):
        self.checkDate(self.month, value, self.year)
        self._day = value

    @property
    def year(self):
        return self._year

    @year.setter
    def year(self, value):
        self._year = value

    def setDate(self, month: int, day: int, year: int) -> None:
        self.checkDate(month, day, year)
        self._month = month
        self._day = day
        self._year = year


class TypedParameterNodeTest(unittest.TestCase):
    def setUp(self):
        slicer.mrmlScene.Clear(0)

    def test_parameter_pack(self):
        @parameterPack
        class ParameterPack:
            x: int
            y: Annotated[int, Default(1)]

        pack = ParameterPack()
        self.assertEqual(pack.x, 0)
        self.assertEqual(pack.y, 1)

        pack2 = ParameterPack(2, 3)
        self.assertEqual(pack2.x, 2)
        self.assertEqual(pack2.y, 3)

        pack3 = ParameterPack(y=2, x=3)
        self.assertEqual(pack3.x, 3)
        self.assertEqual(pack3.y, 2)

        pack4 = ParameterPack(6, y=8)
        self.assertEqual(pack4.x, 6)
        self.assertEqual(pack4.y, 8)

        pack5 = ParameterPack(6)
        self.assertEqual(pack5.x, 6)
        self.assertEqual(pack5.y, 1)

        pack6 = ParameterPack(y=6)
        self.assertEqual(pack6.x, 0)
        self.assertEqual(pack6.y, 6)

        with self.assertRaises(ValueError):
            pack.x = None
        with self.assertRaises(TypeError):
            pack.x = "hi"

    def test_parameter_pack_validation(self):
        @parameterPack
        class ParameterPack:
            x: Annotated[float, WithinRange(0, 10)]
            y: Annotated[str, Choice(["a", "b"]), Default("b")]

        pack = ParameterPack()
        self.assertEqual(pack.x, 0)
        self.assertEqual(pack.y, "b")

        with self.assertRaises(ValueError):
            ParameterPack(-1, "a")
        with self.assertRaises(ValueError):
            ParameterPack(10.001, "a")
        with self.assertRaises(ValueError):
            ParameterPack(0, "c")
        with self.assertRaises(ValueError):
            ParameterPack(None, "a")
        with self.assertRaises(TypeError):
            ParameterPack(2.2, 4)

    def test_parameter_pack_nesting(self):
        box = BoundingBox()
        self.assertEqual(box.topLeft, Point())
        self.assertEqual(box.bottomRight, Point())

        box.topLeft = Point(-2, 3)
        box.bottomRight.x = 2
        box.bottomRight.y = -3

        self.assertEqual(box, BoundingBox(Point(-2, 3), Point(2, -3)))

    def test_parameter_pack_custom_init_eq_repr_str(self):
        @parameterPack
        class ParameterPack:
            i: int
            j: int
            k: str

            def __init__(self, k):
                self.i = 1
                self.j = 4
                self.k = k

            def __str__(self) -> str:
                return "mystr"

            def __repr__(self) -> str:
                return "myrepr"

            def __eq__(self, other) -> bool:
                return self.j == other.j

        pack = ParameterPack("hey")
        self.assertEqual(pack.i, 1)
        self.assertEqual(pack.j, 4)
        self.assertEqual(pack.k, "hey")

        pack.j = 5
        pack2 = ParameterPack("words")
        pack2.i = 6
        pack2.j = 5

        self.assertEqual(pack, pack2)

        self.assertEqual(str(pack), "mystr")
        self.assertEqual(repr(pack), "myrepr")

    def test_serialization(self):
        @parameterPack
        class ParameterPack:
            x: int
            y: str

        @parameterNodeWrapper
        class ParameterNodeType:
            pack: ParameterPack

        param = ParameterNodeType(newParameterNode())

        self.assertEqual(param.pack, ParameterPack())

        # set whole
        param.pack = ParameterPack(4, "hello")
        self.assertEqual(param.pack, ParameterPack(4, "hello"))
        self.assertEqual(param.pack.x, 4)
        self.assertEqual(param.pack.y, "hello")

        # set piecewise
        param.pack.x = 77
        param.pack.y = "goodbye"
        self.assertEqual(param.pack, ParameterPack(77, "goodbye"))
        self.assertEqual(param.pack.x, 77)
        self.assertEqual(param.pack.y, "goodbye")

        param2 = ParameterNodeType(param.parameterNode)

        self.assertIsNot(param.pack, param2.pack)
        self.assertEqual(param.pack, param2.pack)

        # assert you can't dynamically add an attribute to the pack
        with self.assertRaises(AttributeError):
            param.pack.notAMember = 44

    def test_serialization_of_nested(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            box: BoundingBox = BoundingBox(Point(0, 1), Point(1, 0))

        param = ParameterNodeType(newParameterNode())

        self.assertEqual(param.box, BoundingBox(Point(0, 1), Point(1, 0)))
        self.assertEqual(param.box.topLeft, Point(0, 1))
        self.assertEqual(param.box.bottomRight, Point(1, 0))

        param.box = BoundingBox(Point(-1, 2), Point(2, 1))
        self.assertEqual(param.box, BoundingBox(Point(-1, 2), Point(2, 1)))

        param.box.topLeft = Point(-4, 5)
        param.box.bottomRight.x = 4
        param.box.bottomRight.y = -5
        self.assertEqual(param.box, BoundingBox(Point(-4, 5), Point(4, -5)))

        param2 = ParameterNodeType(param.parameterNode)

        self.assertIsNot(param.box, param2.box)
        self.assertEqual(param.box, param2.box)

        # some people may rely on underlying serialization name to access from C++
        # (not necessarily recommended, but could be needed), so at least alert someone
        # if they do something that changes the names
        names = param.parameterNode.GetParameterNames()
        self.assertEqual(len(names), 4)
        self.assertIn("box.topLeft.x", names)
        self.assertIn("box.topLeft.y", names)
        self.assertIn("box.bottomRight.x", names)
        self.assertIn("box.bottomRight.y", names)

    def test_serialization_list_of_parameter_pack(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            cloud: list[Point]

        param = ParameterNodeType(newParameterNode())

        param.cloud.append(Point(0, 0))
        param.cloud.append(Point(1, 1))
        param.cloud.append(Point(2, 2))

        self.assertEqual(param.cloud, [Point(0, 0), Point(1, 1), Point(2, 2)])

        cloud = param.cloud
        point = cloud[0]
        point.x = 7

        self.assertEqual(param.cloud, [Point(7, 0), Point(1, 1), Point(2, 2)])

    def test_serialization_with_node(self):
        @parameterPack
        class ModelInfo:
            model: vtkMRMLModelNode
            treatSpecial: bool
        
        @parameterNodeWrapper
        class ParameterNodeType:
            nodes: list[ModelInfo]

        param = ParameterNodeType(newParameterNode())

        param.nodes.append(ModelInfo(
            slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode", "n1"),
            True
        ))
        param.nodes.append(ModelInfo(
            slicer.mrmlScene.AddNewNodeByClass("vtkMRMLModelNode", "n2"),
            False
        ))

        self.assertEqual(param.nodes[0].model.GetName(), "n1")
        self.assertEqual(param.nodes[0].treatSpecial, True)
        self.assertEqual(param.nodes[1].model.GetName(), "n2")
        self.assertEqual(param.nodes[1].treatSpecial, False)

    def test_parameter_pack_reserved_error(self):
        # Make sure an error raises if any of the reserved member names are used.
        # Very much white-box testing. I doubt these errors will ever come
        # up in real use.

        # reserved names
        with self.assertRaises(ValueError):
            @parameterPack
            class ParameterPack:
                allParameters: int
        with self.assertRaises(ValueError):
            @parameterPack
            class ParameterPack:
                _is_parameterPack: int

        # reserved names based off of other names in the pack
        with self.assertRaises(ValueError):
            @parameterPack
            class ParameterPack:
                myName: int
                _parameterPack_myName_impl: int
        with self.assertRaises(ValueError):
            @parameterPack
            class ParameterPack:
                _parameterPack_myName_impl: int
                myName: int
        with self.assertRaises(ValueError):
            @parameterPack
            class ParameterPack:
                myName: int
                _parameterPack_myName_serializer: int
        with self.assertRaises(ValueError):
            @parameterPack
            class ParameterPack:
                _parameterPack_myName_serializer: int
                myName: int

    def test_parameter_pack_serializer_remove(self):
        @parameterPack
        class ParameterPack:
            cloud: list[Point]
            value: int
        serializer = ParameterPackSerializer(ParameterPack)

        pack = ParameterPack([Point(1, 2), Point(3, 4)], 5)

        parameterNode = newParameterNode()
        serializer.write(parameterNode, "pack", pack)

        serializer.remove(parameterNode, "pack")

        self.assertFalse(parameterNode.GetParameterNames())

    def test_isParameterPack(self):
        @parameterPack
        class LocalPack:
            i: int

        self.assertTrue(isParameterPack(LocalPack))  # class
        self.assertTrue(isParameterPack(LocalPack()))  # object
        self.assertTrue(isParameterPack(BoundingBox))  # class
        self.assertTrue(isParameterPack(BoundingBox()))  # object
        self.assertFalse(isParameterPack(int))  # class
        self.assertFalse(isParameterPack(int()))  # object
        self.assertFalse(isParameterPack(str))  # class
        self.assertFalse(isParameterPack(''))  # object

    def test_parameter_pack_getSetValue(self):
        @parameterPack
        class ParameterPack:
            box: BoundingBox
            value: int

        pack = ParameterPack(box=BoundingBox(Point(-20, 3), Point(2, -30)), value=778)
        self.assertEqual(pack.getValue("box"), BoundingBox(Point(-20, 3), Point(2, -30)))
        self.assertEqual(pack.getValue("box.topLeft"), Point(-20, 3))
        self.assertEqual(pack.getValue("box.bottomRight.y"), -30)
        self.assertEqual(pack.getValue("value"), 778)

        with self.assertRaises(ValueError):
            pack.getValue("invalid")
        with self.assertRaises(ValueError):
            pack.getValue("box.invalid")
        with self.assertRaises(ValueError):
            pack.getValue("box.topLeft.invalid")
        with self.assertRaises(ValueError):
            pack.getValue("value.invalid")

        pack.setValue("box", BoundingBox(Point(9, 8), Point(7, 6)))
        self.assertEqual(pack.getValue("box"), BoundingBox(Point(9, 8), Point(7, 6)))
        pack.setValue("box.bottomRight", Point(11, 10))
        self.assertEqual(pack.getValue("box"), BoundingBox(Point(9, 8), Point(11, 10)))
        self.assertEqual(pack.getValue("box.bottomRight"), Point(11, 10))
        pack.setValue("box.topLeft.x", -99)
        self.assertEqual(pack.getValue("box"), BoundingBox(Point(-99, 8), Point(11, 10)))
        self.assertEqual(pack.getValue("box.topLeft.x"), -99)

    def test_parameter_pack_getSetValue_updates(self):
        @parameterPack
        class ParameterPack:
            box: BoundingBox
            value: int

        @parameterNodeWrapper
        class ParameterNodeType:
            pack: ParameterPack

        param = ParameterNodeType(newParameterNode())

        param.pack.setValue("value", 123)

        self.assertEqual(param.pack.value, 123)

        param.pack.setValue("box.bottomRight.x", 33)

        self.assertEqual(param.pack.box.bottomRight.x, 33)

    def test_parameter_pack_dataType(self):
        @parameterPack
        class AnnotatedSub:
            iterations: Annotated[int, Default(44)]

        @parameterPack
        class ParameterPack:
            box: BoundingBox
            value: int
            union: Union[int, str]
            annotated: Annotated[bool, Default(True)]
            annotatedBox: BoundingBox = BoundingBox(Point(-99, 8), Point(11, 10))
            annotatedSub: AnnotatedSub

        self.assertEqual(ParameterPack.dataType("box"), BoundingBox)
        self.assertEqual(ParameterPack.dataType("box.topLeft"), Point)
        self.assertEqual(ParameterPack.dataType("box.topLeft.x"), float)
        self.assertEqual(ParameterPack.dataType("value"), int)
        self.assertEqual(ParameterPack.dataType("union"), Union[int, str])
        self.assertEqual(ParameterPack.dataType("annotated"), Annotated[bool, Default(True)])
        self.assertEqual(ParameterPack.dataType("annotatedBox"), BoundingBox)
        self.assertEqual(ParameterPack.dataType("annotatedBox.topLeft"), Point)
        self.assertEqual(ParameterPack.dataType("annotatedSub"), AnnotatedSub)
        self.assertEqual(ParameterPack.dataType("annotatedSub.iterations"), Annotated[int, Default(44)])

        param = ParameterPack()
        self.assertEqual(param.dataType("box"), BoundingBox)
        self.assertEqual(param.dataType("box.topLeft"), Point)
        self.assertEqual(param.dataType("box.topLeft.x"), float)
        self.assertEqual(param.dataType("value"), int)
        self.assertEqual(param.dataType("union"), Union[int, str])
        self.assertEqual(param.dataType("annotated"), Annotated[bool, Default(True)])
        self.assertEqual(param.dataType("annotatedBox"), BoundingBox)
        self.assertEqual(param.dataType("annotatedBox.topLeft"), Point)
        self.assertEqual(param.dataType("annotatedSub"), AnnotatedSub)
        self.assertEqual(param.dataType("annotatedSub.iterations"), Annotated[int, Default(44)])

    def test_parameter_pack_any(self):
        @parameterPack
        class AnyPack:
            any: Any

        param = AnyPack()
        self.assertIsNone(param.any)

        param.any = 1
        self.assertEqual(param.any, 1)
        param.any = "some string"
        self.assertEqual(param.any, "some string")

        # add weird recursive use
        param.any = AnyPack(Point(3, 4))
        self.assertEqual(param.any.any, Point(3, 4))

    def test_parameter_pack_invariants(self):
        self.assertEqual(Date(), Date(1, 1, 1970))
        with self.assertRaises(BadDateException):
            Date(0, 1, 1970)
        with self.assertRaises(BadDateException):
            Date(13, 1, 1970)
        with self.assertRaises(BadDateException):
            Date(1, 0, 1970)
        with self.assertRaises(BadDateException):
            Date(1, 32, 1970)
        with self.assertRaises(BadDateException):
            Date(2, 29, 1970)

        date = Date()
        with self.assertRaises(BadDateException):
            date.month = 0
        self.assertEqual(date.month, 1)
        date = Date()
        with self.assertRaises(BadDateException):
            date.month = 55
        self.assertEqual(date.month, 1)
        date.month = 2
        with self.assertRaises(BadDateException):
            date.day = 30

        date.setValue("month", 4)
        with self.assertRaises(BadDateException):
            date.setValue("day", 31)

        date.setDate(1, 31, 1980)
        self.assertEqual(date, Date(1, 31, 1980))

        with self.assertRaises(BadDateException):
            date.month = 44

    def test_parameter_pack_invariants_in_wrapper(self):
        @parameterNodeWrapper
        class ParameterNodeType:
            date: Date
            date2: Date = Date(2, 28, 2028)

        param = ParameterNodeType(newParameterNode())
        self.assertEqual(param.date, Date())
        self.assertEqual(param.date2, Date(2, 28, 2028))

        with self.assertRaises(BadDateException):
            param.date.month = 0
        self.assertEqual(param.date.month, 1)
        with self.assertRaises(BadDateException):
            param.date.month = 55
        self.assertEqual(param.date.month, 1)
        param.date.month = 2
        with self.assertRaises(BadDateException):
            param.date.day = 30

        param.date.setDate(1, 31, 1980)
        self.assertEqual(param.date, Date(1, 31, 1980))

        with self.assertRaises(BadDateException):
            param.date.month = 44

        self.assertEqual(param.date, Date(1, 31, 1980))
        self.assertEqual(param.date2, Date(2, 28, 2028))

        param2 = ParameterNodeType(param.parameterNode)
        self.assertEqual(param2.date, Date(1, 31, 1980))
        self.assertEqual(param2.date2, Date(2, 28, 2028))

    def test_equals_default(self):
        @parameterPack
        class Pack:
            i: int = 50
            j: Annotated[int, Default(60)]

        pack = Pack()
        self.assertEqual(pack.i, 50)
        self.assertEqual(pack.j, 60)
