### Using BearLibTerminal

There are two ways to setup BearLibTerminal for Python.

#### 1. Using the package from PyPi

Install the package via pip:

    pip install bearlibterminal

Depending on the OS and Python installation, you might also want to

- Replace `pip` with `pip3` or `python3 -m pip` to select a correct version of Python.
- Add `--user` flag to install package locally (i. e. user-wide).

The package contains both wrapper and an appropriate binary for the platform, so you do not need to copy anything else anywhere. Just import the library in the source:

    from bearlibterminal import terminal
    
    terminal.open()
    terminal.printf(2, 1, "Hello, world!")
    terminal.refresh()
    while terminal.read() != terminal.TK_CLOSE:
        pass
    terminal.close()

#### 2. Copying package files into the project source

This makes project a bit more stable and self-sufficient but less portable.

Copy the following files into your project source:
- The `bearlibterminal` directory from here
- The library binary into the previously copied `bearlibterminal` directory

E. g.

    Game/
        bearlibterminal/
            __init__.py
            terminal.py
            BearLibTerminal.dll
        game.py

Then you can use it similarly to an installed package:

    from bearlibterminal import terminal


#### 2b. Copying the wrapper and binary files only

Or you can copy only the bare minimum:

    Game/
        terminal.py
        BearLibTerminal.dll
        game.py

And import the module like this:

    import terminal

The wrapper may be placed/renamed however you like, the only requirement is for the binary to be in the same directory with the wrapper.


### Building a Python wheel package

This directory contains a package skeleton. To build a package, a few more files are necessary:

- CHANGELOG.md here in the root (setup.py uses it to fetch the version)
- A library binary in the `bearlibterminal` directory (near the `terminal.py`)

One has to manually copy these files from source and build trees because pip does not copy files from outside directories when building the package.

For example, from a Linux build directory:

    cp -R ../Terminal/Include/Python ./
    cp ../CHANGELOG.md ./Python/
    cp ../Output/Linux64/libBearLibTerminal.so ./Python/bearlibterminal/
    pip wheel --build-option '--python-tag=py2.py3' --build-option '--plat-name=manylinux1_x86_64' ./Python
