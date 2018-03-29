﻿using System;
using System.Collections.ObjectModel;
using System.Timers;
using InteractiveDataDisplay.WPF;
using System.Windows;
using System.Diagnostics;
using System.Globalization;

namespace MERT
{
    public class MainWindowViewModel : BaseViewModel
    {

        public ObservableCollection<ListViewModel> MoteObservableCollection { get; set; }

        public ObservableCollection<Arduino> UnknownConnectDevicesObservableCollection { get; set; }
        public ObservableCollection<Arduino> ServerConnectDevicesObservableCollection { get; set; }
        public ObservableCollection<Arduino> ClientConnectDevicesObservableCollection { get; set; }

        public ObservableCollection<SensorReadingsModel> SensorReadingsObservableCollection { get; set; }

        public ObservableCollection<string> SensorReadingTypes { get; private set; }

        public string SelectedReadingTypeFilter { get; set; }
        public string MinimumDateTimeFilter { get; set; }
        public string MaximumDateTimeFilter { get; set; }

        public bool DatePickerEnabled { get; set; }

        private bool _selectTop2000Rows = true;
        public bool SelectTop2000Rows
        {
            get { return _selectTop2000Rows; }
            set
            {
                DatePickerEnabled = !value;
                _selectTop2000Rows = value;
            }
        }

        private int _selectedAddress = 0;
        public int SelectedAddress
        {
            get { return _selectedAddress; }
            set
            {
                if (value == -1)
                    return;
                _selectedAddress = value;
            }
        }

        private DbHelper _dbHelper;

        public MainWindowViewModel()
        {
            _dbHelper = new DbHelper();

            //Timer t = new Timer(UpdateMoteObservableCollection, new AutoResetEvent(false), 30000, Timeout.Infinite);
            Timer timerMoteRefresh = new Timer(5000);
            timerMoteRefresh.Elapsed += new ElapsedEventHandler(UpdateMoteObservableCollection);
            timerMoteRefresh.Enabled = true;

            Timer timerGridViewRefresh = new Timer(2000);
            timerGridViewRefresh.Elapsed += new ElapsedEventHandler(UpdateGridViewData);
            timerGridViewRefresh.Enabled = true;            

            UnknownConnectDevicesObservableCollection = new ObservableCollection<Arduino>();
            ServerConnectDevicesObservableCollection = new ObservableCollection<Arduino>();
            ClientConnectDevicesObservableCollection = new ObservableCollection<Arduino>();

            SensorReadingsObservableCollection = new ObservableCollection<SensorReadingsModel>();


            SensorReadingTypes = new ObservableCollection<string>();
            SensorReadingTypes.Add("ALL");
            SensorReadingTypes.Add(Keys.TEMP_DIE_KEY);
            SensorReadingTypes.Add(Keys.TEMP_IR_KEY);
            SensorReadingTypes.Add(Keys.VIBRATION_KEY);

            //MoteObservableCollection.CollectionChanged += MoteActiveStatusChanged;

            ArduinoManager am = new ArduinoManager();

            MoteObservableCollection = _dbHelper.GetMotesbservableCollection();

            SetupDevicesAndCollectionsAsync(am);

        }

        private void UpdateGridViewData(object sender, ElapsedEventArgs e)
        {
            string typeFilter = SelectedReadingTypeFilter == null || SelectedReadingTypeFilter == "ALL" ? "" : SelectedReadingTypeFilter;


            if (SelectedAddress < 0 || SelectedAddress > 15)
                return;

            if(SelectTop2000Rows)
            {
                SensorReadingsObservableCollection = _dbHelper.GetSensorReadingDataTable(SelectedAddress, typeFilter, null, null);
            }

            string minDTFilter;
            string maxDTFilter;
            try
            {
                DateTime minDT = DateTime.ParseExact(MinimumDateTimeFilter, "dddd, MMMM dd, yyyy h:mm:ss tt", CultureInfo.InvariantCulture);
                minDTFilter = minDT.ToString("yyyy-MM-dd HH:mm:ss");
                DateTime maxDT = DateTime.ParseExact(MaximumDateTimeFilter, "dddd, MMMM dd, yyyy h:mm:ss tt", CultureInfo.InvariantCulture);
                maxDTFilter = maxDT.ToString("yyyy-MM-dd HH:mm:ss");

                SensorReadingsObservableCollection = _dbHelper.GetSensorReadingDataTable(SelectedAddress, typeFilter, minDTFilter, maxDTFilter);
                
            }
            catch (Exception ex)
            {
                //Debug.WriteLine(ex.Message);
                //Debug.WriteLine(ex.StackTrace);
            }            
        }

        private void UpdateMoteObservableCollection(object source, ElapsedEventArgs e)
        {
            MoteObservableCollection = _dbHelper.GetMotesbservableCollection();
        }

        private void MoteActiveStatusChanged(object o, EventArgs e)
        {
            try
            {
                MoteObservableCollection[0].IsActive = !MoteObservableCollection[0].IsActive;
                MoteObservableCollection[0].IsActive = !MoteObservableCollection[0].IsActive;
            }
            catch
            { }
            
            //var temp = new ObservableCollection<ListViewModel>(MoteObservableCollection);
            //MoteObservableCollection.Clear();
            //foreach (var item in temp)
            //    MoteObservableCollection.Add(item);
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
            if (a.DeviceType == Values.DeviceTypes.Unknown)
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.UnknownConnectDevicesObservableCollection.Add(a)));
            else if (a.DeviceType == Values.DeviceTypes.Server)
            {
                a.ClientsCollection = MoteObservableCollection;
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.ServerConnectDevicesObservableCollection.Add(a)));
            }
            else
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.ClientConnectDevicesObservableCollection.Add(a)));
        }

        private void DeviceRemoved(object o, EventArgs e)
        {
            Arduino a = (Arduino)o;
            if (a.DeviceType == Values.DeviceTypes.Unknown)
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.UnknownConnectDevicesObservableCollection.Remove(a)));
            else if (a.DeviceType == Values.DeviceTypes.Server)
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
    }
}
