#include <iostream>
#include <chrono>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <Xinput.h>

#include <ViGEm/Client.h>

#pragma comment(lib, "setupapi.lib")

// Virutal Device Callback
VOID CALLBACK notification(
	PVIGEM_CLIENT Client,
	PVIGEM_TARGET Target,
	UCHAR LargeMotor,
	UCHAR SmallMotor,
	UCHAR LedNumber,
	LPVOID UserData
)
{
	static int count = 1;

	std::cout.width(3);
	std::cout << count++ << " ";
	std::cout.width(3);
	std::cout << (int)LargeMotor << " ";
	std::cout.width(3);
	std::cout << (int)SmallMotor << std::endl;
}

int main()
{
	//
	// Initialization
	//
	const auto client = vigem_alloc();

	if (client == nullptr)
	{
		std::cerr << "Not enough memory to Initialize apparently;" << std::endl;
		return -1;
	}
	const auto retval = vigem_connect(client);
	if (!VIGEM_SUCCESS(retval))
	{
		std::cerr << "ViGEm Bus connection failed with error code: 0x" << std::hex << retval << std::endl;
		return -1;
	}
	//
	

	//
	// Allocate handle to identify new pad
	//
	const auto pad = vigem_target_x360_alloc();

	//
	// Add client to the bus, this equals a plug-in event
	//
	const auto pir = vigem_target_add(client, pad);
	
	//
	// Error handling
	//
	if (!VIGEM_SUCCESS(pir))
	{
		std::cerr << "Target plugin failed with error code: 0x" << std::hex << pir << std::endl;
		return -1;
	}

	// Register callback
	const auto retrieval = vigem_target_x360_register_notification(client, pad, &notification, nullptr);
	if (!VIGEM_SUCCESS(retrieval))
	{
		std::cerr << "Registering for notification failed with error code: 0x" << std::hex << retval << std::endl;
		return -1;
	}
	//
	// The XINPUT_GAMEPAD structure is identical to the XUSB_REPORT structure
	// so we can simply take it "as-is" and cast it.
	//
	// Call this function on every input state change e.g. in a loop polling
	// another joystick or network device or thermometer or... you get the idea.
	//
	// vigem_target_x360_update(client, pad, *reinterpret_cast<XUSB_REPORT*>(&state.Gamepad));
	
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0;
	ip.ki.time = 0;
	ip.ki.wVk = 0x4C;
	ip.ki.dwFlags = 0;
	while (true)
	{
		using namespace std::chrono_literals;
		XINPUT_STATE xState{};
		if (GetKeyState('W') & 0x8000)
		{
		//	SendInput(1, &ip, sizeof(INPUT));
			std::this_thread::sleep_for(2000ms);
			xState.Gamepad.wButtons = 0x0040;
			vigem_target_x360_update(client, pad, *reinterpret_cast<XUSB_REPORT*>(&xState.Gamepad));
		}
		else
		{
			xState.Gamepad.wButtons = 0;
			vigem_target_x360_update(client, pad, *reinterpret_cast<XUSB_REPORT*>(&xState.Gamepad));
		}
		if (GetKeyState(0x23) & 0x8000)
		{
			break;
		}

	}


	//
	// We're done with this pad, free resources (this disconnects the virtual device)
	//
	vigem_target_remove(client, pad);
	vigem_target_free(pad);
	vigem_disconnect(client);
	vigem_free(client);
	return 0;
}