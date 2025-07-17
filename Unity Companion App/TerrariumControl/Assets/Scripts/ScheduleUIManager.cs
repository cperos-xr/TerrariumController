using System;
using System.Collections;
using TMPro;
using UnityEngine;

public class ScheduleUIManager : MonoBehaviour
{
    // Schedule Text
    public TextMeshProUGUI waterDuration;
    public TextMeshProUGUI waterFrequency;
    public TextMeshProUGUI lightDuration;
    public TextMeshProUGUI lightFrequency;
    public TextMeshProUGUI foggerDuration;
    public TextMeshProUGUI foggerFrequency;
    
    // Add a button reference for manual update
    public UnityEngine.UI.Button readSchedulesButton;

    private void OnEnable()
    {
        TerrariumBleController.OnConnectionComplete += HandleOnConnectionComplete;
        
        // Add button listener if assigned
        if (readSchedulesButton != null)
            readSchedulesButton.onClick.AddListener(ManualReadSchedules);
    }

    private void OnDisable()
    {
        TerrariumBleController.OnConnectionComplete -= HandleOnConnectionComplete;
        
        // Remove button listener
        if (readSchedulesButton != null)
            readSchedulesButton.onClick.RemoveListener(ManualReadSchedules);
    }
    
    private void ManualReadSchedules()
    {
        Debug.Log("Manual schedules read requested");
        TerrariumBleController.Instance.ReadSchedules();
        StartCoroutine(UpdateSchedulesAfterDelay(1.0f));
    }

    private void HandleOnConnectionComplete()
    {
        Debug.Log("Connection complete, waiting before retrieving schedules...");
        // Add a delay before the first read to ensure BLE is fully ready
        StartCoroutine(InitialSchedulesReadWithDelay());
    }
    
    private IEnumerator InitialSchedulesReadWithDelay()
    {
        yield return new WaitForSeconds(1.0f);
        
        Debug.Log("Attempting to read schedules after delay");
        TerrariumBleController.Instance.ReadSchedules();
        
        // Start waiting for data with multiple attempts
        StartCoroutine(UpdateSchedulesWithMultipleAttempts());
    }
    
    private IEnumerator UpdateSchedulesWithMultipleAttempts()
    {
        // Try up to 5 times with increasing delays
        for (int attempt = 0; attempt < 5; attempt++)
        {
            yield return new WaitForSeconds(1.0f + attempt * 0.5f); // Increasing delay
            
            try
            {
                string schedulesJson = TerrariumBleController.Instance._schedulesString;
                Debug.Log("Attempt " + (attempt + 1) + " raw JSON: " + schedulesJson);
                
                if (!string.IsNullOrEmpty(schedulesJson))
                {
                    SchedulesResponse data = JsonUtility.FromJson<SchedulesResponse>(schedulesJson);
                    if (data != null && (data.light != null || data.water != null))
                    {
                        Debug.Log("Successfully parsed schedules data");
                        DisplaySchedules(data);
                        
                        // Success - exit the retry loop
                        yield break;
                    }
                    else
                    {
                        Debug.LogWarning("Schedule data parsing returned null or empty object");
                    }
                }
                
                // Try reading again
                TerrariumBleController.Instance.ReadSchedules();
            }
            catch (Exception ex)
            {
                Debug.LogError("Retry attempt " + (attempt + 1) + ": " + ex.Message);
                TerrariumBleController.Instance.ReadSchedules();
            }
        }
        
        Debug.LogError("Failed to retrieve schedule data after multiple attempts");
    }
    
    private IEnumerator UpdateSchedulesAfterDelay(float delay)
    {
        yield return new WaitForSeconds(delay);
        
        try
        {
            string schedulesJson = TerrariumBleController.Instance._schedulesString;
            Debug.Log("Raw schedules JSON: " + schedulesJson);
            
            if (!string.IsNullOrEmpty(schedulesJson))
            {
                SchedulesResponse data = JsonUtility.FromJson<SchedulesResponse>(schedulesJson);
                if (data != null)
                {
                    Debug.Log("Successfully parsed schedules data");
                    DisplaySchedules(data);
                }
                else
                {
                    Debug.LogError("Could not parse schedules JSON: " + schedulesJson);
                }
            }
            else
            {
                Debug.LogWarning("Schedules JSON is empty, trying again in 2 seconds");
                // Try again after delay
                StartCoroutine(RetrySchedulesRead());
            }
        }
        catch (Exception ex)
        {
            Debug.LogError("Error processing schedules data: " + ex.Message);
            // Try again after delay
            StartCoroutine(RetrySchedulesRead());
        }
    }
    
    private IEnumerator RetrySchedulesRead()
    {
        yield return new WaitForSeconds(2.0f);
        TerrariumBleController.Instance.ReadSchedules();
        StartCoroutine(UpdateSchedulesAfterDelay(1.0f));
    }

    public void DisplaySchedules(SchedulesResponse schedules)
    {
        // First log the entire response for debugging
        Debug.Log($"Received schedules: {JsonUtility.ToJson(schedules)}");
        
        // Check if fogger data exists
        if (schedules.fogger != null) {
            Debug.Log($"Fogger schedule: Type={schedules.fogger.type}, Hour1={schedules.fogger.hour1}, Minute1={schedules.fogger.minute1}");
            
            foggerFrequency.text = schedules.fogger.GetTypeDescription();
            
            if (schedules.fogger.type == "ALWAYS_ON") {
                foggerDuration.text = "Always On";
            }
            else if (schedules.fogger.type == "NONE") {
                foggerDuration.text = "Not scheduled";
            }
            else {
                string durationText = schedules.fogger.GetFormattedTime1() + " (runs for 4 hours)";
                
                if (schedules.fogger.type.StartsWith("TWICE_")) {
                    durationText += "\n" + schedules.fogger.GetFormattedTime2() + " (runs for 4 hours)";
                }
                
                foggerDuration.text = durationText;
            }
        }
        else {
            Debug.LogError("Fogger schedule data is null!");
            foggerFrequency.text = "Error";
            foggerDuration.text = "Error loading fogger data";
        }
        
        // Display water schedule info
        if (schedules.water != null)
        {
            waterFrequency.text = schedules.water.GetTypeDescription();
            
            if (schedules.water.type == "ALWAYS_ON")
            {
                waterDuration.text = "Always On";
            }
            else if (schedules.water.type == "NONE")
            {
                waterDuration.text = "Not scheduled";
            }
            else
            {
                string durationText = schedules.water.GetFormattedTime1() + " for " + schedules.water.GetFormattedDuration1();
                
                // Add second time for TWICE_* schedules
                if (schedules.water.type.StartsWith("TWICE_"))
                {
                    durationText += "\n" + schedules.water.GetFormattedTime2() + " for " + schedules.water.GetFormattedDuration2();
                }
                
                waterDuration.text = durationText;
            }
        }
        
        // Display light schedule info
        if (schedules.light != null)
        {
            lightFrequency.text = schedules.light.GetTypeDescription();
            
            if (schedules.light.type == "ALWAYS_ON")
            {
                lightDuration.text = "Always On";
            }
            else if (schedules.light.type == "NONE")
            {
                lightDuration.text = "Not scheduled";
            }
            else
            {
                string durationText = schedules.light.GetFormattedTime1() + " for " + schedules.light.GetFormattedDuration1();
                
                // Add second time for TWICE_* schedules
                if (schedules.light.type.StartsWith("TWICE_"))
                {
                    durationText += "\n" + schedules.light.GetFormattedTime2() + " for " + schedules.light.GetFormattedDuration2();
                }
                
                lightDuration.text = durationText;
            }
        }
    }
}
