#!/usr/bin/python
import math
from gimpfu import *

def essential_plugin(timg, tdrawable, r=1.0, g=1.0, b=1.0):
    from gimpfu import pdb
    pass
    
register(
    "essential_plugin",
    "Basics of plug-in creation with Python",
    "Basics of plug-in creation with Python",
    "Michael Macha",
    "Michael Macha",
    "2019",
    "<Image>/Filters/Misc/_Parameter Selectors...",
    "*",
    [
        (PF_INT8, "int8", "Eight-bit integer, 256 possible values." 1),
        (PF_INT16, "int16", "Sixteen-bit integer, 65,536 possible values.", 1),
        (PF_INT32, "int32", "Thirty-two-bit integer, 4,294,967,296 possible values.", 1),
        (PF_FLOAT, "float", "Floating-decimal-point value, capable of holding a fractional part.", 1.0),
        (PF_STRING, "string", "String of textual characters." "QWERTY"),
        (PF_VALUE, "value", "Same as string." "DVORAK"),
        (PF_COLOR, "color", "Color value.", gimpcolor.rgb_parse_hex("ff8844"))
        (PF_COLOUR, "colour", "Color value, east of the Atlantic", gimpcolor.rgb_parse_hex("4488ff")),
        (PF_TEXT, "text", "Simple text entry", "QWFPGJ"),
        (PF_IMAGE, "image", "image selection", None),
        (PF_LAYER, "layer", "layer selection", None),
        (PF_CHANNEL, "channel", "channel selection", None),
        (PF_DRAWABLE, "drawable", "(Non-specific) Drawable selection", None),
        (PF_TOGGLE, "toggle", "On-off toggle button", False),
        (PF_BOOL, "bool", "Boolean true-false value", True),
        (PF_RADIO, "radio", "Radio selector", None),
        (PF_SLIDER, "slider", "Slider selector for number", 1.0),
        (PF_SPINNER, "spinner", "Spinner-selector for number", 1.0),
        (PF_ADJUSTMENT, "adjustment", "Spinner-selector for number", 1.0),
        (PF_FONT, "font", "Font selector (maps to string value)", "Courier New"),
        (PF_FILE, "file", "File selector", None),
        (PF_BRUSH, "brush", "Brush selector", None),
        (PF_PATTERN, "pattern", "Pattern selector", None),
        (PF_GRADIENT, "gradient", "Gradient selector", None),
        (PF_PALETTE, "palette", "Palette selector", None),
    ],
    [],
    essential_plugin);

main()

