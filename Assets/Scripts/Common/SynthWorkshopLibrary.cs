using System;
using System.Runtime.InteropServices;
using System.Threading;
using UnityEngine;

static class SynthWorkshopLibrary
{
#if UNITY_STANDALONE_OSX
    private const string LibName = "SynthWorkshopBundle_M1";
#elif UNITY_EDITOR_WIN || UNITY_STANDALONE_WIN
    private const string LibName = "SynthWorkshopLibrary_x64";
#elif UNITY_IOS
    private const string LibName = "__Internal";
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
    private static extern void resumeAudio(IntPtr mc);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern bool createSingleModule(IntPtr mc, int type, string jsonText);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void destroyModule(IntPtr mc, int moduleType, int globalIndex);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern bool setModuleInputIndex(IntPtr mc, bool audioOutput, bool add, int moduleId, int outputIndex, int targetIndex);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern bool setTriggerTarget(IntPtr mc, bool add, int senderId, int targetId);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void setCvParam(IntPtr mc, int index, float value);
    
    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern float getCvParam(IntPtr mc, int index);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern bool getTriggerableState(IntPtr mc, int moduleId);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void triggerCallback(IntPtr mc, int moduleId, bool state);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void createCvBufferWithKey(IntPtr mc, int key);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void createCvBufferWithKeyAndValue(IntPtr mc, int key, float value);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void setAudioMathsIncomingSignal(IntPtr mc, int globalId, int type);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void setNumberBoxValue(IntPtr mc, int globalId, float value);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern bool setSoundfileModuleSample(IntPtr mc, int globalId, string path);

    [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
    private static extern void setMasterVolume(IntPtr mc, float value);

    private static readonly IntPtr MainComponent;

    private static readonly bool IsLoaded;

    static SynthWorkshopLibrary()
    {
        Application.quitting += Shutdown;

        MainComponent = initialise();
        IsLoaded = true;        
    }

    public static void Shutdown()
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

    public static void ResumeAudio()
    {
        resumeAudio(MainComponent);
    }

    public static bool CreateNewModule(int type, string jsonText)
    {
        return createSingleModule(MainComponent, type, jsonText);
    }

    public static void ModuleDestroyed(int moduleType, int globalIndex)
    {
        destroyModule(MainComponent, moduleType, globalIndex);
    }

    public static bool SetModuleInputIndex(bool audioOutput, bool add, int moduleId, int outputIndex, int targetIndex)
    {
        return setModuleInputIndex(MainComponent, audioOutput, add, moduleId, outputIndex, targetIndex);
    }

    public static bool SetTriggerTarget(bool add, int senderId, int targetId)
    {
        return setTriggerTarget(MainComponent, add, senderId, targetId);
    }
    
    public static void SetCvParam(int index, float value)
    {
        setCvParam(MainComponent, index, value);
    }

    public static float GetCvParam(int index)
    {
        return getCvParam(MainComponent, index);
    }

    public static bool GetTriggerableState(int moduleId)
    {
        return getTriggerableState(MainComponent, moduleId);
    }

    public static void TriggerCallback(int moduleId, bool state)
    {
        triggerCallback(MainComponent, moduleId, state);
    }

    public static void CreateCvBufferWithKey(int key)
    {
        createCvBufferWithKey(MainComponent, key);
    }

    public static void CreateCvBufferWithKey(int key, float value)
    {
        createCvBufferWithKeyAndValue(MainComponent, key, value);
    }

    public static void SetAudioMathsIncomingSignal(int globalId, int type)
    {
        setAudioMathsIncomingSignal(MainComponent, globalId, type);
    }

    public static void SetNumberBoxValue(int globalId, float value)
    {
        setNumberBoxValue(MainComponent, globalId, value);
    }

    public static bool SetSoundfileSample(int globalId, string path)
    {
        return setSoundfileModuleSample(MainComponent, globalId, path);
    }
    
    public static void SetMasterVolume(float value)
    {
        setMasterVolume(MainComponent, value);
    }
}

