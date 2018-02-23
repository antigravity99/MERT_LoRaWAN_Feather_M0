#include <Arduino.h>

//Const Keys
static const String HUMIDITY_KEY = "HUMIDITY";
static const String TEMP_KEY = "TEMP";
static const String TYPE_KEY = "TYPE";
static const String VIBRATION_KEY = "VIBRATION";
//Const device types
static const String AMBIENT_TEMP_TYPE = "AmbientTemp";
static const String INFRARED_TEMP_TYPE = "InfraredTemp";
static const String VIBRATION_TYPE = "Vibration";
static const String HUMIDITY_TYPE = "Humidity";
static const String SERVER_TYPE = "Server";
//Const commands
static const String REQUEST_CMD = "LOCAL_REQUEST";
static const String UPDATE_CMD = "LOCAL_UPDATE";

struct CmdKey
{
  String cmd;
  String key;
};
