SameBoy Emulator emitting bap-frames traces
===========================================

This is a patched version of the SameBoy Game Boy emulator that can emit
traces in the [bap-frames](https://github.com/BinaryAnalysisPlatform/bap-frames)
format, primarily to be used in combination with
[rz-tracetest](https://github.com/rizinorg/rz-tracetest) for testing gb (sm83) lifting.

Quirks
------

* Because there is no trivial way to determine which registers are read or
  written during the execution of an instruction, the pre-operands always
  contain all registers and post-operands contain only the registers that have
  changed. The register information is thus not sufficient for comparing
  exact events, but only for pre/post contents.

Building
--------

First, make sure the bap-frames submodule is up to date:
```
git submodule update --init
```

Then:
```
make sdl # or make cocoa for the macOS Cocoa version
```

Usage
-----

Start sameboy from `build/bin/SDL/sameboy` (or `build/bin/SameBoy.app` for
Cocoa), then to start tracing enter the debugger and type e.g.:
```
bapo mytrace.frames
```

After continuing execution, every instruction will be recorded to the file.
Later, to close the trace from the debugger, use:
```
bapc
```
