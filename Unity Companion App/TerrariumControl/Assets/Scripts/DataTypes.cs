using System;

[Serializable]
public class SensorData
{
    public float temperature;
    public float humidity;
}

[Serializable]
public class DailyWeekly
{
    public float highTemp;
    public float lowTemp;
    public float highHumid;
    public float lowHumid;
}

[Serializable]
public class RecordsResponse
{
    public DailyWeekly daily;
    public DailyWeekly weekly;
}
