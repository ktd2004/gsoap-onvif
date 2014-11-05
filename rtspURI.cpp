#include <iostream>
#include "stdio.h"
#include "wsdd.nsmap"
#include "wsseapi.h"
#include "wsaapi.h"
#include  <openssl/rsa.h>
#include  "ipcLog.h"
#include "soapDeviceBindingProxy.h"
#include "soapMediaBindingProxy.h"

#define DEV_PASSWORD "admin"
#define MAX_HOSTNAME_LEN 128
#define MAX_LOGMSG_LEN 256 
using namespace std;

void PrintErr(struct soap* _psoap)
{
	fflush(stdout);
	processEventLog(__FILE__, __LINE__, stdout, "error:%d faultstring:%s faultcode:%s faultsubcode:%s faultdetail:%s", 
					_psoap->error, *soap_faultstring(_psoap), *soap_faultcode(_psoap), *soap_faultsubcode(_psoap), *soap_faultdetail(_psoap));
}

int main(int argc, char* argv[])
{
	
	bool blSupportPTZ = false;
	char szHostName[MAX_HOSTNAME_LEN] = { 0 };
	char sLogMsg[MAX_LOGMSG_LEN] = { 0 };

	DeviceBindingProxy proxyDevice;
	MediaBindingProxy proxyMedia;


	if (argc > 1)
	{
		strcat(szHostName, "http://");
		strcat(szHostName, argv[1]);
		strcat(szHostName, "/onvif/device_service");

		proxyDevice.soap_endpoint = szHostName;
	}
	else
	{
		//proxyDevice.soap_endpoint = "http://172.18.4.110:1018/onvif/device_service";
		proxyDevice.soap_endpoint = "http://172.18.4.100/onvif/device_service";
	}

	soap_register_plugin(proxyDevice.soap, soap_wsse);
	soap_register_plugin(proxyMedia.soap, soap_wsse);

	struct soap *soap = soap_new();

	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyDevice.soap, NULL, "admin", DEV_PASSWORD))
	{
		return -1;
	}

	if (SOAP_OK != soap_wsse_add_Timestamp(proxyDevice.soap, "Time", 10)) // 10 seconds lifetime	
	{
		return -1;
	}

	//Get WSDL URL
	_tds__GetWsdlUrl *tds__GetWsdlUrl = soap_new__tds__GetWsdlUrl(soap, -1);
	_tds__GetWsdlUrlResponse *tds__GetWsdlUrlResponse = soap_new__tds__GetWsdlUrlResponse(soap, -1);

	if (SOAP_OK == proxyDevice.GetWsdlUrl(tds__GetWsdlUrl, tds__GetWsdlUrlResponse))
	{
		processEventLog(__FILE__, __LINE__, stdout, "-------------------WsdlUrl-------------------");
		processEventLog(__FILE__, __LINE__, stdout, "WsdlUrl:%s ", tds__GetWsdlUrlResponse->WsdlUrl.c_str());
	}
	else
	{
		PrintErr(proxyDevice.soap);
	}

	soap_destroy(soap); 
	soap_end(soap); 

	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyDevice.soap, NULL, "admin", DEV_PASSWORD))
	{
		return -1;
	}

	//Capability exchange
	_tds__GetCapabilities *tds__GetCapabilities = soap_new__tds__GetCapabilities(soap, -1);
	tds__GetCapabilities->Category.push_back(tt__CapabilityCategory__All);

	_tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse = soap_new__tds__GetCapabilitiesResponse(soap, -1);

	if (SOAP_OK == proxyDevice.GetCapabilities(tds__GetCapabilities, tds__GetCapabilitiesResponse))
	{
		
		if (tds__GetCapabilitiesResponse->Capabilities->Imaging != NULL)
		{
			processEventLog(__FILE__, __LINE__, stdout, "-------------------Imaging-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "XAddr:%s", tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr.c_str());
		}

		if (tds__GetCapabilitiesResponse->Capabilities->Media != NULL)
		{
			processEventLog(__FILE__, __LINE__, stdout, "-------------------Media-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "XAddr:%s", tds__GetCapabilitiesResponse->Capabilities->Media->XAddr.c_str());

			processEventLog(__FILE__, __LINE__, stdout, "-------------------streaming-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "RTPMulticast:%s", (tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "RTP_TCP:%s", (tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "RTP_RTSP_TCP:%s", (tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP) ? "Y" : "N");

			proxyMedia.soap_endpoint = tds__GetCapabilitiesResponse->Capabilities->Media->XAddr.c_str();
		}

		
	}
	else
	{
		PrintErr(proxyDevice.soap);
	}

	soap_destroy(soap); 
	soap_end(soap); 
	

	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, "admin", DEV_PASSWORD))
	{
		return -1;
	}

	if (SOAP_OK != soap_wsse_add_Timestamp(proxyMedia.soap, "Time", 10)) // 10 seconds lifetime	
	{
		return -1;
	}

	_trt__GetProfiles *trt__GetProfiles = soap_new__trt__GetProfiles(soap, -1);
	_trt__GetProfilesResponse *trt__GetProfilesResponse = soap_new__trt__GetProfilesResponse(soap, -1);

	if (SOAP_OK == proxyMedia.GetProfiles(trt__GetProfiles, trt__GetProfilesResponse))
	{
		_trt__GetStreamUri *trt__GetStreamUri = soap_new__trt__GetStreamUri(soap, -1);
		trt__GetStreamUri->StreamSetup = soap_new_tt__StreamSetup(soap, -1);
		trt__GetStreamUri->StreamSetup->Stream = tt__StreamType__RTP_Unicast;
		trt__GetStreamUri->StreamSetup->Transport = soap_new_tt__Transport(soap, -1);
		trt__GetStreamUri->StreamSetup->Transport->Protocol = tt__TransportProtocol__RTSP;

		_trt__GetStreamUriResponse *trt__GetStreamUriResponse = soap_new__trt__GetStreamUriResponse(soap, -1);

		processEventLog(__FILE__, __LINE__, stdout, "-------------------MediaProfiles-------------------");
		for (int i = 0; i < trt__GetProfilesResponse->Profiles.size(); i++)
		{
			processEventLog(__FILE__, __LINE__, stdout, "profile%d:%s Token:%s", i, trt__GetProfilesResponse->Profiles[i]->Name.c_str(), trt__GetProfilesResponse->Profiles[i]->token.c_str());
			trt__GetStreamUri->ProfileToken = trt__GetProfilesResponse->Profiles[i]->token;

			if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, "admin", DEV_PASSWORD))
			{
				return -1;
			}

			if (SOAP_OK == proxyMedia.GetStreamUri(trt__GetStreamUri, trt__GetStreamUriResponse))
			{
				processEventLog(__FILE__, __LINE__, stdout, "RTSP URI:%s", trt__GetStreamUriResponse->MediaUri->Uri.c_str());
			}
			else
			{
				PrintErr(proxyMedia.soap);
			}

		}
	}
	else
	{
		PrintErr(proxyMedia.soap);
	}

	soap_destroy(soap); 
	soap_end(soap); 

	
	
	return 0;
}

