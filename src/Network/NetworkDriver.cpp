#include <Platform/Platform.h>
#include <Network/NetworkDriver.h>

#if CURRENT_PLATFORM == PLATFORM_PSP
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>
#include <pspnet.h>
#include <psputility.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <psphttp.h>
#include <pspsdk.h>
#include <pspwlan.h>
#include <sys/socket.h>
#include <unistd.h> 
#include <queue>
#endif

namespace Stardust::Network {

#if CURRENT_PLATFORM == PLATFORM_PSP
	NetworkDriver::NetworkDriver()
	{
		m_Socket = Socket();
	}
	bool NetworkDriver::Init() {
		sceUtilityLoadNetModule(PSP_NET_MODULE_HTTP);
		sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEHTTP);
		sceUtilityLoadNetModule(PSP_NET_MODULE_PARSEURI);
		sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
		sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
		sceUtilityLoadNetModule(PSP_NET_MODULE_SSL);

		//Result stores our codes from the initialization process
		int result = 0;
		result = sceNetInit(128 * 1024, 42, 0, 42, 0); //Creates the network manager with a buffer
		if (result < 0) { //These If Blocks close the game on an error
			sceKernelExitGame();
		}
		result = sceNetInetInit(); //Initializes Inet
		if (result < 0) {
			sceKernelExitGame();
		}
		result = sceNetApctlInit(0x10000, 48); //Initializes Access Point Control
		if (result < 0) {
			sceKernelExitGame();
		}
		result = sceNetResolverInit(); //Initializes DNS resolver (unused)
		if (result < 0) {
			sceKernelExitGame();
		}

		int result = sceNetApctlConnect(1);	//Connects to your first (primary) internet connection.

		//Displays connection status
		int err;
		while (1) {
			int state;
			err = sceNetApctlGetState(&state);
			if (err != 0)
			{
				return false;
			}
			if (state == 4)
				return true;  // connected!

			sceKernelDelayThread(1000 * 50); //50 MS Delay;
		}
	}

	void NetworkDriver::Cleanup() {
		//NET
		sceNetApctlTerm();
		sceNetResolverTerm();
		sceNetInetTerm();
		sceNetTerm();

		//Modules
		sceUtilityUnloadNetModule(PSP_NET_MODULE_HTTP);
		sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEHTTP);
		sceUtilityUnloadNetModule(PSP_NET_MODULE_PARSEURI);
		sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
		sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
	}

#endif

	bool NetworkDriver::Connect(unsigned short port, const char* ip) {
		return m_Socket.Connect(port, ip);
	}

	void NetworkDriver::AddPacket(PacketOut* p)
	{
		packetQueue.push(p);
	}
	void NetworkDriver::ClearPacketQueue()
	{
		for (int i = 0; i < packetQueue.size(); i++) {
			delete packetQueue.front();
			packetQueue.pop();
		}
	}

	void NetworkDriver::SendPackets()
	{
		std::vector<byte> endByteBuffer;

		for (int i = 0; i < packetQueue.size(); i++) {
			endByteBuffer.clear();

			int packetLength = packetQueue.front()->bytes.size() + 2;

			//Header
			encodeVarInt(packetLength, endByteBuffer);
			encodeShort(packetQueue.front()->ID, endByteBuffer);

			//Add body
			for (int x = 0; x < packetQueue.front()->bytes.size(); i++) {
				endByteBuffer.push_back(packetQueue.front()->bytes[i]);
			}

			//Send over socket
			m_Socket.Send(endByteBuffer.size(), endByteBuffer.data());
		}
	}

}