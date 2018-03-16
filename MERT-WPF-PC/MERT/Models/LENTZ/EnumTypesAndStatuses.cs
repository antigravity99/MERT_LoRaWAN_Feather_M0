namespace MERT
{
    public enum DeviceTypes
    {
        Server,
        AmbientTemp,
        InfraredTemp,
        Vibration,
        Humidity,
        Unknown
    }

    public enum State
    {
        Active,
        Inactive,
        Error
    }

    public enum DeviceUSBStatus
    {
        Connected,
        Connecting,
        Disconnected,
        Error
    }
}