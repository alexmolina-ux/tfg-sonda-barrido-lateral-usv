using UnityEngine;

public class BlueBoatManager : MonoBehaviour
{
    [Header("Navigation")]
    public NavigationData navigation = new NavigationData();

    // Variables internas
    private Vector3 previousPosition;

    void Start()
    {
        previousPosition = transform.position;

        navigation.position = transform.position;
        navigation.speed = 0f;
        navigation.heading = transform.eulerAngles.y;
        navigation.distanceTravelled = 0f;
        navigation.missionTime = 0f;
    }

    void Update()
    {
        // Tiempo de misión
        navigation.missionTime += Time.deltaTime;

        // Posición actual
        navigation.position = transform.position;

        // Distancia recorrida desde el último frame
        float distance = Vector3.Distance(transform.position, previousPosition);

        // Acumular distancia total
        navigation.distanceTravelled += distance;

        // Velocidad instantánea (m/s)
        if (Time.deltaTime > 0f)
            navigation.speed = distance / Time.deltaTime;

        // Rumbo (Heading)
        navigation.heading = transform.eulerAngles.y;

        // Guardar posición para el siguiente frame
        previousPosition = transform.position;
    }

    //==============================
    // MÉTODOS DE CONSULTA
    //==============================

    public Vector3 GetPosition()
    {
        return navigation.position;
    }

    public float GetSpeed()
    {
        return navigation.speed;
    }

    public float GetHeading()
    {
        return navigation.heading;
    }

    public float GetDistanceTravelled()
    {
        return navigation.distanceTravelled;
    }

    public float GetMissionTime()
    {
        return navigation.missionTime;
    }
}
