using UnityEngine;
using System.IO;
using System.Text;
using System;
using System.Globalization;

public class MissionLogger : MonoBehaviour
{
    [Header("Referencias")]
    public BlueBoatManager blueBoat;
    public TowfishManager towfish;
    public SideScanSonar sonar;

    [Header("Configuración Registro")]
    public float retardoInicial = 2.0f; // Segundos de gracia para estabilizar físicas
    public float logRate = 2f;

    private string filePath;
    private StreamWriter writer;
    private float nextLogTime = 0f;
    private bool isLogging = false;

    void Start()
    {
        if (blueBoat == null || towfish == null || sonar == null) return;

        string folder = Application.dataPath + "/Datos_Mision";
        if (!Directory.Exists(folder)) Directory.CreateDirectory(folder);

        string timestamp = DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss");
        filePath = folder + "/Registro_Mision_" + timestamp + ".csv";

        writer = new StreamWriter(filePath, false, Encoding.UTF8);

        string cabecera =
            "Tiempo_Mision(s),Barco_X,Barco_Y,Barco_Z,Barco_Velocidad(m/s),Barco_Rumbo(deg)," +
            "Towfish_X,Towfish_Y,Towfish_Z,Roll(deg),Pitch(deg),Yaw(deg)," +
            "Profundidad(m),Presion(bar),Temperatura(C),LongitudCable(m)," +
            "EcoBabor_Dist(m),EcoBabor_Int,EcoEstribor_Dist(m),EcoEstribor_Int";

        writer.WriteLine(cabecera);

        // Configuramos la alarma para empezar a grabar pasados los segundos de retardo
        nextLogTime = Time.time + retardoInicial;
        isLogging = true;
        Debug.Log("Logger inicializado. Comenzará a registrar datos en " + retardoInicial + " segundos.");
    }

    void Update()
    {
        if (isLogging && Time.time >= nextLogTime)
        {
            RegistrarDatos();
            nextLogTime = Time.time + (1f / logRate);
        }
    }

    void RegistrarDatos()
    {
        float distBabor = 0f, intBabor = 0f;
        float distEstribor = 0f, intEstribor = 0f;

        foreach (var eco in sonar.historialEcos)
        {
            if (eco.lado == "Babor") { distBabor = eco.distancia; intBabor = eco.intensidad; }
            if (eco.lado == "Estribor") { distEstribor = eco.distancia; intEstribor = eco.intensidad; }
        }

        string linea = string.Format(CultureInfo.InvariantCulture,
            "{0:F2},{1:F3},{2:F3},{3:F3},{4:F3},{5:F2}," +
            "{6:F3},{7:F3},{8:F3},{9:F2},{10:F2},{11:F2}," +
            "{12:F3},{13:F3},{14:F2},{15:F2}," +
            "{16:F3},{17:F3},{18:F3},{19:F3}",

            blueBoat.GetMissionTime(),
            blueBoat.GetPosition().x, blueBoat.GetPosition().y, blueBoat.GetPosition().z,
            blueBoat.GetSpeed(), blueBoat.GetHeading(),

            towfish.transform.position.x, towfish.transform.position.y, towfish.transform.position.z,
            towfish.telemetry.roll, towfish.telemetry.pitch, towfish.telemetry.yaw,

            towfish.telemetry.depth, towfish.telemetry.pressure, towfish.telemetry.temperature,
            towfish.telemetry.cableLength,

            distBabor, intBabor, distEstribor, intEstribor
            );

        writer.WriteLine(linea);
        writer.Flush();
    }

    void OnApplicationQuit()
    {
        if (writer != null)
        {
            writer.Close();
            Debug.Log("Misión finalizada. CSV cerrado correctamente.");
        }
    }

}
