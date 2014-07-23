# IDA Splode

A tool that I wrote to help reversing on Windows.

## Requirements

- `%PIN_HOME%` points to an installation of Intel's Pin.
- MongoDB
- IDA Pro
- VS 2010

## Usage

- Run build.bat from a MSVC 2010 console
- Optionally, enable page heap for test.exe
- Run release.bat to trace the test.exe program in release mode
- Start MongoDB
- Run `demo.exe.py` to import the traces
- Start IDA Pro, open `demo.exe`
- Run `py\idapython_script.py` from within IDA
- If everything worked, `ida-splode` should automatically recognize all traces for the open binary from the database, and present a list of options.

## Presentation

See the PowerPoint presentation in `slides/` for some examples on the sample application.

## Caveats

This is pulled from a working copy, so some things may not work properly.  If you run into any issues, feel free to contact me at [@ebeip90](https://twitter.com/ebeip90) or ebeip90 on Freenode.net.
