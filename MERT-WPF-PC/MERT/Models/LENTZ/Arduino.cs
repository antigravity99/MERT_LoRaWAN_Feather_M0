using System.Management;
using System.IO.Ports;
using System;
using System.Diagnostics;
using System.Text;
using Newtonsoft.Json;
using System.Threading.Tasks;
using System.Collections.ObjectModel;
using System.Linq;

namespace MERT
{
    public class Arduino
    {
        public static readonly int MAX_ADDRESS = 99;

        //private ObservableCollection<ListViewModel> _clientsCollection;


        private ObservableCollection<ListViewModel> _clientsCollection;

        public ObservableCollection<ListViewModel> ClientsCollection
        {
            get { return _clientsCollection; }
            set { _clientsCollection = value; }
        }


        public event EventHandler TypeChanged;

        public ManagementBaseObject MObject { get; private set; }
        public string ComPort { get; private set; }
        public int Address { get; set; }

        public DeviceUSBStatus USBStatus { get; set; }

        private Values.DeviceTypes _deviceType;

        public Values.DeviceTypes DeviceType
        {
            get { return _deviceType; }
            set
            {
                _deviceType = value;
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
            DeviceType = Values.DeviceTypes.Unknown;
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
            //DeviceTypeRequest();
            /*WaitToGetDeviceInfo*/();
        }

        //private async Task WaitToGetDeviceInfo()
        //{
        //    await Task.Delay(3000);
        //    Request reqResponse = new Request()
        //    {
        //        Address = 98,
        //        Cmd = Cmds.REQUEST_CMD,
        //        Key = Keys.INIT_KEY,
        //        Value = Values.INIT_VALUE
        //    };
        //    DeviceTypeRequest();
        //}

        private void SerialDataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            SerialPort sp = (SerialPort)sender;
            
            //int bs = sp.ReadBufferSize;
            //Debug.WriteLine("buffer size: " + bs);

            try
            {
                string indata = sp.ReadLine();
                Debug.WriteLine(indata);

                Request req = JsonConvert.DeserializeObject<Request>(indata);

                Address = req.Address;

                if (req.Cmd.Equals(Cmds.REQUEST_RESPONSE_CMD))
                {
                    if (req.Key.Equals(Keys.TYPE_KEY))
                    {
                        DeviceType = (Values.DeviceTypes)Enum.Parse(typeof(Values.DeviceTypes), req.Value);
                    }
                }
                else if(req.Cmd.Equals(Cmds.UPDATE_CMD) && _deviceType.Equals(Values.DeviceTypes.Server.ToString()))
                {
                    //if(req.Key.Equals(Keys.))
                }
                else if (req.Cmd.Equals(Cmds.REQUEST_CMD) && _deviceType.Equals(Values.DeviceTypes.Server.ToString()))
                {
                    if (req.Key.Equals(Keys.INIT_KEY))
                    {
                        var items = (from i in _clientsCollection
                                     where i.MoteAddress == req.Address
                                     select i).ToList();
                        if (items.Count > 0)
                        {
                            items[0].IsActive = true;
                            Request reqResponse = new Request()
                            {
                                Address = req.Address,
                                Cmd = Cmds.REQUEST_RESPONSE_CMD,
                                Key = Keys.INIT_KEY,
                                Value = Values.INIT_VALUE
                            };

                        }
                    }
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

        private void DeviceTypeRequest(Request req)
        {
            string serializedReq = JsonConvert.SerializeObject(req) + "\n";
            byte[] buffer = Encoding.ASCII.GetBytes(serializedReq);
            _serialPort.Write(buffer, 0, buffer.Length);
        }
    }
}
