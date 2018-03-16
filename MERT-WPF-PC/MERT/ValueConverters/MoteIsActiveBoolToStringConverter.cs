using System;
using System.Globalization;
using System.Windows.Data;
using System.Windows.Media;

namespace MERT
{
    class MoteIsActiveBoolToStringConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if ((bool)value)
                return "Active";
            else
                return "Inactive";
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
