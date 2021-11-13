using UnityEngine;

public class AudioMathsCreator : IModuleCreator
{
    public GameObject CreateModule(GameObject prefab, Transform parent, Vector3 position, string[] input)
    {
        GameObject module;

        switch (input.Length)
        {
            case 1:
                {
                    module = Object.Instantiate(prefab, position, Quaternion.identity, parent);

                    MathsSign sign;
                    switch (input[0])
                    {
                        case "sin~":
                            sign = MathsSign.Sin;
                            break;
                        case "cos~":
                            sign = MathsSign.Cos;
                            break;
                        case "tan~":
                            sign = MathsSign.Tan;
                            break;
                        case "asin~":
                            sign = MathsSign.Asin;
                            break;
                        case "acos~":
                            sign = MathsSign.Acos;
                            break;
                        case "atan~":
                            sign = MathsSign.Atan;
                            break;
                        case "abs~":
                            sign = MathsSign.Abs;
                            break;
                        case "+~":
                            sign = MathsSign.Plus;
                            break;
                        case "-~":
                            sign = MathsSign.Minus;
                            break;
                        case "*~":
                            sign = MathsSign.Multiply;
                            break;
                        case "/~":
                            sign = MathsSign.Divide;
                            break;
                        default: return null;
                    }

                    module.GetComponent<MathsModuleController>().SetType(sign, AudioCV.Audio);
                    return module;
                }
            case 2:
                {
                    if (!float.TryParse(input[1], out var value)) 
                    {
                        return null;
                    }

                    module = Object.Instantiate(prefab, position, Quaternion.identity, parent);
                    MathsSign sign;
                    switch (input[0])
                    {
                        case "+~":
                            sign = MathsSign.Plus;
                            break;
                        case "-~":
                            sign = MathsSign.Minus;
                            break;
                        case "*~":
                            sign = MathsSign.Multiply;
                            break;
                        case "/~":
                            sign = MathsSign.Divide;
                            break;
                        case "pow~":
                            sign = MathsSign.Pow;
                            break;
                        default: return null;
                    }

                    module.GetComponent<MathsModuleController>().SetType(sign, AudioCV.Audio, value);
                    return module;
                }
        }

        return null;
    }
}