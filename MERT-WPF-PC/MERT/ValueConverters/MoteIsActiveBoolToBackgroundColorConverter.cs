using System;
using System.Globalization;
using System.Windows.Data;
using System.Windows.Media;

namespace MERT
{
    class MoteIsActiveBoolToBackgroundColorConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if ((bool)value)
                return new SolidColorBrush(Color.FromArgb(255, (byte)67, (byte)146, (byte)241));
            else
                return new SolidColorBrush(Color.FromArgb(255, (byte)220, (byte)49, (byte)58));
            throw new NotImplementedException();
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
