using System.Collections;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public static class Extensions
{
    public static IEnumerator InterpolateScale(this GameObject g, Vector3 targetScale, float time, AnimationCurve curve = null)
    {
        var startScale = g.transform.localScale;
        var timeCounter = 0f;
        while(timeCounter <= time)
        {
            g.transform.localScale = Vector3.Lerp(startScale, targetScale, 
                curve?.Evaluate(timeCounter / time) ?? (timeCounter / time));
            timeCounter += Time.deltaTime;
            yield return new WaitForEndOfFrame();
        }
        g.transform.localScale = targetScale;
    }

    public static IEnumerator InterpolateSize(this GameObject g, Vector3 targetSize, float time, AnimationCurve curve = null)
    {
        var rect = g.GetComponent<RectTransform>();
        var startSize = rect.sizeDelta;
        var timeCounter = 0f;
        while (timeCounter <= time)
        {
            rect.sizeDelta = Vector3.Lerp(startSize, targetSize,
                curve?.Evaluate(timeCounter / time) ?? (timeCounter / time));
            timeCounter += Time.deltaTime;
            yield return new WaitForEndOfFrame();
        }
        rect.sizeDelta = targetSize;
    }

    public static IEnumerator InterpolatePosition(this GameObject g, Vector3 targetPos, float time, AnimationCurve curve = null)
    {
        var startPos = g.transform.localPosition;
        var timeCounter = 0f;
        while (timeCounter <= time)
        {
            g.transform.localPosition = Vector3.Lerp(startPos, targetPos,
                curve?.Evaluate(timeCounter / time) ?? (timeCounter / time));
            timeCounter += Time.deltaTime;
            yield return new WaitForEndOfFrame();
        }
        g.transform.localPosition = targetPos;
    }

    public static IEnumerator LerpColour(this Image img, Color colour, float time, AnimationCurve curve = null)
    {
        var startColour = img.color;
        float timeCounter = 0f;
        while (timeCounter <= time)
        {
            img.color = Color.Lerp(startColour, colour, curve?.Evaluate(timeCounter / time) ?? (timeCounter / time));
            timeCounter += Time.deltaTime;
            yield return null;
        }
        img.color = colour;
    }

    public static IEnumerator LerpColour(this TextMeshProUGUI txt, Color colour, float time, AnimationCurve curve = null)
    {
        var startColour = txt.color;
        float timeCounter = 0f;
        while (timeCounter <= time)
        {
            txt.color = Color.Lerp(startColour, colour, curve?.Evaluate(timeCounter / time) ?? (timeCounter / time));
            timeCounter += Time.deltaTime;
            yield return null;
        }
        txt.color = colour;
    }

    public static Color ColourFromHex(this string hex)
    {
        if (ColorUtility.TryParseHtmlString(hex, out Color col))
        {
            return col;
        }
        Debug.LogWarning("Unable to parse HTML string.");
        return Color.magenta;
    }
    
    public static void SetTruncatedValueText(this Text text, float value, string unit = "")
    {
        var asStr = value.ToString();
        text.text = (asStr.Length > 5
            ? asStr.Substring(0, 5) + (asStr.Contains("E") ? "e-05" : string.Empty)
            : asStr) + unit;
    }
    
    public static float Map(this float x, float inMin, float inMax, float outMin, float outMax)
    {
        var inFraction = (x - inMin) / (inMax - inMin);
        return ((outMax - outMin) * inFraction) + outMin;
    }
    
    public static float Limit(this float x, float min, float max)
    {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }
}
