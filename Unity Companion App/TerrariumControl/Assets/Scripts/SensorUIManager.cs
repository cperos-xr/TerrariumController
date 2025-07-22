using System;
using System.Collections;
using TMPro;
using UnityEngine;

public class SensorUIManager : MonoBehaviour
{
    public TextMeshProUGUI currentTemperatureText;
    public TextMeshProUGUI currentHumidityText;
    
    // Add a button reference for manual update
    public UnityEngine.UI.Button readSensorButton;
    
    // Update interval in seconds
    public float updateInterval = 3.0f;
    private Coroutine periodicUpdateCoroutine;

    private void OnEnable()
    {
        TerrariumBleController.OnConnectionComplete += HandleOnConnectionComplete;
        
        // Add button listener if assigned
        if (readSensorButton != null)
            readSensorButton.onClick.AddListener(ManualReadSensor);
    }

    private void OnDisable()
    {
        TerrariumBleController.OnConnectionComplete -= HandleOnConnectionComplete;
        
        if (periodicUpdateCoroutine != null)
            StopCoroutine(periodicUpdateCoroutine);
            
        // Remove button listener
        if (readSensorButton != null)
            readSensorButton.onClick.RemoveListener(ManualReadSensor);
    }
    
    private void ManualReadSensor()
    {
        Debug.Log("Manual sensor read requested");
        TerrariumBleController.Instance.ReadSensor();
        StartCoroutine(UpdateSensorAfterDelay(1.0f));
    }

    private void HandleOnConnectionComplete()
    {
        Debug.Log("Connection complete, waiting before retrieving sensor data...");
        // Add a delay before the first read to ensure BLE is fully ready
        StartCoroutine(InitialSensorReadWithDelay());
    }
    
    private IEnumerator InitialSensorReadWithDelay()
    {
        // Reduce delay from 3.0 to 1.0 seconds
        yield return new WaitForSeconds(1.0f);
        
        Debug.Log("Attempting to read sensor after delay");
        TerrariumBleController.Instance.ReadSensor();
        
        // Start waiting for data with multiple attempts
        StartCoroutine(UpdateSensorWithMultipleAttempts());
    }
    
    private IEnumerator UpdateSensorWithMultipleAttempts()
    {
        // Try up to 5 times with increasing delays
        for (int attempt = 0; attempt < 5; attempt++)
        {
            yield return new WaitForSeconds(1.0f + attempt * 0.5f); // Increasing delay
            
            try
            {
                string sensorJson = TerrariumBleController.Instance._sensorJson;
                
                if (!string.IsNullOrEmpty(sensorJson))
                {
                    SensorData data = JsonUtility.FromJson<SensorData>(sensorJson);
                    if (data != null)
                    {
                        Debug.Log("Successfully parsed sensor data: " + data.temperature + "°C, " + data.humidity + "%");
                        DisplayCurrentSensorData(data);
                        
                        // Start periodic updates
                        if (periodicUpdateCoroutine != null)
                            StopCoroutine(periodicUpdateCoroutine);
                        periodicUpdateCoroutine = StartCoroutine(PeriodicSensorUpdate());
                        
                        // Success - exit the retry loop
                        yield break;
                    }
                }
                
                // Try reading again
                TerrariumBleController.Instance.ReadSensor();
            }
            catch (Exception ex)
            {
                Debug.Log("Retry attempt " + (attempt + 1) + ": " + ex.Message);
                TerrariumBleController.Instance.ReadSensor();
            }
        }
        
        // After all attempts, start periodic updates anyway
        // It might work on subsequent reads
        if (periodicUpdateCoroutine != null)
            StopCoroutine(periodicUpdateCoroutine);
        periodicUpdateCoroutine = StartCoroutine(PeriodicSensorUpdate());
    }
    
    private IEnumerator UpdateSensorAfterDelay(float delay)
    {
        yield return new WaitForSeconds(delay);
        
        try
        {
            string sensorJson = TerrariumBleController.Instance._sensorJson;
            Debug.Log("Raw sensor JSON: " + sensorJson);
            
            if (!string.IsNullOrEmpty(sensorJson))
            {
                SensorData data = JsonUtility.FromJson<SensorData>(sensorJson);
                if (data != null)
                {
                    Debug.Log("Successfully parsed sensor data: " + data.temperature + "°C, " + data.humidity + "%");
                    DisplayCurrentSensorData(data);
                    
                    // Start periodic updates
                    if (periodicUpdateCoroutine != null)
                        StopCoroutine(periodicUpdateCoroutine);
                    periodicUpdateCoroutine = StartCoroutine(PeriodicSensorUpdate());
                }
                else
                {
                    Debug.LogError("Could not parse sensor JSON: " + sensorJson);
                    StartCoroutine(RetrySensorRead());
                }
            }
            else
            {
                Debug.LogWarning("Sensor JSON is empty, trying again in 2 seconds");
                StartCoroutine(RetrySensorRead());
            }
        }
        catch (Exception ex)
        {
            Debug.LogError("Error processing sensor data: " + ex.Message);
            StartCoroutine(RetrySensorRead());
        }
    }
    
    private IEnumerator RetrySensorRead()
    {
        yield return new WaitForSeconds(2.0f);
        TerrariumBleController.Instance.ReadSensor();
        StartCoroutine(UpdateSensorAfterDelay(1.0f));
    }
    
    private IEnumerator PeriodicSensorUpdate()
    {
        while (true)
        {
            yield return new WaitForSeconds(updateInterval);
            Debug.Log("Periodic sensor update");
            TerrariumBleController.Instance.ReadSensor();
            
            yield return new WaitForSeconds(1.0f);
            try
            {
                SensorData data = JsonUtility.FromJson<SensorData>(TerrariumBleController.Instance._sensorJson);
                if (data != null)
                {
                    DisplayCurrentSensorData(data);
                }
            }
            catch (Exception ex)
            {
                Debug.LogWarning("Error in periodic update: " + ex.Message);
                // Continue trying
            }
        }
    }

    public void DisplayCurrentSensorData(SensorData sensorData)
    {
        currentTemperatureText.text = sensorData.temperature.ToString("F1") + " °C";
        currentHumidityText.text = sensorData.humidity.ToString("F1") + " %";
    }
}