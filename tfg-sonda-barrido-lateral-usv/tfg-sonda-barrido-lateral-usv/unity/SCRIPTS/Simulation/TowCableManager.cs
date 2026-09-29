using UnityEngine;

public class TowCableManager : MonoBehaviour
{
    [Header("Referencias")]
    public BlueBoatManager blueBoat;
    public TowfishManager towfish;
    public Transform towPointBoat;
    public Transform towPointTowfish;

    private LineRenderer lineRenderer;

    [Header("Configuración del Cable")]
    public int cableSegments = 25;
    public float cableSag = 2.5f;

    private void Awake()
    {
        lineRenderer = GetComponent<LineRenderer>();
    }

    private void Update()
    {
        CalcularPosicionTowfish();
        ActualizarCable();
    }

    private void CalcularPosicionTowfish()
    {
        if (blueBoat == null || towfish == null)
        {
            Debug.LogWarning("Faltan referencias en TowCableManager. Asigna el BlueBoat y el Towfish en el Inspector.");
            return;
        }

        float cable = towfish.telemetry.cableLength;
        float profundidad = towfish.telemetry.depth;

        profundidad = Mathf.Min(profundidad, cable);

        float distanciaHorizontal = Mathf.Sqrt(cable * cable - profundidad * profundidad);

        Vector3 direccion = -blueBoat.transform.forward;
        Vector3 nuevaPosicion = blueBoat.transform.position + direccion * distanciaHorizontal;
        nuevaPosicion.y = -profundidad;

        towfish.SetTargetPosition(nuevaPosicion);
    }

    private void ActualizarCable()
    {
        if (lineRenderer == null || towPointBoat == null || towPointTowfish == null) return;

        lineRenderer.positionCount = cableSegments;

        Vector3 inicio = towPointBoat.position;
        Vector3 final = towPointTowfish.position;

        // MODIFICADO: se sustituye el cálculo local con Mathf.Sin por una llamada
        // directa a CablePhysics.CalculateCable(), que hasta ahora era código muerto.
        Vector3[] puntosCable = CablePhysics.CalculateCable(inicio, final, cableSegments, cableSag);

        for (int i = 0; i < cableSegments; i++)
        {
            lineRenderer.SetPosition(i, puntosCable[i]);
        }
    }
}
