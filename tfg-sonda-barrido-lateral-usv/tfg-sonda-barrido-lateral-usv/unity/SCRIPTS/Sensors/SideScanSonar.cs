using UnityEngine;
using System.Collections.Generic;

public class SideScanSonar : MonoBehaviour
{
    [Header("Referencias")]
    public Transform hazBabor;
    public Transform hazEstribor;
    public SonarWaterfall cascadaUI;
    public MosaicManager mapaMosaico;

    [Header("Configuración Acústica")]
    public float rango = 50f;
    public float anguloBase = 30f;
    public float aperturaAbanico = 60f;
    public int rayosPorLado = 60;
    public float pingRate = 5f;

    [Header("Realismo (Ruido)")]
    [Range(0f, 0.5f)] public float nivelRuido = 0.05f; // +/- 5 cm de dispersión de la señal

    [Header("Datos Acústicos")]
    public List<SonarHit> historialEcos = new List<SonarHit>();
    private int maxEcosMemoria = 10;
    private float tiempoUltimoPulso = 0f;

    // OPTIMIZACIÓN DE MEMORIA: Variables globales instanciadas una sola vez
    private int resolucionPantalla = 128;
    private float[] ecosBabor;
    private float[] ecosEstribor;
    private List<Vector3> puntosImpacto;
    private List<float> intensidadesImpacto;

    void Start()
    {
        // Pre-asignamos la memoria al arrancar la escena para evitar tirones
        ecosBabor = new float[resolucionPantalla];
        ecosEstribor = new float[resolucionPantalla];
        puntosImpacto = new List<Vector3>(rayosPorLado * 2);
        intensidadesImpacto = new List<float>(rayosPorLado * 2);
    }

    void Update()
    {
        if (Time.time - tiempoUltimoPulso >= 1f / pingRate)
        {
            EmitirAbanico();
            tiempoUltimoPulso = Time.time;
        }
    }

    void EmitirAbanico()
    {
        if (hazBabor == null || hazEstribor == null) return;

        // Vaciamos los contenedores reutilizables en lugar de crear nuevos
        System.Array.Clear(ecosBabor, 0, ecosBabor.Length);
        System.Array.Clear(ecosEstribor, 0, ecosEstribor.Length);
        puntosImpacto.Clear();
        intensidadesImpacto.Clear();

        float anguloInicial = anguloBase - (aperturaAbanico / 2f);
        float pasoAngulo = aperturaAbanico / (rayosPorLado - 1);

        for (int i = 0; i < rayosPorLado; i++)
        {
            float anguloActual = anguloInicial + (i * pasoAngulo);
            bool esRayoCentral = (i == rayosPorLado / 2);

            Vector3 dirBabor = Quaternion.AngleAxis(anguloActual, transform.forward) * (-transform.right);
            ProcesarRayo(hazBabor.position, dirBabor, "Babor", ecosBabor, esRayoCentral);

            Vector3 dirEstribor = Quaternion.AngleAxis(-anguloActual, transform.forward) * transform.right;
            ProcesarRayo(hazEstribor.position, dirEstribor, "Estribor", ecosEstribor, esRayoCentral);
        }

        if (cascadaUI != null) cascadaUI.AgregarLineaCascadaArray(ecosBabor, ecosEstribor);

        if (mapaMosaico != null) mapaMosaico.PintarPuntos(puntosImpacto.ToArray(),
            intensidadesImpacto.ToArray());
    }

    void ProcesarRayo(Vector3 origen, Vector3 direccion, string lado, float[] arrayEcos, bool esCentral)
    {
        RaycastHit hit;
        if (Physics.Raycast(origen, direccion, out hit, rango))
        {
            // RUIDO SENSORIAL: Alteramos sutilmente la distancia medida por el impacto
            float fluctuacion = UnityEngine.Random.Range(-nivelRuido, nivelRuido);
            float distanciaConRuido = Mathf.Max(0.01f, hit.distance + fluctuacion);

            float atenuacion = 1f - (distanciaConRuido / rango);
            float incidencia = Vector3.Dot(-direccion.normalized, hit.normal);
            float intensidad = Mathf.Clamp01(atenuacion * incidencia);

            if (esCentral) RegistrarEco(distanciaConRuido, hit.point, hit.normal, intensidad, lado);

            int pixelIndex = Mathf.RoundToInt((distanciaConRuido / rango) * (resolucionPantalla - 1));
            pixelIndex = Mathf.Clamp(pixelIndex, 0, resolucionPantalla - 1);

            if (intensidad > arrayEcos[pixelIndex]) arrayEcos[pixelIndex] = intensidad;

            puntosImpacto.Add(hit.point);
            intensidadesImpacto.Add(intensidad);
        }
    }

    void RegistrarEco(float distancia, Vector3 punto, Vector3 normal, float intensidad, string lado)
    {
        SonarHit nuevoEco = new SonarHit(distancia, punto, normal, intensidad, lado);
        historialEcos.Add(nuevoEco);
        if (historialEcos.Count > maxEcosMemoria) historialEcos.RemoveAt(0);
    }
}
