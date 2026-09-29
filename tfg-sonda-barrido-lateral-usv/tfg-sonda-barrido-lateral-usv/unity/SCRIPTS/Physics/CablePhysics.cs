using UnityEngine;

public static class CablePhysics
{
    /// <summary>
    /// Calcula la geometría del cable entre el BlueBoat y el Towfish.
    /// Actualmente utiliza una parábola como aproximación de la catenaria.
    /// En futuras versiones se sustituirá por una catenaria física.
    /// </summary>
    public static Vector3[] CalculateCable(
        Vector3 start,
        Vector3 end,
        int segments,
        float cableSag)

    {
        Vector3[] points = new Vector3[segments];

        for (int i = 0; i < segments; i++)
        {
            float t = (float)i / (segments - 1);

            // Punto sobre la línea entre barco y towfish
            Vector3 punto = Vector3.Lerp(start, end, t);

            // Curvatura parabólica
            float caida = 4f * cableSag * t * (1f - t);

            // Aplicar la caída vertical
            punto.y -= caida;

            points[i] = punto;
        }

        return points;
    }
}
