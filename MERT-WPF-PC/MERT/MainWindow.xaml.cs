using InteractiveDataDisplay.WPF;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

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

            double[] x = new double[200];
            for (int i = 0; i < x.Length; i++)
                x[i] = 3.1415 * i / (x.Length - 1);

            for (int i = 0; i < 25; i++)
            {
                var lg = new LineGraph();
                lines.Children.Add(lg);
                lg.Stroke = new SolidColorBrush(Color.FromArgb(255, 0, (byte)(i * 10), 0));
                lg.Description = String.Format("Data series {0}", i + 1);
                lg.StrokeThickness = 2;
                lg.Plot(x, x.Select(v => Math.Sin(v + i / 10.0)).ToArray());
            }
        }
    }
}
