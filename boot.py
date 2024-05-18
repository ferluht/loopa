#!/usr/bin/env python

import subprocess
from gpiozero import LED

RST = LED(26, initial_value=True)
RST.on()

# std_font = {
#     "A": bytearray(b"\x05\x07?DDD?"),
#     "B": bytearray(b"\x05\x07\x7fAII6"),
#     "C": bytearray(b'\x05\x07>AAA"'),
#     "D": bytearray(b'\x05\x07\x7fAA"\x1c'),
#     "E": bytearray(b"\x05\x07\x7fIIIA"),
#     "F": bytearray(b"\x05\x07\x7fHH@@"),
#     "G": bytearray(b"\x05\x07>AII."),
#     "H": bytearray(b"\x05\x07\x7f\x08\x08\x08\x7f"),
#     "I": bytearray(b"\x05\x07AA\x7fAA"),
#     "J": bytearray(b"\x05\x07FA~@@"),
#     "K": bytearray(b"\x05\x07\x7f\x08\x08t\x03"),
#     "L": bytearray(b"\x05\x07\x7f\x01\x01\x01\x01"),
#     "M": bytearray(b"\x05\x07\x7f \x10 \x7f"),
#     "N": bytearray(b"\x05\x07\x7f \x1c\x02\x7f"),
#     "O": bytearray(b"\x05\x07>AAA>"),
#     "P": bytearray(b"\x05\x07\x7fHHH0"),
#     "Q": bytearray(b"\x05\x07>AEB="),
#     "R": bytearray(b"\x05\x07\x7fHLJ1"),
#     "S": bytearray(b"\x05\x072III&"),
#     "T": bytearray(b"\x05\x07@@\x7f@@"),
#     "U": bytearray(b"\x05\x07~\x01\x01\x01~"),
#     "V": bytearray(b"\x05\x07p\x0e\x01\x0ep"),
#     "W": bytearray(b"\x05\x07|\x03\x04\x03|"),
#     "X": bytearray(b"\x05\x07c\x14\x08\x14c"),
#     "Y": bytearray(b"\x05\x07`\x10\x0f\x10`"),
#     "Z": bytearray(b"\x05\x07CEIQa"),
#     "0": bytearray(b"\x05\x07>EIQ>"),
#     "1": bytearray(b"\x05\x07\x11!\x7f\x01\x01"),
#     "2": bytearray(b"\x05\x07!CEI1"),
#     "3": bytearray(b"\x05\x07FAQiF"),
#     "4": bytearray(b"\x05\x07x\x08\x08\x08\x7f"),
#     "5": bytearray(b"\x05\x07rQQQN"),
#     "6": bytearray(b"\x05\x07\x1e)II\x06"),
#     "7": bytearray(b"\x05\x07@GHP`"),
#     "8": bytearray(b"\x05\x076III6"),
#     "9": bytearray(b"\x05\x070IIJ<"),
#     ")": bytearray(b"\x05\x07\x00A>\x00\x00"),
#     "(": bytearray(b"\x05\x07\x00\x00>A\x00"),
#     "[": bytearray(b"\x05\x07\x00\x00\x7fA\x00"),
#     "]": bytearray(b"\x05\x07\x00A\x7f\x00\x00"),
#     ".": bytearray(b"\x05\x07\x00\x03\x03\x00\x00"),
#     "'": bytearray(b"\x05\x07\x00\x000\x00\x00"),
#     ":": bytearray(b"\x05\x07\x00\x0066\x00"),
#     "?CHAR?": bytearray(b"\x05\x07\x7f_RG\x7f"),
#     "!": bytearray(b"\x05\x07\x00{{\x00\x00"),
#     "?": bytearray(b"\x05\x07 @EH0"),
#     ",": bytearray(b"\x05\x07\x00\x05\x06\x00\x00"),
#     ";": bytearray(b"\x05\x07\x0056\x00\x00"),
#     "/": bytearray(b"\x05\x07\x01\x06\x080@"),
#     ">": bytearray(b"\x05\x07Ac6\x1c\x08"),
#     "<": bytearray(b"\x05\x07\x08\x1c6cA"),
#     "%": bytearray(b"\x05\x07af\x083C"),
#     "@": bytearray(b"\x05\x07&IOA>"),
#     "#": bytearray(b"\x05\x07\x14\x7f\x14\x7f\x14"),
#     "$": bytearray(b"\x05\x072I\x7fI&"),
#     "&": bytearray(b'\x05\x076IU"\x05'),
#     "*": bytearray(b"\x05\x07(\x10|\x10("),
#     "-": bytearray(b"\x05\x07\x00\x08\x08\x08\x00"),
#     "_": bytearray(b"\x05\x07\x01\x01\x01\x01\x01"),
#     "+": bytearray(b"\x05\x07\x08\x08>\x08\x08"),
#     "=": bytearray(b"\x05\x07\x00\x14\x14\x14\x00"),
#     '"': bytearray(b"\x05\x07\x00p\x00p\x00"),
#     "`": bytearray(b"\x05\x07\x00\x00 \x10\x00"),
#     "~": bytearray(b"\x05\x07\x08\x10\x08\x04\x08"),
#     " ": bytearray(b"\x05\x07\x00\x00\x00\x00\x00"),
#     "^": bytearray(b"\x05\x07\x10 @ \x10"),
#     "NONE": bytearray(b"\x00\x07"),
#     "BLANK": bytearray(b"\x05\x07\x00\x00\x00\x00\x00"),
#     "BATA0": bytearray(b"\x0b\x07\x7fAAAAAAAA\x7f\x1c"),
#     "BATA1": bytearray(b"\x0b\x07\x7fA]AAAAAA\x7f\x1c"),
#     "BATA2": bytearray(b"\x0b\x07\x7fA]]AAAAA\x7f\x1c"),
#     "BATA3": bytearray(b"\x0b\x07\x7fA]]]AAAA\x7f\x1c"),
#     "BATA4": bytearray(b"\x0b\x07\x7fA]]]]AAA\x7f\x1c"),
#     "BATA5": bytearray(b"\x0b\x07\x7fA]]]]]AA\x7f\x1c"),
#     "BATA6": bytearray(b"\x0b\x07\x7fA]]]]]]A\x7f\x1c"),
#     "BATACHRG": bytearray(b"\x07\x08\x7fAIYyOMIA\x7f\x1c"),
#     "BATB0": bytearray(b"\x0b\x07\x7fAAAAAAAA\x7f\x1c"),
#     "FULL": bytearray(b"\x05\x07\x7f\x7f\x7f\x7f\x7f"),
#     "\n": bytearray(b"\x05\x07\x00\x00\x00\x00\x00"),
#     "DEGREESIGN": bytearray(b"\x05\x07\x18$$\x18\x00"),
# }
#
#
# # pylint: disable=invalid-name
# class GFX:
#     # pylint: disable=too-many-instance-attributes
#     """Create an instance of the GFX drawing class.
#
#     :param width: The width of the drawing area in pixels.
#     :param height: The height of the drawing area in pixels.
#     :param pixel: A function to call when a pixel is drawn on the display. This function
#                   should take at least an x and y position and then any number of optional
#                   color or other parameters.
#     :param hline: A function to quickly draw a horizontal line on the display.
#                   This should take at least an x, y, and width parameter and
#                   any number of optional color or other parameters.
#     :param vline: A function to quickly draw a vertical line on the display.
#                   This should take at least an x, y, and height paraemter and
#                   any number of optional color or other parameters.
#     :param fill_rect: A function to quickly draw a solid rectangle with four
#                   input parameters: x,y, width, and height. Any number of other
#                   parameters for color or screen specific data.
#     :param text: A function to quickly place text on the screen. The inputs include:
#                   x, y data(top left as starting point).
#     :param font: An optional input to augment the default text method with a new font.
#                   The input should be a properly formatted dict.
#     """
#
#     # pylint: disable=too-many-arguments
#     def __init__(
#             self,
#             width,
#             height,
#             pixel,
#             hline=None,
#             vline=None,
#             fill_rect=None,
#             text=None,
#             font=None,
#     ):
#         # pylint: disable=too-many-instance-attributes
#         self.width = width
#         self.height = height
#         self._pixel = pixel
#         # Default to slow horizontal & vertical line implementations if no
#         # faster versions are provided.
#         if hline is None:
#             self.hline = self._slow_hline
#         else:
#             self.hline = hline
#         if vline is None:
#             self.vline = self._slow_vline
#         else:
#             self.vline = vline
#         if fill_rect is None:
#             self.fill_rect = self._fill_rect
#         else:
#             self.fill_rect = fill_rect
#         if text is None:
#             self.text = self._very_slow_text
#             # if no supplied font set to std
#             if font is None:
#                 self.font = std_font
#                 self.set_text_background()
#             else:
#                 self.font = font
#                 if not isinstance(self.font, dict):
#                     raise ValueError(
#                         "Font definitions must be contained in a dictionary object."
#                     )
#                 del self.set_text_background
#
#         else:
#             self.text = text
#
#     def pixel(self, x0, y0, *args, **kwargs):
#         """A function to pass through in input pixel functionality."""
#         # This was added to mainitatn the abstraction between gfx and the dislay library
#         self._pixel(x0, y0, *args, **kwargs)
#
#     def _slow_hline(self, x0, y0, width, *args, **kwargs):
#         """Slow implementation of a horizontal line using pixel drawing.
#         This is used as the default horizontal line if no faster override
#         is provided."""
#         if y0 < 0 or y0 > self.height or x0 < -width or x0 > self.width:
#             return
#         for i in range(width):
#             self._pixel(x0 + i, y0, *args, **kwargs)
#
#     def _slow_vline(self, x0, y0, height, *args, **kwargs):
#         """Slow implementation of a vertical line using pixel drawing.
#         This is used as the default vertical line if no faster override
#         is provided."""
#         if y0 < -height or y0 > self.height or x0 < 0 or x0 > self.width:
#             return
#         for i in range(height):
#             self._pixel(x0, y0 + i, *args, **kwargs)
#
#     def rect(self, x0, y0, width, height, *args, **kwargs):
#         """Rectangle drawing function.  Will draw a single pixel wide rectangle
#         starting in the upper left x0, y0 position and width, height pixels in
#         size."""
#         if y0 < -height or y0 > self.height or x0 < -width or x0 > self.width:
#             return
#         self.hline(x0, y0, width, *args, **kwargs)
#         self.hline(x0, y0 + height - 1, width, *args, **kwargs)
#         self.vline(x0, y0, height, *args, **kwargs)
#         self.vline(x0 + width - 1, y0, height, *args, **kwargs)
#
#     def _fill_rect(self, x0, y0, width, height, *args, **kwargs):
#         """Filled rectangle drawing function.  Will draw a single pixel wide
#         rectangle starting in the upper left x0, y0 position and width, height
#         pixels in size."""
#         if y0 < -height or y0 > self.height or x0 < -width or x0 > self.width:
#             return
#         for i in range(x0, x0 + width):
#             self.vline(i, y0, height, *args, **kwargs)
#
#     def line(self, x0, y0, x1, y1, *args, **kwargs):
#         """Line drawing function.  Will draw a single pixel wide line starting at
#         x0, y0 and ending at x1, y1."""
#         steep = abs(y1 - y0) > abs(x1 - x0)
#         if steep:
#             x0, y0 = y0, x0
#             x1, y1 = y1, x1
#         if x0 > x1:
#             x0, x1 = x1, x0
#             y0, y1 = y1, y0
#         dx = x1 - x0
#         dy = abs(y1 - y0)
#         err = dx // 2
#         ystep = 0
#         if y0 < y1:
#             ystep = 1
#         else:
#             ystep = -1
#         while x0 <= x1:
#             if steep:
#                 self._pixel(y0, x0, *args, **kwargs)
#             else:
#                 self._pixel(x0, y0, *args, **kwargs)
#             err -= dy
#             if err < 0:
#                 y0 += ystep
#                 err += dx
#             x0 += 1
#
#     def circle(self, x0, y0, radius, *args, **kwargs):
#         """Circle drawing function.  Will draw a single pixel wide circle with
#         center at x0, y0 and the specified radius."""
#         f = 1 - radius
#         ddF_x = 1
#         ddF_y = -2 * radius
#         x = 0
#         y = radius
#         self._pixel(x0, y0 + radius, *args, **kwargs)  # bottom
#         self._pixel(x0, y0 - radius, *args, **kwargs)  # top
#         self._pixel(x0 + radius, y0, *args, **kwargs)  # right
#         self._pixel(x0 - radius, y0, *args, **kwargs)  # left
#         while x < y:
#             if f >= 0:
#                 y -= 1
#                 ddF_y += 2
#                 f += ddF_y
#             x += 1
#             ddF_x += 2
#             f += ddF_x
#             # angle notations are based on the unit circle and in diection of being drawn
#             self._pixel(x0 + x, y0 + y, *args, **kwargs)  # 270 to 315
#             self._pixel(x0 - x, y0 + y, *args, **kwargs)  # 270 to 255
#             self._pixel(x0 + x, y0 - y, *args, **kwargs)  # 90 to 45
#             self._pixel(x0 - x, y0 - y, *args, **kwargs)  # 90 to 135
#             self._pixel(x0 + y, y0 + x, *args, **kwargs)  # 0 to 315
#             self._pixel(x0 - y, y0 + x, *args, **kwargs)  # 180 to 225
#             self._pixel(x0 + y, y0 - x, *args, **kwargs)  # 0 to 45
#             self._pixel(x0 - y, y0 - x, *args, **kwargs)  # 180 to 135
#
#     def fill_circle(self, x0, y0, radius, *args, **kwargs):
#         """Filled circle drawing function.  Will draw a filled circle with
#         center at x0, y0 and the specified radius."""
#         self.vline(x0, y0 - radius, 2 * radius + 1, *args, **kwargs)
#         f = 1 - radius
#         ddF_x = 1
#         ddF_y = -2 * radius
#         x = 0
#         y = radius
#         while x < y:
#             if f >= 0:
#                 y -= 1
#                 ddF_y += 2
#                 f += ddF_y
#             x += 1
#             ddF_x += 2
#             f += ddF_x
#             self.vline(x0 + x, y0 - y, 2 * y + 1, *args, **kwargs)
#             self.vline(x0 + y, y0 - x, 2 * x + 1, *args, **kwargs)
#             self.vline(x0 - x, y0 - y, 2 * y + 1, *args, **kwargs)
#             self.vline(x0 - y, y0 - x, 2 * x + 1, *args, **kwargs)
#
#     def triangle(self, x0, y0, x1, y1, x2, y2, *args, **kwargs):
#         # pylint: disable=too-many-arguments
#         """Triangle drawing function.  Will draw a single pixel wide triangle
#         around the points (x0, y0), (x1, y1), and (x2, y2)."""
#         self.line(x0, y0, x1, y1, *args, **kwargs)
#         self.line(x1, y1, x2, y2, *args, **kwargs)
#         self.line(x2, y2, x0, y0, *args, **kwargs)
#
#     def fill_triangle(self, x0, y0, x1, y1, x2, y2, *args, **kwargs):
#         # pylint: disable=too-many-arguments, too-many-locals, too-many-statements, too-many-branches
#         """Filled triangle drawing function.  Will draw a filled triangle around
#         the points (x0, y0), (x1, y1), and (x2, y2)."""
#         if y0 > y1:
#             y0, y1 = y1, y0
#             x0, x1 = x1, x0
#         if y1 > y2:
#             y2, y1 = y1, y2
#             x2, x1 = x1, x2
#         if y0 > y1:
#             y0, y1 = y1, y0
#             x0, x1 = x1, x0
#         a = 0
#         b = 0
#         last = 0
#         if y0 == y2:
#             a = x0
#             b = x0
#             if x1 < a:
#                 a = x1
#             elif x1 > b:
#                 b = x1
#             if x2 < a:
#                 a = x2
#             elif x2 > b:
#                 b = x2
#             self.hline(a, y0, b - a + 1, *args, **kwargs)
#             return
#         dx01 = x1 - x0
#         dy01 = y1 - y0
#         dx02 = x2 - x0
#         dy02 = y2 - y0
#         dx12 = x2 - x1
#         dy12 = y2 - y1
#         if dy01 == 0:
#             dy01 = 1
#         if dy02 == 0:
#             dy02 = 1
#         if dy12 == 0:
#             dy12 = 1
#         sa = 0
#         sb = 0
#         y = y0
#         if y0 == y1:
#             last = y1 - 1
#         else:
#             last = y1
#         while y <= last:
#             a = x0 + sa // dy01
#             b = x0 + sb // dy02
#             sa += dx01
#             sb += dx02
#             if a > b:
#                 a, b = b, a
#             self.hline(a, y, b - a + 1, *args, **kwargs)
#             y += 1
#         sa = dx12 * (y - y1)
#         sb = dx02 * (y - y0)
#         while y <= y2:
#             a = x1 + sa // dy12
#             b = x0 + sb // dy02
#             sa += dx12
#             sb += dx02
#             if a > b:
#                 a, b = b, a
#             self.hline(a, y, b - a + 1, *args, **kwargs)
#             y += 1
#
#     def round_rect(self, x0, y0, width, height, radius, *args, **kwargs):
#         """Rectangle with rounded corners drawing function.
#         This works like a regular rect though! if radius = 0
#         Will draw the outline of a rectangle with rounded corners with (x0,y0) at the top left
#         """
#         # shift to correct for start point location
#         x0 += radius
#         y0 += radius
#
#         # ensure that the radius will only ever half of the shortest side or less
#         radius = int(min(radius, width / 2, height / 2))
#
#         if radius:
#             f = 1 - radius
#             ddF_x = 1
#             ddF_y = -2 * radius
#             x = 0
#             y = radius
#             self.vline(
#                 x0 - radius, y0, height - 2 * radius + 1, *args, **kwargs
#             )  # left
#             self.vline(
#                 x0 + width - radius, y0, height - 2 * radius + 1, *args, **kwargs
#             )  # right
#             self.hline(
#                 x0, y0 + height - radius + 1, width - 2 * radius + 1, *args, **kwargs
#             )  # bottom
#             self.hline(x0, y0 - radius, width - 2 * radius + 1, *args, **kwargs)  # top
#             while x < y:
#                 if f >= 0:
#                     y -= 1
#                     ddF_y += 2
#                     f += ddF_y
#                 x += 1
#                 ddF_x += 2
#                 f += ddF_x
#                 # angle notations are based on the unit circle and in diection of being drawn
#
#                 # top left
#                 self._pixel(x0 - y, y0 - x, *args, **kwargs)  # 180 to 135
#                 self._pixel(x0 - x, y0 - y, *args, **kwargs)  # 90 to 135
#                 # top right
#                 self._pixel(
#                     x0 + x + width - 2 * radius, y0 - y, *args, **kwargs
#                 )  # 90 to 45
#                 self._pixel(
#                     x0 + y + width - 2 * radius, y0 - x, *args, **kwargs
#                 )  # 0 to 45
#                 # bottom right
#                 self._pixel(
#                     x0 + y + width - 2 * radius,
#                     y0 + x + height - 2 * radius,
#                     *args,
#                     **kwargs,
#                     )  # 0 to 315
#                 self._pixel(
#                     x0 + x + width - 2 * radius,
#                     y0 + y + height - 2 * radius,
#                     *args,
#                     **kwargs,
#                     )  # 270 to 315
#                 # bottom left
#                 self._pixel(
#                     x0 - x, y0 + y + height - 2 * radius, *args, **kwargs
#                 )  # 270 to 255
#                 self._pixel(
#                     x0 - y, y0 + x + height - 2 * radius, *args, **kwargs
#                 )  # 180 to 225
#
#     def fill_round_rect(self, x0, y0, width, height, radius, *args, **kwargs):
#         """Filled circle drawing function.  Will draw a filled circle with
#         center at x0, y0 and the specified radius."""
#         # shift to correct for start point location
#         x0 += radius
#         y0 += radius
#
#         # ensure that the radius will only ever half of the shortest side or less
#         radius = int(min(radius, width / 2, height / 2))
#
#         self.fill_rect(
#             x0, y0 - radius, width - 2 * radius + 2, height + 2, *args, **kwargs
#         )
#
#         if radius:
#             f = 1 - radius
#             ddF_x = 1
#             ddF_y = -2 * radius
#             x = 0
#             y = radius
#             while x < y:
#                 if f >= 0:
#                     y -= 1
#                     ddF_y += 2
#                     f += ddF_y
#                 x += 1
#                 ddF_x += 2
#                 f += ddF_x
#                 # part notation starts with 0 on left and 1 on right, and direction is noted
#                 # top left
#                 self.vline(
#                     x0 - y, y0 - x, 2 * x + 1 + height - 2 * radius, *args, **kwargs
#                 )  # 0 to .25
#                 self.vline(
#                     x0 - x, y0 - y, 2 * y + 1 + height - 2 * radius, *args, **kwargs
#                 )  # .5 to .25
#                 # top right
#                 self.vline(
#                     x0 + x + width - 2 * radius,
#                     y0 - y,
#                     2 * y + 1 + height - 2 * radius,
#                     *args,
#                     **kwargs,
#                     )  # .5 to .75
#                 self.vline(
#                     x0 + y + width - 2 * radius,
#                     y0 - x,
#                     2 * x + 1 + height - 2 * radius,
#                     *args,
#                     **kwargs,
#                     )  # 1 to .75
#
#     def _place_char(self, x0, y0, char, size, *args, **kwargs):
#         """A sub class used for placing a single character on the screen"""
#         # pylint: disable=undefined-loop-variable
#         arr = self.font[char]
#         width = arr[0]
#         height = arr[1]
#         # extract the char section of the data
#         data = arr[2:]
#         for x in range(width):
#             for y in range(height):
#                 bit = bool(data[x] & 2**y)
#                 # char pixel
#                 if bit:
#                     self.fill_rect(
#                         size * x + x0,
#                         size * (height - y - 1) + y0,
#                         size,
#                         size,
#                         *args,
#                         **kwargs,
#                         )
#                 # else background pixel
#                 # else:
#                 #     try:
#                 #         self.fill_rect(
#                 #             size * x + x0,
#                 #             size * (height - y - 1) + y0,
#                 #             size,
#                 #             size,
#                 #             *self.text_bkgnd_args,
#                 #             **self.text_bkgnd_kwargs,
#                 #             )
#                 #     except TypeError:
#                 #         pass
#         del arr, width, height, data, x, y, x0, y0, char, size
#
#     def _very_slow_text(self, x0, y0, string, size, *args, **kwargs):
#         """A function to place text on the display.(temporary)
#         to use special characters put "__" on either side of the desired characters.
#         letter format:
#         {'character_here' : bytearray(b',WIDTH,HEIGHT,right-most-data,
#                                             more-bytes-here,left-most-data') ,}
#
#         (replace the "," with backslashes!!)
#         each byte:
#                         | lower most bit(lowest on display)
#                         V
#                  x0110100
#                   ^c
#                   | top most bit (highest on display)"""
#
#         x_roll = x0  # rolling x
#         y_roll = y0  # rolling y
#
#         # highest_height = 0#wrap
#         sep_string = string.split("__")
#
#         for chunk in sep_string:
#             # print(chunk)
#             try:
#                 self._place_char(x_roll, y_roll, chunk, size, *args, **kwargs)
#                 x_roll += size * self.font[chunk][0] + size
#                 # highest_height = max(highest_height, size*self.font[chunk][1] + 1) #wrap
#             except KeyError:
#                 while chunk:
#                     char = chunk[0]
#
#                     # make sure something is sent even if not in font dict
#                     try:
#                         self._place_char(x_roll, y_roll, char, size, *args, **kwargs)
#                     except KeyError:
#                         self._place_char(
#                             x_roll, y_roll, "?CHAR?", size, *args, **kwargs
#                         )
#                         char = "?CHAR?"
#
#                     x_roll += size * self.font[char][0]
#
#                     # gap between letters
#                     # try:
#                     #     self.fill_rect(
#                     #         x_roll,
#                     #         y_roll,
#                     #         size,
#                     #         size * self.font[char][1],
#                     #         *self.text_bkgnd_args,
#                     #         **self.text_bkgnd_kwargs,
#                     #         )
#                     # except TypeError:
#                     #     pass
#
#                     x_roll += size
#                     # highest_height = max(highest_height, size*self.font[char][1] + 1) #wrap
#                     chunk = chunk[1:]  # wrap
#                     # if (x_roll >= self.width) or (chunk[0:2] == """\n"""): #wrap
#                     # self._text(x0,y0+highest_height,"__".join(sep_string),size) #wrap
#                     # print(highest_height) #wrap
#
#     def set_text_background(self, *args, **kwargs):
#         """A function to change the background color of text, input any and all color params.
#         Run without any inputs to return to "clear" background
#         """
#         self.text_bkgnd_args = args
#         self.text_bkgnd_kwargs = kwargs
#
#     # pylint: enable=too-many-arguments
#
# import urllib.request
# import json
# import os
# import serial
# import numpy as np
# import time
#
# crc8Table = np.array([
#     0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
#     0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
#     0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
#     0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
#     0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
#     0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
#     0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
#     0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
#     0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
#     0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
#     0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
#     0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
#     0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
#     0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
#     0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
#     0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
#     0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
#     0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
#     0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
#     0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
#     0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
#     0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
#     0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
#     0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
#     0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
#     0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
#     0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
#     0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
#     0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
#     0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
#     0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
#     0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
# ], dtype=np.uint8)
#
#
# def getCrc(crc, data, length):
#     for i in range(length):
#         crc = crc8Table[crc ^ data[i]]
#     return crc
#
#
# def show_update_question(prt):
#     screen_data = np.zeros(512 + 18 + 1, dtype=np.uint8)
#     def pixel(x, y, *args, **kwargs):
#         n = x + y*128
#         screen_data[n // 8] |= 1 << (7 - (n % 8))
#     screen = GFX(128, 32, pixel)
#     screen.text(3, 1, "NEW SOFTWARE VERSION", 1)
#     screen.text(26, 15, "UPDATE?", 2)
#     screen_data[512 + 0] = 10
#     screen_data[512 + 2] = 1
#     screen_data[512 + 4] = 10
#     screen_data[512 + 5] = 1
#     screen_data[-1] = getCrc(0, screen_data, screen_data.shape[0] - 1)
#     prt.write(screen_data.tostring())
#
# def show_update_message(prt):
#     screen_data = np.zeros(512 + 18 + 1, dtype=np.uint8)
#     def pixel(x, y, *args, **kwargs):
#         n = x + y*128
#         screen_data[n // 8] |= 1 << (7 - (n % 8))
#     screen = GFX(128, 32, pixel)
#     screen.text(1, 8, "UPDATING...", 2)
#     screen_data[-1] = getCrc(0, screen_data, screen_data.shape[0] - 1)
#     prt.write(screen_data.tostring())
#
# def show_loading_message(prt):
#     screen_data = np.zeros(512 + 18 + 1, dtype=np.uint8)
#     def pixel(x, y, *args, **kwargs):
#         n = x + y*128
#         screen_data[n // 8] |= 1 << (7 - (n % 8))
#     screen = GFX(128, 32, pixel)
#     screen.text(5, 8, "LOADING...", 2)
#     screen_data[-1] = getCrc(0, screen_data, screen_data.shape[0] - 1)
#     prt.write(screen_data.tostring())
#
# def show_blank_screen(prt):
#     screen_data = np.zeros(512 + 18 + 1, dtype=np.uint8)
#     screen_data[-1] = getCrc(0, screen_data, screen_data.shape[0] - 1)
#     prt.write(screen_data.tostring())
#
#
# version_file = "version.txt"
#
# current_tag = None
# if os.path.exists(version_file):
#     file = open(version_file, "r")
#     current_tag = file.read()
# else:
#     popen = subprocess.Popen(["/home/pi/.arduino15/packages/arduino/tools/avrdude/6.3.0-arduino17/bin/avrdude",
#                               "-C", "/home/pi/.arduino15/packages/arduino/tools/avrdude/6.3.0-arduino17/etc/avrdude.conf",
#                               "-v", "-patmega328p", "-carduino", "-b57600", "-P", "/dev/ttyS0", "-D",
#                               "-Uflash:w:arduino/loopa/loopa.hex"],
#                              stdout=subprocess.PIPE,
#                              stdin=subprocess.PIPE,
#                              stderr=subprocess.PIPE)
#     RST.off()
#     time.sleep(0.05)
#     RST.on()
#     output, err = popen.communicate()
#     rc = popen.returncode
#
#     if rc == 0:
#         current_tag = "v0"
#
# print(current_tag)
#
# prt = serial.Serial(port="/dev/ttyS0", baudrate=115200)
# for i in range(5):
#     show_blank_screen(prt)
# prt.close()
#
# try:
#     with urllib.request.urlopen("https://api.github.com/repos/ferluht/loopa/releases", timeout=3) as url:
#         resp = json.loads(url.read().decode("utf-8"))
#         for rel in resp:
#             newtag = rel["tag_name"][1:]
#             if float(newtag[1:]) > float(current_tag[1:]):
#                 prt = serial.Serial(port="/dev/ttyS0", baudrate=115200)
#                 show_update_question(prt)
#                 while True:
#                     recv = prt.readline()
#                     try:
#                         if recv == b'\xb0f\x7f\n':
#                             show_update_message(prt)
#                             print("updating from", current_tag, "to", newtag)
#                             subprocess.run(["wget", rel['assets'][0]['browser_download_url']])
#                             subprocess.run(["unzip", "-o", rel['assets'][0]['name']])
#                             subprocess.run(["rm", rel['assets'][0]['name']])
#
#                             subprocess.run(["mv", "-f", "loopa.hex", "arduino/loopa/"])
#                             popen = subprocess.Popen(["/home/pi/.arduino15/packages/arduino/tools/avrdude/6.3.0-arduino17/bin/avrdude",
#                                                       "-C", "/home/pi/.arduino15/packages/arduino/tools/avrdude/6.3.0-arduino17/etc/avrdude.conf",
#                                                       "-v", "-patmega328p", "-carduino", "-b57600", "-P", "/dev/ttyS0", "-D",
#                                                       "-Uflash:w:arduino/loopa/loopa.hex"])
#                             RST.off()
#                             time.sleep(0.05)
#                             RST.on()
#                             popen.wait()
#
#                             subprocess.run(["chmod", "+x", "loopa"])
#                             subprocess.run(["mv", "-f", "loopa", "build/"])
#                             with open(version_file, "w") as f:
#                                 f.write(newtag)
#                                 current_tag = newtag
#                             break
#                         if recv == b'\xb0e\x7f\n':
#                             break
#                     except Exception as e:
#                         pass
#                     time.sleep(0.5)
#                 prt.close()
#                 break
# except Exception as e:
#     pass
#
# RST.off()
# time.sleep(0.05)
# RST.on()
#
# prt = serial.Serial(port="/dev/ttyS0", baudrate=115200)
# for i in range(5):
#     show_blank_screen(prt)
# prt.close()

while True:
    try:
        popen = subprocess.Popen("./loopa", stdout=subprocess.PIPE, cwd='./build')
        popen.wait()
        output = popen.stdout.read()
        print(output)
    except Exception as e:
        print(str(e))