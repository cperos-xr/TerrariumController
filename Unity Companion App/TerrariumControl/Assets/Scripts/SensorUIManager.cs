using System;
using System.Collections;
using TMPro;
using UnityEngine;

public class SensorUIManager : MonoBehaviour
{
    public TextMeshProUGUI currentTemperatureText;
    public TextMeshProUGUI currentHumidityText;

    private void OnEnable()
    {
        TerrariumBleController.OnConnectionComplete += HandleOnConnectionComplete;
    }

    private void OnDisable()
    {
        TerrariumBleController.OnConnectionComplete -= HandleOnConnectionComplete;
    }

    private void HandleOnConnectionComplete()
    {
        Debug.Log("Connection complete, retrieving sensor data...");
        TerrariumBleController.Instance.ReadSensor();
        StartCoroutine(WaitForSensorDataAndUpdate());
    }

    private IEnumerator WaitForSensorDataAndUpdate()
    {
        float timeout = 5f;
        float elapsed = 0f;

        while (true)
        {
            SensorData data = null;
            try
            {
                data = TerrariumBleController.Instance.GetSensorData();
            }
            catch
            {
                // Not yet read, keep waiting
            }

            if (data != null)
            {
                DisplayCurrentSensorData(data);
                break;
            }

            if (elapsed > timeout)
            {
                Debug.LogError("Timeout waiting for sensor data from BLE device.");
                yield break;
            }
            yield return null;
            elapsed += Time.deltaTime;
        }
    }

    public void DisplayCurrentSensorData(SensorData sensorData)
    {
        currentTemperatureText.text = sensorData.temperature.ToString("F1") + " °C";
        currentHumidityText.text = sensorData.humidity.ToString("F1") + " %";
    }
}