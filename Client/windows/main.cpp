#include <stdio.h>
#include <memory>
#include "WindowsBluetoothConnector.h"
#include "CommandSerializer.h"
#include "BluetoothWrapper.h"

#include "WindowsGUI.h"

int main()
{
	std::cout << "Sony Companion App — Initializing..." << std::endl;
	try
	{
		std::unique_ptr<IBluetoothConnector> connector = std::make_unique<WindowsBluetoothConnector>();
		BluetoothWrapper bt(std::move(connector));
		EnterGUIMainLoop(std::move(bt));
	}
	catch (const std::exception& e)
	{
		DisplayErrorMessagebox(e.what());
	}
	return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return main();
}
#endif

