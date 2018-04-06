using DSPLib;
using InteractiveDataDisplay.WPF;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Numerics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace MERT
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        MainWindowViewModel _mainWindowViewModel;
        public MainWindow()
        {
            InitializeComponent();
            _mainWindowViewModel = new MainWindowViewModel();
            this.DataContext = _mainWindowViewModel;

            //double[] x = new double[200];
            //for (int i = 0; i < x.Length; i++)
            //    x[i] = 3.1415 * i / (x.Length - 1);

            //for (int i = 0; i < 25; i++)
            //{
            //    var lg = new LineGraph();
            //    lines.Children.Add(lg);
            //    lg.Stroke = new SolidColorBrush(Color.FromArgb(255, 0, (byte)(i * 10), 0));
            //    lg.Description = String.Format("Data series {0}", i + 1);
            //    lg.StrokeThickness = 2;
            //    lg.Plot(x, x.Select(v => Math.Sin(v + i / 10.0)).ToArray());
            //}
        }

        private void Row_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {

        }

        private void dataGrid_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {

            // Ensure row was clicked and not empty space
            try
            {
                DataGridRow row = (DataGridRow)dataGrid.ItemContainerGenerator.ContainerFromItem(e.AddedItems[0]);
                if (row == null) return;

                SensorReadingsModel s = (SensorReadingsModel)dataGrid.SelectedItem;
                if (s.Reading_Type.Equals(Keys.VIBRATION_KEY))
                {
                    //use a lower number if the PC doesn't have the CPU to handle large zero padding
                    UInt32 zeroPadding = 12000; // NOTE: Zero Padding

                    List<double> y = JsonConvert.DeserializeObject<List<double>>(s.Reading);

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
                    double[] freqSpan = dft.FrequencySpan(s.Vib_Sample_Rate);

                    // At this point a XY Scatter plot can be generated from,
                    // X axis => freqSpan
                    // Y axis => lmSpectrum


                    lines.Children.Clear();
                    //double[] x = new double[y.Count];
                    //for (int i = 0; i < x.Length; i++)
                    //    x[i] = (1 / s.Vib_Sample_Rate) * i;

                    var lg = new LineGraph();
                    lines.Children.Add(lg);
                    lg.Stroke = new SolidColorBrush(Color.FromArgb(255, 0, 0, 255));
                    lg.Description = "Accel plot";
                    lg.StrokeThickness = 2;
                    //lg.Plot(x, y);
                    lg.Plot(freqSpan, lmSpectrum);
                }
            }
            catch
            {

            }

            //DataGridRow row = ItemsControl.ContainerFromElement((DataGrid)sender, e.OriginalSource as DependencyObject) as DataGridRow;
        }
    }
}
