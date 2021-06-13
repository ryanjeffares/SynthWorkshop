using System;
using System.Runtime.InteropServices;
using UnityEngine;

static internal class SynthWorkshopLibrary
{
    private const string _libName = "SynthWorkshopLibrary";

    [DllImport(_libName, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr helloWorld();

    [DllImport(_libName, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr initialise();

    [DllImport(_libName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void shutdown(IntPtr mc);

    [DllImport(_libName, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr helloWorldFromMain(IntPtr mc);

    [DllImport(_libName, CallingConvention = CallingConvention.Cdecl)]
    private static extern bool createModulesFromJson(IntPtr mc, string jsonText);

    [DllImport(_libName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void setCvParam(IntPtr mc, int index, float value);

    private static readonly IntPtr _mainComponent;

    public static bool IsLoaded { get; }

    static SynthWorkshopLibrary()
    {
        _mainComponent = initialise();
        Application.quitting += Shutdown;
        IsLoaded = true;
    }

    public static void Shutdown()
    {
        if (!IsLoaded) return;
        shutdown(_mainComponent);
    }

    public static void HelloWorldFromMain()
    {
        if (!IsLoaded) return;
        var res = helloWorldFromMain(_mainComponent);
        var str = Marshal.PtrToStringAnsi(res);
        Debug.Log(str);
    }

    public static void HelloWorld()
    {
        if (!IsLoaded) return;
        var res = helloWorld();
        var str = Marshal.PtrToStringAnsi(res);
        Debug.Log(str);
    }

    public static bool CreateModulesFromJson(string jsonText)
    {
        return createModulesFromJson(_mainComponent, jsonText);
    }

    public static void SetCvParam(int index, float value)
    {
        setCvParam(_mainComponent, index, value);
    }
}

