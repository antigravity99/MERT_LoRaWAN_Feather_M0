#include <Arduino.h>

static const String HUMIDITY_KEY = "HUMIDITY";
static const String TEMP_KEY = "TEMP";
static const String TYPE_KEY = "TYPE";
static const String VIBRATION_KEY = "VIBRATION";

static const String DEVICE_TYPE = "AmbientTemp";

static const String REQUEST_CMD = "LOCAL_REQUEST";
static const String UPDATE_CMD = "LOCAL_UPDATE";

struct CmdKey
{
  String cmd;
  String key;
};