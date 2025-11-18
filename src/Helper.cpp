#include "Helper.hpp"


void NormalizeString(std::string* string) {
	while(string->length()){
			char c = string->at(0);
			if(!(c == ' ' || c == '\t')){ break; }
			string->erase(string->begin());
		}
	for(int i = string->length() - 1; i > 0; i--){
		char c = string->at(i);
		if(!(c == ' ' || c == '\t')){ break; }
		string->pop_back();
	}
}

bool CreateSingleSelectDialogWindows(COMDLG_FILTERSPEC* filter_data, int filter_cnt, std::string* result) {
	//File Dialog
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr)) {
		IFileOpenDialog* pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
		hr = pFileOpen->SetFileTypes(filter_cnt, filter_data);
		if (SUCCEEDED(hr)) {
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr)) {
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				printf("Trying to get result\n");
				if (SUCCEEDED(hr)) {
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					printf("Trying to get DisplayName\n");
					// Display the file name to the user.
					if (SUCCEEDED(hr)) {
						char buffer[1024] = "";
						size_t sz = 0;
						printf("Selection: %ls\n", pszFilePath);
						wcstombs_s(&sz, buffer, 1024, pszFilePath, wcslen(pszFilePath));
						CoTaskMemFree(pszFilePath);
						*result = std::string(buffer);
						return true;
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	printf("Failed with HRESULT: %x", hr);
	return false;
}

bool CreateMultiSelectDialogWindows(COMDLG_FILTERSPEC* filter_data, int filter_cnt, std::vector<std::string>* result) {
	//File Dialog
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr)) {
		IFileOpenDialog* pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
		hr = pFileOpen->SetFileTypes(filter_cnt, filter_data);

		FILEOPENDIALOGOPTIONS options;
		pFileOpen->GetOptions(&options);
		pFileOpen->SetOptions(options | FOS_ALLOWMULTISELECT);
		if (SUCCEEDED(hr)) {
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr)) {
				IShellItemArray* item_array;
				printf("Trying to get results\n");
				if (SUCCEEDED(pFileOpen->GetResults(&item_array))) {
					DWORD count;
					item_array->GetCount(&count);
					IShellItem* item;
					for(DWORD i = 0; i < count; i++) {
						printf("Trying to get Shell Item at %d\n", i);
						if(SUCCEEDED(item_array->GetItemAt(i, &item))) {
						PWSTR pszFilePath;
						printf("Trying to get DisplayName\n");
						// Display the file name to the user.
						if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
								char buffer[1024] = "";
								size_t sz = 0;
								printf("Selection: %ls\n", pszFilePath);
								wcstombs_s(&sz, buffer, 1024, pszFilePath, wcslen(pszFilePath));
								CoTaskMemFree(pszFilePath);
								result->emplace_back(buffer);
								return true;
							}
							item->Release();
						}
					}
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	printf("Failed with HRESULT: %x", hr);
	return false;
}
