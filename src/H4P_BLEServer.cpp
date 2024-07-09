#include <H4P_BLEServer.h>

#if H4P_BLE_AVAILABLE
#include <H4P_SerialCmd.h>
#include <esp_gatts_api.h>
#include <BLEUtils.h>

void H4P_BLEServer::H4BLEServerCallbacks::onConnect(BLEServer *pServer, esp_ble_gatts_cb_param_t *param) {
	h4puncheckedcall<H4P_BLEServer>(blesrvTag())->onConnect(param);
}
void H4P_BLEServer::H4BLEServerCallbacks::onDisconnect(BLEServer *pServer, esp_ble_gatts_cb_param_t *param) {
	h4puncheckedcall<H4P_BLEServer>(blesrvTag())->onDisconnect(param);
}
void H4P_BLEServer::H4BLEServerCallbacks::onMtuChanged(BLEServer *pServer, esp_ble_gatts_cb_param_t *param) {
	Serial.printf("H4BLEServerCallbacks", "Device: %s MTU: %d", BLEDevice::toString().c_str(), param->mtu.mtu);
}

void H4P_BLEServer::H4CmdCharacteristicCallbacks::onWrite(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t *param) {
	// Redirect the command to execCmd();
	std::string cmd{pCharacteristic->getValue().c_str()};
	std::string input(blesrvTag()); input.append("/").append(cmd);
	Serial.printf("BLESRV onWrite [%s]\n", input.c_str());
	h4puncheckedcall<H4P_BLEServer>(blesrvTag())->h4CmdCharacteristic->setValue("");
	h4p.invokeCmd(std::string(blesrvTag()).append("/").append(cmd),"",blesrvTag());
}
void H4P_BLEServer::H4ReplyCharacteristicCallbacks::onRead(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t *param) {
	Serial.printf("H4ReplyDesc...::onRead()\n");
}

void H4P_BLEServer::onConnect(esp_ble_gatts_cb_param_t *param) {
	connected = true;
	H4Service::svcUp();
	esp_ble_gatts_cb_param_t::gatts_connect_evt_param connect = param->connect;
	conn_id = connect.conn_id;
	uint8_t link_role = connect.link_role;
	auto remote_bda = connect.remote_bda;
	esp_gatt_conn_params_t conn_params = connect.conn_params;
	esp_ble_addr_type_t ble_addr_type = connect.ble_addr_type;
	uint16_t conn_handle = connect.conn_handle;
	Serial.printf("BLESRV onConnect() cid [%d] lr [%s] rmt [%s] cp->intvl [%u] cp->ltnc [%u] cp->TO [%u] addr_type [%s] cnhndl [%u]\n", conn_id, 
																														link_role ? "SLAVE" : "MASTER", 
																														BLEAddress(remote_bda).toString().c_str(), 
																														conn_params.interval,
																														conn_params.latency,
																														conn_params.timeout,
																														BLEUtils::addressTypeToString(ble_addr_type),
																														conn_handle
																														);

/* ESP_GATTS_CONNECT_EVT
    struct gatts_connect_evt_param {
        uint16_t conn_id;               // Connection id
        uint8_t link_role;              // Link role : master role = 0  ; slave role = 
        esp_bd_addr_t remote_bda;       // Remote bluetooth device address
        esp_gatt_conn_params_t conn_params; // current Connection parameters
        esp_ble_addr_type_t ble_addr_type;  // Remote BLE device address type
        uint16_t conn_handle;           // HCI connection handle
    } connect;                          // Gatt server callback param of ESP_GATTS_CONNECT_EVT
*/
}
void H4P_BLEServer::onDisconnect(esp_ble_gatts_cb_param_t *param) {
	connected = false;
	H4Service::svcDown();
	auto disconnect = param->disconnect;
	uint16_t conn_id = disconnect.conn_id;
	auto remote_bda = disconnect.remote_bda;
	auto reason = disconnect.reason;
	Serial.printf("BLESRV onDisconnect() cid [%d] rmt [%s] rsn [%s]\n", conn_id, BLEAddress(remote_bda).toString().c_str(), BLEUtils::gattCloseReasonToString(reason).c_str());

	if (H4P_BLEServer::conn_id == conn_id) H4P_BLEServer::conn_id = 0;



/* ESP_GATTS_DISCONNECT_EVT
    struct gatts_disconnect_evt_param {
        uint16_t conn_id;               // Connection id
        esp_bd_addr_t remote_bda;       // Remote bluetooth device address
        esp_gatt_conn_reason_t reason;  // Indicate the reason of disconnection
    } disconnect;
*/
}

void H4P_BLEServer::_handleEvent(const std::string& svc,H4PE_TYPE t,const std::string& msg) 
{
	switch(t){
        case H4PE_VIEWERS:
            {
                    h4puiAdd(blesrvTag(),H4P_UI_BOOL,"b", "", H4P_UILED_BLUE); // OR H4P_UI_IMGBTN // 3 state-> (Disabled / Enabled / Connected)
                    // h4puiAdd(stateTag(),H4P_UI_IMGBTN,"b");
            }
            break;
        case H4PE_GVCHANGE:
			if (svc == nameTag())
				h4DeviceNameCharacteristic->setValue(msg.c_str());
			else if (svc == deviceTag() && _running) _restart();
			break;
		case H4PE_SERVICE:
		// if (connected && (svc == MQTT || svc == WiFi)) --> notify
			break;
		case H4PE_REBOOT:
			// if (connected) Notify(rebooting)
			break;
		case H4PE_FACTORY:
			// if (connected) Notify(factory)
			break;
		// case H4PE_GPIO:
		// 	// if (connected) Notify(factory)
    }

			
}

void H4P_BLEServer::_reply(std::string msg) {
	Serial.printf("H4P_BLEServer::_reply(%s)\n", msg.data());
	h4ReplyCharacteristic->setValue(String(msg.data()));
	h4ReplyCharacteristic->notify();
}

void H4P_BLEServer::_init() {
	auto b = BLEDevice::setMTU(H4_BLE_MTU);
	Serial.printf("setMTU(%u) -> %d\n", H4_BLE_MTU, b);
	// auto tag = h4p.gvExists(nameTag()) ? h4p[nameTag()] : h4p[deviceTag()];
	auto tag = h4p[deviceTag()];
	BLEDevice::init(tag.c_str());

	//
	// BLEServer
	//
	h4Server = BLEDevice::createServer();
	h4Server->setCallbacks(new H4BLEServerCallbacks());

/* BLEServer APIs
	uint32_t 					BLEServer::getConnectedCount();
	BLEService *				BLEServer::createService(const char *uuid);
	BLEService *				BLEServer::createService(BLEUUID uuid, uint32_t numHandles = 15, uint8_t inst_id = 0);
	BLEAdvertising *			BLEServer::getAdvertising();
	void 						BLEServer::setCallbacks(BLEServerCallbacks *pCallbacks);
	void 						BLEServer::startAdvertising();
	void 						BLEServer::removeService(BLEService *service);
	BLEService *				BLEServer::getServiceByUUID(const char *uuid);
	BLEService *				BLEServer::getServiceByUUID(BLEUUID uuid);
	bool 						BLEServer::connect(BLEAddress address);
	void 						BLEServer::disconnect(uint16_t connId);
	uint16_t 					BLEServer::m_appId;
	void 						BLEServer::updateConnParams(esp_bd_addr_t remote_bda, uint16_t minInterval, uint16_t maxInterval, uint16_t latency, uint16_t timeout);

	// multi connection support //
	std::map<uint16_t, conn_status_t> BLEServer::getPeerDevices(bool client);
	void 						BLEServer::addPeerDevice(void *peer, bool is_client, uint16_t conn_id);
	bool 						BLEServer::removePeerDevice(uint16_t conn_id, bool client);
	BLEServer *				BLEServer::getServerByConnId(uint16_t conn_id);
	void 						BLEServer::updatePeerMTU(uint16_t connId, uint16_t mtu);
	uint16_t 					BLEServer::getPeerMTU(uint16_t conn_id);
	uint16_t 					BLEServer::getConnId();
*/

	//
	// Services.
	//
	h4Service = h4Server->createService(H4_SERVICE_UUID);
/* BLEService APIs
	void 				BLEService::addCharacteristic(BLECharacteristic *pCharacteristic);
	BLECharacteristic *BLEService::createCharacteristic(const char *uuid, uint32_t properties);
	BLECharacteristic *BLEService::createCharacteristic(BLEUUID uuid, uint32_t properties);
	void 				BLEService::dump();
	void 				BLEService::executeCreate(BLEServer *pServer);
	void 				BLEService::executeDelete();
	BLECharacteristic *BLEService::getCharacteristic(const char *uuid);
	BLECharacteristic *BLEService::getCharacteristic(BLEUUID uuid);
	BLEUUID 			BLEService::getUUID();
	BLEServer *		BLEService::getServer();
	void 				BLEService::start();
	void 				BLEService::stop();
	String 			BLEService::toString();
	uint16_t 			BLEService::getHandle();
	uint8_t 			BLEService::m_instId = 0;
*/
	//
	// Characteristics
	//

							/*  CMD */

	h4DeviceNameCharacteristic = h4Service->createCharacteristic(BLEUUID((uint16_t)ESP_GATT_UUID_GAP_DEVICE_NAME), BLECharacteristic::PROPERTY_READ);
	// if (h4p.gvExists(nameTag())) {
	h4DeviceNameCharacteristic->setValue(h4p[nameTag()].c_str()); // Fail safe, returns "" if not found.
	// } 

	h4CmdCharacteristic = h4Service->createCharacteristic(CMD_CHAR_UUID, BLECharacteristic::PROPERTY_WRITE);

	// Creates BLE Descriptor 0x2902: Client Characteristic Configuration Descriptor (CCCD)
	h4CmdCharacteristic->addDescriptor(new BLE2902()); // [ ] Investigate/read more about it.
	// Adds also the Characteristic User Description - 0x2901 descriptor
	h4CmdDescriptor_2901 = new BLE2901();
	h4CmdDescriptor_2901->setDescription("H4-Style Commands");
	h4CmdDescriptor_2901->setAccessPermissions(ESP_GATT_PERM_READ);
/* esp_gatt_perm_t
	ESP_GATT_PERM_READ
	ESP_GATT_PERM_READ_ENCRYPTED
	ESP_GATT_PERM_READ_ENC_MITM
	ESP_GATT_PERM_WRITE
	ESP_GATT_PERM_WRITE_ENCRYPTED
	ESP_GATT_PERM_WRITE_ENC_MITM
	ESP_GATT_PERM_WRITE_SIGNED
	ESP_GATT_PERM_WRITE_SIGNED_MITM
	ESP_GATT_PERM_READ_AUTHORIZATION
	ESP_GATT_PERM_WRITE_AUTHORIZATION
*/
	h4CmdCharacteristic->addDescriptor(h4CmdDescriptor_2901);

	h4CmdCharacteristic->setCallbacks(new H4CmdCharacteristicCallbacks);


							/*  REPLY */
							
	h4ReplyCharacteristic = h4Service->createCharacteristic(REPLY_CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
	// Adds also the Characteristic User Description - 0x2901 descriptor
	h4ReplyDescriptor_2901 = new BLE2901();
	h4ReplyDescriptor_2901->setDescription("Reply to H4-Style Commands");
	h4ReplyDescriptor_2901->setAccessPermissions(ESP_GATT_PERM_READ);  // enforce read only - default is Read|Write
	h4ReplyCharacteristic->addDescriptor(h4ReplyDescriptor_2901);
	// Creates BLE Descriptor 0x2902: Client Characteristic Configuration Descriptor (CCCD)
	h4ReplyDescriptor_2902 = new BLE2902();
	h4ReplyDescriptor_2902->setNotifications(true);

	h4ReplyCharacteristic->addDescriptor(h4ReplyDescriptor_2902);

	h4ReplyCharacteristic->setCallbacks(new H4ReplyCharacteristicCallbacks);
	
							/*  EVENTS */
							
	h4EventsCharacteristic = h4Service->createCharacteristic(EVENTS_CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY);
	// Adds also the Characteristic User Description - 0x2901 descriptor
	h4EventsDescriptor_2901 = new BLE2901();
	h4EventsDescriptor_2901->setDescription("H4 Events");
	h4EventsDescriptor_2901->setAccessPermissions(ESP_GATT_PERM_READ);  // enforce read only - default is Read|Write
	h4EventsCharacteristic->addDescriptor(h4EventsDescriptor_2901);
	// Creates BLE Descriptor 0x2902: Client Characteristic Configuration Descriptor (CCCD)
	h4EventsDescriptor_2902 = new BLE2902();
	h4EventsDescriptor_2902->setNotifications(true);

	h4EventsCharacteristic->addDescriptor(h4EventsDescriptor_2902);

	// h4EventsCharacteristic->setCallbacks(new H4EventsDescriptorCallbacks);

/* BLECharacteristic APIs
	void BLECharacteristic::addDescriptor();
	void BLECharacteristic::indicate();
	void BLECharacteristic::notify(bool is_notification = true);
	void BLECharacteristic::setBroadcastProperty(bool value);
	void BLECharacteristic::setCallbacks(BLECharacteristicCallbacks *pCallbacks);
	void BLECharacteristic::setIndicateProperty(bool value);
	void BLECharacteristic::setNotifyProperty(bool value);
	void BLECharacteristic::setReadProperty(bool value);
	void BLECharacteristic::setValue(uint8_t *data, size_t size);
	void BLECharacteristic::setValue(String value);
	void BLECharacteristic::setValue(uint16_t &data16);
	void BLECharacteristic::setValue(uint32_t &data32);
	void BLECharacteristic::setValue(int &data32);
	void BLECharacteristic::setValue(float &data32);
	void BLECharacteristic::setValue(double &data64);
	void BLECharacteristic::setWriteProperty(bool value);
	void BLECharacteristic::setWriteNoResponseProperty(bool value);
	String BLECharacteristic::toString();
	uint16_t BLECharacteristic::getHandle();
	void BLECharacteristic::setAccessPermissions(esp_gatt_perm_t perm);
*/

	//
	//	Emit Event: H4PE_BLEServerInit
	//


	//
	// Start Advertizing
	//
		// [ ] Move to svcUp/svcDown ?
	h4Advertising = BLEDevice::getAdvertising();
	h4Advertising->addServiceUUID(H4_SERVICE_UUID);
	h4Advertising->setScanResponse(true);
	h4Advertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
	h4Advertising->setMinPreferred(0x12);
	svcUp();
	
}



void H4P_BLEServer::svcDown() {
	Serial.printf("BLESRV svcDown connected %d conn_id %d\n", connected, conn_id);
	// Stop advertizsing
	BLEDevice::stopAdvertising();
	// stop/disconnect
	if (connected && conn_id) {
		h4Server->disconnect(conn_id); // [ ] Perhaps advanced management of conn_ids (std::set<uint16_t>::insert onConnect/erase onDisconnect)
	}
	h4Service->stop();
	H4Service::svcDown();
};
void H4P_BLEServer::svcUp() {
	Serial.printf("BLESRV svcUp\n", connected, conn_id);
	// Start advertizsing
	h4Service->start();
	BLEDevice::startAdvertising();
};
#endif

