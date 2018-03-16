using System;
using System.Collections.ObjectModel;

namespace MERT
{
    class ArduinoObservableCollection<Arduino> : ObservableCollection<Arduino>
    {

        public event EventHandler OnAdd;
        public event EventHandler OnRemove;

        public new void Add(Arduino item)
        {
            if (null != OnAdd)
            {
                OnAdd(this, null);
            }
            base.Add(item);
        }

        public void Remove(Arduino item)
        {
            if (null != OnRemove)
            {
                OnRemove(this, null);
            }
            base.Add(item);
        }

    }
}
