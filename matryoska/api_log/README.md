# api_log

This tool logs a target program's API calls and their return addresses.

It re-treads ground already extensively covered by [tiny_tracer](https://github.com/hasherezade/tiny_tracer) but is stripped down to handle one task as simply as possible. 
It has repeatedly proved useful as a jumping off point to modify for other needs as they arise and is being shared in hopes that it can prove similarly useful to someone else.

This is experimental software with no support.  Feel free to fork, copy, modify, redistribute, cut-and-paste pieces you need for other projects -- whatever gets you the most use out of it.

# Building

The project has been tested to build correctly with Pin 3.28 and Microsoft Visual Studio 2022.

Download and extract Intel Pin from:  
[https://www.intel.com/content/www/us/en/developer/articles/tool/pin-a-binary-instrumentation-tool-downloads.html](https://www.intel.com/content/www/us/en/developer/articles/tool/pin-a-binary-instrumentation-tool-downloads.html)

Copy the api_log directory into Pin's tools directory (pin\source\tools\) and build api_log.sln in Release mode for the same architecture as the target program.

There is nothing special about the project files.  If you have issues building, just make a copy of MyPinTool from Pin's tools directory and replace the source with api_log.cpp

# Running

## Usage
```
pin.exe -t api_log.dll [-b exe_base] [-q 1] [-o log_file] -- target.exe

-b exe_base   Set exe base address for labeling returns into .text
              Typically 0x400000 for x86 or 0x140000000 for x64 disassembly listings
              Hex addresses must be labeled with 0x

-o log_file   Output file for API call log [default: api_log.txt]

-q 1          Quiet -- log only calls returning to main exe [default: 0]
              Also suppresses DLL load logging
```

Make sure log_file is writable.  If it can't be opened for writing, an error will be logged to pintool.log and the tool will exit, but it's not the most obvious error. 
If the tool is dying before it even creates log_file, that's likely the problem.
