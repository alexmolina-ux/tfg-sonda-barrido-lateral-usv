using UnityEngine;
using TMPro;

public class PanelTelemetria : MonoBehaviour
{
    [Header("Referencias")]
    public BlueBoatManager blueBoat; // Añadimos la conexión al barco
    public TowfishManager towfish;
    public TextMeshProUGUI textoPantalla;

    void Update()
    {
        if (blueBoat != null && towfish != null && textoPantalla != null)
        {
            // Extracción de datos del Blueboat
            float velocidad = blueBoat.GetSpeed();
            float rumbo = blueBoat.GetHeading();
            float distancia = blueBoat.GetDistanceTravelled();
            float tiempo = blueBoat.GetMissionTime();

            // Extracción de datos del Towfish
            float roll = towfish.telemetry.roll;
            float pitch = towfish.telemetry.pitch;
            float yaw = towfish.telemetry.yaw;

            float profundidad = towfish.telemetry.depth;
            float presion = towfish.telemetry.pressure;
            float temperatura = towfish.telemetry.temperature;
            float longitudCable = towfish.telemetry.cableLength;

            // Construcción del panel unificado
            textoPantalla.text = "===== BLUEBOAT =====\n\n" +
                $"Velocidad : {velocidad:F2} m/s\n" +
                $"Rumbo : {rumbo:F1}°\n" +
                $"Distancia : {distancia:F1} m\n" +
                $"Tiempo : {tiempo:F1} s\n\n" +
                "===== TOWFISH =====\n\n" +
                "IMU\n" +
                $"Roll : {roll:F1}°\n" +
                $"Pitch : {pitch:F1}°\n" +
                $"Yaw : {yaw:F1}°\n\n" +
                "SENSORES\n" +
                $"Profundidad: {profundidad:F2} m\n" +
                $"Presión : {presion:F2} bar\n" +
                $"Temperatura: {temperatura:F1} °C\n\n" +

                "CABLE\n" +
                $"Longitud : {longitudCable:F2} m";
        }
    }
}
