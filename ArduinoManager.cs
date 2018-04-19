using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO.Ports;
using System.Linq;
using System.Management;
using System.Text;
using System.Threading.Tasks;

namespace MERT
{
    class ArduinoManager
    {
        public event EventHandler DeviceAdded;
        public event EventHandler DeviceRemoved;
        public delegate void DeviceRemovedEvent(Arduino arduino);
        private List<Arduino> _connectedDevices;

        public List<Arduino> ConnectedDevices
        {
            get { return _connectedDevices; }
            //set { _connectedDevices = value; }
        }

        public enum DeviceStatus
        {
            Connected,
            Connecting,
            Disconnected,
            Error
        }
        
        ManagementEventWatcher _watcherAdded;
        ManagementEventWatcher _watcherRemoved;

        public ArduinoManager()
        {
            _connectedDevices = new List<Arduino>();
            GetConnectedArduinos();
            _watcherAdded = new ManagementEventWatcher();
            WqlEventQuery queryAdded = new WqlEventQuery("SELECT * FROM __InstanceCreationEvent WITHIN 2 WHERE TargetInstance ISA 'Win32_PnPEntity'");
            _watcherAdded.EventArrived += new EventArrivedEventHandler(watcher_EventArrived_Added);
            _watcherAdded.Query = queryAdded;
            _watcherAdded.Start();

            _watcherRemoved = new ManagementEventWatcher();
            WqlEventQuery queryRemoved = new WqlEventQuery("SELECT * FROM __InstanceDeletionEvent WITHIN 2 WHERE TargetInstance ISA 'Win32_PnPEntity'");
            _watcherRemoved.EventArrived += new EventArrivedEventHandler(watcher_EventArrived_Removed);
            _watcherRemoved.Query = queryRemoved;
            _watcherRemoved.Start();
        }

        private void watcher_EventArrived_Added(object sender, EventArrivedEventArgs e)
        {
            Debug.WriteLine("New USB Device Detected");
            Debug.WriteLine(e.NewEvent.GetText(new TextFormat()));

            ManagementBaseObject mObject = (ManagementBaseObject)e.NewEvent["TargetInstance"];
            //_guid = instance["ClassGuid"].ToString();
            string caption = mObject["Caption"].ToString();

            if (caption.Contains("Arduino Uno") || caption.Contains("USB Serial Device (COM"))
            {
                Arduino arduino = new Arduino(mObject);
                _connectedDevices.Add(arduino);
                if (null != DeviceAdded)
                    DeviceAdded(arduino, null);
            }            
        }

        private void watcher_EventArrived_Removed(object sender, EventArrivedEventArgs e)
        {
            Debug.WriteLine("USB Device Removed");
            Debug.WriteLine(e.NewEvent.GetText(new TextFormat()));

            ManagementBaseObject instance = (ManagementBaseObject)e.NewEvent["TargetInstance"];

            
            string deviceId = instance["DeviceId"].ToString();
            string caption = instance["Caption"].ToString();

            Arduino arduino = null;
            foreach (Arduino a in _connectedDevices)
            {
                string aId = a.MObject[ArduinoPropKeys.DeviceID].ToString();
                if (aId.Equals(deviceId))
                {
                    arduino = a;
                }
                    
            }

            if (arduino != null)
            {
                _connectedDevices.Remove(arduino);
                if (null != DeviceRemoved)
                    DeviceRemoved(arduino, null);
            }
                
        }

        private void GetConnectedArduinos()
        {
            ManagementScope connectionScope = new ManagementScope();
            SelectQuery serialQuery = new SelectQuery("SELECT * FROM Win32_PnPEntity");
            ManagementObjectSearcher searcher = new ManagementObjectSearcher(connectionScope, serialQuery);

            try
            {
                foreach (ManagementBaseObject item in searcher.Get())
                {
                    Debug.WriteLine(item.GetText(new TextFormat()));
                    try
                    {
                        string caption = item["Caption"].ToString();
                        string deviceId = item["DeviceId"].ToString();

                        if (caption.Contains("Arduino Uno") || caption.Contains("USB Serial Device (COM"))
                        {
                            Arduino arduino = new Arduino(item);
                            bool exist = false;
                            foreach (Arduino a in _connectedDevices)
                            {
                                if (a.MObject[ArduinoPropKeys.DeviceID].Equals(deviceId))
                                    exist = true;
                            }

                            if (!exist)
                                _connectedDevices.Add(arduino);
                        }
                    }
                    catch
                    {
                        //Do nothing
                    }
                }
            }
            catch (ManagementException e)
            {
                /* Do Nothing */
            }
        }
    }
}
