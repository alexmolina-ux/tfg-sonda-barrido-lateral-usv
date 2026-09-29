using UnityEngine;

[System.Serializable]
public class TelemetryData
{
    public float roll;
    public float pitch;
    public float yaw;

    public float pressure;

    public float depth;

    public float temperature;

    public float cableLength;

    public bool sonarLeft;

    public bool sonarRight;
}
