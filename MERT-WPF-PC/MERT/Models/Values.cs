using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MERT
{
    public class Values
    {
        public const string SERVER_VALUE = "Server";
        public const string MOTE_VALUE = "Mote";
        public const string BLANK_VALUE = "";

        public enum DeviceTypes
        {
            Server,
            Mote,
            Unknown
        }
    }
}
