#ifndef HELPER_INCLUDED
#define HELPER_INCLUDED

#include <vector>
#include <string>

void NormalizeString(std::string* string);

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shobjidl.h>

bool CreateSingleSelectDialogWindows(COMDLG_FILTERSPEC* filter_data, int filter_cnt, std::string* result);
bool CreateMultiSelectDialogWindows(COMDLG_FILTERSPEC* filter_data, int filet_cnt, std::vector<std::string>* result);

#endif

#endif
