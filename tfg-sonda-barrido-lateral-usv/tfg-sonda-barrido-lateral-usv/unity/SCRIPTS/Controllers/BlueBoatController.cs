using UnityEngine;

[RequireComponent(typeof(Rigidbody))]
public class BlueBoatController : MonoBehaviour
{
    [Header("Movimiento")]
    public float velocidad = 2.0f;   // m/s
    public float velocidadGiro = 40f; // grados/s

    private Rigidbody rb;

    private float avance;
    private float giro;

    void Awake()
    {
        rb = GetComponent<Rigidbody>();
    }

    void Update()
    {
        // Leer teclado
        avance = Input.GetAxis("Vertical");
        giro = Input.GetAxis("Horizontal");
    }

    void FixedUpdate()
    {
        // Movimiento hacia delante y atrás
        Vector3 movimiento = transform.forward * avance * velocidad * Time.fixedDeltaTime;
        rb.MovePosition(rb.position + movimiento);

        // Giro
        Quaternion rotacion = Quaternion.Euler(
            0,
            giro * velocidadGiro * Time.fixedDeltaTime,
            0);

        rb.MoveRotation(rb.rotation * rotacion);
    }
}
