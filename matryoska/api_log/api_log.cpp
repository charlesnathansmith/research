#include "pin.H"
#include <iostream>
#include <fstream>
#include <map>

// Output log file
KNOB<std::string> log_filename(KNOB_MODE_WRITEONCE, "pintool", "o", "log.txt", "log file");

std::ofstream logfile;
std::map<ADDRINT, std::string> api_map;

VOID log_api(ADDRINT ip, ADDRINT ret_addr)
{
    PIN_LockClient();
    logfile << api_map[ip] << " returns to " << ret_addr << std::endl;
    PIN_UnlockClient();
}

// Instrument modules on load
VOID imgInstrumentation(IMG img, VOID* val)
{
    // Instrument all APIs
    if (!IMG_IsMainExecutable(img))
    {
        std::string img_name = IMG_Name(img);
        logfile << "Loaded module " << img_name << " at " << IMG_LowAddress(img) << std::endl;

        for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
        {
            for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
            {
                ADDRINT addr = RTN_Address(rtn);
                api_map[addr] = img_name + '!' + RTN_Name(rtn);

                if ((api_map[addr] != ".text") && (api_map[addr] != "unnamedImageEntryPoint"))
                {
                    RTN_Open(rtn);
                    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)log_api, IARG_INST_PTR, IARG_RETURN_IP, IARG_END);
                    RTN_Close(rtn);
                }
            }
        }
    }
    else
    {
        logfile << "Loaded main module " << IMG_Name(img) << " at " << IMG_LowAddress(img) << std::endl;

        for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
        {
            for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
            {
                ADDRINT addr = RTN_Address(rtn);
                logfile << "Name: " << RTN_Name(rtn) << "\tAddress: " << RTN_Address(rtn) << std::endl;
            }
        }
    }
}

VOID finished(INT32 code, VOID* v)
{
    logfile << "Finished" << std::endl;
    logfile.close();
}

int main(int argc, char* argv[])
{
    // Init PIN
    PIN_InitSymbols();

    // Parse command line
    if (PIN_Init(argc, argv))
        return -1;

    // Open log file
    logfile.open(log_filename.Value().c_str());

    // Return if unable to open log file
    // No way to record errors up to this point
    if (!logfile)
        return -1;

    // Setup PIN instrumentation callbacks
    IMG_AddInstrumentFunction(imgInstrumentation, 0);
    PIN_AddFiniFunction(finished, 0);

    // Start analysis
    logfile << "Logging API calls...\n" << std::hex << std::endl;
    PIN_StartProgram();

    return 0;
}
