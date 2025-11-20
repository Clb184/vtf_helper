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
		IFileOpenDialog* dlg;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&dlg));
		if (SUCCEEDED(dlg->SetFileTypes(filter_cnt, filter_data))) {
			// Show the Open dialog box.
			hr = dlg->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr)) {
				IShellItem* item;
				printf("Trying to get result\n");
				if (SUCCEEDED(dlg->GetResult(&item))) {
					PWSTR pszFilePath;
					printf("Trying to get DisplayName\n");
					// Display the file name to the user.
					if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
						char buffer[1024] = "";
						size_t sz = 0;
						printf("Selection: %ls\n", pszFilePath);
						wcstombs_s(&sz, buffer, 1024, pszFilePath, wcslen(pszFilePath));
						CoTaskMemFree(pszFilePath);
						*result = std::string(buffer);
						return true;
					}
					item->Release();
				}
			}
			dlg->Release();
		}
		CoUninitialize();
	}
	if(hr != ERROR_CANCELLED) {
		printf("Failed with HRESULT: %x\n", hr);
	}
	return false;
}

bool CreateMultiSelectDialogWindows(COMDLG_FILTERSPEC* filter_data, int filter_cnt, std::vector<std::string>* result) {
	//File Dialog
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	bool on_success = false;

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
					on_success = true;
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
	if(hr != ERROR_CANCELLED) {
		printf("Failed with HRESULT: %x\n", hr);
	}
	return on_success;
}

bool CreateSaveDialogWindows(COMDLG_FILTERSPEC* filter_data, int filter_cnt, std::string* result, LPCWSTR default_ext) {
	//File Dialog
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr)) {
		IFileSaveDialog* dlg;
		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&dlg));
		if (SUCCEEDED(dlg->SetFileTypes(filter_cnt, filter_data))) {
			// Show the Open dialog box.
			dlg->SetDefaultExtension(default_ext);
			hr = dlg->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr)) {
				IShellItem* item;
				printf("Trying to get result\n");
				if (SUCCEEDED(dlg->GetResult(&item))) {
					PWSTR pszFilePath;
					printf("Trying to get DisplayName\n");
					// Display the file name to the user.
					if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath))) {
						char buffer[1024] = "";
						size_t sz = 0;
						printf("Selection: %ls\n", pszFilePath);
						wcstombs_s(&sz, buffer, 1024, pszFilePath, wcslen(pszFilePath));
						CoTaskMemFree(pszFilePath);
						*result = std::string(buffer);
						return true;
					}
					item->Release();
				}
			}
			dlg->Release();
		}
		CoUninitialize();
	}
	if(hr != ERROR_CANCELLED) {
		printf("Failed with HRESULT: %x\n", hr);
	}
	return false;
}
