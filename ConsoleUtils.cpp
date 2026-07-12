#include "ConsoleUtils.h"
#include "CCore.h"

extern CCore* pCore;

#define ANSI_RESET  "\x1b[0m"
#define ANSI_CYAN   "\x1b[1;36m"
#define ANSI_GREEN  "\x1b[1;32m"
#define ANSI_YELLOW "\x1b[1;33m"
#define ANSI_RED    "\x1b[1;31m"

void OutputScriptInfo(const char* msg) {
    pCore->printf(ANSI_CYAN "[SqSCRIPT]  " ANSI_RESET "%s\n", msg);
}

void OutputMessage(const char* msg) {
    pCore->printf(ANSI_GREEN "[SqINFO]    " ANSI_RESET "%s\n", msg);
}

void OutputWarning(const char* msg) {
    pCore->printf(ANSI_YELLOW "[SqWARN]    " ANSI_RESET "%s\n", msg);
}

void OutputError(const char* msg) {
    pCore->printf(ANSI_RED "[SqERROR]   " ANSI_RESET "%s\n", msg);
}

void OutputDebug(const char* msg) {
#ifdef _DEBUG
    OutputMessage(msg);
#endif
}