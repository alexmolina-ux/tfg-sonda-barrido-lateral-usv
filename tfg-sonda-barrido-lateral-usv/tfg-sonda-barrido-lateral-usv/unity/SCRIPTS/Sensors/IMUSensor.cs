using UnityEngine;

/// <summary>
/// Encapsula la generación de la actitud angular simulada del IMU (MPU-6050 virtual).
/// Traslada aquí las mismas funciones senoidales que antes se calculaban de forma
/// centralizada en TowfishManager.cs.
/// </summary>
public class IMUSensor : MonoBehaviour
{
    [Header("Amplitud de Oscilación Simulada (grados)")]
    public float amplitudRoll = 15f;
    public float amplitudPitch = 10f;
    public float amplitudYaw = 20f;

    [Header("Frecuencia de Oscilación Simulada (rad/s)")]
    public float frecuenciaRoll = 0.8f;
    public float frecuenciaPitch = 0.5f;
    public float frecuenciaYaw = 0.3f;

    public void GetSimulatedAttitude(out float roll, out float pitch, out float yaw)
    {
        roll = Mathf.Sin(Time.time * frecuenciaRoll) * amplitudRoll;
        pitch = Mathf.Sin(Time.time * frecuenciaPitch) * amplitudPitch;
        yaw = Mathf.Sin(Time.time * frecuenciaYaw) * amplitudYaw;
    }
}
