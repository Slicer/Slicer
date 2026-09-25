"""Tests for the interactive Matplotlib backend built on PythonQt.

The tests are skipped when a supported matplotlib version is not installed, since it is
not part of the Slicer distribution.
"""

import gc
import unittest
import weakref

import qt
import slicer

try:
    import matplotlib

    # Raises ImportError if the installed matplotlib is too old for the backend.
    import slicer.matplotlibbackend  # noqa: F401

    matplotlib.use("module://slicer.matplotlibbackend", force=True)
    import matplotlib.pyplot as plt
    import numpy as np
    from matplotlib.backend_bases import MouseButton

    MATPLOTLIB_AVAILABLE = True
except ImportError:
    MATPLOTLIB_AVAILABLE = False


def _send(widget, event):
    qt.QCoreApplication.sendEvent(widget, event)
    slicer.app.processEvents()


def _mouse(kind, x, y, button=None, buttons=None):
    button = qt.Qt.NoButton if button is None else button
    buttons = qt.Qt.NoButton if buttons is None else buttons
    return qt.QMouseEvent(kind, qt.QPointF(x, y), button, buttons, qt.Qt.NoModifier)


def _display_to_widget(canvas, point):
    """Convert matplotlib display coordinates (physical pixels, origin at the bottom)
    to Qt widget coordinates (logical pixels, origin at the top).
    """
    ratio = canvas.device_pixel_ratio
    x, y = point
    return x / ratio, canvas.get_width_height()[1] - y / ratio


@unittest.skipUnless(MATPLOTLIB_AVAILABLE, "matplotlib >= 3.10 is not installed")
class MatplotlibBackendTest(unittest.TestCase):
    def setUp(self):
        matplotlib.use("module://slicer.matplotlibbackend", force=True)
        self.figure, self.axes = plt.subplots(figsize=(5, 4), dpi=100)
        t = np.linspace(0, 5, 200)
        self.axes.plot(t, np.cos(2 * np.pi * t) * np.exp(-t))
        self.canvas = self.figure.canvas
        self.canvas.manager.show()
        slicer.app.processEvents()

    def tearDown(self):
        plt.close("all")
        slicer.app.processEvents()

    def test_backend_is_selected(self):
        """The backend can be selected through matplotlib.use()."""
        self.assertEqual(matplotlib.get_backend(), "module://slicer.matplotlibbackend")

    def test_canvas_provides_qt_widget(self):
        """The canvas exposes a QWidget that can be embedded in Slicer layouts."""
        self.assertIsInstance(self.canvas.get_widget(), qt.QWidget)
        self.assertIs(self.canvas.get_widget(), self.canvas.widget)

    def test_rendering_reaches_the_widget(self):
        """Drawing the figure produces a non-empty image in the widget."""
        self.canvas.draw()
        slicer.app.processEvents()

        image = self.canvas._qimage()
        self.assertIsNotNone(image)
        self.assertFalse(image.isNull())

        width, height = self.canvas.get_width_height(physical=True)
        self.assertEqual(image.width(), width)
        self.assertEqual(image.height(), height)

        # The figure contains a plotted curve, so some pixels are not white.
        buffer = np.asarray(self.canvas.buffer_rgba())
        self.assertGreater(int((buffer[..., :3] < 250).any(axis=-1).sum()), 0)

    def test_toolbar_is_created(self):
        """The manager builds a navigation toolbar backed by a QToolBar."""
        toolbar = self.canvas.manager.toolbar
        self.assertIsNotNone(toolbar)
        self.assertIsInstance(toolbar.get_widget(), qt.QToolBar)

    def test_qt_events_are_forwarded_to_matplotlib(self):
        """Qt input events are translated into matplotlib events."""
        received = []
        self.canvas.mpl_connect("button_press_event", lambda e: received.append(("press", e.button)))
        self.canvas.mpl_connect("motion_notify_event", lambda e: received.append("motion"))
        self.canvas.mpl_connect("scroll_event", lambda e: received.append(("scroll", e.step)))
        self.canvas.mpl_connect("key_press_event", lambda e: received.append(("key", e.key)))

        widget = self.canvas.get_widget()
        widget.setFocus()
        slicer.app.processEvents()

        _send(widget, _mouse(qt.QEvent.MouseButtonPress, 250, 200,
                             qt.Qt.LeftButton, qt.Qt.LeftButton))
        _send(widget, _mouse(qt.QEvent.MouseMove, 260, 210))
        _send(widget, qt.QWheelEvent(
            qt.QPointF(250, 200), qt.QPointF(250, 200), qt.QPoint(0, 0),
            qt.QPoint(0, 120), qt.Qt.NoButton, qt.Qt.NoModifier,
            qt.Qt.NoScrollPhase, False))
        _send(widget, qt.QKeyEvent(qt.QEvent.KeyPress, qt.Qt.Key_G, qt.Qt.NoModifier, "g"))

        self.assertIn(("press", MouseButton.LEFT), received)
        self.assertIn("motion", received)
        self.assertIn(("scroll", 1.0), received)
        self.assertIn(("key", "g"), received)

    def test_zoom_and_home(self):
        """Dragging with the zoom tool changes the view and Home restores it."""
        widget = self.canvas.get_widget()
        toolbar = self.canvas.manager.toolbar
        original = self.axes.get_xlim()

        toolbar.zoom()
        _send(widget, _mouse(qt.QEvent.MouseButtonPress, 120, 120,
                             qt.Qt.LeftButton, qt.Qt.LeftButton))
        _send(widget, _mouse(qt.QEvent.MouseMove, 300, 250, qt.Qt.NoButton, qt.Qt.LeftButton))
        _send(widget, _mouse(qt.QEvent.MouseButtonRelease, 300, 250,
                             qt.Qt.LeftButton, qt.Qt.NoButton))
        toolbar.zoom()

        self.assertNotEqual(self.axes.get_xlim(), original)

        toolbar.home()
        slicer.app.processEvents()
        self.assertEqual(self.axes.get_xlim(), original)

    def test_pan(self):
        """Dragging with the pan tool shifts the view."""
        widget = self.canvas.get_widget()
        toolbar = self.canvas.manager.toolbar
        original = self.axes.get_xlim()

        toolbar.pan()
        _send(widget, _mouse(qt.QEvent.MouseButtonPress, 250, 200,
                             qt.Qt.LeftButton, qt.Qt.LeftButton))
        _send(widget, _mouse(qt.QEvent.MouseMove, 150, 200, qt.Qt.NoButton, qt.Qt.LeftButton))
        _send(widget, _mouse(qt.QEvent.MouseButtonRelease, 150, 200,
                             qt.Qt.LeftButton, qt.Qt.NoButton))
        toolbar.pan()

        self.assertNotEqual(self.axes.get_xlim(), original)

    def test_picking(self):
        """Clicking on a picker-enabled artist emits a pick event."""
        picked = []
        figure, axes = plt.subplots(figsize=(4, 3))
        axes.plot([0, 1, 2, 3], [0, 1, 0, 1], "o-", picker=10)
        figure.canvas.mpl_connect("pick_event", lambda e: picked.append(list(e.ind)))
        figure.canvas.manager.show()
        figure.canvas.draw()
        slicer.app.processEvents()

        x, y = _display_to_widget(figure.canvas, axes.transData.transform((0, 0)))
        _send(figure.canvas.get_widget(),
              _mouse(qt.QEvent.MouseButtonPress, x, y,
                     qt.Qt.LeftButton, qt.Qt.LeftButton))

        self.assertTrue(picked)

    def test_interactive_widget(self):
        """matplotlib.widgets receive the forwarded events."""
        from matplotlib.widgets import Slider

        changed = []
        figure, _ = plt.subplots(figsize=(4, 3))
        figure.subplots_adjust(bottom=0.3)
        slider_axes = figure.add_axes([0.2, 0.1, 0.6, 0.05])
        slider = Slider(slider_axes, "gain", 0.0, 10.0, valinit=1.0)
        slider.on_changed(changed.append)
        figure.canvas.manager.show()
        figure.canvas.draw()
        slicer.app.processEvents()

        x, y = _display_to_widget(figure.canvas, slider_axes.transData.transform((5.0, 0.5)))
        widget = figure.canvas.get_widget()
        _send(widget, _mouse(qt.QEvent.MouseButtonPress, x, y,
                             qt.Qt.LeftButton, qt.Qt.LeftButton))
        _send(widget, _mouse(qt.QEvent.MouseButtonRelease, x, y,
                             qt.Qt.LeftButton, qt.Qt.NoButton))

        self.assertTrue(changed)
        self.assertAlmostEqual(slider.val, 5.0, places=1)

    def test_timer_drives_callbacks(self):
        """Canvas timers are driven by the Qt event loop, which animations rely on."""
        import time

        ticks = []
        timer = self.canvas.new_timer(interval=10)
        timer.add_callback(lambda: ticks.append(1))
        timer.start()
        for _ in range(60):
            slicer.app.processEvents()
            time.sleep(0.005)
        timer.stop()

        self.assertTrue(ticks)

    def test_unreferenced_canvas_is_released(self):
        """Signal connections made by the canvas, its timers and its toolbar do not keep
        it alive after the last Python reference is dropped.
        """
        from matplotlib.figure import Figure

        from slicer.matplotlibbackend import FigureCanvasSlicer, NavigationToolbar2Slicer

        canvas = FigureCanvasSlicer(Figure())
        canvas.new_timer(interval=10)
        toolbar = NavigationToolbar2Slicer(canvas)
        canvas_ref = weakref.ref(canvas)
        del canvas, toolbar
        for _ in range(3):
            slicer.app.processEvents()
            gc.collect()

        self.assertIsNone(canvas_ref())

    def test_resizing_reflows_figure(self):
        """Resizing the widget updates the figure size."""
        self.canvas.get_widget().resize(700, 500)
        slicer.app.processEvents()

        ratio = self.canvas.device_pixel_ratio
        dpi = self.figure.dpi
        self.assertAlmostEqual(self.figure.get_size_inches()[0], 700 * ratio / dpi, places=3)
        self.assertAlmostEqual(self.figure.get_size_inches()[1], 500 * ratio / dpi, places=3)

    def test_size_hint_override(self):
        """The preferred size reported to Qt can be overridden and restored."""
        widget = self.canvas.get_widget()
        # sizeHint/minimumSizeHint are overridden in Python, so they are called.
        self.assertEqual(widget.sizeHint().width(), self.canvas.get_width_height()[0])

        self.canvas.set_size_hint(120, 90)
        self.assertEqual(widget.sizeHint().width(), 120)
        self.assertEqual(widget.sizeHint().height(), 90)
        # A small hint must not prevent the widget from growing.
        self.assertLessEqual(widget.minimumSizeHint().height(), 90)

        self.canvas.set_size_hint()
        self.assertEqual(widget.sizeHint().width(), self.canvas.get_width_height()[0])

    def test_saving_figure(self):
        """Rendering to a file still works while the interactive backend is active."""
        import os
        import tempfile

        path = os.path.join(tempfile.gettempdir(), "slicer_matplotlibbackend_test.png")
        try:
            self.figure.savefig(path)
            self.assertGreater(os.path.getsize(path), 0)
        finally:
            if os.path.exists(path):
                os.remove(path)

    def test_closing_figure_emits_close_event(self):
        """Closing a figure emits close_event once and unregisters the figure."""
        closed = []
        self.canvas.mpl_connect("close_event", lambda e: closed.append(1))
        number = self.figure.number

        plt.close(self.figure)
        slicer.app.processEvents()

        self.assertEqual(len(closed), 1)
        self.assertNotIn(number, plt.get_fignums())

    def test_embedding_in_a_layout(self):
        """The canvas and toolbar can be placed in an ordinary Qt layout."""
        from matplotlib.figure import Figure

        from slicer.matplotlibbackend import FigureCanvasSlicer, NavigationToolbar2Slicer

        container = qt.QWidget()
        layout = qt.QVBoxLayout(container)
        figure = Figure(figsize=(4, 3))
        figure.add_subplot(111).plot([0, 1, 2], [2, 0, 1])
        canvas = FigureCanvasSlicer(figure)
        toolbar = NavigationToolbar2Slicer(canvas, container)
        layout.addWidget(canvas.get_widget())
        layout.addWidget(toolbar.get_widget())
        container.resize(420, 360)
        container.show()
        slicer.app.processEvents()
        canvas.draw()
        slicer.app.processEvents()

        self.assertFalse(canvas.get_widget().grab().toImage().isNull())
        container.hide()


if __name__ == "__main__":
    unittest.main()
