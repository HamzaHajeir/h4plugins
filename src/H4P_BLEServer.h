#pragma once


#include <H4Service.h>
#if H4P_BLE_AVAILABLE
#include <H4P_BLECommon.h>
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
#define ELEMENTS_CHAR_UUID		"d684fb38-8fdc-484f-a3a5-15233de0dd9d"
#define H4UIDATA_CHAR_UUID		"53922702-8a3a-41c2-9e5e-d8c90609855e"
#define H4MSG_CHAR_UUID		 	"7edd30f1-d7a5-4dbe-8b83-123e30bbce7f"


enum class H4P_H4BLECharacteristic : std::uint8_t {
	DEVICE_NAME,
	CMD,
	REPLY,
	UIELEMENTS,
	UIDATA
};

enum class H4P_Descriptor2902 : int8_t {
	DONT_ADD=-1,
	ADD,
	SETNOTIFY
};

struct H4P_BLEServerCharacteristic {
	BLECharacteristic* 	characteristic;
	BLE2901* 			descriptor_2901;
	BLE2902* 			descriptor_2902;
};

class H4P_BLEServer : public H4Service {
		BLEServer* 			h4Server;
		BLEService 			*h4Service;
		BLEAdvertising 		*h4Advertising;

		std::map<H4P_H4BLECharacteristic, H4P_BLEServerCharacteristic> h4Characteristics; // later on.

		std::uint16_t 		conn_id = 0;
			void 			_addCharacteristic(H4P_H4BLECharacteristic, BLEUUID, std::uint32_t props, const std::string& desc="", H4P_Descriptor2902 notif=H4P_Descriptor2902::DONT_ADD, const std::string& value="", BLECharacteristicCallbacks *cb=nullptr);
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
		void 				restart() { Serial.println("BLESRV _restart"); svcDown(); svcUp(); }
		void 				notify(const std::string& key, const std::string& value);

        void                _init() override;
		void 				_sendElems();
		void 				_elemAdd(const std::string& name,H4P_UI_TYPE t,const std::string& h="u",const std::string& v="",uint8_t c=H4P_UILED_BI);
		void				__elemAdd(const std::string& msg);
		void 				_elemSetValue(const std::string& name, const std::string& value);
		void 				_elemSetValue(const std::string& name, std::vector<std::uint8_t>& value);
		void 				_defaultSync(const std::string &svc, const std::string &msg);

		void 				svcDown() override;
		void                svcUp() override;

	public:


		H4P_BLEServer() : H4Service(blesrvTag(), H4PE_GVCHANGE|H4PE_VIEWERS|H4PE_SERVICE|H4PE_REBOOT|H4PE_FACTORY|H4PE_GPIO|H4PE_BLEADD) {
			
			// 
			// AddLocals .. 
			//

		}

				void 			elemAdd(const std::string& name,H4P_UI_TYPE t,const std::string& h,H4P_FN_UIGET f,uint8_t color);
				void 			elemRemove(const std::string& name);
				void 			clearElems();				
				void 			sendElems() { _sendElems(); }

				void            elemAddBoolean(const std::string& name,const std::string& section="u"){ _elemAdd(name,H4P_UI_BOOL,section); }
				void            elemAddDropdown(const std::string& name,H4P_NVP_MAP options,const std::string& section="u");
				void            elemAddGlobal(const std::string& name,const std::string& section="u"){ _elemAdd(name,H4P_UI_TEXT,section); }
				void            elemAddImg(const std::string& name,const std::string& url,const std::string& section="u"){ _elemAdd(name,H4P_UI_IMG,section,url); }
				void            elemAddImgButton(const std::string& name,const std::string& section="u"){ _elemAdd(name,H4P_UI_IMGBTN,section); }
				void            elemAddInput(const std::string& name,const std::string& section="u"){ _elemAdd(name,H4P_UI_INPUT,section); }
				void            elemAddText(const std::string& name,const std::string& v,const std::string& section="u"){ _elemAdd(name,H4P_UI_TEXT,section,v); }
				void            elemAddText(const std::string& name,int v,const std::string& section="u"){ _elemAdd(name,H4P_UI_TEXT,section,stringFromInt(v)); }
				void            elemAddAllUsrFields(const std::string& section="u");

				void 			elemSetValue(const std::string &name, std::vector<std::uint8_t> &raw);
				void 			elemSetValue(const std::string& name, const std::string& value);
				void            elemSetValue(const std::string& ui,const int f){ elemSetValue(ui,CSTR(stringFromInt(f))); }

				bool 			addServiceToAdvertise(BLEUUID uuid);
				bool 			removeAdvertisingService(BLEUUID uuid);
				BLEAdvertising* getAdvertising() { return h4Advertising; }
				BLEService* 	getH4Service() { return h4Service; }
};

#endif