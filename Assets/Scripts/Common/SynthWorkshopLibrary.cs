using System;
using System.Runtime.InteropServices;
using UnityEngine;

static class SynthWorkshopLibrary
{
#if UNITY_EDITOR_OSX || UNITY_STANDALONE_OSX
    private const string LibName = "SynthWorkshopBundle_M1";
#elif UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
    private const string LibName = "SynthWorkshopLibrary_x86";
#endif

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr helloWorld();

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr initialise();

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void shutdown(IntPtr mc);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr helloWorldFromMain(IntPtr mc);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void stopAudio(IntPtr mc);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern bool createModulesFromJson(IntPtr mc, string jsonText);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void setCvParam(IntPtr mc, int index, float value);
    
    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern float getCvParam(IntPtr mc, int index);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void setMasterVolume(IntPtr mc, float value);

    private static readonly IntPtr MainComponent;

    private static readonly bool IsLoaded;

    static SynthWorkshopLibrary()
    {
        MainComponent = initialise();
        Application.quitting += Shutdown;
        IsLoaded = true;        
    }

    private static void Shutdown()
    {
        if (!IsLoaded) return;
        shutdown(MainComponent);
    }

    public static void HelloWorldFromMain()
    {
        if (!IsLoaded) return;
        var res = helloWorldFromMain(MainComponent);
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

    public static void StopAudio()
    {
        stopAudio(MainComponent);
    }

    public static bool CreateModulesFromJson(string jsonText)
    {
        return createModulesFromJson(MainComponent, jsonText);
    }

    public static void SetCvParam(int index, float value)
    {
        setCvParam(MainComponent, index, value);
    }

    public static float GetCvParam(int index)
    {
        return getCvParam(MainComponent, index);
    }

    public static void SetMasterVolume(float value)
    {
        setMasterVolume(MainComponent, value);
    }
}

