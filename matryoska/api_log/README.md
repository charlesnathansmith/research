# api_log

A Pin instrumentation tool for logging API calls made by a target program.

This is currently experimental software.  It comes with no support or guarantees.

# Building

The project has been tested to build correctly with Pin 3.28 and Microsoft Visual Studio 2022.

Download and extract Intel Pin from:  
[https://www.intel.com/content/www/us/en/developer/articles/tool/pin-a-binary-instrumentation-tool-downloads.html](https://www.intel.com/content/www/us/en/developer/articles/tool/pin-a-binary-instrumentation-tool-downloads.html)

Copy the api_log directory into Pin's tools directory (pin\source\tools\) and build api_log.sln in Release mode for the same architecture as the target program.

# Running

You'll of course have to specify the full paths for these:
```
pin.exe -t api_log.dll -o log.txt -- target.exe
```
