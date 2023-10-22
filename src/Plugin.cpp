#include "XPLMProcessing.h"
#include "XPLMUtilities.h"

#include "string.h"
#include "Server.cpp"

#ifndef XPLM300
#error This is made to be compiled against the XPLM300 SDK
#endif

static float LoopCallback(
		float, // inElapsedSinceLastCall
		float, // inElapsedTimeSinceLastFlightLoop
		int,	 // inCounter
		void * // inRefcon
)
{
	serverLoop();

	return -1;
}

PLUGIN_API int XPluginStart(
		char *outName,
		char *outSig,
		char *outDesc)
{
	strcpy(outName, "Fly with HTTP");
	strcpy(outSig, "com.git.sergeiterehov.fly_with_http");
	strcpy(outDesc, "Plugin for Fly with HTTP");

	return 1;
}

PLUGIN_API int XPluginEnable(void)
{
	int status = ServerListen();

	if (status < 0)
	{
		return 0;
	}

	XPLMRegisterFlightLoopCallback(LoopCallback, -1.0, NULL);

	return 1;
}

PLUGIN_API void XPluginDisable(void)
{
	XPLMUnregisterFlightLoopCallback(LoopCallback, NULL);
}

PLUGIN_API void XPluginStop(void) {
	ServerStop();
}

PLUGIN_API void XPluginReceiveMessage(
		XPLMPluginID, // inFrom
		int,					// inMsg
		void *				// inParam
)
{
}
