using UnityEngine;
using UnityEngine.UI;

public class SonarWaterfall : MonoBehaviour
{
    [Header("Referencias")]
    public RawImage pantallaUI;

    [Header("Configuración Mosaico")]
    public int ancho = 256;
    public int alto = 512;

    private Texture2D textura;
    private Color colorFondo = new Color(0f, 0f, 0.15f);
    private Color colorEco = new Color(1f, 0.7f, 0f);

    void Start()
    {
        textura = new Texture2D(ancho, alto, TextureFormat.RGBA32, false);
        pantallaUI.texture = textura;

        for (int y = 0; y < alto; y++)
        {
            for (int x = 0; x < ancho; x++)
            {
                textura.SetPixel(x, y, colorFondo);
            }
        }
        textura.Apply();
    }

    // NUEVO: Ahora recibe matrices completas de ecos (alta resolución)
    public void AgregarLineaCascadaArray(float[] ecosBabor, float[] ecosEstribor)
    {
        Color[] pixelesAnteriores = textura.GetPixels(0, 1, ancho, alto - 1);
        textura.SetPixels(0, 0, ancho, alto - 1, pixelesAnteriores);

        Color[] nuevaLinea = new Color[ancho];
        for (int i = 0; i < ancho; i++) nuevaLinea[i] = colorFondo;

        int centro = ancho / 2;

        // Pintar el abanico de Babor (espejado hacia la izquierda)
        for (int i = 0; i < ecosBabor.Length; i++)
        {
            if (ecosBabor[i] > 0.01f && i < centro)
            {
                int pixel = centro - 1 - i; // Dibuja del centro hacia el borde izquierdo
                nuevaLinea[pixel] = Color.Lerp(colorFondo, colorEco, ecosBabor[i]);
            }
        }

        // Pintar el abanico de Estribor (hacia la derecha)
        for (int i = 0; i < ecosEstribor.Length; i++)
        {
            if (ecosEstribor[i] > 0.01f && i < centro)
            {
                int pixel = centro + i; // Dibuja del centro hacia el borde derecho
                nuevaLinea[pixel] = Color.Lerp(colorFondo, colorEco, ecosEstribor[i]);
            }
        }

        textura.SetPixels(0, alto - 1, ancho, 1, nuevaLinea);

        textura.Apply();
    }
}
