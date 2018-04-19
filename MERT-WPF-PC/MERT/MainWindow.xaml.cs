using DSPLib;
using InteractiveDataDisplay.WPF;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Numerics;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;

namespace MERT
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private SensorReadingsModel _lastSensorReadingsModel = null;
        MainWindowViewModel _mainWindowViewModel;

        private double[] _x;
        private double[] _y;

        public MainWindow()
        {
            InitializeComponent();
            _mainWindowViewModel = new MainWindowViewModel();
            this.DataContext = _mainWindowViewModel;
        }

        private void dataGrid_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            try
            {
                SensorReadingsModel s = (SensorReadingsModel)dataGrid.SelectedItem;
                //DataGridRow row = (DataGridRow)dataGrid.ItemContainerGenerator.ContainerFromItem(e.AddedItems[0]);
                if (s == null) return;
                _lastSensorReadingsModel = s;

                double zeroOffset = 0.0;
                if (zeroOffsetTextBox.Text.Length != 0)
                    zeroOffset = Convert.ToDouble(zeroOffsetTextBox.Text);
                ShowPlotAsync((bool)dftCheckBox.IsChecked, zeroOffset);
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Debug.WriteLine(ex.StackTrace);
            }
            

        }


        async private Task CalcXYAsync(bool isDFT, double zeroOffset)
        {
            // Ensure row was clicked and not empty space
            try
            {
                //SensorReadingsModel s = (SensorReadingsModel)dataGrid.SelectedItem;
                if (_lastSensorReadingsModel.Reading_Type.Equals(Keys.VIBRATION_KEY))
                {
                    List<double> y = JsonConvert.DeserializeObject<List<double>>(_lastSensorReadingsModel.Reading);

                    for (int i = 0; i < y.Count; i++)
                    {
                        y[i] = y[i] - zeroOffset;
                    }

                    if (isDFT)
                    {
                        //use a lower number if the PC doesn't have the CPU to handle large zero padding
                        UInt32 zeroPadding = 12000; // NOTE: Zero Padding

                        // Apply window to the Input Data & calculate Scale Factor
                        double[] wCoefs = DSP.Window.Coefficients(DSP.Window.Type.Hamming, (UInt32)y.Count);
                        double[] wInputData = DSP.Math.Multiply(y.ToArray(), wCoefs);
                        double wScaleFactor = DSP.Window.ScaleFactor.Signal(wCoefs);

                        DFT dft = new DFT();
                        dft.Initialize((UInt32)y.Count, zeroPadding);
                        // Call the DFT and get the scaled spectrum back
                        Complex[] cSpectrum = dft.Execute(y.ToArray());

                        // Convert the complex spectrum to magnitude
                        double[] lmSpectrum = DSP.ConvertComplex.ToMagnitude(cSpectrum);

                        // Properly scale the spectrum for the added window
                        lmSpectrum = DSP.Math.Multiply(lmSpectrum, wScaleFactor);

                        // Note: At this point lmSpectrum is a 501 byte array that 
                        // contains a properly scaled Spectrum from 0 - 50,000 Hz (1/2 the Sampling Frequency)

                        // For plotting on an XY Scatter plot generate the X Axis frequency Span
                        double[] freqSpan = dft.FrequencySpan(_lastSensorReadingsModel.Vib_Sample_Rate);

                        // At this point a XY Scatter plot can be generated from,
                        // X axis => freqSpan
                        // Y axis => lmSpectrum

                        _x = freqSpan;
                        _y = lmSpectrum;
                        
                    }
                    else
                    {
                        double[] x = new double[y.Count];
                        for (int i = 0; i < x.Length; i++)
                            x[i] = (1 / _lastSensorReadingsModel.Vib_Sample_Rate) * i;

                        _x = x;
                        _y = y.ToArray();
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Debug.WriteLine(ex.StackTrace);
            }

            progressBar.Visibility = Visibility.Hidden;
            dftText.Visibility = Visibility.Hidden;
        }

        private void dftCheckBox_Click(object sender, RoutedEventArgs e)
        {
            double zeroOffset = 0.0;
            if (zeroOffsetTextBox.Text.Length != 0)
                zeroOffset = Convert.ToDouble(zeroOffsetTextBox.Text);
            ShowPlotAsync((bool)dftCheckBox.IsChecked, zeroOffset);    
        }

        async private Task ShowPlotAsync(bool isDFT, double zeroOffset)
        {
            if (_lastSensorReadingsModel == null)
                return;

            progressBar.Visibility = Visibility.Visible;
            dftText.Visibility = Visibility.Visible;
            
            await Task.Run(() =>
            {
                CalcXYAsync(isDFT, zeroOffset);
            });

            ShowPlot();
        }

        private void ShowPlot()
        {
            lines.Children.Clear();

            var lg = new LineGraph();
            lines.Children.Add(lg);
            lg.Stroke = new SolidColorBrush(Color.FromArgb(255, 0, 0, 255));
            lg.Description = "Accel plot";
            lg.StrokeThickness = 2;
            lg.Plot(_x, _y);

            progressBar.Visibility = Visibility.Hidden;
            dftText.Visibility = Visibility.Hidden;

            graphRun.Text = $"Accelerometer Graph From Address {_lastSensorReadingsModel.Address} and row {_lastSensorReadingsModel.TempId}";
        }

        private void zeroOffsetTextBox_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            var textBox = sender as TextBox;
            e.Handled = Regex.IsMatch(e.Text, "[^0-9.-]");
        }

        private void refreshButton_Click(object sender, RoutedEventArgs e)
        {
            double zeroOffset = 0.0;
            if (zeroOffsetTextBox.Text.Length != 0)
                zeroOffset = Convert.ToDouble(zeroOffsetTextBox.Text);
            ShowPlotAsync((bool)dftCheckBox.IsChecked, zeroOffset);
        }

        private void MenuItem_Click(object sender, RoutedEventArgs e)
        {
            FlashFeatherBoardWindow flashFeatherBoardWindow = new FlashFeatherBoardWindow(_mainWindowViewModel);
            flashFeatherBoardWindow.Show();
        }
    }
}
