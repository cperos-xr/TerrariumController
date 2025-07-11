using System;
using System.Collections;
using TMPro;
using UnityEngine;

public class ClockUIManager : MonoBehaviour
{
    public TextMeshProUGUI currentRTCTimeText;
    private DateTime rtcDateTime;

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
        Debug.Log("Connection complete, retrieving RTC time...");
        TerrariumBleController.Instance.ReadRTC();
        StartCoroutine(WaitForRTCAndUpdate());
    }

    private IEnumerator WaitForRTCAndUpdate()
    {
        // Wait until _rtcString is set by ReadRTC
        float timeout = 5f; // seconds
        float elapsed = 0f;

        while (string.IsNullOrEmpty(TerrariumBleController.Instance.GetRawRTCString()))
        {
            if (elapsed > timeout)
            {
                Debug.LogError("Timeout waiting for RTC value from BLE device.");
                yield break;
            }
            yield return null;
            elapsed += Time.deltaTime;
        }

        rtcDateTime = TerrariumBleController.Instance.GetRTCDateTime();
        currentRTCTimeText.text = rtcDateTime.ToString("yyyy-MM-dd HH:mm:ss");
        StartCoroutine(UpdateTime());
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
