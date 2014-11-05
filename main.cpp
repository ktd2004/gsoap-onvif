#include <iostream>
#include "stdio.h"
#include "wsdd.nsmap"
#include "plugin/wsseapi.h"
#include "plugin/wsaapi.h"
#include  <openssl/rsa.h>
#include  "ErrorLog.h"
 
#include "include/soapDeviceBindingProxy.h"
#include "include/soapMediaBindingProxy.h"
#include "include/soapPTZBindingProxy.h"

#include "include/soapPullPointSubscriptionBindingProxy.h"
#include "include/soapRemoteDiscoveryBindingProxy.h" 

using namespace std;



#define DEV_PASSWORD "admin"
#define MAX_HOSTNAME_LEN 128
#define MAX_LOGMSG_LEN 256 


void PrintErr(struct soap* _psoap)
{
	fflush(stdout);
	processEventLog(__FILE__, __LINE__, stdout, "error:%d faultstring:%s faultcode:%s faultsubcode:%s faultdetail:%s", _psoap->error, 
	*soap_faultstring(_psoap), *soap_faultcode(_psoap),*soap_faultsubcode(_psoap), *soap_faultdetail(_psoap));
}

int main(int argc, char* argv[])
{

	bool blSupportPTZ = false;
	char szHostName[MAX_HOSTNAME_LEN] = { 0 };
	char sLogMsg[MAX_LOGMSG_LEN] = { 0 };

	DeviceBindingProxy proxyDevice;
	RemoteDiscoveryBindingProxy proxyDiscovery; 
	MediaBindingProxy proxyMedia;
	PTZBindingProxy proxyPTZ;
	PullPointSubscriptionBindingProxy proxyEvent;

	if (argc > 1)
	{
		strcat(szHostName, "http://");
		strcat(szHostName, argv[1]);
		strcat(szHostName, "/onvif/device_service");

		proxyDevice.soap_endpoint = szHostName;
	}
	else
	{
		processEventLog(__FILE__, __LINE__, stdout, "wrong args,usage: ./a.out 172.18.4.100 ");
		return -1;
	}

	soap_register_plugin(proxyDevice.soap, soap_wsse);
	soap_register_plugin(proxyDiscovery.soap, soap_wsse);
	soap_register_plugin(proxyMedia.soap, soap_wsse);
	soap_register_plugin(proxyPTZ.soap, soap_wsse);
	soap_register_plugin(proxyEvent.soap, soap_wsse);

	soap_register_plugin(proxyEvent.soap, soap_wsa);

	struct soap *soap = soap_new();

	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyDevice.soap, NULL, "admin", DEV_PASSWORD))
	{
		return -1;
	}

	if (SOAP_OK != soap_wsse_add_Timestamp(proxyDevice.soap, "Time", 10)) 
	{
		return -1;
	}

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

	_tds__GetCapabilities *tds__GetCapabilities = soap_new__tds__GetCapabilities(soap, -1);
	tds__GetCapabilities->Category.push_back(tt__CapabilityCategory__All);

	_tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse = soap_new__tds__GetCapabilitiesResponse(soap, -1);

	if (SOAP_OK == proxyDevice.GetCapabilities(tds__GetCapabilities, tds__GetCapabilitiesResponse))
	{
		if (tds__GetCapabilitiesResponse->Capabilities->Analytics != NULL)
		{
			processEventLog(__FILE__, __LINE__, stdout, "-------------------Analytics-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "XAddr:%s", tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr.c_str());
			processEventLog(__FILE__, __LINE__, stdout, "RuleSupport:%s", (tds__GetCapabilitiesResponse->Capabilities->Analytics->RuleSupport) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "AnalyticsModuleSupport:%s", (tds__GetCapabilitiesResponse->Capabilities->Analytics->AnalyticsModuleSupport) ? "Y" : "N");
		}

		if (tds__GetCapabilitiesResponse->Capabilities->Device != NULL)
		{
			processEventLog(__FILE__, __LINE__, stdout, "-------------------Device-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "XAddr:%s", tds__GetCapabilitiesResponse->Capabilities->Device->XAddr.c_str());

			processEventLog(__FILE__, __LINE__, stdout, "-------------------Network-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "IPFilter:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "ZeroConfiguration:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "IPVersion6:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "DynDNS:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS) ? "Y" : "N");

			processEventLog(__FILE__, __LINE__, stdout, "-------------------System-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "DiscoveryResolve:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryResolve) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "DiscoveryBye:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryBye) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "RemoteDiscovery:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->System->RemoteDiscovery) ? "Y" : "N");

			int iSize = tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions.size();

			if (iSize > 0)
			{
				processEventLog(__FILE__, __LINE__, stdout, "SupportedVersions:");

				for (int i = 0; i < iSize; i++)
				{
					processEventLog(__FILE__, __LINE__, stdout, "%d.%d ", tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[i]->Major,
																		  tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[i]->Minor);
				}

				processEventLog(__FILE__, __LINE__, stdout, "");
			}

			processEventLog(__FILE__, __LINE__, stdout, "SystemBackup:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemBackup) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "FirmwareUpgrade:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->System->FirmwareUpgrade) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "SystemLogging:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemLogging) ? "Y" : "N");

			processEventLog(__FILE__, __LINE__, stdout, "-------------------IO-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "InputConnectors:%d", tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors);
			processEventLog(__FILE__, __LINE__, stdout, "RelayOutputs:%d", tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs);

			processEventLog(__FILE__, __LINE__, stdout, "-------------------Security-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "TLS1.1:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e1) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "TLS1.2:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e2) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "OnboardKeyGeneration:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->OnboardKeyGeneration) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "AccessPolicyConfig:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->AccessPolicyConfig) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "X.509Token:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->X_x002e509Token) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "SAMLToken:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->SAMLToken) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "KerberosToken:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->KerberosToken) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "RELToken:%s", (tds__GetCapabilitiesResponse->Capabilities->Device->Security->RELToken) ? "Y" : "N");
		}

		if (tds__GetCapabilitiesResponse->Capabilities->Events != NULL)
		{
			processEventLog(__FILE__, __LINE__, stdout, "-------------------Events-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "XAddr:%s", tds__GetCapabilitiesResponse->Capabilities->Events->XAddr.c_str());
			processEventLog(__FILE__, __LINE__, stdout, "WSSubscriptionPolicySupport:%s", (tds__GetCapabilitiesResponse->Capabilities->Events->WSSubscriptionPolicySupport) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "WSPullPointSupport:%s", (tds__GetCapabilitiesResponse->Capabilities->Events->WSPullPointSupport) ? "Y" : "N");
			processEventLog(__FILE__, __LINE__, stdout, "WSPausableSubscriptionManagerInterfaceSupport:%s", 
																				 (tds__GetCapabilitiesResponse->Capabilities->Events->WSPausableSubscriptionManagerInterfaceSupport) ? "Y" : "N");

			proxyEvent.soap_endpoint = tds__GetCapabilitiesResponse->Capabilities->Events->XAddr.c_str();
		}

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

		if (tds__GetCapabilitiesResponse->Capabilities->PTZ != NULL)
		{
			processEventLog(__FILE__, __LINE__, stdout, "-------------------PTZ-------------------");
			processEventLog(__FILE__, __LINE__, stdout, "XAddr:%s", tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr.c_str());

			proxyPTZ.soap_endpoint = tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr.c_str();
			blSupportPTZ = true;
		}
	}
	else
	{
		PrintErr(proxyDevice.soap);
	}

	soap_destroy(soap); 
	soap_end(soap); 

	_tds__GetDeviceInformation *tds__GetDeviceInformation = soap_new__tds__GetDeviceInformation(soap, -1);
	_tds__GetDeviceInformationResponse *tds__GetDeviceInformationResponse = soap_new__tds__GetDeviceInformationResponse(soap, -1);

	if (SOAP_OK == proxyDevice.GetDeviceInformation(tds__GetDeviceInformation, tds__GetDeviceInformationResponse))
	{
		processEventLog(__FILE__, __LINE__, stdout, "-------------------DeviceInformation-------------------");
		processEventLog(__FILE__, __LINE__, stdout, "Manufacturer:%sModel:%s\r\nFirmwareVersion:%s\r\nSerialNumber:%s\r\nHardwareId:%s", tds__GetDeviceInformationResponse->Manufacturer.c_str(),
			tds__GetDeviceInformationResponse->Model.c_str(), tds__GetDeviceInformationResponse->FirmwareVersion.c_str(),
			tds__GetDeviceInformationResponse->SerialNumber.c_str(), tds__GetDeviceInformationResponse->HardwareId.c_str());
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

	_tds__GetNetworkInterfaces *tds__GetNetworkInterfaces = soap_new__tds__GetNetworkInterfaces(soap, -1);
	_tds__GetNetworkInterfacesResponse *tds__GetNetworkInterfacesResponse = soap_new__tds__GetNetworkInterfacesResponse(soap, -1);

	if (SOAP_OK == proxyDevice.GetNetworkInterfaces(tds__GetNetworkInterfaces, tds__GetNetworkInterfacesResponse))
	{
		processEventLog(__FILE__, __LINE__, stdout, "-------------------NetworkInterfaces-------------------");
		processEventLog(__FILE__, __LINE__, stdout, "%s", tds__GetNetworkInterfacesResponse->NetworkInterfaces[0]->token.c_str());
		processEventLog(__FILE__, __LINE__, stdout, "%s", tds__GetNetworkInterfacesResponse->NetworkInterfaces[0]->Info->HwAddress.c_str());
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

	if (SOAP_OK != soap_wsse_add_Timestamp(proxyMedia.soap, "Time", 10)) 
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
			processEventLog(__FILE__, __LINE__, stdout, "profile%d:%s Token:%s\r", i, trt__GetProfilesResponse->Profiles[i]->Name.c_str(), trt__GetProfilesResponse->Profiles[i]->token.c_str());
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

	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, "admin", DEV_PASSWORD))
	{
		return -1;
	}

	_trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations = soap_new__trt__GetVideoEncoderConfigurations(soap, -1);
	_trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse = soap_new__trt__GetVideoEncoderConfigurationsResponse(soap, -1);

	if (SOAP_OK == proxyMedia.GetVideoEncoderConfigurations(trt__GetVideoEncoderConfigurations, trt__GetVideoEncoderConfigurationsResponse))
	{
		 processEventLog(__FILE__, __LINE__, stdout, "-------------------VideoEncoderConfigurations-------------------");

		for (int i = 0; i < trt__GetVideoEncoderConfigurationsResponse->Configurations.size(); i++)
		{
			_trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration = soap_new__trt__GetVideoEncoderConfiguration(soap, -1);
			_trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse = soap_new__trt__GetVideoEncoderConfigurationResponse(soap, -1);

			trt__GetVideoEncoderConfiguration->ConfigurationToken = trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->token;

			if (SOAP_OK == proxyMedia.GetVideoEncoderConfiguration(trt__GetVideoEncoderConfiguration, trt__GetVideoEncoderConfigurationResponse))
			{

			}
			else
			{
				PrintErr(proxyMedia.soap);
			}

			 processEventLog(__FILE__, __LINE__, stdout, "Encoding:%s", 
					 					(trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__VideoEncoding__JPEG) ? "tt__VideoEncoding__JPEG" : 
										(trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__VideoEncoding__MPEG4) ? "tt__VideoEncoding__MPEG4" :
										(trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Encoding == tt__VideoEncoding__H264) ? "tt__VideoEncoding__H264" : "Error VideoEncoding");
			 processEventLog(__FILE__, __LINE__, stdout, "name:%s UseCount:%d token:%s\r\n", trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Name.c_str(),
										trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->UseCount, trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->token.c_str());
			 processEventLog(__FILE__, __LINE__, stdout, "Width:%d Height:%d\r\n", trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Resolution->Width,
					 					trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->Resolution->Height);

			if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, "admin", DEV_PASSWORD))
			{
				return -1;
			}

			_trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions = soap_new__trt__GetVideoEncoderConfigurationOptions(soap, -1);
			_trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse = soap_new__trt__GetVideoEncoderConfigurationOptionsResponse(soap, -1);

			trt__GetVideoEncoderConfigurationOptions->ConfigurationToken = &trt__GetVideoEncoderConfigurationsResponse->Configurations[i]->token;

			if (SOAP_OK == proxyMedia.GetVideoEncoderConfigurationOptions(trt__GetVideoEncoderConfigurationOptions, trt__GetVideoEncoderConfigurationOptionsResponse))
			{
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
	
	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyEvent.soap, NULL, "admin", DEV_PASSWORD))
	{
		return -1;
	}

	if (SOAP_OK != soap_wsse_add_Timestamp(proxyEvent.soap, "Time", 10)) 
	{
		return -1;
	}

	_tev__GetEventProperties *tev__GetEventProperties = soap_new__tev__GetEventProperties(soap, -1);
	_tev__GetEventPropertiesResponse *tev__GetEventPropertiesResponse = soap_new__tev__GetEventPropertiesResponse(soap, -1);

	if (SOAP_OK != soap_wsa_request(proxyEvent.soap, NULL, NULL, "http://www.onvif.org/ver10/events/wsdl/EventPortType/GetEventPropertiesRequest"))
	{
		return -1;
	}

	if (proxyEvent.GetEventProperties(tev__GetEventProperties, tev__GetEventPropertiesResponse) == SOAP_OK)
	{
		 processEventLog(__FILE__, __LINE__, stdout, "-------------------EventProperties-------------------");

		for (int i = 0; i < tev__GetEventPropertiesResponse->TopicNamespaceLocation.size(); i++)
		{
			 processEventLog(__FILE__, __LINE__, stdout, "TopicNamespaceLocation[%d]:%s", i, tev__GetEventPropertiesResponse->TopicNamespaceLocation[i].c_str());
		}
		

		for (int i = 0; i < tev__GetEventPropertiesResponse->MessageContentFilterDialect.size(); i++)
		{
			 processEventLog(__FILE__, __LINE__, stdout, "MessageContentFilterDialect[%d]:%s", i, tev__GetEventPropertiesResponse->MessageContentFilterDialect[i].c_str());
		}

		for (int i = 0; i < tev__GetEventPropertiesResponse->MessageContentSchemaLocation.size(); i++)
		{
			 processEventLog(__FILE__, __LINE__, stdout, "MessageContentSchemaLocation[%d]:%s", i, tev__GetEventPropertiesResponse->MessageContentSchemaLocation[i].c_str());
		}
	}
	else
	{
		PrintErr(proxyEvent.soap);
	}

	soap_destroy(soap);
	soap_end(soap); 
	
	//_tev__CreatePullPointSubscription *tev__CreatePullPointSubscription = soap_new__tev__CreatePullPointSubscription(soap,-1);
	//_tev__CreatePullPointSubscriptionResponse *tev__CreatePullPointSubscriptionResponse = soap_new__tev__CreatePullPointSubscriptionResponse(soap,-1);

	//if (SOAP_OK == proxyEvent.__ns11__CreatePullPointSubscription(tev__CreatePullPointSubscription,tev__CreatePullPointSubscriptionResponse))
	//{
	//	 processEventLog(__FILE__, __LINE__, stdout,"%s",tev__CreatePullPointSubscriptionResponse->SubscriptionReference.Address);
	//}
	//else
	//{
	//	PrintErr(proxyEvent.soap);	
	//}

	//soap_destroy(soap); // remove deserialized class instances (C++ only)
	//soap_end(soap); // clean up and remove deserialized data

	/*add by huguanjun 20140924*/
	/*
	proxyNP.endpoint = proxyEvent.soap_endpoint;
	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyNP.soap, NULL, "admin", DEV_PASSWORD))
	{
		return -1;
	}

	if (SOAP_OK != soap_wsse_add_Timestamp(proxyNP.soap, "Time", 10)) // 10 seconds lifetime	
	{
		return -1;
	}

	_ns1__Subscribe *ns1__Subscribe = soap_new__ns1__Subscribe(soap, -1);
	_ns1__SubscribeResponse *ns1__SubscribeResponse = soap_new__ns1__SubscribeResponse(soap, -1);
	ns1__Subscribe->ConsumerReference.Address = "http://192.168.17.133:8022/service";


	if (proxyNP.__ns13__Subscribe(ns1__Subscribe, ns1__SubscribeResponse) == SOAP_OK)
	{

	}
	else
	{
		PrintErr(proxyNP.soap);
	}

	soap_destroy(soap); // remove deserialized class instances (C++ only)
	soap_end(soap); // clean up and remove deserialized data
	*/
	/*end by huguanjun 20140924*/
	/*
	if (blSupportPTZ)
	{
		if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyPTZ.soap, NULL, "admin", DEV_PASSWORD))
		{
			return -1;
		}

		if (SOAP_OK != soap_wsse_add_Timestamp(proxyPTZ.soap, "Time", 10)) // 10 seconds lifetime	
		{
			return -1;
		}

		_tptz__GetNodes *tptz__GetNodes = soap_new__tptz__GetNodes(soap, -1);
		_tptz__GetNodesResponse *tptz__GetNodesResponse = soap_new__tptz__GetNodesResponse(soap, -1);
		if (proxyPTZ.GetNodes(tptz__GetNodes, tptz__GetNodesResponse) == SOAP_OK)
		{

		}
		else
		{
			PrintErr(proxyPTZ.soap);
		}

		soap_destroy(soap); // remove deserialized class instances (C++ only)
		soap_end(soap); // clean up and remove deserialized data

		if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyPTZ.soap, NULL, "admin", DEV_PASSWORD))
		{
			return -1;
		}

		_tptz__GetConfigurations *tptz__GetConfigurations = soap_new__tptz__GetConfigurations(soap, -1);
		_tptz__GetConfigurationsResponse *tptz__GetConfigurationsResponse = soap_new__tptz__GetConfigurationsResponse(soap, -1);

		if (proxyPTZ.GetConfigurations(tptz__GetConfigurations, tptz__GetConfigurationsResponse) == SOAP_OK)
		{
			if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyPTZ.soap, NULL, "admin", DEV_PASSWORD))
			{
				return -1;
			}

			_tptz__GetConfiguration *tptz__GetConfiguration = soap_new__tptz__GetConfiguration(soap, -1);
			_tptz__GetConfigurationResponse *tptz__GetConfigurationResponse = soap_new__tptz__GetConfigurationResponse(soap, -1);

			tptz__GetConfiguration->PTZConfigurationToken = tptz__GetConfigurationsResponse->PTZConfiguration[0]->token;
			if (proxyPTZ.GetConfiguration(tptz__GetConfiguration, tptz__GetConfigurationResponse) == SOAP_OK)
			{

			}
			else
			{
				PrintErr(proxyPTZ.soap);
			}

			_tptz__ContinuousMove *tptz__ContinuousMove = soap_new__tptz__ContinuousMove(soap, -1);
			_tptz__ContinuousMoveResponse *tptz__ContinuousMoveResponse = soap_new__tptz__ContinuousMoveResponse(soap, -1);

			tptz__ContinuousMove->ProfileToken = tptz__GetConfiguration->PTZConfigurationToken;
			tptz__ContinuousMove->Velocity = soap_new_tt__PTZSpeed(soap, -1);
			tptz__ContinuousMove->Velocity->PanTilt = soap_new_tt__Vector2D(soap, -1);

			HANDLE   g_hIn = GetStdHandle(STD_INPUT_HANDLE);
			INPUT_RECORD   iBuffer;
			DWORD   dwResult = 0;

			//------------------------Control PTZ------------------------//
			 processEventLog(__FILE__, __LINE__, stdout, "\r\n-------------------Control PTZ-------------------\r\n");
			 processEventLog(__FILE__, __LINE__, stdout, "\r\nUP:w DOWN:x LEFT:a RIGHT:d STOP:s\r\n");
			while (1)
			{
				ReadConsoleInput(g_hIn, &iBuffer, 1, &dwResult);
				//   some   a   key   was   pressed 
				if (iBuffer.EventType == KEY_EVENT   &&   iBuffer.Event.KeyEvent.bKeyDown)
				{
					if (iBuffer.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
					{
						break;
					}
					else if (87 == iBuffer.Event.KeyEvent.wVirtualKeyCode)//w
					{
						if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyPTZ.soap, NULL, "admin", DEV_PASSWORD))
						{
							return -1;
						}

						tptz__ContinuousMove->Velocity->PanTilt->x = 0;
						tptz__ContinuousMove->Velocity->PanTilt->y = 1;

						if (proxyPTZ.ContinuousMove(tptz__ContinuousMove, tptz__ContinuousMoveResponse) == SOAP_OK)
						{
						}
						else
						{
							PrintErr(proxyPTZ.soap);
						}
					}
					else if (65 == iBuffer.Event.KeyEvent.wVirtualKeyCode)//a
					{
						if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyPTZ.soap, NULL, "admin", DEV_PASSWORD))
						{
							return -1;
						}

						tptz__ContinuousMove->Velocity->PanTilt->x = 1;
						tptz__ContinuousMove->Velocity->PanTilt->y = 0;

						if (proxyPTZ.ContinuousMove(tptz__ContinuousMove, tptz__ContinuousMoveResponse) == SOAP_OK)
						{
						}
						else
						{
							PrintErr(proxyPTZ.soap);
						}

					}
					else if (68 == iBuffer.Event.KeyEvent.wVirtualKeyCode)//d
					{
						if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyPTZ.soap, NULL, "admin", DEV_PASSWORD))
						{
							return -1;
						}

						tptz__ContinuousMove->Velocity->PanTilt->x = -1;
						tptz__ContinuousMove->Velocity->PanTilt->y = 0;

						if (proxyPTZ.ContinuousMove(tptz__ContinuousMove, tptz__ContinuousMoveResponse) == SOAP_OK)
						{
						}
						else
						{
							PrintErr(proxyPTZ.soap);
						}
					}
					else if (83 == iBuffer.Event.KeyEvent.wVirtualKeyCode)//s
					{
						if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyPTZ.soap, NULL, "admin", DEV_PASSWORD))
						{
							return -1;
						}

						_tptz__Stop *tptz__Stop = soap_new__tptz__Stop(soap, -1);
						_tptz__StopResponse *tptz__StopResponse = soap_new__tptz__StopResponse(soap, -1);

						if (proxyPTZ.Stop(tptz__Stop, tptz__StopResponse) == SOAP_OK)
						{
						}
						else
						{
							PrintErr(proxyPTZ.soap);
						}

					}
					else if (88 == iBuffer.Event.KeyEvent.wVirtualKeyCode)//x
					{
						if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyPTZ.soap, NULL, "admin", DEV_PASSWORD))
						{
							return -1;
						}

						tptz__ContinuousMove->Velocity->PanTilt->x = 0;
						tptz__ContinuousMove->Velocity->PanTilt->y = -1;

						if (proxyPTZ.ContinuousMove(tptz__ContinuousMove, tptz__ContinuousMoveResponse) == SOAP_OK)
						{
						}
						else
						{
							PrintErr(proxyPTZ.soap);
						}
					}
				}
				Sleep(1);
			}
		}
		else
		{
			PrintErr(proxyPTZ.soap);
		}

		soap_destroy(soap); // remove deserialized class instances (C++ only)
		soap_end(soap); // clean up and remove deserialized data
	}


	getchar();
	soap_free(soap);//detach and free runtime context
	soap_done(soap); // detach context (last use and no longer in scope)
	*/

	return 0;
}

