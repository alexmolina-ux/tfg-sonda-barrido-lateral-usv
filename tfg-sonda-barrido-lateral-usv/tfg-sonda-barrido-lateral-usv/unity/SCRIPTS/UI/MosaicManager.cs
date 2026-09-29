using UnityEngine;
using System.IO;
using System;

public class MosaicManager : MonoBehaviour
{
    [Header("Referencias")]
    public Renderer mosaicoRenderer;

    [Header("Configuración del Mapa")]
    public float dimensionMundo = 100f; // Ajustado a 100 para hacer "Zoom" a tus rutas
    public int resolucionTextura = 2048;
    [Range(1, 10)] public int grosorPincel = 3; // NUEVO: Pincel grueso para trazos sólidos

    private Texture2D texturaMosaico;
    private Color colorFondo = new Color(0.02f, 0.05f, 0.15f, 1f);

    void Start()
    {
        if (mosaicoRenderer == null) return;

        texturaMosaico = new Texture2D(resolucionTextura, resolucionTextura, TextureFormat.RGBA32,
            false);

        Color[] pixelesFondo = new Color[resolucionTextura * resolucionTextura];
        for (int i = 0; i < pixelesFondo.Length; i++)
        {
            pixelesFondo[i] = colorFondo;
        }
        texturaMosaico.SetPixels(pixelesFondo);
        texturaMosaico.Apply();

        mosaicoRenderer.material.mainTexture = texturaMosaico;
    }

    public void PintarPuntos(Vector3[] posiciones, float[] intensidades)
    {
        bool modificado = false;

        for (int i = 0; i < posiciones.Length; i++)
        {
            if (intensidades[i] > 0.01f)
            {
                float u = (posiciones[i].x / dimensionMundo) + 0.5f;
                float v = (posiciones[i].z / dimensionMundo) + 0.5f;

                int xCentro = Mathf.RoundToInt(u * resolucionTextura);
                int yCentro = Mathf.RoundToInt(v * resolucionTextura);

                // En lugar de 1 píxel, pintamos un cuadrado sólido (brocha)
                for (int x = -grosorPincel; x <= grosorPincel; x++)
                {
                    for (int y = -grosorPincel; y <= grosorPincel; y++)
                    {
                        int xPixel = xCentro + x;
                        int yPixel = yCentro + y;

                        if (xPixel >= 0 && xPixel < resolucionTextura && yPixel >= 0 && yPixel < resolucionTextura)
                        {

                            float señalBrillante = Mathf.Clamp01(intensidades[i] * 2.5f);
                            Color colorEco = new Color(señalBrillante, señalBrillante * 0.6f, 0f, 1f);

                            texturaMosaico.SetPixel(xPixel, yPixel, colorEco);
                            modificado = true;
                        }
                    }
                }
            }
        }

        if (modificado)
        {
            texturaMosaico.Apply();
        }
    }

    private void OnApplicationQuit()
    {
        if (texturaMosaico != null)
        {
            byte[] bytes = texturaMosaico.EncodeToPNG();
            string timestamp = DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss");
            string carpeta = Application.dataPath + "/Datos_Mision";

            if (!Directory.Exists(carpeta))
            {
                Directory.CreateDirectory(carpeta);
            }

            string ruta = carpeta + "/Mapa_Batimetrico_" + timestamp + ".png";
            File.WriteAllBytes(ruta, bytes);
            Debug.Log("Mosaico guardado en formato PNG en la ruta: " + ruta);
        }
    }
}
