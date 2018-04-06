using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MERT
{
    public class SensorReadingsModel
    {
        public string TempId { get; set; }
        public int Address { get; set; }
        public string Reading_Type { get; set; }
        public string Reading { get; set; }
        public double Vib_Sample_Rate { get; set; }
        public string Date_Time { get; set; }
    }
}
