using System;
using System.Collections.ObjectModel;
using System.Timers;
using System.Windows;
using System.Diagnostics;
using System.Globalization;
using System.Windows.Input;
using ClosedXML.Excel;
using System.Data;
using Microsoft.Win32;
using LentzArduinoManager;
using System.Linq;

namespace MERT
{
    public class MainWindowViewModel : BaseViewModel
    {
        public ICommand ExportAllCommand { get; set; }
        public ICommand ExportDataGridDataCommand { get; set; }
        public ICommand FlashServerCommand { get; set; }
        public ICommand FlashMoteCommand { get; set; }
        public ICommand FlashUnknownCommand { get; set; }

        public ObservableCollection<ListViewModel> MoteObservableCollection { get; set; }

        public ObservableCollection<FeatherBoard> UnknownConnectDevicesObservableCollection { get; set; }
        public ObservableCollection<FeatherBoard> ServerConnectDevicesObservableCollection { get; set; }
        public ObservableCollection<FeatherBoard> ClientConnectDevicesObservableCollection { get; set; }

        public ObservableCollection<SensorReadingsModel> SensorReadingsObservableCollection { get; set; }

        public ObservableCollection<string> SensorReadingTypes { get; private set; }

        public ObservableCollection<string> ElfTypes { get; private set; }

        public string SelectedReadingTypeFilter { get; set; }
        public string SelectedFlashConfig { get; set; }
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
            ExportAllCommand = new RelayCommand(ExportAllDatabaseData);
            ExportDataGridDataCommand = new RelayCommand(ExportDataGridData);
            FlashServerCommand = new RelayCommand(FlashSelectedServer);
            FlashMoteCommand = new RelayCommand(FlashSelectedMote);
            FlashUnknownCommand = new RelayCommand(FlashUnknownServer);

            _dbHelper = new DbHelper();

            //Timer t = new Timer(UpdateMoteObservableCollection, new AutoResetEvent(false), 30000, Timeout.Infinite);
            Timer timerMoteRefresh = new Timer(5000);
            timerMoteRefresh.Elapsed += new ElapsedEventHandler(UpdateMoteObservableCollection);
            timerMoteRefresh.Enabled = true;

            Timer timerGridViewRefresh = new Timer(2000);
            timerGridViewRefresh.Elapsed += new ElapsedEventHandler(UpdateGridViewData);
            timerGridViewRefresh.Enabled = true;            

            UnknownConnectDevicesObservableCollection = new ObservableCollection<FeatherBoard>();
            ServerConnectDevicesObservableCollection = new ObservableCollection<FeatherBoard>();
            ClientConnectDevicesObservableCollection = new ObservableCollection<FeatherBoard>();

            SensorReadingsObservableCollection = new ObservableCollection<SensorReadingsModel>();

            ElfTypes = new ObservableCollection<string>();
            ElfTypes.Add("Server");
            ElfTypes.Add("Mote All");
            ElfTypes.Add("Ambient Temp");
            ElfTypes.Add("IR Temp");
            ElfTypes.Add("Vibration");
            ElfTypes.Add("Ambient & IR Temp");
            ElfTypes.Add("Ambient Temp & Vibration");
            ElfTypes.Add("IR Temp & Vibration");

            SensorReadingTypes = new ObservableCollection<string>();
            SensorReadingTypes.Add("ALL");
            SensorReadingTypes.Add(Keys.TEMP_DIE_KEY);
            SensorReadingTypes.Add(Keys.TEMP_IR_KEY);
            SensorReadingTypes.Add(Keys.VIBRATION_KEY);

            //MoteObservableCollection.CollectionChanged += MoteActiveStatusChanged;

            ArduinoManager am = new ArduinoManager("USB Serial Device (COM");

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
                SensorReadingsObservableCollection = _dbHelper.GetSensorReadingModelCollection(SelectedAddress, typeFilter, null, null);
            }

            string minDTFilter;
            string maxDTFilter;
            try
            {
                DateTime minDT = DateTime.ParseExact(MinimumDateTimeFilter, "dddd, MMMM dd, yyyy h:mm:ss tt", CultureInfo.InvariantCulture);
                minDTFilter = minDT.ToString("yyyy-MM-dd HH:mm:ss");
                DateTime maxDT = DateTime.ParseExact(MaximumDateTimeFilter, "dddd, MMMM dd, yyyy h:mm:ss tt", CultureInfo.InvariantCulture);
                maxDTFilter = maxDT.ToString("yyyy-MM-dd HH:mm:ss");

                SensorReadingsObservableCollection = _dbHelper.GetSensorReadingModelCollection(SelectedAddress, typeFilter, minDTFilter, maxDTFilter);
                
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
                FeatherBoard f = new FeatherBoard(a);
                f.TypeChanged += DeviceTypeChanged;
                AddArduino(f);
            }

            am.DeviceAdded += DeviceAdded;

            am.DeviceRemoved += DeviceRemoved;
        }

        private void DeviceAdded(object o, EventArgs e)
        {
            FeatherBoard f = new FeatherBoard((Arduino)o);
            AddArduino(f);
        }

        private void AddArduino(FeatherBoard f)
        { 
            f.TypeChanged += DeviceTypeChanged;
            if (f.DeviceType == Values.DeviceTypes.Unknown)
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.UnknownConnectDevicesObservableCollection.Add(f)));
            else if (f.DeviceType == Values.DeviceTypes.Server)
            {
                f.ClientsCollection = MoteObservableCollection;
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.ServerConnectDevicesObservableCollection.Add(f)));
            }
            else
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.ClientConnectDevicesObservableCollection.Add(f)));
        }

        private void DeviceRemoved(object o, EventArgs e)
        {
            FeatherBoard f = FindFeatheBoard((Arduino)o);

            if (f == null)
                return;
        
            if (f.DeviceType == Values.DeviceTypes.Unknown)
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.UnknownConnectDevicesObservableCollection.Remove(f)));
            else if (f.DeviceType == Values.DeviceTypes.Server)
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.ServerConnectDevicesObservableCollection.Remove(f)));
            else
                Application.Current.Dispatcher.BeginInvoke(new Action(() => this.ClientConnectDevicesObservableCollection.Remove(f)));
        }


        private FeatherBoard FindFeatheBoard(Arduino a)
        {
            FeatherBoard f = null;
            var s = ServerConnectDevicesObservableCollection.Where(x => x.Arduino == a).ToList();
            var c = ClientConnectDevicesObservableCollection.Where(x => x.Arduino == a).ToList();
            var u = UnknownConnectDevicesObservableCollection.Where(x => x.Arduino == a).ToList();
            if (s.Count != 0)
            {
                f = s[0];
            }
            else if (c.Count != 0)
            {
                f = c[0];
            }
            else if (u.Count != 0)
            {
                f = u[0];
            }

            return f;

        }

        private void DeviceTypeChanged(object o, EventArgs e)
        {
            FeatherBoard f = (FeatherBoard)o;

            Application.Current.Dispatcher.BeginInvoke(new Action(() => this.ServerConnectDevicesObservableCollection.Remove(f)));
            Application.Current.Dispatcher.BeginInvoke(new Action(() => this.ClientConnectDevicesObservableCollection.Remove(f)));
            Application.Current.Dispatcher.BeginInvoke(new Action(() => this.UnknownConnectDevicesObservableCollection.Remove(f)));
            AddArduino(f);
        }

        private string SaveFileDialog()
        {
            string filename = null;
            // Configure save file dialog box
            SaveFileDialog dlg = new SaveFileDialog();
            //dlg.FileName = "Document"; // Default file name
            //dlg.DefaultExt = ".text"; // Default file extension
            dlg.Filter = "xlsx files (*.xlsx)|*.xlsx"; // Filter files by extension

            // Show save file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            // Process save file dialog box results
            if (result == true)
            {
                // Save document
                filename = dlg.FileName;
            }
            return filename;
        }

        private void ExportAllDatabaseData()
        {
            XLWorkbook workbook = new XLWorkbook();

            //Add the DataTables to the workbook in thier own worksheets
            workbook.Worksheets.Add(_dbHelper.GetMoteTableAsDataTable(), "Motes");
            workbook.Worksheets.Add(_dbHelper.GetSensorReadingDataTable(), "Sensor Readings");

            string filename = SaveFileDialog();

            if (filename == null)
                return;

            try
            {
                workbook.SaveAs(filename);
                Process.Start("explorer.exe", "/select," + filename);
            }
            catch (Exception ex)
            {
                string message = $"There was an error while trying write the Excel file to disk.\n\nThe File may be open and in use by another application.\n\nException Message:\n{ex.Message}";
                Xceed.Wpf.Toolkit.MessageBox.Show("Error writing the file to disk!", message, MessageBoxButton.OK, MessageBoxImage.Question);
            }
        }

        private void ExportDataGridData()
        {
            if (SelectedAddress < 0 || SelectedAddress > 15)
                return;

            XLWorkbook workbook = new XLWorkbook();

            //Add the DataTables to the workbook in thier own worksheets
            workbook.Worksheets.Add(_dbHelper.GetMoteTableAsDataTable(_selectedAddress.ToString()), "Mote");
            workbook.Worksheets.Add(GetSensorFilteredDataTable(), "Sensor Readings");

            string filename = SaveFileDialog();

            if (filename == null)
                return;

            try
            {
                workbook.SaveAs(filename);
                Process.Start("explorer.exe", "/select," + filename);
            }
            catch (Exception ex)
            {
                string message = $"There was an error while trying write the Excel file to disk.\n\nThe File may be open and in use by another application.\n\nException Message:\n{ex.Message}";
                Xceed.Wpf.Toolkit.MessageBox.Show("Error writing the file to disk!", message, MessageBoxButton.OK, MessageBoxImage.Question);

            }
        }

        private DataTable GetSensorFilteredDataTable()
        {
            
            DataTable sensorReadings = new DataTable();

            string typeFilter = SelectedReadingTypeFilter == null || SelectedReadingTypeFilter == "ALL" ? "" : SelectedReadingTypeFilter;

            if (SelectTop2000Rows)
            {
                sensorReadings = _dbHelper.GetSensorReadingDataTableFiltered(SelectedAddress, typeFilter, null, null);
            }

            string minDTFilter;
            string maxDTFilter;
            try
            {
                DateTime minDT = DateTime.ParseExact(MinimumDateTimeFilter, "dddd, MMMM dd, yyyy h:mm:ss tt", CultureInfo.InvariantCulture);
                minDTFilter = minDT.ToString("yyyy-MM-dd HH:mm:ss");
                DateTime maxDT = DateTime.ParseExact(MaximumDateTimeFilter, "dddd, MMMM dd, yyyy h:mm:ss tt", CultureInfo.InvariantCulture);
                maxDTFilter = maxDT.ToString("yyyy-MM-dd HH:mm:ss");

                sensorReadings = _dbHelper.GetSensorReadingDataTableFiltered(SelectedAddress, typeFilter, minDTFilter, maxDTFilter);

            }
            catch (Exception ex)
            {
                //Debug.WriteLine(ex.Message);
                //Debug.WriteLine(ex.StackTrace);
            }
            return sensorReadings;
        }


        private void FlashSelectedServer()
        {
            MessageBox.Show("Flash Server!");
        }

        private void FlashUnknownServer()
        {
            MessageBox.Show("Flash Unknown!");
        }

        private void FlashSelectedMote()
        {
            MessageBox.Show("Flash Mote!");
        }
    }
}
