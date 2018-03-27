using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MERT
{
    public class ListViewModel
    {
        public int MoteAddress { get; set; }
        public string MoteType { get; set; }
        public bool IsActive { get; set; }

        public ListViewModel()
        {

        }
    }
}
