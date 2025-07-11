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

    private void OnEnable()
    {
        //TerrariumBleController.OnConnectionComplete += HandleOnConnectionComplete;
    }

    private void OnDisable()
    {
        //TerrariumBleController.OnConnectionComplete -= HandleOnConnectionComplete;
    }

    private void HandleOnConnectionComplete()
    {
        Debug.Log("Connection complete, retrieving schedules...");
        // Placeholder: call the BLE controller to read schedules when implemented
        // TerrariumBleController.Instance.ReadSchedules();
        StartCoroutine(WaitForSchedulesAndUpdate());
    }

    private IEnumerator WaitForSchedulesAndUpdate()
    {
        float timeout = 5f;
        float elapsed = 0f;

        // Placeholder: replace with actual schedule data retrieval when available
        bool hasScheduleData = false;

        while (!hasScheduleData)
        {
            // TODO: Replace this with actual check for schedule data
            // Example:
            // var schedule = TerrariumBleController.Instance.GetCurrentScheduleData();
            // if (!string.IsNullOrEmpty(schedule)) { hasScheduleData = true; }

            if (elapsed > timeout)
            {
                Debug.LogError("Timeout waiting for schedules from BLE device.");
                yield break;
            }
            yield return null;
            elapsed += Time.deltaTime;
        }

        // Placeholder: replace with actual schedule data display
        DisplaySchedules("N/A", "N/A", "N/A", "N/A");
    }

    public void DisplaySchedules(string waterDur, string waterFreq, string lightDur, string lightFreq)
    {
        waterDuration.text = waterDur;
        waterFrequency.text = waterFreq;
        lightDuration.text = lightDur;
        lightFrequency.text = lightFreq;
    }
}
