#include "real_audio_device_interface.h"
#include <vector>
int main(int ,char**)
{
    long id = CreateDevice();
    DestroyDevice(id);
    return 0;
}