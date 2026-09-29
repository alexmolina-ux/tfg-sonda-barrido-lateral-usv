using UnityEngine;

/// <summary>
/// Encapsula la generación de la temperatura simulada.
/// </summary>
public class TemperatureSensor : MonoBehaviour
{
    [Range(0f, 1f)] public float ruidoTemperatura = 0.1f;

    public float GetSimulatedTemperature(float temperaturaBase)
    {
        return temperaturaBase + UnityEngine.Random.Range(-ruidoTemperatura, ruidoTemperatura);

    }
}
