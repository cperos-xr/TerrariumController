using System;
using System.Collections;
using TMPro;
using UnityEngine;

public class ClockUIManager : MonoBehaviour
{
    public TextMeshProUGUI currentRTCTimeText;
    private DateTime rtcDateTime;
    
    // Add a button reference for manual update
    public UnityEngine.UI.Button readRTCButton;

    private void OnEnable()
    {
        TerrariumBleController.OnConnectionComplete += HandleOnConnectionComplete;
        
        // Add button listener if assigned
        if (readRTCButton != null)
            readRTCButton.onClick.AddListener(ManualReadRTC);
    }

    private void OnDisable()
    {
        TerrariumBleController.OnConnectionComplete -= HandleOnConnectionComplete;
        
        // Remove button listener
        if (readRTCButton != null)
            readRTCButton.onClick.RemoveListener(ManualReadRTC);
    }

    public void ManualReadRTC()
    {
        Debug.Log("Manual RTC read requested");
        TerrariumBleController.Instance.ReadRTC();
        StopAllCoroutines();
        StartCoroutine(UpdateRTCAfterDelay(1.0f));
    }
    
    private void HandleOnConnectionComplete()
    {
        Debug.Log("Connection complete, waiting before retrieving RTC time...");
        // Add a delay before the first read to ensure BLE is fully ready
        StartCoroutine(InitialRTCReadWithDelay());
    }

    private IEnumerator InitialRTCReadWithDelay()
    {
        // Reduce delay from 3.0 to 1.0 seconds
        yield return new WaitForSeconds(1.0f);
        
        Debug.Log("Attempting to read RTC after delay");
        TerrariumBleController.Instance.ReadRTC();
        
        // Start waiting for data with multiple attempts
        StartCoroutine(UpdateRTCWithMultipleAttempts());
    }

    private IEnumerator UpdateRTCWithMultipleAttempts()
    {
        // Try up to 5 times with increasing delays
        for (int attempt = 0; attempt < 5; attempt++)
        {
            yield return new WaitForSeconds(1.0f + attempt * 0.5f); // Increasing delay
            
            try
            {
                string rawRTC = TerrariumBleController.Instance.GetRawRTCString();
                
                if (!string.IsNullOrEmpty(rawRTC))
                {
                    if (DateTime.TryParse(rawRTC, out rtcDateTime))
                    {
                        Debug.Log("Successfully parsed RTC: " + rtcDateTime);
                        currentRTCTimeText.text = rtcDateTime.ToString("yyyy-MM-dd HH:mm:ss");
                        StartCoroutine(UpdateTime());
                        
                        // Success - exit the retry loop
                        yield break;
                    }
                }
                
                // Try reading again
                TerrariumBleController.Instance.ReadRTC();
            }
            catch (Exception ex)
            {
                Debug.Log("Retry attempt " + (attempt + 1) + ": " + ex.Message);
                TerrariumBleController.Instance.ReadRTC();
            }
        }
    }
    
    private IEnumerator UpdateRTCAfterDelay(float delay)
    {
        yield return new WaitForSeconds(delay);
        
        try
        {
            string rawRTC = TerrariumBleController.Instance.GetRawRTCString();
            Debug.Log("Raw RTC string: " + rawRTC);
            
            if (!string.IsNullOrEmpty(rawRTC))
            {
                if (DateTime.TryParse(rawRTC, out rtcDateTime))
                {
                    Debug.Log("Successfully parsed RTC: " + rtcDateTime);
                    currentRTCTimeText.text = rtcDateTime.ToString("yyyy-MM-dd HH:mm:ss");
                    StartCoroutine(UpdateTime());
                }
                else
                {
                    Debug.LogError("Could not parse RTC string: " + rawRTC);
                }
            }
            else
            {
                Debug.LogWarning("RTC string is empty, trying again in 2 seconds");
                // Try again after delay
                StartCoroutine(RetryRTCRead());
            }
        }
        catch (Exception ex)
        {
            Debug.LogError("Error processing RTC: " + ex.Message);
            // Try again after delay
            StartCoroutine(RetryRTCRead());
        }
    }
    
    private IEnumerator RetryRTCRead()
    {
        yield return new WaitForSeconds(2.0f);
        TerrariumBleController.Instance.ReadRTC();
        StartCoroutine(UpdateRTCAfterDelay(1.0f));
    }

    private IEnumerator UpdateTime()
    {
        while (true)
        {
            yield return new WaitForSeconds(1f);
            rtcDateTime = rtcDateTime.AddSeconds(1);
            currentRTCTimeText.text = rtcDateTime.ToString("yyyy-MM-dd HH:mm:ss");
        }
    }
}
