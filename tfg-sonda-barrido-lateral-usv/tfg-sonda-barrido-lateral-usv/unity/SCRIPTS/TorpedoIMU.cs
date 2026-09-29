using UnityEngine;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System;

public class TorpedoIMU : MonoBehaviour
{
    public int puertoUDP = 8080;
    public TowfishManager towfishManager;
    private UdpClient udpClient;
    private Thread receiveThread;
    private bool isRunning = true;

    private float roll = 0f;
    private float pitch = 0f;
    private float yaw = 0f;

    void Start()
    {
        if (towfishManager == null)
            towfishManager = GetComponent<TowfishManager>();

        receiveThread = new Thread(new ThreadStart(ReceiveData));
        receiveThread.IsBackground = true;

        receiveThread.Start();
    }

    void ReceiveData()
    {
        udpClient = new UdpClient(puertoUDP);
        IPEndPoint anyIP = new IPEndPoint(IPAddress.Any, 0);

        while (isRunning)
        {
            try
            {
                byte[] data = udpClient.Receive(ref anyIP);
                string text = Encoding.UTF8.GetString(data);
                string[] parts = text.Split(',');

                if (parts.Length >= 3)
                {
                    float.TryParse(parts[0], System.Globalization.NumberStyles.Float,
                        System.Globalization.CultureInfo.InvariantCulture, out roll);
                    float.TryParse(parts[1], System.Globalization.NumberStyles.Float,
                        System.Globalization.CultureInfo.InvariantCulture, out pitch);
                    float.TryParse(parts[2], System.Globalization.NumberStyles.Float,
                        System.Globalization.CultureInfo.InvariantCulture, out yaw);

                    if (towfishManager != null)
                        towfishManager.SetIMU(roll, pitch, yaw);

                    if (parts.Length >= 5)
                    {
                        float profundidad, temperatura;
                        float.TryParse(parts[3], System.Globalization.NumberStyles.Float,
                            System.Globalization.CultureInfo.InvariantCulture, out profundidad);
                        float.TryParse(parts[4], System.Globalization.NumberStyles.Float,
                            System.Globalization.CultureInfo.InvariantCulture, out temperatura);

                        if (towfishManager != null)
                        {
                            towfishManager.SetDepth(profundidad);
                            towfishManager.SetTemperature(temperatura);
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Debug.Log("Error UDP: " + e.Message);
            }
        }
    }

    void Update()
    {
        Quaternion targetRotation = Quaternion.Euler(pitch, yaw, roll);
        transform.rotation = Quaternion.Slerp(transform.rotation, targetRotation, Time.deltaTime * 10f);
    }

    void OnApplicationQuit()
    {
        isRunning = false;
        if (udpClient != null) udpClient.Close();
        if (receiveThread != null) receiveThread.Abort();
    }
}
