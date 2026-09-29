using UnityEngine;

/// <summary>
/// Encapsula la generación de la presión hidrostática simulada a partir de la
/// profundidad instantánea del Towfish.
/// </summary>
public class PressureSensor : MonoBehaviour
{
    [Header("Modelo de Presión Simulada")]
    public float factorPresionPorMetro = 0.1f;
    [Range(0f, 0.05f)] public float ruidoPresion = 0.005f;

    public float GetSimulatedPressure(float depth)
    {
        float ruido = UnityEngine.Random.Range(-ruidoPresion, ruidoPresion);
        return (depth * factorPresionPorMetro) + ruido;
    }
}
