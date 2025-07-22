using UnityEngine;
using TMPro;
using System;
using System.Text;
using System.Collections;

public class TerrariumBleController : MonoBehaviour
{
    public static TerrariumBleController Instance { get; private set; }
    void Awake()
    {
        if (Instance != null && Instance != this)
        {
            Destroy(this.gameObject);
            return;
        }
        Instance = this;
        DontDestroyOnLoad(this.gameObject);
    }

    [Header("BLE Device Info")]
    public string DeviceName = "TerrariumController";
    public string ServiceUUID = "12345678-1234-5678-1234-56789abcdef0";

    [Header("Characteristic UUIDs")]
    public string ReadRTCUUID = "12345678-1234-5678-1234-56789abcdef4";
    public string WriteRTCUUID = "12345678-1234-5678-1234-56789abcdefa";
    public string ReadSensorUUID = "12345678-1234-5678-1234-56789abcdef6";
    public string ReadRecordsUUID = "12345678-1234-5678-1234-56789abcdef7";
    public string WriteScheduleUUID = "12345678-1234-5678-1234-56789abcdef3";
    public string ReadSchedulesUUID = "12345678-1234-5678-1234-56789abcdef5";
    public string ClearSchedulesUUID = "12345678-1234-5678-1234-56789abcdef8";
    public string ClearRecordsUUID = "12345678-1234-5678-1234-56789abcdef9";
    public string FoggerButtonUUID = "12345678-1234-5678-1234-56789abcdefb";

    [Header("UI Elements (assign in Inspector)")]
    public TextMeshProUGUI statusText;
    public TextMeshProUGUI outputText;

    private string _deviceAddress;
    private bool _serviceDiscovered;
    private bool _mtuDone;

    private string _rtcString = "";
    public string _sensorJson = "";
    private string _recordsJson = "";
    public string _schedulesString = "";

    public delegate void Connection();
    public static event Connection OnConnectionComplete;

    public bool IsConnected
{
    get { return _mtuDone; }
}

    void Start()
    {
        InitBLE();
    }

    public void InitBLE()
    {
        statusText.text = "Initializing BLE...";
        BluetoothLEHardwareInterface.Initialize(
            true, false,
            InitComplete,
            InitError
        );
    }

    void InitComplete()
    {
        statusText.text = "Scanning for " + DeviceName;
        BluetoothLEHardwareInterface.ScanForPeripheralsWithServices(
            null,
            PeripheralFound,
            null,
            false
        );
    }

    void InitError(string error)
    {
        // IMPORTANT: Don't change the status text if we're already connected and ready
        if (_mtuDone)
        {
            // If we're already connected, ignore this error
            Debug.Log("Ignoring BLE error after successful connection: " + error);
            return;
        }

        // For the common BLE initialization error, use a more user-friendly message
        if (error.Contains("Failed to read characteristic"))
        {
            statusText.text = "Establishing connection...";
        }
        else
        {
            // For other errors, show the full error message
            statusText.text = "Init error: " + error;
        }
    }

    private IEnumerator UpdateStatusAfterDelay()
    {
        // Wait a few seconds for the connection to stabilize
        yield return new WaitForSeconds(4.0f);
        
        // If connection succeeded after the error (which your logs show it does)
        if (_mtuDone)
        {
            statusText.text = "Ready";
        }
    }

    void PeripheralFound(string address, string name)
    {
        if (name.Contains(DeviceName))
        {
            _deviceAddress = address;
            BluetoothLEHardwareInterface.StopScan();
            Connect();
        }
    }

    void Connect()
    {
        statusText.text = "Connecting...";
        BluetoothLEHardwareInterface.ConnectToPeripheral(
            _deviceAddress,
            null,
            null,
            ConnectComplete,
            Disconnect
        );
    }

    void ConnectComplete(string address, string serviceUUID, string characteristicUUID)
    {
        if (serviceUUID == ServiceUUID && !_serviceDiscovered)
        {
            _serviceDiscovered = true;
            statusText.text = "Service found, requesting MTU...";
            Debug.Log("Connected to service: " + serviceUUID);
            BluetoothLEHardwareInterface.RequestMtu(
                _deviceAddress, 185,
                MtuComplete
            );
        }
    }

    void MtuComplete(string address, int mtu)
    {
        Debug.Log("MTU set to: " + mtu);
        _mtuDone = true;
        
        // Always update status to Ready when MTU is complete
        statusText.text = "Ready";

        // Add a delay before triggering connection complete
        StartCoroutine(DelayedConnectionComplete());
    }

    private IEnumerator DelayedConnectionComplete()
    {
        // Reduce delay from 3.0 to 1.0 seconds
        yield return new WaitForSeconds(1.0f);

        // Now that BLE stack and MTU are stable, update UI:
        statusText.text = "Ready";
        Debug.Log("BLE connection stabilized, notifying components");
        OnConnectionComplete?.Invoke();
    }

    void Disconnect(string address)
    {
        _serviceDiscovered = false;
        _mtuDone = false;
        statusText.text = "Disconnected, retrying in 2s...";
        Invoke("Start", 2f);
    }

    public void ReadRTC()
    {
        if (!_mtuDone) { statusText.text = "Not ready"; return; }

        // Silent error handling with retry
        try
        {
            BluetoothLEHardwareInterface.ReadCharacteristic(
                _deviceAddress, ServiceUUID, ReadRTCUUID,
                (chr, bytes) =>
                {
                    _rtcString = Encoding.UTF8.GetString(bytes);
                    outputText.text = "RTC -> " + _rtcString;
                }
            );
        }
        catch (Exception ex)
        {
            // Log but don't treat as critical error
            Debug.Log("ReadRTC non-critical error: " + ex.Message);

            // Try once more after a brief delay
            StartCoroutine(RetryReadAfterDelay("RTC"));
        }
    }

    public void ReadSensor()
    {
        if (!_mtuDone) { statusText.text = "Not ready"; return; }

        // Silent error handling with retry
        try
        {
            BluetoothLEHardwareInterface.ReadCharacteristic(
                _deviceAddress, ServiceUUID, ReadSensorUUID,
                (chr, bytes) =>
                {
                    _sensorJson = Encoding.UTF8.GetString(bytes);
                    outputText.text = "Sensor -> " + _sensorJson;
                }
            );
        }
        catch (Exception ex)
        {
            // Log but don't treat as critical error
            Debug.Log("ReadSensor non-critical error: " + ex.Message);

            // Try once more after a brief delay
            StartCoroutine(RetryReadAfterDelay("Sensor"));
        }
    }

    public void ReadRecords()
    {
        if (!_mtuDone) { statusText.text = "Not ready"; return; }
        BluetoothLEHardwareInterface.ReadCharacteristic(
            _deviceAddress, ServiceUUID, ReadRecordsUUID,
            (chr, bytes) =>
            {
                _recordsJson = Encoding.UTF8.GetString(bytes);
                outputText.text = "Records -> " + _recordsJson;
            }
        );
    }

    public void ReadSchedules()
    {
        if (!_mtuDone) { statusText.text = "Not ready"; return; }
        BluetoothLEHardwareInterface.ReadCharacteristic(
            _deviceAddress, ServiceUUID, ReadSchedulesUUID,
            (chr, bytes) =>
            {
                _schedulesString = Encoding.UTF8.GetString(bytes);
                outputText.text = "Schedules -> " + _schedulesString;
            }
        );
    }

    public void WriteRTCNow() => WriteRTC(DateTime.Now);

    public void WriteRTC(DateTime dt)
    {
        if (!_mtuDone) { statusText.text = "Not ready"; return; }
        var s = dt.ToString("yyyy-MM-dd HH:mm:ss");
        var data = Encoding.UTF8.GetBytes(s);
        BluetoothLEHardwareInterface.WriteCharacteristic(
            _deviceAddress, ServiceUUID, WriteRTCUUID,
            data, data.Length, true,
            chr => statusText.text = "RTC set"
        );
    }

    /// <summary>
    /// Writes a schedule command to the ESP32
    /// </summary>
    /// <param name="command">Command string in format: TARGET,TYPE,h1,m1,d1[,h2,m2]</param>
    public void WriteSchedule(string command)
    {
        if (!_mtuDone) {
            statusText.text = "Not ready";
            Debug.LogWarning("[BLE] MTU not complete – cannot send schedule");
            return;
        }

        // Log *before* sending so you can see the exact command
        Debug.Log($"[BLE] → Writing schedule to {WriteScheduleUUID}: \"{command}\"");

        byte[] data = Encoding.UTF8.GetBytes(command);
        BluetoothLEHardwareInterface.WriteCharacteristic(
            _deviceAddress,
            ServiceUUID,
            WriteScheduleUUID,
            data,
            data.Length,
            false,  // <-- Write WITHOUT response to match ESP32 PROPERTY_WRITE
            chr => {
                statusText.text = "Schedule sent";
                Debug.Log($"[BLE] ✓ Write succeeded for \"{command}\"");
            }
        );
    }


    public void ClearSchedules()
    {
        if (!_mtuDone) { statusText.text = "Not ready"; return; }
        var data = Encoding.UTF8.GetBytes("CLEAR");
        BluetoothLEHardwareInterface.WriteCharacteristic(
            _deviceAddress, ServiceUUID, ClearSchedulesUUID,
            data, data.Length, true,
            chr => statusText.text = "Schedules cleared"
        );
    }

    public void ClearRecords()
    {
        if (!_mtuDone) { statusText.text = "Not ready"; return; }
        var data = Encoding.UTF8.GetBytes("CLEAR");
        BluetoothLEHardwareInterface.WriteCharacteristic(
            _deviceAddress, ServiceUUID, ClearRecordsUUID,
            data, data.Length, true,
            chr => statusText.text = "Records cleared"
        );
    }

    public string GetRawRTCString()
    {
        return _rtcString;
    }

    public DateTime GetRTCDateTime()
    {
        if (DateTime.TryParse(_rtcString, out var dt))
            return dt;
        throw new InvalidOperationException("RTC not yet read or invalid: " + _rtcString);
    }

    public SensorData GetSensorData()
    {
        if (string.IsNullOrEmpty(_sensorJson))
            throw new InvalidOperationException("Sensor data not yet read");
        return JsonUtility.FromJson<SensorData>(_sensorJson);
    }

    public RecordsResponse GetRecords()
    {
        if (string.IsNullOrEmpty(_recordsJson))
            throw new InvalidOperationException("Records not yet read");
        return JsonUtility.FromJson<RecordsResponse>(_recordsJson);
    }

    public string GetCurrentScheduleData()
    {
        if (string.IsNullOrEmpty(_schedulesString))
            throw new InvalidOperationException("Schedules not yet read");
        return _schedulesString;
    }

    public SchedulesResponse GetSchedules()
    {
        if (string.IsNullOrEmpty(_schedulesString))
            return null;
        
        try
        {
            return JsonUtility.FromJson<SchedulesResponse>(_schedulesString);
        }
        catch (Exception ex)
        {
            Debug.LogError("Error parsing schedules: " + ex.Message);
            return null;
        }
    }

    private IEnumerator RetryReadAfterDelay(string readType)
    {
        yield return new WaitForSeconds(0.5f);

        if (readType == "RTC")
        {
            Debug.Log("Automatically retrying RTC read after error");
            try
            {
                BluetoothLEHardwareInterface.ReadCharacteristic(
                    _deviceAddress, ServiceUUID, ReadRTCUUID,
                    (chr, bytes) =>
                    {
                        _rtcString = Encoding.UTF8.GetString(bytes);
                        outputText.text = "RTC -> " + _rtcString;
                    }
                );
            }
            catch { /* Ignore any errors on retry */ }
        }
        else if (readType == "Sensor")
        {
            Debug.Log("Automatically retrying Sensor read after error");
            try
            {
                BluetoothLEHardwareInterface.ReadCharacteristic(
                    _deviceAddress, ServiceUUID, ReadSensorUUID,
                    (chr, bytes) =>
                    {
                        _sensorJson = Encoding.UTF8.GetString(bytes);
                        outputText.text = "Sensor -> " + _sensorJson;
                    }
                );
            }
            catch { /* Ignore any errors on retry */ }
        }
    }

    /// <summary>
    /// Simulates pressing the fogger button
    /// </summary>
    public void PressFoggerButton()
    {
        if (!IsConnected)
        {
            Debug.LogError("Cannot press fogger button: BLE not connected");
            return;
        }
        
        try
        {
            byte[] bytes = Encoding.UTF8.GetBytes("PRESS");
            BluetoothLEHardwareInterface.WriteCharacteristic(
                _deviceAddress, ServiceUUID, FoggerButtonUUID,
                bytes, bytes.Length,
                false, (characteristic) => {
                    Debug.Log("Fogger button press command sent successfully");
                }
            );
            Debug.Log("Fogger button press command sent");
        }
        catch (Exception ex)
        {
            Debug.LogError($"Error pressing fogger button: {ex.Message}");
        }
    }
}
