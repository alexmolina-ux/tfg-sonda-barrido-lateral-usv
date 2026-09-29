using UnityEngine;
using System.Net.Sockets;
using System.Text;
using System.Globalization;

public class SimuladorESP32 : MonoBehaviour
{
    public string ip = "127.0.0.1"; // Dirección IP local
    public int puerto = 8080;

    // Estos atributos crean barras deslizantes en el Inspector
    [Range(-180f, 180f)] public float simulacionRoll = 0f;
    [Range(-90f, 90f)] public float simulacionPitch = 0f;
    [Range(-180f, 180f)] public float simulacionYaw = 0f;

    private UdpClient udpClient;

    void Start()
    {
        udpClient = new UdpClient();
    }

    void Update()
    {
        // Formateamos los números para que usen punto decimal
        string sRoll = simulacionRoll.ToString("F2", CultureInfo.InvariantCulture);
        string sPitch = simulacionPitch.ToString("F2", CultureInfo.InvariantCulture);
        string sYaw = simulacionYaw.ToString("F2", CultureInfo.InvariantCulture);

        // Creamos el mensaje idéntico al que enviará tu placa física
        string mensaje = $"{sRoll},{sPitch},{sYaw}";
        byte[] data = Encoding.UTF8.GetBytes(mensaje);

        // Enviamos el paquete
        udpClient.Send(data, data.Length, ip, puerto);
    }

    void OnApplicationQuit()
    {
        if (udpClient != null) udpClient.Close();
    }
}
