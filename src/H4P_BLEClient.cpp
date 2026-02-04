/* 
MIT License

Copyright (c) 2026 H4Group

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Contact Email: TBD
*/
#include "H4P_BLEClient.h"

#if H4P_BLE_AVAILABLE
#include <H4P_BLECommon.h>

class H4AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
		static std::set<H4P_BLEClient*> clients;
		void 	onResult(BLEAdvertisedDevice advertised);
		H4AdvertisedDeviceCallbacks() {}
		H4AdvertisedDeviceCallbacks(const H4AdvertisedDeviceCallbacks&) = delete;
		H4AdvertisedDeviceCallbacks(const H4AdvertisedDeviceCallbacks&&) = delete;
		H4AdvertisedDeviceCallbacks operator =(const H4AdvertisedDeviceCallbacks&) = delete;
		H4AdvertisedDeviceCallbacks operator =(const H4AdvertisedDeviceCallbacks&&) = delete;
	public:
		static std::set<H4P_BLEClient*> getClients() { return clients; }
		static void addClient(H4P_BLEClient* c) { clients.insert(c); }
		static void removeClient(H4P_BLEClient* c) { clients.erase(c); }
		static H4AdvertisedDeviceCallbacks& GetInstance(H4P_BLEClient* c=nullptr){
			static H4AdvertisedDeviceCallbacks instance;
			if (c) addClient(c);
			return instance;
		}
};
class H4ClientCallbacks : public BLEClientCallbacks {
	H4P_BLEClient* client;
	void onConnect(BLEClient* pclient) {
		H4PBC_PRINTF("H4ClientCallbacks::onConnect(%p)\n", pclient);
	}

	void onDisconnect(BLEClient *pclient) {
		H4PBC_PRINTF("H4ClientCallbacks::onDisconnect()\n");
		client->onDisconnect(pclient);
	}

	friend class H4P_BLEClient;

	public:
	H4ClientCallbacks() {}
	H4ClientCallbacks& set(H4P_BLEClient* h4c) { client = h4c; return *this; }
} clientCallbacks;
BLEClient*	H4P_BLEClient::h4Client = nullptr;
std::set<H4P_BLEClient*> H4AdvertisedDeviceCallbacks::clients;

void H4AdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertised)
{
	// [ ] check if advertized
	H4PBC_PRINTF("H4AdvertisedDeviceCallbacks::onResult(%s)\n", advertised.getServiceUUID().toString().c_str());
	for (auto& client:H4AdvertisedDeviceCallbacks::getClients()) {

		if (client->_servicesMap.empty()) {
			H4PBC_PRINTF("client %p does not specified any service to connect to, it will not connect\n");
			continue;
		}
		bool continueToNext = false;
		for (auto& s : client->_servicesMap){
			const auto& service = s.first;
			auto foundService = advertised.haveServiceUUID() && advertised.isAdvertisingService(service.uuid);
			if (service.mandatory && !foundService)					{
				H4PBC_PRINTF("Service %s is mandatory, it's not found\n", const_cast<H4P_BLEService&>(service).uuid.toString().c_str());
				continueToNext = true;
				break;
			}
		}
		if (continueToNext) continue;

		H4PBC_PRINTF("All services are found, connecting...\n");

		BLEDevice::getScan()->stop();
		
		if (client->_advertised) delete client->_advertised;
		client->_advertised = new BLEAdvertisedDevice(advertised); // where it's deleted?
		h4.queueFunction([client]{
			if (client->_connectToServer()) client->onConnect(nullptr);
		});
		return; // because the advertised device was correctly found for one client. 
	}

}
void H4P_BLEClient::_init()
{
	H4PBC_PRINTF("H4P_BLEClient::_init() this=%p\n", this);
	H4P_BLE::init();
	h4Client = BLEDevice::createClient();
	BLEDevice::setMTU(H4_BLE_MTU);
	_scan = BLEDevice::getScan();
	_scan->setAdvertisedDeviceCallbacks(&H4AdvertisedDeviceCallbacks::GetInstance(this));
	_scan->setInterval(H4BC_SCAN_INTERVAL);
	_scan->setWindow(H4BC_SCAN_WINDOW);
	_scan->setActiveScan(H4BC_ACTIVE_SCAN);

}
void H4P_BLEClient::svcUp()
{
	H4PBC_PRINTF("H4P_BLEClient::svcUp() _shouldStart %d\n", _shoudStart);
	if (_shoudStart){
#ifdef SOC_BLE_50_SUPPORTED
		static bool started = false;
		if (!started) {
			_scan->start(H4BC_SCAN_DURATION, H4BC_SCAN_CONTINUOUS);
		} else { 
			_scan->startExtScan(H4BC_SCAN_DURATION, H4BC_SCAN_INTERVAL);
			started = true;
		}
#else
		_scan->start(H4BC_SCAN_DURATION, H4BC_SCAN_CONTINUOUS);
#endif
		
		auto res = h4Client->setMTU(H4_BLE_MTU);
		H4PBC_PRINTF("setMTU->%d\n", res);
	}
	H4PBC_PRINTF("svcUp() END\n");
}
void H4P_BLEClient::svcDown()
{
	H4PBC_PRINTF("H4P_BLEClient::svcDown()\n");
	H4Service::svcDown();
	if (_connected) {
		h4Client->disconnect();
		_connected = false;
		// [ ] The library should callback onDisconnect()
	}
#ifdef SOC_BLE_50_SUPPORTED
	if (_scan) _scan->stopExtScan();
#endif
}
void H4P_BLEClient::_handleEvent(const std::string &svc, H4PE_TYPE t, const std::string &msg)
{
}
bool H4P_BLEClient::_connectToServer()
{
	H4PBC_PRINTF("Forming a connection to %s\n", _advertised->getAddress().toString().c_str());

	H4PBC_PRINTF(" - Created client\n");

	h4Client->setClientCallbacks(&clientCallbacks.set(this));

	// Connect to the remove BLE Server.
	auto b = h4Client->connect(_advertised);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
	if (!b) {
		H4PBC_PRINTF(" Connecting to server Failed!\n");
		return false;
	}

	// Obtain a reference to the service we are after in the remote BLE server.
	auto services = h4Client->getServices();
	for (auto& svcTree: _servicesMap) {
		auto uuidStr = std::string(const_cast<H4P_BLEService&>(svcTree.first).uuid.toString().c_str());
		if (services->count(uuidStr)){
			auto remoteService = (*services)[uuidStr];
			H4PBC_PRINTF(" - Found our service\n");

			// auto chars = remoteService->getCharacteristics();
			for (auto& c:svcTree.second) {
				c.remote = remoteService->getCharacteristic(c.uuid);
				if (c.mandatory && _findCharacteristic(remoteService, c.remote) == false) {
					H4PBC_PRINTF("Missing characteristic %s\n", c.uuid.toString().c_str());
					h4Client->disconnect();
					_resetRemoteChars();
					return false;
				}
			}
			_connected = true;
			H4Service::svcUp();
			return true;
		}
		else if (svcTree.first.mandatory){
			H4PBC_PRINTF("Missing service %s !", uuidStr.c_str());
			h4Client->disconnect();
			return false;
		}
	}
	return false;
}
bool H4P_BLEClient::_findCharacteristic(BLERemoteService *pRemoteService, BLERemoteCharacteristic *l_BLERemoteChar)
{	
	// Obtain a reference to the characteristic in the service of the remote BLE server.
	if (l_BLERemoteChar == nullptr) {
		H4PBC_PRINTF("Failed to find one of the characteristics");
		H4PBC_PRINTF(l_BLERemoteChar->getUUID().toString().c_str());
		return false;
	}
	H4PBC_PRINTF(" - Found characteristic: \n" + String(l_BLERemoteChar->getUUID().toString().c_str()));

	if(l_BLERemoteChar->canNotify())
		l_BLERemoteChar->registerForNotify([this](BLERemoteCharacteristic* rc, uint8_t* d, size_t l, bool n){_notifyCallback(rc,d,l,n);});

	return true;
}
void H4P_BLEClient::_notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
	// [x] Find the services key
	for (auto& svcTree : _servicesMap) {
		auto& chars = svcTree.second;
		auto it = std::find(chars.begin(), chars.end(), pBLERemoteCharacteristic->getUUID());
		if (it != chars.end()) {
			// Use the svcTree.first as the key.
			if (_servicesNotify.count(svcTree.first)) {
				_servicesNotify.at(svcTree.first)(pBLERemoteCharacteristic, pData, length, isNotify);
				// _servicesNotify[svcTree.first](pBLERemoteCharacteristic, pData, length, isNotify);
				return;
			}
		}
	}
}

void H4P_BLEClient::add(std::pair<BLEUUID,bool> service, std::initializer_list<std::pair<BLEUUID, bool>> characteristics, notify_callback onNotify)
{
	std::vector<H4P_BLECharacteristic> chars;
	for (const auto& c:characteristics) chars.push_back({c.first,c.second});
	_servicesMap.insert_or_assign({service.first,service.second}, std::move(chars));
	H4PBC_PRINTF("add().. _servicesMap.size() %d this=%p\n", _servicesMap.size(), this);

	if (onNotify) _servicesNotify.insert_or_assign({service.first,service.second}, onNotify);
}
void H4P_BLEClient::removeService(BLEUUID service)
{
	_servicesMap.erase(service);
	if (_servicesNotify.count(service)) _servicesNotify.erase(service); 
}

void H4P_BLEClient::removeCharacteristic(BLEUUID service, const BLEUUID &characteristic)
 {
	if (_servicesMap.count(service)){ 
		auto& chars = _servicesMap[service];
		chars.erase(std::remove(chars.begin(), chars.end(), characteristic));
	}
}

void H4P_BLEClient::removeCharacteristics(BLEUUID service, const std::vector<BLEUUID> &characteristics)
{
	for (const auto &c : characteristics){
		removeCharacteristic(service,c);
	}
}

#endif

