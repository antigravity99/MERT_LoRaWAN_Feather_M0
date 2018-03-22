using System.Management;
using System.IO.Ports;
using System;
using System.Diagnostics;
using System.Text;
using System.ComponentModel;
using System.Threading;

namespace MERT
{
    public class Arduino
    {
        public static readonly int MAX_ADDRESS = 15;

        public event EventHandler TypeChanged;

        public ManagementBaseObject MObject { get; private set; }
        public string ComPort { get; private set; }
        public string Humidity { get; set; }
        public string Temp { get; set; }
        public string Vibration { get; set; }
        public int Address { get; set; }

        public DeviceUSBStatus USBStatus { get; set; }

        private DeviceTypes _deviceTypes;

        public DeviceTypes DeviceType
        {
            get { return _deviceTypes; }
            set
            {
                _deviceTypes = value;
                if(this.TypeChanged != null)
                    TypeChanged(this, null);

            }
        }


        private const string HUMIDITY_KEY = "HUMIDITY";
        private const string TEMP_KEY = "TEMP";
        private const string TYPE_KEY = "TYPE";
        private const string VIBRATION_KEY = "VIBRATION";

        private const string LOCAL_REQUEST = "LOCAL_REQUEST";
        private const string LOCAL_UPDATE = "LOCAL_UPDATE";
        private const string REMOTE_REQUEST = "REMOTE_REQUEST";
        private const string REMOTE_UPDATE = "REMOTE_UPDATE";

        private SerialDataReceivedEventHandler _serialDataReceivedEventHandler;
        private SerialPort _serialPort;

        public Arduino(ManagementBaseObject mbObject)
        {
            DeviceType = DeviceTypes.Unknown;
            MObject = mbObject;
            string caption = MObject[ArduinoPropKeys.Caption].ToString();
            int start = caption.IndexOf("COM");
            //Will return something like COM11
            string com = caption.Substring(start, 5);
            //if on COM3 the previous statement will return COM3) this removes the unwanted )
            ComPort = com.Replace(")", "");

            _serialDataReceivedEventHandler = new SerialDataReceivedEventHandler(SerialDataReceived);
            if (!ConnectToArduino())
                return;

            GetConfigValueFromDevice(TYPE_KEY);
        }

        private void SerialDataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            SerialPort sp = (SerialPort)sender;
            
            //int bs = sp.ReadBufferSize;
            //Debug.WriteLine("buffer size: " + bs);

            try
            {
                string indata = sp.ReadLine();
                Debug.WriteLine(indata);
                string[] split = indata.Split(',');

                string s = split[0];
                int address = -1;
                bool parsed = int.TryParse(s, out address);
                if(!parsed || address == -1 || address > MAX_ADDRESS)
                {
                    return;
                }

                Address = address;

                if (!(split.Length > 2))
                    return;

                string key = split[1];
                string value = split[2];
                value = value.Replace("\r", "");
                
                switch (key)
                {
                    case HUMIDITY_KEY:
                        Humidity = value;
                        break;
                    case TEMP_KEY:
                        Temp = value;
                        break;
                    case TYPE_KEY:
                        DeviceType = (DeviceTypes)Enum.Parse(typeof(DeviceTypes), value);
                        break;
                    case VIBRATION_KEY:
                        Vibration = value;
                        break;
                    default:
                        break;
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("Error reading line:");
                Debug.WriteLine(ex.Message);
                Debug.WriteLine(ex.StackTrace);
            }
        }

        private bool ConnectToArduino()
        {
            USBStatus = DeviceUSBStatus.Connecting;
            _serialPort = new SerialPort(ComPort, 9600);
            _serialPort.Encoding = Encoding.Default;
            try
            {
                _serialPort.Open();
                USBStatus = DeviceUSBStatus.Connected;
                _serialPort.DataReceived += _serialDataReceivedEventHandler;
                return true;
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                USBStatus = DeviceUSBStatus.Error;
                return false;
            }
        }

        private void GetConfigValueFromDevice(string key)
        {
            string request = LOCAL_REQUEST + "," + key;
            byte[] buffer = Encoding.ASCII.GetBytes(request);
            _serialPort.Write(buffer, 0, buffer.Length);
        }
    }
}
