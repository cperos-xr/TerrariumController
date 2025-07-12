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

[Serializable]
public class ScheduleData
{
    public string type;
    public int hour1;
    public int minute1;
    public int duration1;
    public int hour2;
    public int minute2;
    public int duration2;
    
    // Helper method to get formatted time string
    public string GetFormattedTime1()
    {
        return string.Format("{0:00}:{1:00}", hour1, minute1);
    }
    
    public string GetFormattedTime2()
    {
        return string.Format("{0:00}:{1:00}", hour2, minute2);
    }
    
    public string GetFormattedDuration1()
    {
        if (duration1 < 60)
            return duration1 + " sec";
        else
            return (duration1 / 60) + " min";
    }
    
    public string GetFormattedDuration2()
    {
        if (duration2 < 60)
            return duration2 + " sec";
        else
            return (duration2 / 60) + " min";
    }
    
    public string GetTypeDescription()
    {
        switch (type)
        {
            case "NONE": return "Not Scheduled";
            case "ALWAYS_ON": return "Always On";
            case "DAILY": return "Once Daily";
            case "WEEKLY": return "Once Weekly";
            case "TWICE_DAILY": return "Twice Daily";
            case "TWICE_WEEKLY": return "Twice Weekly";
            case "MONTHLY": return "Monthly";
            case "TWICE_MONTHLY": return "Twice Monthly";
            default: return "Unknown";
        }
    }
}

[Serializable]
public class SchedulesResponse
{
    public ScheduleData light;
    public ScheduleData water;
}
