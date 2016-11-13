#include "HostGUI.h"
#include "Host.h"
class Host;
LRESULT CALLBACK HostGUI::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_USER + 1: {
			dupa();
			std::cout << (void*)this << std::endl;
			HWND hwnd_pluginlist = GetDlgItem(hWnd, IDC_PLUGINLIST);
			//int i = 0;
			//for (auto &p : host.plugins) { // const auto
			/*
			for (unsigned i = 0; i < host.plugins.size(); ++i) {
				int pos = (int)SendMessage(hwnd_pluginlist, LB_ADDSTRING, 0, (LPARAM)host.plugins[i]->GetPluginName().c_str());
				std::cout << host.plugins[i]->GetPluginName() << std::endl;
				SendMessage(hwnd_pluginlist, LB_SETITEMDATA, pos, (LPARAM)i++);
			}*/
		}
		case WM_DESTROY:
			std::cout << "dzieje sue tu co?" << std::endl;
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}