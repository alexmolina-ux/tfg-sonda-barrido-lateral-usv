using UnityEngine;

[System.Serializable]
public class TowfishTelemetry
{
    [Header("IMU")]
    public float roll;
    public float pitch;
    public float yaw;

    [Header("Sensores")]
    public float pressure;
    public float depth;
    public float temperature;

    [Header("Remolque")]
    public float cableLength = 10f;
}

public class TowfishManager : MonoBehaviour
{
    [Header("Telemetry")]
    public TowfishTelemetry telemetry = new TowfishTelemetry();

    [Header("Simulation")]
    public bool simulationMode = true;
    [Range(0.1f, 100f)] public float simulatedCableLength = 0.4f;
    [Range(0f, 50f)] public float simulatedDepth = 0.3f;
    [Range(0f, 30f)] public float simulatedTemperature = 18f;

    [Header("Realismo (Ruido)")]
    [Range(0f, 0.1f)] public float ruidoProfundidad = 0.02f;

    [Header("Movement")]
    public float followSpeed = 2.0f;
    public float rotationSpeed = 3.0f;

    [Header("Sensores Modulares")]
    public IMUSensor imuSensor;
    public PressureSensor pressureSensor;
    public TemperatureSensor temperatureSensor;

    private Vector3 targetPosition;

    void Start()
    {
        targetPosition = transform.position;
    }

    void Update()
    {
        if (simulationMode)
        {
            telemetry.cableLength = simulatedCableLength;

            float fluctuacionProfundidad = UnityEngine.Random.Range(-ruidoProfundidad, ruidoProfundidad);
            telemetry.depth = simulatedDepth + fluctuacionProfundidad;

            // MODIFICADO: se delega en PressureSensor si está asignado; si no, se
            // conserva la fórmula original como fallback (no rompe si no lo has enlazado aún).
            telemetry.pressure = pressureSensor != null
                ? pressureSensor.GetSimulatedPressure(telemetry.depth)
                : (telemetry.depth * 0.1f) + UnityEngine.Random.Range(-0.005f, 0.005f);

            // MODIFICADO: se delega en TemperatureSensor si está asignado.

            telemetry.temperature = temperatureSensor != null
                ? temperatureSensor.GetSimulatedTemperature(simulatedTemperature)
                : simulatedTemperature + UnityEngine.Random.Range(-0.1f, 0.1f);

            // MODIFICADO: se delega en IMUSensor si está asignado.
            if (imuSensor != null)
            {
                imuSensor.GetSimulatedAttitude(out telemetry.roll, out telemetry.pitch, out telemetry.yaw);
            }
            else
            {
                telemetry.roll = Mathf.Sin(Time.time * 0.8f) * 15f;
                telemetry.pitch = Mathf.Sin(Time.time * 0.5f) * 10f;
                telemetry.yaw = Mathf.Sin(Time.time * 0.3f) * 20f;
            }
        }

        // El torpedo siempre persigue la posición física detrás del barco
        transform.position = Vector3.MoveTowards(transform.position, targetPosition, followSpeed *
            Time.deltaTime);

        // CORRECCIÓN HIL: Solo forzamos la rotación visual automática si estamos en modo simulación
        Vector3 direccion = targetPosition - transform.position;
        if (simulationMode && direccion.magnitude > 0.05f)
        {
            Quaternion objetivoRotacion = Quaternion.LookRotation(direccion);
            transform.rotation = Quaternion.Slerp(transform.rotation, objetivoRotacion, rotationSpeed *
                Time.deltaTime);
        }
    }

    public void SetTargetPosition(Vector3 position) { targetPosition = position; }
    public void SetIMU(float roll, float pitch, float yaw) { if (!simulationMode) { telemetry.roll = roll;
            telemetry.pitch = pitch; telemetry.yaw = yaw; } }
    public void SetPressure(float pressure) { if (!simulationMode) telemetry.pressure = pressure; }
    public void SetDepth(float depth) { if (!simulationMode) telemetry.depth = depth; }
    public void SetTemperature(float temperature) { if (!simulationMode) telemetry.temperature =
            temperature; }
    public void SetCableLength(float cable) { if (!simulationMode) telemetry.cableLength = cable; }
}
