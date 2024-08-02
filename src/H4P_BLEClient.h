#pragma once 

#include <H4Service.h>

#if H4P_BLE_AVAILABLE

#include <BLEDevice.h>
#include <map>
#include <set>


STAG(bleclt)

struct H4P_BLECharacteristic {
	bool found = false;
	bool mandatory;
	BLEUUID uuid;
	H4P_BLECharacteristic(): mandatory(false){}
	H4P_BLECharacteristic(BLEUUID uuid, bool mandatory = true) : uuid(uuid), mandatory(mandatory) {}
	~H4P_BLECharacteristic() = default;
	H4P_BLECharacteristic(H4P_BLECharacteristic& other) = default;
	H4P_BLECharacteristic(H4P_BLECharacteristic&& other) = default;
	H4P_BLECharacteristic& operator=(H4P_BLECharacteristic& other) = default;
	H4P_BLECharacteristic& operator=(H4P_BLECharacteristic&& other) = default;
	bool operator==(H4P_BLECharacteristic& other) { return uuid.equals(other.uuid); };
	bool operator==(const BLEUUID& other) const { return const_cast<BLEUUID&>(uuid).equals(other);};

	bool operator<(H4P_BLECharacteristic& other) { return std::string(other.uuid.toString().c_str()) < std::string(uuid.toString().c_str()); };
	bool operator<(const BLEUUID& other) const { return std::string(const_cast<BLEUUID&>(other).toString().c_str()) < std::string(const_cast<BLEUUID&>(uuid).toString().c_str());};


	BLERemoteCharacteristic* remote;

};

struct H4P_BLEService {
	bool found = false;
	bool mandatory;
	BLEUUID uuid;
	H4P_BLEService() : mandatory(false) {}
	H4P_BLEService(BLEUUID uuid, bool mandatory = true) : uuid(uuid), mandatory(mandatory) {}
	~H4P_BLEService() = default;
	H4P_BLEService(H4P_BLEService& other) = default;
	H4P_BLEService(H4P_BLEService&& other) = default;
	H4P_BLEService& operator=(H4P_BLEService& other) = default;
	H4P_BLEService& operator=(H4P_BLEService&& other) = default;
	bool operator==(const H4P_BLEService& other) { return uuid.equals(other.uuid); };
	bool operator==(const BLEUUID& other) { return uuid.equals(other); };
	bool operator<(const H4P_BLEService& other) const { return const_cast<BLEUUID&>(uuid).toString() < const_cast<H4P_BLEService&>(other).uuid.toString(); }


};
class H4P_BLEClient : public H4Service {

		static 	BLEClient*		h4Client;
		BLERemoteService* 		h4RemoteService;
		BLEScan* 				_scan;
		BLEAdvertisedDevice*	_advertised;
		bool 					_connected;

		H4_FN_VOID 				_cbConnect=nullptr;
		H4_FN_VOID 				_cbDisconnect=nullptr;
		bool 					_shoudStart=false;

		H4_TIMER 				_connector=nullptr;
		
		friend class H4ClientCallbacks;

				void			onConnect(BLEClient* pclient) { SYSINFO("CNX"); if (_cbConnect) _cbConnect(); /* h4.cancel(_connector); */ }
				void 			onDisconnect(BLEClient* pclient) {
					_connected = false;
					SYSINFO("DCX");
					// _connector = h4.every(5000, [this]{ Serial.printf("_connector cb _connected %d _shoudStart %d\n", _connected, _shoudStart); if (!_connected && _shoudStart) svcUp(); });
					// Serial.printf("_connector %p\n", _connector);
					if(_cbDisconnect) _cbDisconnect();
					svcUp();
				}

	std::map<H4P_BLEService, 
				std::vector<H4P_BLECharacteristic>> 	_servicesMap;
	std::map<H4P_BLEService, notify_callback> 			_servicesNotify;


	

	friend class H4AdvertisedDeviceCallbacks;
				void 		_init() override;
				void 		svcUp() override;
				void 		svcDown() override;
				void 		_handleEvent(const std::string& svc, H4PE_TYPE t, const std::string& msg) override;

				bool 		_connectToServer();
				bool 		_findCharacteristic(BLERemoteService* pRemoteService, BLERemoteCharacteristic* l_BLERemoteChar);
				void 		_notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
				void 		_resetRemoteChars() {
								for (auto& s : _servicesMap) {
									for (auto& c : s.second) {
										c.remote = nullptr; // [ ] delete something?
									}
								}
							}
	public: 
		H4P_BLEClient() : H4Service(blecltTag(), H4PE_GVCHANGE /* [ ] ... */) {

		}

				void 			start() { _shoudStart=true; svcUp(); }
				void 			stop() { _shoudStart=false; svcDown(); }
				void 			add(std::pair<BLEUUID,bool> service, std::initializer_list<std::pair<BLEUUID, bool>> characteristics, notify_callback onNotify = nullptr);
				void 			removeService(BLEUUID service);
				void 			removeCharacteristic(BLEUUID service, const BLEUUID& characteristic);
				void 			removeCharacteristics(BLEUUID service, const std::vector<BLEUUID>& characteristics);
				void 			clearServices() { _servicesMap.clear(); } // [ ] Check if the library holds any footprint of them.
				void 			setCallbacks(H4_FN_VOID onConnect, H4_FN_VOID onDisconnect) // [ ] Should change to h4 events.
								{ _cbConnect = onConnect; _cbDisconnect = onDisconnect; }
	

				BLERemoteCharacteristic* getRemoteCharacteristic(BLEUUID s, BLEUUID c) {
					if (_servicesMap.count(s)) {
						auto& cs = _servicesMap[s];
						auto it = std::find(cs.begin(), cs.end(), c);
						if (it != cs.end()) return it->remote;
					}
					return nullptr;
				}
};



#endif