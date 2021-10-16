
using System;

public class Ref<T>
{
    public bool HasException
    {
        get;
        private set;
    }
    
    public bool HasValue
    {
        get;
        private set;
    }
    
    private T _value;
    public T Value
    {
        get => _value;
        set
        {
            _value = value;
            HasValue = true;
            HasException = false;
        }
    }

    private Exception _exception;
    public Exception Exception
    {
        get => _exception;
        set
        {
            _exception = value;
            HasException = true;
            HasValue = false;
        }
    }
    
    public Ref()
    {
    }
}