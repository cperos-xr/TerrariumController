using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class ScheduleWizard : MonoBehaviour
{
    [Header("Wizard UI Elements")]
    public TextMeshProUGUI pageTitle;
    public TextMeshProUGUI instructionText;
    
    [Header("Navigation Buttons")]
    public Button nextButton;
    public Button backButton;
    public Button finishButton;
    public Button closeButton;
    
    [Header("Primary Time Settings")]
    public TMP_Dropdown frequencyDropdown;
    public TMP_Dropdown hourDropdown;
    public TMP_Dropdown minuteDropdown;
    public TMP_Dropdown amPmDropdown;
    public TMP_Dropdown durationDropdown;
    
    [Header("Secondary Time Settings (for twice-daily, etc.)")]
    public GameObject secondTimeContainer; // Parent for second time settings
    public TMP_Dropdown hourDropdown2;
    public TMP_Dropdown minuteDropdown2;
    public TMP_Dropdown amPmDropdown2;
    public TMP_Dropdown durationDropdown2;
    
    [Header("Status and Feedback")]
    public TextMeshProUGUI statusText;
    public GameObject loadingIndicator;
    
    [Header("Summary View")]
    public GameObject summaryContainer;
    public TextMeshProUGUI lightSummary;
    public TextMeshProUGUI waterSummary;
    public TextMeshProUGUI foggerSummary;
    
    // Schedule Update Manager Reference
    public ScheduleUpdateManager updateManager;
    
    private int _currentPage = 0;
    private const int TOTAL_PAGES = 4; // 0=Light, 1=Water, 2=Fogger, 3=Summary
    
    // Store the selected values
    private int _lightFrequency, _lightHour, _lightMinute, _lightAmPm, _lightDuration;
    private int _lightHour2, _lightMinute2, _lightAmPm2, _lightDuration2;
    
    private int _waterFrequency, _waterHour, _waterMinute, _waterAmPm, _waterDuration;
    private int _waterHour2, _waterMinute2, _waterAmPm2, _waterDuration2;
    
    private int _foggerFrequency, _foggerHour, _foggerMinute, _foggerAmPm;
    private int _foggerHour2, _foggerMinute2, _foggerAmPm2;
    
    // Duration options in seconds - removed 10 and 30 second options
    private int[] _durationValues = new int[] { 
        60, 300, 600, 900, 1800, 3600, 10800, 21600, 32400, 43200, 64800 
    };
    
    // Duration display labels - removed 10 and 30 second options
    private string[] _durationLabels = new string[] {
        "1 minute", "5 minutes", "10 minutes", 
        "15 minutes", "30 minutes", "1 hour", "3 hours", "6 hours", 
        "9 hours", "12 hours", "18 hours"
    };
    
    void Start()
    {
        // Setup navigation buttons
        nextButton.onClick.AddListener(NextPage);
        backButton.onClick.AddListener(PreviousPage);
        finishButton.onClick.AddListener(ApplySettings);
        closeButton.onClick.AddListener(CloseWizard);  // Add listener for close button
        
        // Initialize dropdowns
        SetupFrequencyDropdown();
        SetupTimeDropdowns();
        SetupDurationDropdown();
        
        // Set initial page
        UpdatePage(0);
        
        // Listen for updates
        ScheduleUpdateManager.OnSchedulesUpdated += OnSchedulesUpdated;
        
        // Attach listener to frequency dropdown to show/hide second time settings
        frequencyDropdown.onValueChanged.AddListener(OnFrequencyChanged);
    }
    
    public void CloseWizard()
    {
        // Hide the wizard
        gameObject.SetActive(false);
        
        // Reset status text and loading indicator
        statusText.text = "";
        loadingIndicator.SetActive(false);
    }
    
    private void SetupFrequencyDropdown()
    {
        frequencyDropdown.ClearOptions();

        List<string> options = new List<string>
        {
            "Always On",
            "Daily",
            "Twice Daily",
            "Weekly",
            "Twice Weekly",
            "Monthly",
            "Twice Monthly",
            "Off"
        };

        frequencyDropdown.AddOptions(options);
    }
    
    private void SetupTimeDropdowns()
    {
        // Hours (1-12)
        SetupHourDropdown(hourDropdown);
        SetupHourDropdown(hourDropdown2);
        
        // Minutes (0, 15, 30, 45)
        SetupMinuteDropdown(minuteDropdown);
        SetupMinuteDropdown(minuteDropdown2);
        
        // AM/PM
        SetupAmPmDropdown(amPmDropdown);
        SetupAmPmDropdown(amPmDropdown2);
    }
    
    private void SetupHourDropdown(TMP_Dropdown dropdown)
    {
        dropdown.ClearOptions();
        List<string> hourOptions = new List<string>();
        for (int i = 1; i <= 12; i++)
        {
            hourOptions.Add(i.ToString());
        }
        dropdown.AddOptions(hourOptions);
    }
    
    private void SetupMinuteDropdown(TMP_Dropdown dropdown)
    {
        dropdown.ClearOptions();
        List<string> minuteOptions = new List<string>();
        
        // Generate options from 0 to 55 minutes in 5-minute increments
        for (int i = 0; i <= 55; i += 5)
        {
            // Format as "00", "05", "10", etc.
            minuteOptions.Add(i.ToString("00"));
        }
        
        dropdown.AddOptions(minuteOptions);
    }
    
    private void SetupAmPmDropdown(TMP_Dropdown dropdown)
    {
        dropdown.ClearOptions();
        List<string> amPmOptions = new List<string>
        {
            "AM",
            "PM"
        };
        dropdown.AddOptions(amPmOptions);
    }
    
    private void SetupDurationDropdown()
    {
        durationDropdown.ClearOptions();
        durationDropdown2.ClearOptions();
        
        List<string> options = new List<string>(_durationLabels);
        durationDropdown.AddOptions(options);
        durationDropdown2.AddOptions(options);
    }
    
    private void OnFrequencyChanged(int frequencyIndex)
    {
        // Show second time settings only for twice-daily/weekly/monthly
        bool showSecondTime = (frequencyIndex == 2 || frequencyIndex == 4 || frequencyIndex == 6);
        secondTimeContainer.SetActive(showSecondTime);
        
        // Also hide duration for "Always On" and "Off"
        bool showDuration = (frequencyIndex != 0 && frequencyIndex != 7);
        durationDropdown.transform.parent.gameObject.SetActive(showDuration);
        
        // For fogger page, hide duration settings completely
        if (_currentPage == 2) // Fogger page
        {
            durationDropdown.transform.parent.gameObject.SetActive(false);
            durationDropdown2.transform.parent.gameObject.SetActive(false);
        }
    }
    
    public void NextPage()
    {
        // Save current page settings
        SaveCurrentPageSettings();
        
        if (_currentPage < TOTAL_PAGES - 1)
        {
            UpdatePage(_currentPage + 1);
        }
    }
    
    public void PreviousPage()
    {
        if (_currentPage > 0)
        {
            UpdatePage(_currentPage - 1);
        }
    }
    
    private void UpdatePage(int pageIndex)
    {
        _currentPage = pageIndex;
        
        // Set visibility of regular inputs vs summary
        bool isOnSummaryPage = (_currentPage == TOTAL_PAGES - 1);
        summaryContainer.SetActive(isOnSummaryPage);
        
        // Show/hide primary settings based on whether we're on summary page
        frequencyDropdown.transform.parent.gameObject.SetActive(!isOnSummaryPage);
        hourDropdown.transform.parent.gameObject.SetActive(!isOnSummaryPage);
        minuteDropdown.transform.parent.gameObject.SetActive(!isOnSummaryPage);
        amPmDropdown.transform.parent.gameObject.SetActive(!isOnSummaryPage);
        
        // Hide finish button except on last page
        finishButton.gameObject.SetActive(isOnSummaryPage);
        nextButton.gameObject.SetActive(!isOnSummaryPage);
        
        // Enable/disable back button based on first page
        backButton.interactable = (_currentPage > 0);
        
        if (isOnSummaryPage)
        {
            // Update summary page content
            UpdateSummaryPage();
            return;
        }
        
        // Update UI based on current page
        switch (_currentPage)
        {
            case 0: // Light
                pageTitle.text = "Light Schedule";
                instructionText.text = "I want my LIGHTS to run:";
                LoadLightSettings();
                break;
                
            case 1: // Water
                pageTitle.text = "Water Schedule";
                instructionText.text = "I want my WATER to run:";
                LoadWaterSettings();
                break;
                
            case 2: // Fogger
                pageTitle.text = "Fogger Schedule";
                instructionText.text = "I want my FOGGER to run:\n(Note: Fogger runs for 4 hours each activation)";
                LoadFoggerSettings();
                
                // Hide duration dropdowns for fogger
                durationDropdown.transform.parent.gameObject.SetActive(false);
                durationDropdown2.transform.parent.gameObject.SetActive(false);
                break;
        }
        
        // Update visibility of second time settings based on current frequency
        OnFrequencyChanged(frequencyDropdown.value);
    }
    
    private void SaveCurrentPageSettings()
    {
        switch (_currentPage)
        {
            case 0: // Light page
                _lightFrequency = frequencyDropdown.value;
                _lightHour = hourDropdown.value;
                _lightMinute = minuteDropdown.value;
                _lightAmPm = amPmDropdown.value;
                _lightDuration = durationDropdown.value;
                
                // Save second time settings if applicable
                if (secondTimeContainer.activeSelf)
                {
                    _lightHour2 = hourDropdown2.value;
                    _lightMinute2 = minuteDropdown2.value;
                    _lightAmPm2 = amPmDropdown2.value;
                    _lightDuration2 = durationDropdown2.value;
                }
                break;
                
            case 1: // Water page
                _waterFrequency = frequencyDropdown.value;
                _waterHour = hourDropdown.value;
                _waterMinute = minuteDropdown.value;
                _waterAmPm = amPmDropdown.value;
                _waterDuration = durationDropdown.value;
                
                // Save second time settings if applicable
                if (secondTimeContainer.activeSelf)
                {
                    _waterHour2 = hourDropdown2.value;
                    _waterMinute2 = minuteDropdown2.value;
                    _waterAmPm2 = amPmDropdown2.value;
                    _waterDuration2 = durationDropdown2.value;
                }
                break;
                
            case 2: // Fogger page
                _foggerFrequency = frequencyDropdown.value;
                _foggerHour = hourDropdown.value;
                _foggerMinute = minuteDropdown.value;
                _foggerAmPm = amPmDropdown.value;
                
                // Save second time settings if applicable
                if (secondTimeContainer.activeSelf)
                {
                    _foggerHour2 = hourDropdown2.value;
                    _foggerMinute2 = minuteDropdown2.value;
                    _foggerAmPm2 = amPmDropdown2.value;
                }
                break;
        }
    }
    
    private void LoadLightSettings()
    {
        frequencyDropdown.value = _lightFrequency;
        hourDropdown.value = _lightHour;
        minuteDropdown.value = _lightMinute;
        amPmDropdown.value = _lightAmPm;
        durationDropdown.value = _lightDuration;
        
        // Load second time settings
        hourDropdown2.value = _lightHour2;
        minuteDropdown2.value = _lightMinute2;
        amPmDropdown2.value = _lightAmPm2;
        durationDropdown2.value = _lightDuration2;
    }
    
    private void LoadWaterSettings()
    {
        frequencyDropdown.value = _waterFrequency;
        hourDropdown.value = _waterHour;
        minuteDropdown.value = _waterMinute;
        amPmDropdown.value = _waterAmPm;
        durationDropdown.value = _waterDuration;
        
        // Load second time settings
        hourDropdown2.value = _waterHour2;
        minuteDropdown2.value = _waterMinute2;
        amPmDropdown2.value = _waterAmPm2;
        durationDropdown2.value = _waterDuration2;
    }
    
    private void LoadFoggerSettings()
    {
        frequencyDropdown.value = _foggerFrequency;
        hourDropdown.value = _foggerHour;
        minuteDropdown.value = _foggerMinute;
        amPmDropdown.value = _foggerAmPm;
        
        // Load second time settings
        hourDropdown2.value = _foggerHour2;
        minuteDropdown2.value = _foggerMinute2;
        amPmDropdown2.value = _foggerAmPm2;
    }
    
    private void UpdateSummaryPage()
    {
        pageTitle.text = "Schedule Summary";
        instructionText.text = "Review your terrarium schedule settings:";
        
        // Update summary texts
        lightSummary.text = FormatSummary("Light", _lightFrequency, _lightHour, _lightMinute, 
                                         _lightAmPm, _lightDuration, _lightHour2, _lightMinute2, 
                                         _lightAmPm2, _lightDuration2);
        
        waterSummary.text = FormatSummary("Water", _waterFrequency, _waterHour, _waterMinute, 
                                         _waterAmPm, _waterDuration, _waterHour2, _waterMinute2, 
                                         _waterAmPm2, _waterDuration2);
        
        foggerSummary.text = FormatSummary("Fogger", _foggerFrequency, _foggerHour, _foggerMinute, 
                                          _foggerAmPm, -1, _foggerHour2, _foggerMinute2, 
                                          _foggerAmPm2, -1, true);
    }
    
    private string FormatSummary(string deviceName, int frequency, int hour1, int minute1, 
                                int amPm1, int duration1, int hour2, int minute2, 
                                int amPm2, int duration2, bool isFogger = false)
    {
        string frequencyStr = GetFrequencyString(frequency);
        
        if (frequency == 0) // Always On
        {
            return $"{deviceName}: {frequencyStr}";
        }
        else if (frequency == 7) // Off
        {
            return $"{deviceName}: {frequencyStr}";
        }
        
        // Format first time
        int displayHour1 = hour1 + 1; // Adjust for 0-based index
        string minuteStr1 = (minute1 * 5).ToString("00"); // Multiply by 5 instead of 15
        string amPmStr1 = amPm1 == 0 ? "AM" : "PM";
        
        string timeStr1 = $"{displayHour1}:{minuteStr1} {amPmStr1}";
        
        // Format second time for twice-daily/weekly/monthly
        string secondTimeStr = "";
        if (frequency == 2 || frequency == 4 || frequency == 6) // Twice-daily, twice-weekly, twice-monthly
        {
            int displayHour2 = hour2 + 1;
            string minuteStr2 = (minute2 * 5).ToString("00"); // Multiply by 5 instead of 15
            string amPmStr2 = amPm2 == 0 ? "AM" : "PM";
            
            string timeStr2 = $"{displayHour2}:{minuteStr2} {amPmStr2}";
            
            if (!isFogger)
            {
                secondTimeStr = $" and {timeStr2} for {_durationLabels[duration2]}";
            }
            else
            {
                secondTimeStr = $" and {timeStr2} (4 hours each)";
            }
        }
        
        // Add duration for non-fogger devices
        if (!isFogger)
        {
            return $"{deviceName}: {frequencyStr} at {timeStr1} for {_durationLabels[duration1]}{secondTimeStr}";
        }
        else
        {
            return $"{deviceName}: {frequencyStr} at {timeStr1} (runs for 4 hours){secondTimeStr}";
        }
    }
    
    private string GetFrequencyString(int frequency)
    {
        switch (frequency)
        {
            case 0: return "Always On";
            case 1: return "Daily";
            case 2: return "Twice Daily";
            case 3: return "Weekly";
            case 4: return "Twice Weekly";
            case 5: return "Monthly";
            case 6: return "Twice Monthly";
            case 7: return "Off";
            default: return "Unknown";
        }
    }
    
    private ScheduleUpdateManager.ScheduleFrequency ConvertFrequency(int frequency)
    {
        switch (frequency)
        {
            case 0: return ScheduleUpdateManager.ScheduleFrequency.AlwaysOn;
            case 1: return ScheduleUpdateManager.ScheduleFrequency.Daily;
            case 2: return ScheduleUpdateManager.ScheduleFrequency.TwiceDaily;
            case 3: return ScheduleUpdateManager.ScheduleFrequency.Weekly;
            case 4: return ScheduleUpdateManager.ScheduleFrequency.TwiceWeekly;
            case 5: return ScheduleUpdateManager.ScheduleFrequency.Monthly;
            case 6: return ScheduleUpdateManager.ScheduleFrequency.TwiceMonthly;
            case 7: return ScheduleUpdateManager.ScheduleFrequency.None;
            default: return ScheduleUpdateManager.ScheduleFrequency.None;
        }
    }
    
    public void ApplySettings()
    {
        // Show loading indicator
        loadingIndicator.SetActive(true);
        statusText.text = "Applying settings...";
        
        // Process in coroutine to avoid blocking main thread
        StartCoroutine(ApplySettingsCoroutine());
    }
    
    private IEnumerator ApplySettingsCoroutine()
    {
        // Apply Light Settings
        yield return StartCoroutine(ApplyDeviceSettings(
            ScheduleUpdateManager.ScheduleTarget.Light,
            _lightFrequency,
            _lightHour,
            _lightMinute,
            _lightAmPm,
            _lightDuration,
            _lightHour2,
            _lightMinute2,
            _lightAmPm2,
            _lightDuration2
        ));
        
        // Apply Water Settings
        yield return StartCoroutine(ApplyDeviceSettings(
            ScheduleUpdateManager.ScheduleTarget.Water,
            _waterFrequency,
            _waterHour,
            _waterMinute,
            _waterAmPm,
            _waterDuration,
            _waterHour2,
            _waterMinute2,
            _waterAmPm2,
            _waterDuration2
        ));

        yield return new WaitForSeconds(2.2f);
        
        // Apply Fogger Settings (use 1 as duration placeholder - fogger has 4-hour built-in timer)
        yield return StartCoroutine(ApplyDeviceSettings(
            ScheduleUpdateManager.ScheduleTarget.Fogger,
            _foggerFrequency,
            _foggerHour,
            _foggerMinute,
            _foggerAmPm,
            60, // Placeholder for fogger
            _foggerHour2,
            _foggerMinute2,
            _foggerAmPm2,
            60  // Placeholder for fogger
        ));
        
        // All done
        statusText.text = "All settings applied successfully!";
        loadingIndicator.SetActive(false);
        
        // Wait 2 seconds then hide the wizard
        yield return new WaitForSeconds(2f);
        gameObject.SetActive(false);
    }
    
    private IEnumerator ApplyDeviceSettings(
        ScheduleUpdateManager.ScheduleTarget target,
        int frequency,
        int hour1,
        int minute1,
        int amPm1,
        int durationIndex1,
        int hour2 = 0,
        int minute2 = 0,
        int amPm2 = 0,
        int durationIndex2 = 0)
    {
        ScheduleUpdateManager.ScheduleFrequency scheduleFrequency = ConvertFrequency(frequency);
        
        // For "Off" or "Always On", no need for time settings
        if (scheduleFrequency == ScheduleUpdateManager.ScheduleFrequency.None)
        {
            updateManager.DisableSchedule(target);
            statusText.text = $"Disabled {target} schedule...";
            yield return new WaitForSeconds(0.5f);
            yield break;
        }
        else if (scheduleFrequency == ScheduleUpdateManager.ScheduleFrequency.AlwaysOn)
        {
            updateManager.SetAlwaysOn(target);
            statusText.text = $"Set {target} to Always On...";
            yield return new WaitForSeconds(0.5f);
            yield break;
        }
        
        // Convert UI values to 24-hour time for first schedule
        int hourValue1 = hour1 + 1; // Adjust for 0-based index
        if (amPm1 == 1 && hourValue1 < 12) // PM and not 12 PM
        {
            hourValue1 += 12;
        }
        else if (amPm1 == 0 && hourValue1 == 12) // 12 AM
        {
            hourValue1 = 0;
        }
        
        // Convert dropdown minute (0-12) to actual minutes (0, 5, 10, 15, ..., 55)
        int minuteValue1 = minute1 * 5;
        
        // Get duration in seconds
        int durationSeconds1 = (target == ScheduleUpdateManager.ScheduleTarget.Fogger) 
            ? 60 // Placeholder for fogger (uses its own 4-hour timer)
            : _durationValues[durationIndex1];
            
        // For twice-daily, twice-weekly, twice-monthly schedules
        if (scheduleFrequency == ScheduleUpdateManager.ScheduleFrequency.TwiceDaily || 
            scheduleFrequency == ScheduleUpdateManager.ScheduleFrequency.TwiceWeekly || 
            scheduleFrequency == ScheduleUpdateManager.ScheduleFrequency.TwiceMonthly)
        {
            // Convert UI values to 24-hour time for second schedule
            int hourValue2 = hour2 + 1; // Adjust for 0-based index
            if (amPm2 == 1 && hourValue2 < 12) // PM and not 12 PM
            {
                hourValue2 += 12;
            }
            else if (amPm2 == 0 && hourValue2 == 12) // 12 AM
            {
                hourValue2 = 0;
            }
            
            // Convert dropdown minute (0-3) to actual minutes (0, 15, 30, 45)
            int minuteValue2 = minute2 * 5;
            
            // Get duration in seconds for second schedule
            int durationSeconds2 = (target == ScheduleUpdateManager.ScheduleTarget.Fogger) 
                ? 1 // Placeholder for fogger (uses its own 4-hour timer)
                : _durationValues[durationIndex2];
            
            // Update the schedule with two times
            updateManager.UpdateTwiceSchedule(
                target,
                scheduleFrequency,
                hourValue1,
                minuteValue1,
                hourValue2,
                minuteValue2,
                durationSeconds1
            );
        }
        else
        {
            // Update the schedule with single time
            updateManager.UpdateSchedule(
                target,
                scheduleFrequency,
                hourValue1,
                minuteValue1,
                durationSeconds1
            );
        }
        
        statusText.text = $"Updated {target} schedule...";
        yield return new WaitForSeconds(0.5f);
    }
    
    private void OnSchedulesUpdated()
    {
        // This is called when schedules are successfully updated
        statusText.text = "All devices scheduled successfully!";
    }
    
    private void OnDestroy()
    {
        // Remove event listener
        ScheduleUpdateManager.OnSchedulesUpdated -= OnSchedulesUpdated;
    }

    /// <summary>
    /// Public method to run the wizard again.
    /// Can be assigned to a button in the UI.
    /// </summary>
    public void RunWizard()
    {
        // Reset the wizard to the first page
        _currentPage = 0;
        
        // Reset status text
        statusText.text = "";
        
        // Ensure loading indicator is hidden
        loadingIndicator.SetActive(false);
        
        // Update the UI for the first page
        UpdatePage(0);
        
        // Make the wizard visible if it was hidden
        gameObject.SetActive(true);
    }
}