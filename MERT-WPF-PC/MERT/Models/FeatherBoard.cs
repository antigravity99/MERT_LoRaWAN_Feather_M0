using System.Management;
using System.IO.Ports;
using System;
using System.Diagnostics;
using System.Text;
using Newtonsoft.Json;
using System.Threading.Tasks;
using System.Collections.ObjectModel;
using System.Collections.Generic;
using LentzArduinoManager;

namespace MERT
{
    public class FeatherBoard
    {
        public static readonly int MAX_ADDRESS = 99;

        //private TrulyObservableCollection<ListViewModel> _clientsCollection;


        private ObservableCollection<ListViewModel> _clientsCollection;

        public ObservableCollection<ListViewModel> ClientsCollection
        {
            get { return _clientsCollection; }
            set { _clientsCollection = value; }
        }


        public event EventHandler TypeChanged;

        //public ManagementBaseObject MObject { get; private set; }
        public string ComPort { get; private set; }
        public int Address { get; private set; }

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

        private SerialDataReceivedEventHandler _serialDataReceivedEventHandler;
        private SerialPort _serialPort;

        private DbHelper _dbHelper = new DbHelper();

        private Arduino _arduino;

        public Arduino Arduino
        {
            get { return _arduino; }
            set { _arduino = value; }
        }

        public FeatherBoard(Arduino arduino)
        {
            _arduino = arduino;

            DeviceType = Values.DeviceTypes.Unknown;
            string caption = _arduino.MObject[ArduinoPropKeys.Caption].ToString();
            int start = caption.IndexOf("COM");
            //Will return something like COM11
            string com = caption.Substring(start, 5);
            //if on COM3 the previous statement will return COM3) this removes the unwanted ')'
            ComPort = com.Replace(")", "");

            _serialDataReceivedEventHandler = new SerialDataReceivedEventHandler(SerialDataReceived);
            if (!ConnectToArduino())
                return;
            //DeviceTypeRequest();
            WaitToGetDeviceInfo();
        }

        private async Task WaitToGetDeviceInfo()
        {
            await Task.Delay(3000);
            Request req = new Request()
            {
                Add = 98,
                Cmd = Cmds.REQUEST_CMD,
                Key = Keys.TYPE_KEY,
                Val = Values.BLANK_VALUE
            };
            SerailRequest(req);
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
                if (!indata.StartsWith("{\"Add"))
                    return;
                Request req = JsonConvert.DeserializeObject<Request>(indata);                

                if (req.Cmd.Equals(Cmds.REQUEST_RESPONSE_CMD))
                {
                    if (req.Key.Equals(Keys.TYPE_KEY))
                    {
                        DeviceType = (Values.DeviceTypes)Enum.Parse(typeof(Values.DeviceTypes), req.Val);
                        Address = req.Add;
                    }
                }
                else if (req.Cmd.Equals(Cmds.SEND_CMD) && _deviceType.Equals(Values.DeviceTypes.Server))
                {

                    if (req.Key.Equals(Keys.VIBRATION_KEY))
                    {
                        List<int> accelArr = JsonConvert.DeserializeObject<List<int>>(req.Val);
                        double sampleRate = accelArr[accelArr.Count - 1] / 100.0;
                        accelArr.RemoveAt(accelArr.Count - 1);
                        
                        List<double> accelDouble = new List<double>();
                        foreach (int i in accelArr)
                            accelDouble.Add(i / 1000.0);

                        string serialArray = JsonConvert.SerializeObject(accelDouble);

                        req.Val = serialArray;

                        _dbHelper.InsertReading(req, sampleRate);

                        Debug.WriteLine(accelArr);
                    }
                    else if(req.Key.Equals(Keys.TEMP_IR_KEY) || req.Key.Equals(Keys.TEMP_DIE_KEY))
                    {
                        _dbHelper.InsertReading(req, null);
                    }
                        

                    //var items = (from i in _clientsCollection
                    //             where i.MoteAddress == req.Address
                    //             select i).ToList();

                    //if (items.Count > 0)
                    //{
                    //    //_clientsCollection.Remove(items[0]);
                    //    items[0].IsActive = true;
                    //    //_clientsCollection.Add(items[0]);

                    //}
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

        private void SerailRequest(Request req)
        {
            string serializedReq = JsonConvert.SerializeObject(req) + "\n";
            byte[] buffer = Encoding.ASCII.GetBytes(serializedReq);
            _serialPort.Write(buffer, 0, buffer.Length);
        }
    }
}
