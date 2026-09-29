using UnityEngine;

[System.Serializable]
public class SonarHit
{
    public float distancia;
    public Vector3 puntoImpacto;
    public Vector3 normal;
    public float intensidad;
    public string lado;
    public float tiempo;

    public SonarHit(float dist, Vector3 punto, Vector3 norm, float intesidadEco, string ladoEco)
    {
        distancia = dist;
        puntoImpacto = punto;
        normal = norm;
        intensidad = intesidadEco;
        lado = ladoEco;
        tiempo = Time.time;
    }
}
