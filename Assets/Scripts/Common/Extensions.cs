using System.Collections;
using UnityEngine;

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

    public static Color ColourFromHex(this string hex)
    {
        if (ColorUtility.TryParseHtmlString(hex, out Color col))
        {
            return col;
        }
        else
        {
            Debug.LogWarning("Unable to parse HTML string.");
            return Color.magenta;
        }
    }
}
