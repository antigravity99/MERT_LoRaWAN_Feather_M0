using System;
using System.Collections.ObjectModel;
using System.Threading.Tasks;
using System.Windows;

namespace MERT
{
    public class MainWindowViewModel : BaseViewModel
    {
        public string TestString { get; set; }

        public ObservableCollection<ListViewModel> MoteObservableCollection { get; set; }

        public ObservableCollection<Arduino> UnknownConnectDevicesObservableCollection { get; set; }
        public ObservableCollection<Arduino> ServerConnectDevicesObservableCollection { get; set; }
        public ObservableCollection<Arduino> ClientConnectDevicesObservableCollection { get; set; }

        public MainWindowViewModel()
        {
            SetStringAsync();

            UnknownConnectDevicesObservableCollection = new ObservableCollection<Arduino>();
            ServerConnectDevicesObservableCollection = new ObservableCollection<Arduino>();
            ClientConnectDevicesObservableCollection = new ObservableCollection<Arduino>();

            MoteObservableCollection = new ObservableCollection<ListViewModel>();

            Random rand = new Random();

            for(int i = 0; i < 16; i++)
            {
                MoteObservableCollection.Add(new ListViewModel()
                {
                    MoteAddress = $"Mote ID#{i}",
                    MoteType = (rand.Next(2) == 0 ) ? DeviceTypes.AmbientTemp.ToString() : (rand.Next(2) == 0 ) ? "Vibration Mote" : "Infrared Temp Mote",
                    IsActive = (rand.Next(2) == 0) ? true : false
                });
            }

            ArduinoManager am = new ArduinoManager();

            SetupDevicesAndCollectionsAsync(am);

        }

        private void SetupDevicesAndCollectionsAsync(ArduinoManager am)
        {
            foreach (Arduino a in am.ConnectedDevices)
            {
                a.TypeChanged += ArduinoTypeChanged;
                AddArduino(a);
            }

            am.DeviceAdded += DeviceAdded;

            am.DeviceRemoved += DeviceRemoved;
        }

        private void DeviceAdded(object o, EventArgs e)
        {
            Arduino a = (Arduino)o;
            AddArduino(a);
        }

        private void AddArduino(Arduino a)
        { 
            a.TypeChanged += ArduinoTypeChanged;
            if (a.DeviceType == DeviceTypes.Unknown)
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.UnknownConnectDevicesObservableCollection.Add(a)));
            else if (a.DeviceType == DeviceTypes.Server)
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.ServerConnectDevicesObservableCollection.Add(a)));
            else
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.ClientConnectDevicesObservableCollection.Add(a)));
        }

        private void DeviceRemoved(object o, EventArgs e)
        {
            Arduino a = (Arduino)o;
            if (a.DeviceType == DeviceTypes.Unknown)
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.UnknownConnectDevicesObservableCollection.Remove(a)));
            else if (a.DeviceType == DeviceTypes.Server)
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.ServerConnectDevicesObservableCollection.Remove(a)));
            else
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.ClientConnectDevicesObservableCollection.Remove(a)));
        }

        private void ArduinoTypeChanged(object o, EventArgs e)
        {
            Arduino a = (Arduino)o;
            Application.Current.Dispatcher.BeginInvoke(new Action(() => this.ServerConnectDevicesObservableCollection.Remove(a)));
            Application.Current.Dispatcher.BeginInvoke(new Action(() => this.ClientConnectDevicesObservableCollection.Remove(a)));
            Application.Current.Dispatcher.BeginInvoke(new Action(() => this.UnknownConnectDevicesObservableCollection.Remove(a)));
            AddArduino(a);
        }

        private async Task SetStringAsync()
        {
            TestString = "Some Stuff";
            await Task.Delay(3000);
            TestString = "Some More Stuff!";
        }
    }
}
