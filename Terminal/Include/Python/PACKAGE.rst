.. contents::
   :backlinks: top

=======
 About
=======
BearLibTerminal is a library that creates a terminal-like window facilitating 
flexible textual output and uncomplicated input processing.

A lot of roguelike games intentionally use asketic textual or pseudographic visual style. 
However, native output via the command line interface ususally have a few annoying 
shortcomings like low speed or palette and font restrictions. Using an extended 
character set (several languages at once or complicated pseudographics) may also be tricky. 
BearLibTerminal solves that by providing it's own window with a grid of character cells 
and simple yet powerful API for configuration and textual output.

Online documentation: http://foo.wyrd.name/en:bearlibterminal:reference

Source code and issue tracker: https://bitbucket.org/cfyzium/bearlibterminal

Discussion forum: http://forums.roguetemple.com/index.php?topic=3896.0 

==============
 Installation
==============
Use pip:

.. code-block::

  pip install bearlibterminal

This will install everything necessary to use BearLibTerminal from Python, both the wrapper and the library binary.

.. code-block:: python

  from bearlibterminal import terminal
  terminal.open()
  terminal.printf(2, 1, "Hello, world!")
  terminal.refresh()
  while terminal.read() != terminal.TK_CLOSE:
      pass
  terminal.close()

==============
 Requirements
==============
Python 2.7+ or 3.x

=========
 License
=========
BearLibTerminal is distributed under the MIT license.
