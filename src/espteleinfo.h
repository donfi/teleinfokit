#ifndef ESPTELEINFO_H
#define ESPTELEINFO_H

#define LINE_MAX_COUNT 50
#define DATA_MAX_SIZE 200
#define NBTRY 5

// PubSubClient sends a PINGREQ after this many seconds without inbound traffic and drops the
// session if the PINGRESP has not arrived one period later. The library default (15 s) is too
// short for a lossy link: we only receive PINGRESPs, so every lost one ended the session.
#define MQTT_KEEPALIVE_S 60
// large enough for the status JSON (topic + payload) and the data messages
#define MQTT_BUFFER_SIZE 384
// at 1200 baud, 1024 bytes hold about 8 s of TIC data while the loop is blocked (e.g. reconnecting)
#define TIC_RX_BUFFER_SIZE 1024

#include <ESP8266WiFi.h>
#include <LibTeleinfo.h>
#include <PubSubClient.h>
#include "version.h"

// Linked list structure containing all values received
typedef struct _UnsentValueList UnsentValueList;
struct _UnsentValueList
{
    UnsentValueList *next; // next element
    char *name;            // LABEL of value name
    char *value;           // value
};

class ESPTeleInfo
{

public:
    ESPTeleInfo();

    void init(_Mode_e tic_mode, bool triphase);
    void initMqtt(char *server, uint16_t port, char *username, char *password, int period_data);
    void loop(void);

    // les données de consommation
    long iinst;               // HIST: IINST  STD IRMS1
    long papp;                // HIST: PAPP   STD SINSTS
    long index;               // HIST: BASE + HCHC + HCHP + TEMPO  STD EAST
    char adresseCompteur[20]; // HIST: ADCO   STD ADSC
    char strDataTopic[50];
    char strDiscoveryTopic[128];
    long ts_analyzeData;
    long ts_startup;
    char analyzeBuffer[20];

    long maxPapp;

    // _Mode_e tic_mode = TINFO_MODE_STANDARD;
    TInfo tic;

    bool LogStartup();
    void SetData(char *name, char *val);
    // 100 char max !
    void Log(String s);

    void sendMqttDiscovery();
    void AnalyzeTicForInternalData();
    _Mode_e ticMode;
    bool triphase;

    // stability: timestamp of the last complete TIC frame, and whether one was seen since init
    unsigned long ts_lastFrame;
    bool frameSeen;
    bool mqttConnected();

    // diagnostics: complete TIC frames decoded, MQTT reconnections and disconnections since the
    // last status report, and the PubSubClient state() code at the last disconnection
    // (-4 keepalive timeout, -3 connection lost, -2 connect failed, -1 disconnected)
    unsigned long frameCount;
    unsigned long mqttReconnects;
    unsigned long mqttDisconnects;
    int mqttLastState;
    // publishes a JSON status payload on <UNIQUE_ID>/status (not retained), only if MQTT is connected
    bool PublishStatus(const char *json);

private:
    // timestamp of the last MQTT connection attempt from SendData (throttled reconnection)
    unsigned long ts_lastMqttConnectAttempt;
    bool connectMqttThrottled();
    // MQTT connection state at the previous loop, to count disconnections
    bool mqttWasConnected;

    char logBuffer[100];
    char mqtt_user[32];
    char mqtt_pwd[32];

    char *_adc0_ = (char *)"ADCO";
    char *_adsc_ = (char *)"ADSC";
    char *_irms1_ = (char *)"IRMS1";
    char *_sinsts_ = (char *)"SINSTS";
    char *_east_ = (char *)"EAST";
    char *_iinst_ = (char *)"IINST";
    char *_papp_ = (char *)"PAPP";
    char *_base_ = (char *)"BASE";
    char *_hchc_ = (char *)"HCHC";
    char *_hchp_ = (char *)"HCHP";
    /*** MODIF TEMPO : labels Tempo ***/
    char *_bbrhcjb_ = (char *)"BBRHCJB";
    char *_bbrhpjb_ = (char *)"BBRHPJB";
    char *_bbrhcjw_ = (char *)"BBRHCJW";
    char *_bbrhpjw_ = (char *)"BBRHPJW";
    char *_bbrhcjr_ = (char *)"BBRHCJR";
    char *_bbrhpjr_ = (char *)"BBRHPJR";

    // to store 9 for hist mode (BASE, HP, HC + TEMPO)
    long indexes[9];

    unsigned int delay_generic;
    bool sendGeneric;
    bool started;

    // timestamp for the last power data send
    unsigned long ts_power;
    // timestamp for the last index data send
    unsigned long ts_index;
    // timestamp for the last generic data send
    unsigned long ts_generic;
    unsigned long ts_maxPapp;

    const unsigned long ONE_DAY_MS = 24UL * 60UL * 60UL * 1000UL;

    char CHIP_ID[7] = {0};
    char UNIQUE_ID[30];
    char bufLabel[12];
    char bufLogTopic[35];
    char bufDataTopic[35];

    void SendAllData();
    void SendAllUnsentData();
    void SendData(char *label, char *value);
    bool sendGenericData();

    void clearAllDiscovery();
    void deleteMqttDiscovery(String label);
    void sendMqttDiscoveryIndex(String label, String friendlyName);
    void sendMqttDiscoveryText(String label, String friendlyName);
    void sendMqttDiscoveryForType(String label, String friendlyName, String deviceClass, String unit, String icon);

    char payloadDiscovery[500];

    bool connectMqtt();
    String sanitizeLabel(String input);

    String discoveryDevice;

    UnsentValueList *unsentList = nullptr;
    void freeList(UnsentValueList *&head);
    void addOrReplaceValueInList(UnsentValueList *&head, const char *name, const char *newValue);
};

#endif /* ESPTELEINFO_H */
