using System.Collections;
using UnityEngine;
using UnityEngine.UI;

public class ScheduleTestHelper : MonoBehaviour
{
    // Reference to update the UI after test
    public ScheduleUIManager scheduleUIManager;
    
    // Test buttons
    public Button testLightDailyButton;
    public Button testLightAlwaysOnButton;
    public Button testLightOffButton;
    public Button testWaterDailyButton;
    public Button testWaterAlwaysOnButton;
    public Button testWaterOffButton;
    
    private void Start()
    {
        // Set up button listeners
        if (testLightDailyButton != null)
            testLightDailyButton.onClick.AddListener(() => TestLightDaily());
            
        if (testLightAlwaysOnButton != null)
            testLightAlwaysOnButton.onClick.AddListener(() => TestLightAlwaysOn());
            
        if (testLightOffButton != null)
            testLightOffButton.onClick.AddListener(() => TestLightOff());
            
        if (testWaterDailyButton != null)
            testWaterDailyButton.onClick.AddListener(() => TestWaterDaily());
            
        if (testWaterAlwaysOnButton != null)
            testWaterAlwaysOnButton.onClick.AddListener(() => TestWaterAlwaysOn());
            
        if (testWaterOffButton != null)
            testWaterOffButton.onClick.AddListener(() => TestWaterOff());
    }
    
    // Light schedule tests
    public void TestLightDaily()
    {
        Debug.Log("Testing LIGHT DAILY schedule");
        SendTestSchedule("LIGHT,DAILY,8,30,3600");
    }
    
    public void TestLightAlwaysOn()
    {
        Debug.Log("Testing LIGHT ALWAYS_ON schedule");
        SendTestSchedule("LIGHT,ALWAYS_ON,0,0,0");
    }
    
    public void TestLightOff()
    {
        Debug.Log("Testing LIGHT OFF (NONE) schedule");
        SendTestSchedule("LIGHT,NONE,0,0,0");
    }
    
    // Water schedule tests
    public void TestWaterDaily()
    {
        Debug.Log("Testing WATER DAILY schedule");
        SendTestSchedule("WATER,DAILY,10,0,300");
    }
    
    public void TestWaterAlwaysOn()
    {
        Debug.Log("Testing WATER ALWAYS_ON schedule");
        SendTestSchedule("WATER,ALWAYS_ON,0,0,0");
    }
    
    public void TestWaterOff()
    {
        Debug.Log("Testing WATER OFF (NONE) schedule");
        SendTestSchedule("WATER,NONE,0,0,0");
    }
    
    // Test any arbitrary schedule command
    public void TestCustomSchedule(string target, string type, int hour, int minute, int durationSecs)
    {
        string command = $"{target},{type},{hour},{minute},{durationSecs}";
        SendTestSchedule(command);
    }
    
    // Core test functionality
    private void SendTestSchedule(string command)
    {
        StartCoroutine(SendTestScheduleCoroutine(command));
    }
    
    private IEnumerator SendTestScheduleCoroutine(string command)
    {
        // Check if connected
        if (!TerrariumBleController.Instance.IsConnected)
        {
            Debug.LogError("Cannot test schedule: BLE not connected");
            yield break;
        }
        
        // Send the command
        Debug.Log($"Sending test schedule command: {command}");
        TerrariumBleController.Instance.WriteSchedule(command);
        
        // Wait for processing
        yield return new WaitForSeconds(1.5f);
        
        // Read back the updated schedules
        Debug.Log("Reading back updated schedules...");
        TerrariumBleController.Instance.ReadSchedules();
        
        // Wait for the read to complete
        yield return new WaitForSeconds(1.5f);
        
        // Update the UI
        if (scheduleUIManager != null)
        {
            try
            {
                string schedulesJson = TerrariumBleController.Instance._schedulesString;
                Debug.Log($"Received schedules: {schedulesJson}");
                
                if (!string.IsNullOrEmpty(schedulesJson))
                {
                    SchedulesResponse data = JsonUtility.FromJson<SchedulesResponse>(schedulesJson);
                    if (data != null)
                    {
                        scheduleUIManager.DisplaySchedules(data);
                        Debug.Log("Schedule UI refreshed successfully");
                    }
                    else
                    {
                        Debug.LogError("Failed to parse schedule data");
                    }
                }
                else
                {
                    Debug.LogError("Received empty schedule data");
                }
            }
            catch (System.Exception ex)
            {
                Debug.LogError($"Error refreshing schedules: {ex.Message}");
            }
        }
    }
}
