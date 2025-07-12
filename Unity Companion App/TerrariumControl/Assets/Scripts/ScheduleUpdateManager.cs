using System;
using System.Collections;
using UnityEngine;

/// <summary>
/// Manages schedule updates from Unity to the ESP32 terrarium controller
/// </summary>
public class ScheduleUpdateManager : MonoBehaviour
{
    public enum ScheduleTarget { Light, Water }
    
    public enum ScheduleFrequency 
    { 
        None,
        AlwaysOn,
        Daily, 
        Weekly, 
        TwiceDaily, 
        TwiceWeekly, 
        Monthly, 
        TwiceMonthly 
    }
    
    // Event that fires when schedules are updated successfully
    public static event Action OnSchedulesUpdated;
    
    // Reference to the ScheduleUIManager to refresh the display
    public ScheduleUIManager scheduleUIManager;
    
    // Helper method to convert enum to string format ESP32 expects
    private string GetFrequencyString(ScheduleFrequency frequency)
    {
        switch (frequency)
        {
            case ScheduleFrequency.None: return "NONE";
            case ScheduleFrequency.AlwaysOn: return "ALWAYS_ON";
            case ScheduleFrequency.Daily: return "DAILY";
            case ScheduleFrequency.Weekly: return "WEEKLY";
            case ScheduleFrequency.TwiceDaily: return "TWICE_DAILY";
            case ScheduleFrequency.TwiceWeekly: return "TWICE_WEEKLY";
            case ScheduleFrequency.Monthly: return "MONTHLY";
            case ScheduleFrequency.TwiceMonthly: return "TWICE_MONTHLY";
            default: return "NONE";
        }
    }
    
    /// <summary>
    /// Updates a schedule for either light or water
    /// </summary>
    /// <param name="target">Light or Water</param>
    /// <param name="frequency">Schedule frequency type</param>
    /// <param name="hour1">First activation hour (0-23)</param>
    /// <param name="minute1">First activation minute (0-59)</param>
    /// <param name="durationSeconds">Duration in seconds</param>
    public void UpdateSchedule(ScheduleTarget target, ScheduleFrequency frequency, 
                              int hour1, int minute1, int durationSeconds)
    {
        string targetStr = target == ScheduleTarget.Light ? "LIGHT" : "WATER";
        string frequencyStr = GetFrequencyString(frequency);
        
        // Format command string: TARGET,TYPE,h1,m1,d1
        string command = $"{targetStr},{frequencyStr},{hour1},{minute1},{durationSeconds}";
        
        StartCoroutine(SendScheduleUpdateAndRefresh(command));
    }
    
    /// <summary>
    /// Updates a twice-daily, twice-weekly, or twice-monthly schedule
    /// </summary>
    public void UpdateTwiceSchedule(ScheduleTarget target, ScheduleFrequency frequency, 
                                  int hour1, int minute1, int hour2, int minute2, int durationSeconds)
    {
        // Only certain frequencies support twice-scheduling
        if (frequency != ScheduleFrequency.TwiceDaily && 
            frequency != ScheduleFrequency.TwiceWeekly && 
            frequency != ScheduleFrequency.TwiceMonthly)
        {
            Debug.LogError($"Frequency {frequency} does not support twice-scheduling");
            return;
        }
        
        string targetStr = target == ScheduleTarget.Light ? "LIGHT" : "WATER";
        string frequencyStr = GetFrequencyString(frequency);
        
        // Format command string: TARGET,TYPE,h1,m1,d1,h2,m2
        string command = $"{targetStr},{frequencyStr},{hour1},{minute1},{durationSeconds},{hour2},{minute2}";
        
        StartCoroutine(SendScheduleUpdateAndRefresh(command));
    }
    
    /// <summary>
    /// Set to NONE (disabled) schedule
    /// </summary>
    public void DisableSchedule(ScheduleTarget target)
    {
        string targetStr = target == ScheduleTarget.Light ? "LIGHT" : "WATER";
        
        // Format command for NONE schedule (with dummy values)
        string command = $"{targetStr},NONE,0,0,0";
        
        StartCoroutine(SendScheduleUpdateAndRefresh(command));
    }
    
    /// <summary>
    /// Set to ALWAYS_ON schedule
    /// </summary>
    public void SetAlwaysOn(ScheduleTarget target)
    {
        string targetStr = target == ScheduleTarget.Light ? "LIGHT" : "WATER";
        
        // Format command for ALWAYS_ON schedule (with dummy values)
        string command = $"{targetStr},ALWAYS_ON,0,0,0";
        
        StartCoroutine(SendScheduleUpdateAndRefresh(command));
    }
    
    private IEnumerator SendScheduleUpdateAndRefresh(string command)
    {
        Debug.Log($"Sending schedule update: {command}");
        
        // Send the command to the ESP32
        TerrariumBleController.Instance.WriteSchedule(command);
        
        // Wait a moment for the ESP32 to process
        yield return new WaitForSeconds(1.0f);
        
        // Read back the updated schedules
        TerrariumBleController.Instance.ReadSchedules();
        
        // Wait a moment for the read to complete
        yield return new WaitForSeconds(1.0f);
        
        // Refresh the UI
        if (scheduleUIManager != null)
        {
            try
            {
                string schedulesJson = TerrariumBleController.Instance._schedulesString;
                if (!string.IsNullOrEmpty(schedulesJson))
                {
                    SchedulesResponse data = JsonUtility.FromJson<SchedulesResponse>(schedulesJson);
                    if (data != null)
                    {
                        scheduleUIManager.DisplaySchedules(data);
                        Debug.Log("Schedule UI refreshed successfully");
                        
                        // Notify listeners that schedules were updated
                        OnSchedulesUpdated?.Invoke();
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.LogError($"Error refreshing schedules: {ex.Message}");
            }
        }
    }
}
