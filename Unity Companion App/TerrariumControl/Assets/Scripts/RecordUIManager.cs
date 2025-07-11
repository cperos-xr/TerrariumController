using System;
using System.Collections;
using TMPro;
using UnityEngine;

public class RecordsUIManager : MonoBehaviour
{
    // Daily High Low Text
    public TextMeshProUGUI dayHighTemp;
    public TextMeshProUGUI dayLowTemp;
    public TextMeshProUGUI dayHighHumidity;
    public TextMeshProUGUI dayLowHumidity;
    // Weekly High Low Text
    public TextMeshProUGUI weekHighTemp;
    public TextMeshProUGUI weekLowTemp;
    public TextMeshProUGUI weekHighHumidity;
    public TextMeshProUGUI weekLowHumidity;


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
        Debug.Log("Connection complete, retrieving records...");
        TerrariumBleController.Instance.ReadRecords();
        StartCoroutine(WaitForRecordsAndUpdate());

    }

    private IEnumerator WaitForRecordsAndUpdate()
    {
        float timeout = 5f;
        float elapsed = 0f;

        while (true)
        {
            RecordsResponse records = null;
            try
            {
                records = TerrariumBleController.Instance.GetRecords();
            }
            catch
            {
                // Not yet read, keep waiting
            }

            if (records != null)
            {
                DisplayRecords(records);
                break;
            }

            if (elapsed > timeout)
            {
                Debug.LogError("Timeout waiting for records from BLE device.");
                yield break;
            }
            yield return null;
            elapsed += Time.deltaTime;
        }
    }

    public void DisplayRecords(RecordsResponse records)
    {
        if (records.daily != null)
        {
            dayHighTemp.text = records.daily.highTemp.ToString("F1") + " °C";
            dayLowTemp.text = records.daily.lowTemp.ToString("F1") + " °C";
            dayHighHumidity.text = records.daily.highHumid.ToString("F1") + " %";
            dayLowHumidity.text = records.daily.lowHumid.ToString("F1") + " %";
        }
        if (records.weekly != null)
        {
            weekHighTemp.text = records.weekly.highTemp.ToString("F1") + " °C";
            weekLowTemp.text = records.weekly.lowTemp.ToString("F1") + " °C";
            weekHighHumidity.text = records.weekly.highHumid.ToString("F1") + " %";
            weekLowHumidity.text = records.weekly.lowHumid.ToString("F1") + " %";
        }
    }

    // You can implement a similar coroutine and display method for schedules if needed
}
