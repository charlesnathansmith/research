**Analysis of the Matroyska Bootstrap Loader**  
Charles Nathan Smith

# Background and Overview

On October 19, 2023, malware research collective **vx-underground** publicly released Matryoshka, an experimental bootstrap loader program for Windows 10, 
issuing an open challenge to reverse engineer it and "tell us how you think it works."

This is one such response.

In its current form, Matryoshka is a non-malicious bootstrap loader that downloads a copy of cmd.exe to a temporary folder under a randomized name and executes it.

While it is currently innocuous, it could easily be adapted to download and execute a different payload.

While it seems a bit premature to start attempting to characterize indicators of compromise until this starts showing up paired with real payloads, 
they currently include randomly named executables present or executing out of temporary folders (eg. "%userprofile%\AppData\Local\457de82a29c49c.exe"), 
though that's by no means specific to this particular loader.

The payload fetch is performed through the IWinHttpRequest COM control, which should serve as a reminder to filter developers that there is a litany of ways to accomplish any particular task of interest.

# Technical Analysis

The target was instrumented with the *api_log* Pin tool (link in **See Also** section) and then screened for API calls originating from the .text segment of the target:
```
Loaded main module C:\Users\charl\Desktop\Matryoshka\Matryoshka.exe at 7ff775b00000
Name: .text	Address: 7ff775b01000
Name: unnamedImageEntryPoint	Address: 7ff775b02e08
C:\WINDOWS\SYSTEM32\ntdll.dll!ZwCreateFile returns to 7ff775b025ba
C:\WINDOWS\SYSTEM32\ntdll.dll!ZwDeviceIoControlFile returns to 7ff775b0260a
C:\WINDOWS\SYSTEM32\ntdll.dll!ZwCreateFile returns to 7ff775b027f2
C:\WINDOWS\SYSTEM32\ntdll.dll!ZwClose returns to 7ff775b0281a
C:\WINDOWS\SYSTEM32\ntdll.dll!LdrLoadDll returns to 7ff775b01dd1
C:\WINDOWS\SYSTEM32\ntdll.dll!LdrLoadDll returns to 7ff775b01cf7
C:\WINDOWS\System32\Combase.dll!CoInitializeEx returns to 7ff775b01ee7
C:\WINDOWS\System32\Combase.dll!CoInitializeSecurity returns to 7ff775b01f34
C:\WINDOWS\System32\Combase.dll!CoCreateInstance returns to 7ff775b02918
C:\WINDOWS\System32\Combase.dll!CoCreateInstance returns to 7ff775b02aba
C:\WINDOWS\system32\wbem\fastprox.dll!?Get@CWbemObject@@UEAAJPEBGJPEAUtagVARIANT@@PEAJ2@Z returns to 7ff775b02d2c
C:\WINDOWS\system32\wbem\fastprox.dll!?Release@CWbemObject@@UEAAKXZ returns to 7ff775b02d88
C:\WINDOWS\system32\wbem\fastprox.dll!?Release@?$CImpl@UIWbemObjectTextSrc@@VCWmiObjectTextSrc@@@@UEAAKXZ returns to 7ff775b02d9a
C:\WINDOWS\System32\Combase.dll!CoCreateInstance returns to 7ff775b01fb3
C:\WINDOWS\SYSTEM32\ntdll.dll!ZwAllocateVirtualMemory returns to 7ff775b021da
C:\WINDOWS\SYSTEM32\ntdll.dll!ZwWriteFile returns to 7ff775b02248
C:\WINDOWS\System32\OleAut32.dll!SysFreeString returns to 7ff775b02258
C:\WINDOWS\System32\Combase.dll!CoUninitialize returns to 7ff775b02275
C:\WINDOWS\SYSTEM32\ntdll.dll!NtFreeVirtualMemory returns to 7ff775b0229a
C:\WINDOWS\SYSTEM32\ntdll.dll!ZwClose returns to 7ff775b022a9
C:\WINDOWS\SYSTEM32\ntdll.dll!NtCreateUserProcess returns to 7ff775b01b21
```

This won't catch every interesting call as some are buried in library wrappers, but is enough to give a high-level overview and a starting point to start setting some breakpoints.

Of particular interest are the COM control and fastprox.dll related calls, as a lot of the workload seems to be getting handed off to them.

Setting a breakpoint on **CoCreateInstance**, we catch instantiation of a **IWinHttpRequest** object (CLSID 016FE2EC-B2C8-45F8-B23B-39E53A75396B) and also a **Windows Management Instrumentation (WMI)** object (CLSID 8BC3F05E-D86B-11D0-A075-00C04FB68820).

Breaking on different variations of **SysAllocString** and **SysFreeString**, we're able to locate how **WMI** is being utilized:
```
L"SELECT * FROM Win32_PingStatus WHERE Address=\"172.67.136.136\""
```

This is just checking if the host is up, where **172.67.136.136** is a CloudFare gateway which isn't particularly illuminating on its own.

The actual URL accessed should be passed into the **IWinHttpRequest** object's methods and is a bit difficult to track down through the present method (see **Additional Research** below.)

For now it suffices just to proxy all of our traffic through **Burp Suite** and catch the HTTPS request:
```
https://samples.vx-underground.org/root/Samples/cmd.exe
```

Which is more or less what we expected.

Just examining the **cmd.exe** processes launched by the target in Task Manager reveals where they are being stored:
```
%userprofile%\AppData\Local\[14 random hex digits].exe

Eg:

%userprofile%\AppData\Local\2be1e631e1893d.exe
%userprofile%\AppData\Local\3cc77c2d91361a.exe
%userprofile%\AppData\Local\5e62cc102f3d53.exe
%userprofile%\AppData\Local\06df402ba121fd.exe
```

The loader doesn't clean up after itself and these start to pile up after several runs.

References to these files can be found in calls to **ZwWriteFile** and **NtCreateUserProcess**, which don't seem to add any additional information.

This appears to more or less cover everything interesting that the loader is doing, but should not be interpreted as an exhaustive analysis.  It is possible that there is other hidden functionality that has been overlooked.

# Further Research

A more detailed analysis could be obtained by a number of means.

Instrumenting the calls to **CoCreateInstance** and analyzing use of the returned object pointers should allow 
locating and instrumenting calls to their methods to confirm precisely how they are being used.

This could be useful for tracking down where the pinged IP and accessed URL come from and where the file name is generated, 
which may prove useful for automated analysis if this loader starts appearing in the wild.

Little was done here in the way of analyzing the actual program assembly in detail.  This wasn't really necessary to "tell us how you think it works", but it's sufficiently cryptic to warrant further investigation in case similar obfuscation techniques are used in the future by other malware based off of this one.

# See Also

**Original call for submissions**  
[https://twitter.com/vxunderground/status/1715088076811235487?t=_GvY26TtEHWW3Gg7A-uDRg&s=19
](https://twitter.com/vxunderground/status/1715088076811235487?t=_GvY26TtEHWW3Gg7A-uDRg&s=19
)

**api_log Pin tool**  
[https://github.com/charlesnathansmith/research/tree/main/matryoska/api_log](https://github.com/charlesnathansmith/research/tree/main/matryoska/api_log)

**Burp Suite and Proxifier**  
[https://portswigger.net/burp/communitydownload](https://portswigger.net/burp/communitydownload)  
[https://www.proxifier.com/](https://www.proxifier.com/)

**WMI related***  
[https://learn.microsoft.com/en-us/windows/win32/wmisdk/wmi-reference](https://learn.microsoft.com/en-us/windows/win32/wmisdk/wmi-reference)

**IWinHttpRequest**  
[https://learn.microsoft.com/en-us/windows/win32/winhttp/iwinhttprequest-interface](https://learn.microsoft.com/en-us/windows/win32/winhttp/iwinhttprequest-interface)
