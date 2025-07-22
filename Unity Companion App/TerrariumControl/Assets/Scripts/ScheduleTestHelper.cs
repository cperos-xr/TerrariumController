using System.Collections;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class ScheduleTestHelper : MonoBehaviour
{
    // Reference to update the UI after test
    public ScheduleUIManager scheduleUIManager;
    
    // Test buttons
    public Button testLightDailyButton;
    public Button testLightAlwaysOnButton;
    public Button testLightOffButton;
    public Button toggleLightButton;
    
    public Button testWaterDailyButton;
    public Button testWaterAlwaysOnButton;
    public Button testWaterOffButton;
    public Button toggleWaterButton;
    
    public Button testFoggerButton;
    public Button testFoggerDailyButton;
    public Button testFoggerAlwaysOnButton;
    public Button testFoggerOffButton;
    
    // Optional Text components to show current status on toggle buttons
    public TextMeshProUGUI toggleLightButtonText;
    public TextMeshProUGUI toggleWaterButtonText;
    
    // Confirmation dialog for toggling when schedules exist
    public GameObject confirmationDialog;
    public TextMeshProUGUI confirmationText;
    public Button confirmButton;
    public Button cancelButton;
    
    // Store original schedules for restoration
    private string _savedLightSchedule = null;
    private string _savedWaterSchedule = null;
    private bool _isRestoringLight = false;
    private bool _isRestoringWater = false;
    
    private void Start()
    {
        // Set up button listeners
        if (testLightDailyButton != null)
            testLightDailyButton.onClick.AddListener(() => TestLightDaily());
            
        if (testLightAlwaysOnButton != null)
            testLightAlwaysOnButton.onClick.AddListener(() => TestLightAlwaysOn());
            
        if (testLightOffButton != null)
            testLightOffButton.onClick.AddListener(() => TestLightOff());
            
        if (toggleLightButton != null)
            toggleLightButton.onClick.AddListener(() => ToggleLight());
            
        if (testWaterDailyButton != null)
            testWaterDailyButton.onClick.AddListener(() => TestWaterDaily());
            
        if (testWaterAlwaysOnButton != null)
            testWaterAlwaysOnButton.onClick.AddListener(() => TestWaterAlwaysOn());
            
        if (testWaterOffButton != null)
            testWaterOffButton.onClick.AddListener(() => TestWaterOff());
            
        if (toggleWaterButton != null)
            toggleWaterButton.onClick.AddListener(() => ToggleWater());
            
        if (testFoggerButton != null)
            testFoggerButton.onClick.AddListener(() => TestFoggerButtonPress());
            
        if (testFoggerDailyButton != null)
            testFoggerDailyButton.onClick.AddListener(() => TestFoggerDaily());
        
        if (testFoggerAlwaysOnButton != null)
            testFoggerAlwaysOnButton.onClick.AddListener(() => TestFoggerAlwaysOn());
        
        if (testFoggerOffButton != null)
            testFoggerOffButton.onClick.AddListener(() => TestFoggerOff());
            
        // Hide confirmation dialog initially
        if (confirmationDialog != null)
            confirmationDialog.SetActive(false);
    }
    
    // Toggle functions
    public void ToggleLight()
    {
        StartCoroutine(ToggleLightCoroutine());
    }
    
    public void ToggleWater()
    {
        StartCoroutine(ToggleWaterCoroutine());
    }
    
    private IEnumerator ToggleLightCoroutine()
    {
        // Check if connected
        if (!TerrariumBleController.Instance.IsConnected)
        {
            Debug.LogError("Cannot toggle light: BLE not connected");
            yield break;
        }
        
        // Read current schedules first
        TerrariumBleController.Instance.ReadSchedules();
        
        // Wait for the read to complete
        yield return new WaitForSeconds(1.0f);
        
        bool isLightOn = false;
        bool hasSchedule = false;
        
        // Check current light state
        try
        {
            string schedulesJson = TerrariumBleController.Instance._schedulesString;
            if (!string.IsNullOrEmpty(schedulesJson))
            {
                SchedulesResponse data = JsonUtility.FromJson<SchedulesResponse>(schedulesJson);
                if (data != null && data.light != null)
                {
                    // Use the correct property name instead of 'frequency'
                    // This could be 'type', 'mode', 'scheduleType', etc.
                    isLightOn = data.light.type == "ALWAYS_ON";  // Replace 'type' with the actual property name
                    
                    // Check if there's a real schedule (not ALWAYS_ON or NONE)
                    hasSchedule = data.light.type != "ALWAYS_ON" && 
                                  data.light.type != "NONE" &&
                                  !string.IsNullOrEmpty(data.light.type);  // Replace 'type' with the actual property name
                    
                    // Save the current schedule if we're not already in a restoration process
                    if (hasSchedule && !_isRestoringLight)
                    {
                        _savedLightSchedule = JsonUtility.ToJson(data.light);
                    }
                }
            }
        }
        catch (System.Exception ex)
        {
            Debug.LogError($"Error checking light state: {ex.Message}");
        }
        
        // If there's a real schedule and we're not restoring, ask for confirmation
        if (hasSchedule && !_isRestoringLight && confirmationDialog != null)
        {
            confirmationDialog.SetActive(true);
            if (confirmationText != null)
            {
                confirmationText.text = "This will temporarily override your light schedule. Continue?";
            }
            
            // Set up confirmation buttons
            if (confirmButton != null)
            {
                confirmButton.onClick.RemoveAllListeners();
                confirmButton.onClick.AddListener(() => {
                    confirmationDialog.SetActive(false);
                    ExecuteLightToggle(isLightOn);
                });
            }
            
            if (cancelButton != null)
            {
                cancelButton.onClick.RemoveAllListeners();
                cancelButton.onClick.AddListener(() => {
                    confirmationDialog.SetActive(false);
                });
            }
            
            yield break;
        }
        
        // If we're here, either there's no schedule or the user confirmed
        ExecuteLightToggle(isLightOn);
    }
    
    private void ExecuteLightToggle(bool isCurrentlyOn)
    {
        if (isCurrentlyOn)
        {
            Debug.Log("Turning light OFF");
            SendTestSchedule("LIGHT,NONE,0,0,0");
            if (toggleLightButtonText != null)
                toggleLightButtonText.text = "Light: OFF";
        }
        else
        {
            Debug.Log("Turning light ON");
            SendTestSchedule("LIGHT,ALWAYS_ON,0,0,0");
            if (toggleLightButtonText != null)
                toggleLightButtonText.text = "Light: ON";
                
            // Add option to restore original schedule
            if (!string.IsNullOrEmpty(_savedLightSchedule) && toggleLightButtonText != null)
            {
                toggleLightButtonText.text = "Light: ON (tap to restore schedule)";
                _isRestoringLight = true;
            }
        }
    }
    
    private IEnumerator ToggleWaterCoroutine()
    {
        // Check if connected
        if (!TerrariumBleController.Instance.IsConnected)
        {
            Debug.LogError("Cannot toggle water: BLE not connected");
            yield break;
        }
        
        // Read current schedules first
        TerrariumBleController.Instance.ReadSchedules();
        
        // Wait for the read to complete
        yield return new WaitForSeconds(1.0f);
        
        bool isWaterOn = false;
        bool hasSchedule = false;
        
        // Check current water state
        try
        {
            string schedulesJson = TerrariumBleController.Instance._schedulesString;
            if (!string.IsNullOrEmpty(schedulesJson))
            {
                SchedulesResponse data = JsonUtility.FromJson<SchedulesResponse>(schedulesJson);
                if (data != null && data.water != null)
                {
                    isWaterOn = data.water.type == "ALWAYS_ON";
                    
                    // Check if there's a real schedule (not ALWAYS_ON or NONE)
                    hasSchedule = data.water.type != "ALWAYS_ON" && 
                                  data.water.type != "NONE" &&
                                  !string.IsNullOrEmpty(data.water.type);
                    
                    // Save the current schedule if we're not already in a restoration process
                    if (hasSchedule && !_isRestoringWater)
                    {
                        _savedWaterSchedule = JsonUtility.ToJson(data.water);
                    }
                }
            }
        }
        catch (System.Exception ex)
        {
            Debug.LogError($"Error checking water state: {ex.Message}");
        }
        
        // If there's a real schedule and we're not restoring, ask for confirmation
        if (hasSchedule && !_isRestoringWater && confirmationDialog != null)
        {
            confirmationDialog.SetActive(true);
            if (confirmationText != null)
            {
                confirmationText.text = "This will temporarily override your water schedule. Continue?";
            }
            
            // Set up confirmation buttons
            if (confirmButton != null)
            {
                confirmButton.onClick.RemoveAllListeners();
                confirmButton.onClick.AddListener(() => {
                    confirmationDialog.SetActive(false);
                    ExecuteWaterToggle(isWaterOn);
                });
            }
            
            if (cancelButton != null)
            {
                cancelButton.onClick.RemoveAllListeners();
                cancelButton.onClick.AddListener(() => {
                    confirmationDialog.SetActive(false);
                });
            }
            
            yield break;
        }
        
        // If we're here, either there's no schedule or the user confirmed
        ExecuteWaterToggle(isWaterOn);
    }
    
    private void ExecuteWaterToggle(bool isCurrentlyOn)
    {
        if (isCurrentlyOn)
        {
            Debug.Log("Turning water OFF");
            SendTestSchedule("WATER,NONE,0,0,0");
            if (toggleWaterButtonText != null)
                toggleWaterButtonText.text = "Water: OFF";
        }
        else
        {
            Debug.Log("Turning water ON");
            SendTestSchedule("WATER,ALWAYS_ON,0,0,0");
            if (toggleWaterButtonText != null)
                toggleWaterButtonText.text = "Water: ON";
                
            // Add option to restore original schedule
            if (!string.IsNullOrEmpty(_savedWaterSchedule) && toggleWaterButtonText != null)
            {
                toggleWaterButtonText.text = "Water: ON (tap to restore schedule)";
                _isRestoringWater = true;
            }
        }
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
    
    // Fogger schedule tests
    public void TestFoggerDaily()
    {
        Debug.Log("Testing FOGGER DAILY schedule");
        SendTestSchedule("FOGGER,DAILY,12,0,120");
    }

    public void TestFoggerAlwaysOn()
    {
        Debug.Log("Testing FOGGER ALWAYS_ON schedule");
        SendTestSchedule("FOGGER,ALWAYS_ON,0,0,0");
    }

    public void TestFoggerOff()
    {
        Debug.Log("Testing FOGGER OFF (NONE) schedule");
        SendTestSchedule("FOGGER,NONE,0,0,0");
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
    
    // Fogger button test
    public void TestFoggerButtonPress()
    {
        Debug.Log("Testing FOGGER button press");
        StartCoroutine(SendFoggerButtonPress());
    }

    private IEnumerator SendFoggerButtonPress()
    {
        // Check if connected
        if (!TerrariumBleController.Instance.IsConnected)
        {
            Debug.LogError("Cannot press fogger button: BLE not connected");
            yield break;
        }
        
        // Send the command
        Debug.Log("Sending fogger button press command");
        TerrariumBleController.Instance.PressFoggerButton();
        
        yield return new WaitForSeconds(0.5f);
        
        Debug.Log("Fogger button press command sent");
    }
}
