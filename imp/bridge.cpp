#include <iostream>
#include <fstream>
#include <map>
#include <string>

using namespace std;

class ISmartHouseDeviceAPI
{
public:
    virtual void sendCommand ( int   aChannel
                             , char* aBuffer)   = 0;
    
    virtual int  getStatus   (int id)           = 0;
    
    virtual int  openChannel (int id)           = 0;
    
    virtual void closeChannel(int aChannel)     = 0;
    
    virtual ~ISmartHouseDeviceAPI() {}  
};

class IRelay
{
public:
    virtual void On()  = 0;
    virtual void Off() = 0;
    
    virtual ~IRelay() {}
};

class Relay
: public IRelay
{
public:

    Relay(int aAddress, ISmartHouseDeviceAPI* aImplApi)
    : mAddress(aAddress)
    , mImplApi(aImplApi)
    {}
    
    ~Relay() {}

    void On()
    {
        int channel = mImplApi->openChannel(mAddress);
        mImplApi->sendCommand(channel, "AT 1");
        mImplApi->closeChannel(channel);     
    }
    
    void Off()
    {
        int channel = mImplApi->openChannel(mAddress);
        if (mImplApi->getStatus(channel))
        {
            mImplApi->sendCommand(channel, "AT 0");
        }
        mImplApi->closeChannel(channel);
    }
    
private:
    int                   mAddress;
    ISmartHouseDeviceAPI* mImplApi;
}

class LikeLinuxDevice
: public ISmartHouseDeviceAPI
{
public:
    LikeLinuxDevice();
    ~LikeLinuxDevice();
    
    void sendCommand(int aChannel, char* aBuffer)
    {
        if (mDevices.find(aChannel) != mDevices.end())
        {
            fstream* device = mDevices.at(aChannel);
            *device << aBuffer;
        }
    }
    
    int getStatus(int id)
    {
        int state = -1;
        
        if (mDevices.find(id) != mDevices.end())
        {
            fstream *device = mDevices.at(id);
            device->unget();
            
            
            *device >> state;
        }
        
        return -1;
    }
    
    int openChannel(int id)
    {
        if (mDevices.find(id) == mDevices.end())
        {
            fstream *device = new fstream("dev-" + to_string(id), fstream::in | fstream::out);
            mDevices.insert( make_pair(id, device));
        }
        return id;
    }
    
    void closeChannel(int aChannel)
    {
        if (mDevices.find(aChannel) != mDevices.end())
        {
            fstream *device = mDevices.at(aChannel);
            device->close();
            mDevices.erease(aChannel);
        }   
    }
    
private:
    map<int, fstream*> mDevices;
    
};

class DebugDevice
: public ISmartHouseDeviceAPI
{
public:
    DebugDevice();
    ~DebugDevice();
    
    void sendCommand(int aChannel, char* aBuffer)
    {
        cout << "Send command: " << aChannel << " " << aBuffer << endl;
    }
    
    int getStatus(int id)
    {
        cout << "Get status for device #" << id << endl;
        return 1;
    }
    
    int openChannel(int id)
    {
        cout << "Open channel for device #" << id << endl;
        return id;
    }
    
    void closeChannel(int aChannel)
    {
        cout << "Close channel " << aChannel << endl;
    }
    
}

int main ()
{
    ISmartHouseDeviceAPI* api = new DebugDevice;
    
    IRelay* relay = new Relay(2, api);
    
    relay->on();
    relay->off();
    
    delete relay;
    delete api;
    
    return 0;
}