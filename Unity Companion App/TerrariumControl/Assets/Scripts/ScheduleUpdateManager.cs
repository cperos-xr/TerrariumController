using System;
using System.Collections;
using UnityEngine;

/// <summary>
/// Manages schedule updates from Unity to the ESP32 terrarium controller
/// </summary>
public class ScheduleUpdateManager : MonoBehaviour
{
    public enum ScheduleTarget { Light, Water, Fogger }
    
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
    
    // Fires when schedules are updated and UI refreshed
    public static event Action OnSchedulesUpdated;
    
    // Inspector‑assigned reference to your UI display class
    public ScheduleUIManager scheduleUIManager;
    
    // Convert our enum into the exact string the ESP32 expects
    private string GetFrequencyString(ScheduleFrequency frequency)
    {
        switch (frequency)
        {
            case ScheduleFrequency.None:         return "NONE";
            case ScheduleFrequency.AlwaysOn:     return "ALWAYS_ON";
            case ScheduleFrequency.Daily:        return "DAILY";
            case ScheduleFrequency.Weekly:       return "WEEKLY";
            case ScheduleFrequency.TwiceDaily:   return "TWICE_DAILY";
            case ScheduleFrequency.TwiceWeekly:  return "TWICE_WEEKLY";
            case ScheduleFrequency.Monthly:      return "MONTHLY";
            case ScheduleFrequency.TwiceMonthly: return "TWICE_MONTHLY";
            default:                             return "NONE";
        }
    }
    
    /// <summary>
    /// Updates a single‑time schedule (e.g. NONE, ALWAYS_ON, DAILY, WEEKLY, MONTHLY)
    /// for Light, Water or Fogger.
    /// </summary>
    public void UpdateSchedule(
        ScheduleTarget target,
        ScheduleFrequency frequency,
        int hour1,
        int minute1,
        int durationSeconds
    ) {
        string targetStr = target == ScheduleTarget.Light  ? "LIGHT"
                         : target == ScheduleTarget.Water  ? "WATER"
                         :                                  "FOGGER";
        string freqStr   = GetFrequencyString(frequency);
        string cmd       = $"{targetStr},{freqStr},{hour1},{minute1},{durationSeconds}";
        
        Debug.Log($"[BLE] Sending schedule update → {cmd}");
        StartCoroutine(SendScheduleUpdateAndRefresh(cmd));
    }
    
    /// <summary>
    /// Updates a “twice‑per‑period” schedule (e.g. TWICE_DAILY, TWICE_WEEKLY, TWICE_MONTHLY).
    /// </summary>
    public void UpdateTwiceSchedule(
        ScheduleTarget target,
        ScheduleFrequency frequency,
        int hour1,
        int minute1,
        int hour2,
        int minute2,
        int durationSeconds
    ) {
        if (frequency != ScheduleFrequency.TwiceDaily &&
            frequency != ScheduleFrequency.TwiceWeekly &&
            frequency != ScheduleFrequency.TwiceMonthly)
        {
            Debug.LogError($"Frequency {frequency} does not support twice‑scheduling");
            return;
        }
        
        string targetStr = target == ScheduleTarget.Light  ? "LIGHT"
                         : target == ScheduleTarget.Water  ? "WATER"
                         :                                  "FOGGER";
        string freqStr   = GetFrequencyString(frequency);
        string cmd       = $"{targetStr},{freqStr},{hour1},{minute1},{durationSeconds},{hour2},{minute2}";
        
        Debug.Log($"[BLE] Sending twice schedule update → {cmd}");
        StartCoroutine(SendScheduleUpdateAndRefresh(cmd));
    }
    
    /// <summary>
    /// Clears (NONE) a schedule for any target.
    /// </summary>
    public void DisableSchedule(ScheduleTarget target)
    {
        string targetStr = target == ScheduleTarget.Light  ? "LIGHT"
                         : target == ScheduleTarget.Water  ? "WATER"
                         :                                  "FOGGER";
        string cmd       = $"{targetStr},NONE,0,0,0";
        
        Debug.Log($"[BLE] Sending disable schedule → {cmd}");
        StartCoroutine(SendScheduleUpdateAndRefresh(cmd));
    }
    
    /// <summary>
    /// Sets an ALWAYS_ON schedule for any target.
    /// </summary>
    public void SetAlwaysOn(ScheduleTarget target)
    {
        string targetStr = target == ScheduleTarget.Light  ? "LIGHT"
                         : target == ScheduleTarget.Water  ? "WATER"
                         :                                  "FOGGER";
        string cmd       = $"{targetStr},ALWAYS_ON,0,0,0";
        
        Debug.Log($"[BLE] Sending always‑on schedule → {cmd}");
        StartCoroutine(SendScheduleUpdateAndRefresh(cmd));
    }
    
    // Sends the command, waits, then reads back & refreshes the UI
    private IEnumerator SendScheduleUpdateAndRefresh(string command)
    {
        TerrariumBleController.Instance.WriteSchedule(command);
        yield return new WaitForSeconds(1f);
        
        TerrariumBleController.Instance.ReadSchedules();
        yield return new WaitForSeconds(1f);
        
        if (scheduleUIManager != null)
        {
            try
            {
                string json = TerrariumBleController.Instance._schedulesString;
                var data = JsonUtility.FromJson<SchedulesResponse>(json);
                if (data != null)
                {
                    scheduleUIManager.DisplaySchedules(data);
                    OnSchedulesUpdated?.Invoke();
                }
            }
            catch (Exception ex)
            {
                Debug.LogError($"Error parsing/refreshing schedules: {ex.Message}");
            }
        }
    }
}
