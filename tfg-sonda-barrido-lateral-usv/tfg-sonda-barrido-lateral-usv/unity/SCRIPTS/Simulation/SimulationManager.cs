using UnityEngine;

/// <summary>
/// Orquesta el cambio entre "Modo Simulación" y "Modo Real" del Gemelo Digital
/// en tiempo de ejecución. Antes ni siquiera heredaba de MonoBehaviour.
/// </summary>
public class SimulationManager : MonoBehaviour
{
    [Header("Referencias")]
    public TowfishManager towfish;

    [Header("Control de Modo")]
    [Tooltip("Tecla para alternar entre Modo Simulación y Modo Real durante la ejecución.")]
    public KeyCode teclaAlternarModo = KeyCode.M;

    void Update()
    {
        if (towfish == null) return;

        if (Input.GetKeyDown(teclaAlternarModo))
        {
            towfish.simulationMode = !towfish.simulationMode;
            Debug.Log("SimulationManager: modo cambiado a " + (towfish.simulationMode ? "Simulación" :
                "Real"));
        }
    }

    public bool EstaEnModoSimulacion()
    {
        return towfish != null && towfish.simulationMode;
    }
}
