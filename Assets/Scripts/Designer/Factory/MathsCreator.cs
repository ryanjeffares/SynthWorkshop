using UnityEngine;

public class MathsCreator : IModuleCreator
{
    public GameObject CreateModule(GameObject prefab, Transform parent, Vector3 position, string[] input)
    {
        switch (input.Length)
        {
            case 1:
                {                    
                    MathsSign sign;
                    switch (input[0])
                    {
                        case "sin":
                            sign = MathsSign.Sin;
                            break;
                        case "cos":
                            sign = MathsSign.Cos;
                            break;
                        case "tan":
                            sign = MathsSign.Tan;
                            break;
                        case "asin":
                            sign = MathsSign.Asin;
                            break;
                        case "acos":
                            sign = MathsSign.Acos;
                            break;
                        case "atan":
                            sign = MathsSign.Atan;
                            break;
                        case "abs":
                            sign = MathsSign.Abs;
                            break;
                        case "+":
                            sign = MathsSign.Plus;
                            break;
                        case "-":
                            sign = MathsSign.Minus;
                            break;
                        case "*":
                            sign = MathsSign.Multiply;
                            break;
                        case "/":
                            sign = MathsSign.Divide;
                            break;
                        case "int":
                            sign = MathsSign.Int;
                            break;
                        case "ftom":
                            sign = MathsSign.Ftom;
                            break;
                        case "mtof":
                            sign = MathsSign.Mtof;
                            break;
                        default: return null;
                    }

                    var module = Object.Instantiate(prefab, position, Quaternion.identity, parent);
                    module.GetComponent<MathsModuleController>().SetType(sign, AudioCV.CV);
                    return module;
                }
            case 2:
                {
                    if (!float.TryParse(input[1], out var value))
                    {
                        return null;
                    }                    

                    MathsSign sign;
                    switch (input[0])
                    {
                        case "+":
                            sign = MathsSign.Plus;
                            break;
                        case "-":
                            sign = MathsSign.Minus;
                            break;
                        case "*":
                            sign = MathsSign.Multiply;
                            break;
                        case "/":
                            sign = MathsSign.Divide;
                            break;
                        case "%":
                            sign = MathsSign.Mod;
                            break;
                        case "pow":
                            sign = MathsSign.Pow;
                            break;
                        default: return null;
                    }

                    var module = Object.Instantiate(prefab, position, Quaternion.identity, parent);
                    module.GetComponent<MathsModuleController>().SetType(sign, AudioCV.CV, value);
                    return module;
                }
        }

        return null;
    }
}