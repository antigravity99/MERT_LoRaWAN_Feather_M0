using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MERT
{
    public class ListViewModel
    {
        //public event EventHandler ActiveStatusChanged;

        public int MoteAddress { get; set; }
        public string MoteType { get; set; }

        //private bool _isActive;

        public bool IsActive { get; set; }
        //{
            //get { return _isActive; }
            //set
            //{
            //    _isActive = value;
                //if (this.ActiveStatusChanged != null)
                //    ActiveStatusChanged(this, null);
            //}
        //}

        public ListViewModel()
        {

        }


    }
}
