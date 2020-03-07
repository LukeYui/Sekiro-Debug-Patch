#include "XinHelper.h"

bool checkControllerState()
{
	DWORD dwResult;
	XINPUT_STATE state;

	dwResult = XInputGetState(0, &state);

	if (dwResult == ERROR_SUCCESS)
	{
		// Controller is connected 
		return true;
	}
	else
	{
		// Controller is not connected 
		return false;
	}
}