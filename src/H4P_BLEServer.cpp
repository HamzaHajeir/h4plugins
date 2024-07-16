#include <H4P_BLEServer.h>

#if H4P_BLE_AVAILABLE

namespace H4P_BLE {
bool initialized = false;
}
H4P_UI_LIST                     h4pBLEUserItems;
std::vector<std::string>        h4pBLEUIorder;

#include <H4P_SerialCmd.h>
#include <esp_gatts_api.h>
#include <BLEUtils.h>
#include <H4P_PinMachine.h>
#include <H4P_BLECommon.h>

void H4P_BLE::init() {
	if (!initialized) {
		auto tag = h4p[deviceTag()];
		BLEDevice::init(tag.c_str());
		initialized = true;
	}
}

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
	Serial.printf("BLESRV onWrite [%s]\n", cmd.c_str());
	auto srv = h4puncheckedcall<H4P_BLEServer>(blesrvTag());
	srv->h4Characteristics[H4P_H4BLECharacteristic::CMD].characteristic->setValue("");
	h4.queueFunction([srv, cmd = std::move(cmd)]
					 { auto r = h4p.invokeCmd(cmd, "", blesrvTag());
					 	if (r==H4_CMD_OK) {srv->h4Characteristics[H4P_H4BLECharacteristic::REPLY].characteristic->setValue("0"); srv->h4Characteristics[H4P_H4BLECharacteristic::REPLY].characteristic->notify(); }
					  });
}
void H4P_BLEServer::H4ReplyCharacteristicCallbacks::onRead(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t *param) {
	Serial.printf("H4ReplyDesc...::onRead()\n");
}

void H4P_BLEServer::onConnect(esp_ble_gatts_cb_param_t *param) {
	connected = true;
	H4Service::svcUp();
	h4p[_me]=stringFromInt(_running);
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
	uint16_t bleID = h4Server->getConnId();
    h4Server->updatePeerMTU(bleID, H4_BLE_MTU);
	SYSINFO("CNX by %s",CSTR(BLEAddress(remote_bda).toString()));
    Serial.printf("updateMTU to: %i\n", h4Server->getPeerMTU(bleID));

	_sendElems();

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
	h4p[_me]=stringFromInt(_running);

	auto disconnect = param->disconnect;
	uint16_t conn_id = disconnect.conn_id;
	auto remote_bda = disconnect.remote_bda;
	auto reason = disconnect.reason;
	Serial.printf("BLESRV onDisconnect() cid [%d] rmt [%s] rsn [%s]\n", conn_id, BLEAddress(remote_bda).toString().c_str(), BLEUtils::gattCloseReasonToString(reason).c_str());
	SYSINFO("DCX by %s",CSTR(BLEAddress(remote_bda).toString()));

	if (H4P_BLEServer::conn_id == conn_id) H4P_BLEServer::conn_id = 0;

	h4Server->startAdvertising();



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
                    h4puiAdd(_me,H4P_UI_BOOL,"b", "", H4P_UILED_BLUE); // OR H4P_UI_IMGBTN // 3 state-> (Disabled / Enabled / Connected)
					h4p[_me]=stringFromInt(_running);
            }
            break;
        case H4PE_GVCHANGE:
			if (svc == nameTag())
				h4Characteristics[H4P_H4BLECharacteristic::DEVICE_NAME].characteristic->setValue(msg.c_str());
			else if (svc == deviceTag() && _running) restart();
// #if H4P_USE_WIFI_AP // [ ] The WiFi/MQTT provisioning.
//             if(svc == GoTag() && STOI(msg)) {
//                 HAL_WIFI_startSTA();
//                 h4.once(1000,[]{ h4pevent(h4pSrc,H4PE_REBOOT,GoTag()); }); // P for holdoff value?
//                 return;
//             }
// #endif
            _defaultSync(svc,msg);
			break;

			// break;
		case H4PE_REBOOT:
			// if (connected) Notify(rebooting)
			break;
		case H4PE_FACTORY:
			// if (connected) Notify(factory)
			break;
		case H4PE_BLEADD:
            __elemAdd(msg);
            break;
        case H4PE_GPIO:
        case H4PE_BLESYNC:
            if(h4pBLEUserItems.count(svc)) _elemSetValue(svc, msg);
            break;

		// case H4PE_GPIO:
		// 	// if (connected) Notify(factory)
    }
}

void H4P_BLEServer::_reply(std::string msg) {
	Serial.printf("H4P_BLEServer::_reply(%s)\n", msg.data());
	h4Characteristics[H4P_H4BLECharacteristic::REPLY].characteristic->setValue(String(msg.data()));
	h4Characteristics[H4P_H4BLECharacteristic::REPLY].characteristic->notify();
}

void H4P_BLEServer::notify(const std::string &key, const std::string &value)
{
	if (connected) h4Characteristics[H4P_H4BLECharacteristic::UIDATA].characteristic->setValue(join({key,value}, UNIT_SEPARATOR).c_str());
}

void H4P_BLEServer::_addCharacteristic(H4P_H4BLECharacteristic charId, BLEUUID uuid, std::uint32_t properties, const std::string& description, H4P_Descriptor2902 desc2902Opts, const std::string& value, BLECharacteristicCallbacks *cb) {
	auto& charItem = h4Characteristics[charId];
	auto& h4char = charItem.characteristic;
	auto& desc2901 = charItem.descriptor_2901;
	auto& desc2902 = charItem.descriptor_2902;
	h4char = h4Service->createCharacteristic(uuid, properties);
	Serial.printf("_addCharacteristic(%d, %s, %lu, %s) - > %p\n", charId, uuid.toString().c_str(), properties, description.c_str(), h4char);

	// Creates BLE Descriptor 0x2902: Client Characteristic Configuration Descriptor (CCCD)
	if (desc2902Opts != H4P_Descriptor2902::DONT_ADD) {
		desc2902 = new BLE2902();
		if (desc2902Opts == H4P_Descriptor2902::SETNOTIFY) {
			desc2902->setNotifications(true);
		}
		h4char->addDescriptor(desc2902);
	}
	if (description.length()) {
		// Adds the Characteristic User Description - 0x2901 descriptor
		desc2901 = new BLE2901();
		desc2901->setDescription(description.c_str());
		desc2901->setAccessPermissions(ESP_GATT_PERM_READ);
		h4char->addDescriptor(desc2901);
	}

	if (value.length()) {
		h4char->setValue(value.c_str());
	}
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

	if (cb) {
		h4char->setCallbacks(cb);
	}
}

void H4P_BLEServer::_init() {
	H4P_BLE::init();
	auto b = BLEDevice::setMTU(H4_BLE_MTU);
	Serial.printf("setMTU(%u) -> %d MTU=%u\n", H4_BLE_MTU, b, BLEDevice::getMTU());

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
	h4Service = h4Server->createService(BLEUUID(H4_SERVICE_UUID), 30);
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
							/*  Device Name */
	_addCharacteristic(H4P_H4BLECharacteristic::DEVICE_NAME, BLEUUID((uint16_t)ESP_GATT_UUID_GAP_DEVICE_NAME), BLECharacteristic::PROPERTY_READ, "", H4P_Descriptor2902::DONT_ADD, h4p[nameTag()]);
	
	auto devChar = h4Characteristics[H4P_H4BLECharacteristic::DEVICE_NAME].characteristic;
	devChar = h4Service->createCharacteristic(BLEUUID((uint16_t)ESP_GATT_UUID_GAP_DEVICE_NAME), BLECharacteristic::PROPERTY_READ);
	// if (h4p.gvExists(nameTag())) {

							/*  CMD */
	_addCharacteristic(H4P_H4BLECharacteristic::CMD, BLEUUID(CMD_CHAR_UUID), BLECharacteristic::PROPERTY_WRITE, "H4-Style Commands", H4P_Descriptor2902::ADD, "", new H4CmdCharacteristicCallbacks);

							/*  REPLY */
	_addCharacteristic(H4P_H4BLECharacteristic::REPLY, BLEUUID(REPLY_CHAR_UUID), BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ, "Reply to H4-Style Commands", H4P_Descriptor2902::SETNOTIFY,"",  new H4ReplyCharacteristicCallbacks);
							/*  UI ELEMENTS */
							
	_addCharacteristic(H4P_H4BLECharacteristic::UIELEMENTS, BLEUUID(ELEMENTS_CHAR_UUID), BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ, "UI Elements", H4P_Descriptor2902::SETNOTIFY);

							/*  UI DATA */
							
	_addCharacteristic(H4P_H4BLECharacteristic::UIDATA, BLEUUID(H4UIDATA_CHAR_UUID), BLECharacteristic::PROPERTY_NOTIFY, "UI Data", H4P_Descriptor2902::SETNOTIFY);

	//
	// Start Advertizing
	//
	h4Advertising = BLEDevice::getAdvertising();
	h4Advertising->addServiceUUID(H4_SERVICE_UUID);
	h4Advertising->setScanResponse(true);
	h4Advertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
	h4Advertising->setMinPreferred(0x12);
	//
	//	Emit Event: H4PE_BLEServerInit
	//
	QEVENT(H4PE_BLESINIT);
	svcUp();
	
}

void H4P_BLEServer::_elemAdd(const std::string &name, H4P_UI_TYPE t, const std::string &h, const std::string &value, uint8_t color)
{
	Serial.printf("_elemAdd(%s)\n", name.c_str());
    std::function<std::string(void)>  f;
    std::string v=value;
    switch(t){
        case H4P_UI_GPIO:
            f=[=](){ return stringFromInt(H4P_PinMachine::logicalRead(STOI(name))); };
            break;
        case H4P_UI_DROPDOWN:
            if(!h4p.gvExists(name)) h4p.gvSetstring(name,"");
            f=[=]{ return value; };
            break;
        default:
            if(v.size()) f=[=](){ return v; };
            else {
                if(!h4p.gvExists(name)) h4p.gvSetstring(name,value,false);
                f=[=](){ return h4p[name]; };
            }
    }
    h4pBLEUIorder.push_back(name); // [ ] Priority management.
    h4pBLEUserItems[name]={t,f,color,h};
	Serial.printf("Items size %d\torder size %d\n", h4pBLEUserItems.size(), h4pBLEUIorder.size());

}

void H4P_BLEServer::__elemAdd(const std::string &msg)
{
	Serial.printf("__elemAdd(%s)\n", msg.c_str());

    std::vector<std::string> m=split(msg,",");
    _elemAdd(m[0],static_cast<H4P_UI_TYPE>(STOI(m[1])),m[3],m[2],STOI(m[4]));    
}
void H4P_BLEServer::_sendElems()
{
	Serial.printf("_sendElems() connected=%d elems=%d\n", connected, h4pBLEUIorder.size());
	if (connected) {
		static bool sending = false;
		if (sending){
			return;
		}
		sending = true;
		h4Chunker(h4pBLEUIorder, [this](std::vector<std::string>::iterator it){
			if (!connected){
				h4.cancel();
				sending = false;
				return;
			}

			auto i=h4pBLEUserItems[*it];
			std::string msg {"ui,"};
			msg.append(*it)
				.append(",").append(stringFromInt(i.type))
				.append(",").append(i.h)
				.append(",0,")
				.append(stringFromInt(i.color))
				.append(",").append(i.f());
			// Serial.printf("uiElem->setValue(%s) char %p\n", msg.c_str(), h4Characteristics[H4P_H4BLECharacteristic::UIELEMENTS].characteristic);
			h4Characteristics[H4P_H4BLECharacteristic::UIELEMENTS].characteristic->setValue(msg.c_str());
			h4Characteristics[H4P_H4BLECharacteristic::UIELEMENTS].characteristic->notify();
		}, 15, 20, []{ sending = false; });
	}
}
void H4P_BLEServer::_elemSetValue(const std::string &name, const std::string &value)
{
	Serial.printf("_elemSetValue(%s,%s)\n", name.c_str(), value.c_str());
	if (connected) {
		std::string concat = name+","+value;
		h4Characteristics[H4P_H4BLECharacteristic::UIDATA].characteristic->setValue(concat.c_str());
		h4Characteristics[H4P_H4BLECharacteristic::UIDATA].characteristic->notify();
	}
}
void H4P_BLEServer::_elemSetValue(const std::string &name, std::vector<std::uint8_t> &value)
{
	// Serial.printf("_elemSetValue(%s, <BINARY>)\n", name.c_str());
	if (connected)
	{
		std::vector<std::uint8_t> data{name.begin(), name.end()};
		data.push_back(',');
		data.insert(data.end(), std::make_move_iterator(value.begin()), std::make_move_iterator(value.end()));
		h4Characteristics[H4P_H4BLECharacteristic::UIDATA].characteristic->setValue(const_cast<uint8_t*>(value.data()), value.size());
		h4Characteristics[H4P_H4BLECharacteristic::UIDATA].characteristic->notify();
	}
}
void H4P_BLEServer::_defaultSync(const std::string &svc, const std::string &msg)
{
	// Serial.printf("_defaultSync(%s, %s) connected %d .count() %d\n", svc.c_str(), msg.c_str(), connected, h4pBLEUserItems.count(svc));
	if(connected && h4pBLEUserItems.count(svc)) {
        std::string sync;
        switch(h4pBLEUserItems[svc].type){
            case H4P_UI_DROPDOWN:
                sync=h4pBLEUserItems[svc].f();
                break;
            case H4P_UI_IMGBTN:
                if(svc==stateTag()) sync=msg;
                else return;
            default:
                sync=msg;
        }
        _elemSetValue(svc, sync);
    }
}
void H4P_BLEServer::elemAdd(const std::string &name, H4P_UI_TYPE t, const std::string &h, H4P_FN_UIGET f, uint8_t color)
{
	h4pBLEUIorder.push_back(name);
	h4pBLEUserItems[name]={t,f,color,h};
}

void H4P_BLEServer::elemRemove(const std::string &name)
{
	// Serial.printf("elemRemove(%s)\n", name.c_str());
	h4pBLEUIorder.erase(std::remove(h4pBLEUIorder.begin(), h4pBLEUIorder.end(), name), h4pBLEUIorder.end());
	h4pBLEUserItems.erase(name);
	// Serial.printf("Items size %d\torder size %d\n", h4pBLEUserItems.size(), h4pBLEUIorder.size());
	// [ ] could update the elems if connected.
}

void H4P_BLEServer::clearElems(){ 
    h4pBLEUserItems.clear();
    h4pBLEUIorder.clear();
    h4pBLEUIorder.shrink_to_fit();
}

void H4P_BLEServer::elemSetValue(const std::string &name, std::vector<std::uint8_t> &raw)
{
	if (connected && h4pBLEUserItems.count(name))
		_elemSetValue(name,raw);
}

void H4P_BLEServer::elemSetValue(const std::string &name, const std::string &value)
{
	if (h4pBLEUserItems.count(name))
		_elemSetValue(name, value);
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
	QEVENT(H4PE_BLESDOWN);
	H4Service::svcDown();
};
void H4P_BLEServer::svcUp() {
	Serial.printf("BLESRV svcUp\n", connected, conn_id);
	// Start advertizsing
	h4Service->start();
	QEVENT(H4PE_BLESUP);
	BLEDevice::startAdvertising();
};

bool H4P_BLEServer::addServiceToAdvertise(BLEUUID uuid)
{
	if (!h4Advertising) return false;
	h4Advertising->addServiceUUID(uuid);
	return true;
}

bool H4P_BLEServer::removeAdvertisingService(BLEUUID uuid)
{
	if (!h4Advertising) return false;
	return h4Advertising->removeServiceUUID(uuid);
}

void H4P_BLEServer::elemAddAllUsrFields(const std::string& section){ for(auto const& g:h4pGlobal) if(g.first.rfind("usr_")!=std::string::npos) _elemAdd(replaceAll(g.first,"usr_",""),H4P_UI_TEXT,section,g.second); }

void H4P_BLEServer::elemAddDropdown(const std::string& name,H4P_NVP_MAP options,const std::string& section){ _elemAdd(name,H4P_UI_DROPDOWN,section,flattenMap(options)); }
#endif