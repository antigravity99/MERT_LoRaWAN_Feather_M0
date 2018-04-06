using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MERT
{
    class Request
    {
        private int _address;

        public int Add
        {
            get { return _address; }
            set { _address = value; }
        }

        private string _cmd;

        public string Cmd
        {
            get { return _cmd; }
            set { _cmd = value; }
        }

        private string _key;

        public string Key
        {
            get { return _key; }
            set { _key = value; }
        }

        private string _value;

        public string Val
        {
            get { return _value; }
            set { _value = value; }
        }

        private string _checksum;

        public string Checksum
        {
            get { return _checksum; }
            set { _checksum = value; }
        }

        public Request() { }

    }
}
