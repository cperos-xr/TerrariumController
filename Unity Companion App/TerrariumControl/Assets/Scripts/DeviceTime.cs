using System;
using TMPro;
using UnityEngine;

public class DeviceTime : MonoBehaviour
{
    public DateTime currentTime;
    public TextMeshProUGUI timeText;

    private void Start()
    {
        // Initialize the current time to the system's current time
        currentTime = DateTime.Now;
        UpdateTimeText();
    }

    private void Update()
    {
        // Update the current time every frame
        currentTime = DateTime.Now;
        UpdateTimeText();
    }

    private void UpdateTimeText()
    {
        // Format the current time and update the UI text
        timeText.text = currentTime.ToString("yyyy-MM-dd HH:mm:ss");
    }

    public void SyncRTC()
    {
        TerrariumBleController.Instance.WriteRTC(currentTime);
    }


}
