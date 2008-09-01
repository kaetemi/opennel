#include <nel/misc/types_nl.h>
#include <nel/misc/app_context.h>

#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>

using namespace std;
using namespace NLMISC;
using namespace NL3D;

// The 3d driver
UDriver        *Driver = NULL;
// This variable is used to display text on the screen
UTextContext   *TextContext = NULL;
// true if you want to exit the main loop
bool            NeedExit = false;

int main(int argc, char **argv)
{
    // Create an application context.
    NLMISC::createDebug();

    // Create a driver
    Driver = UDriver::createDriver();
    nlassert(Driver);

    // Create the window with config file values
    Driver->setDisplay (UDriver::CMode(640, 480, 32));

    // Create a Text context for later text rendering
    TextContext = Driver->createTextContext ("data/n019003l.pfb");
    nlassert(TextContext);

    while ((!NeedExit) && Driver->isActive())
    {
        // Clear all buffers
        Driver->clearBuffers (CRGBA (0, 0, 0));

        // Display a text on the screen
        TextContext->setHotSpot (UTextContext::TopLeft);
        TextContext->setColor (CRGBA(255, 255, 255, 255));
        TextContext->setFontSize (14);
        TextContext->printfAt (0.01f, 0.99f, "Hello, World!");

        // Swap 3d buffers
        Driver->swapBuffers ();

        // Pump user input messages
        Driver->EventServer.pump();

        // Manage the keyboard
        if (Driver->AsyncListener.isKeyDown (KeySHIFT)
        && Driver->AsyncListener.isKeyDown (KeyESCAPE))
        {
            // Shift Escape -> quit
            NeedExit = true;
        }
    }
}
