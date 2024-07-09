#pragma once


#include <H4Service.h>
#if H4P_BLE_AVAILABLE

#pragma message ("We're compiling H4P_BLEServer ;)")
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2901.h>
#include <BLE2902.h>
#include <map>

STAG(blesrv)


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define H4_SERVICE_UUID        	"9652c3ca-1231-4607-9d40-6afd67609443"
#define CMD_CHAR_UUID 			"3c1eb836-4223-4f3e-9c9d-10c5dae1d9b1"
#define REPLY_CHAR_UUID			"5f1c2e8d-f531-488e-8198-0132ec230a6f"
#define EVENTS_CHAR_UUID		"d684fb38-8fdc-484f-a3a5-15233de0dd9d"

#define H4_BLE_MTU 			512

enum class H4P_BLEServerCharacteristic : std::uint8_t {
	CMD,
	REPLY,
	EVENTS
};
struct H4P_BleCharacteristic {
	BLECharacteristic* 	characteristic;
	BLE2901* 			descriptor_2901;
	BLE2902* 			descriptor_2902;
};

class H4P_BLEServer : public H4Service {
		BLEServer* 			h4Server;
		BLEService 			*h4Service;
		BLECharacteristic 	*h4DeviceNameCharacteristic;
		BLECharacteristic 	*h4CmdCharacteristic;
		BLE2901 			*h4CmdDescriptor_2901;
		BLECharacteristic 	*h4ReplyCharacteristic;
		BLE2901 			*h4ReplyDescriptor_2901;
		BLE2902				*h4ReplyDescriptor_2902;
		BLECharacteristic 	*h4EventsCharacteristic;
		BLE2901 			*h4EventsDescriptor_2901;
		BLE2902				*h4EventsDescriptor_2902;
		BLEAdvertising 		*h4Advertising;

		std::map<H4P_BLEServerCharacteristic, H4P_BleCharacteristic> h4Characteristics; // later on.

		std::uint16_t 		conn_id = 0;
		
		bool connected = false;
		void 				onConnect(esp_ble_gatts_cb_param_t *param);
		void 				onDisconnect(esp_ble_gatts_cb_param_t *param);

		class H4BLEServerCallbacks : public BLEServerCallbacks {
					void 			onConnect(BLEServer *pServer, esp_ble_gatts_cb_param_t *param) override;
					void 			onDisconnect(BLEServer *pServer, esp_ble_gatts_cb_param_t *param) override;
					void 			onMtuChanged(BLEServer *pServer, esp_ble_gatts_cb_param_t *param) override;
		};

		class H4CmdCharacteristicCallbacks : public BLECharacteristicCallbacks  {
					void 			onWrite(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t *param) override;
		};
		class H4ReplyCharacteristicCallbacks : public BLECharacteristicCallbacks {
					void 			onRead(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t *param) override;
		};

        void                _handleEvent(const std::string& svc,H4PE_TYPE t,const std::string& msg) override;
        void                _reply(std::string msg) override;
#if H4P_LOG_MESSAGES
        // void                info() override;
#endif
        void                _init() override;
        void                svcDown() override;
        void                svcUp() override;

		void 				_restart() { Serial.println("BLESRV _restart"); svcDown(); svcUp(); }
	public:


		H4P_BLEServer() : H4Service(blesrvTag(), H4PE_GVCHANGE|H4PE_VIEWERS|H4PE_SERVICE|H4PE_REBOOT|H4PE_FACTORY|H4PE_GPIO) {
			
			// 
			// AddLocals .. 
			//

		}

	void BLEAdd();


	BLEAdvertising* getAdvertising() { return h4Advertising; }
	BLEService* getH4Service() { return h4Service; }
};

#endif